#include "SearchForWords.h"

SearchForWords* SearchForWords::ptr = NULL;
HWND hDialog, hEnterWords, hShowRes, hAmountWords, hChooseFile, hStarter, hStop, hExit, hContinue, hProgress1;
HANDLE hMutex;

HANDLE hThreads[3];
HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

//СЧЕТЧИК СЛОВ
int totalWordCount = 0;
BOOL progressCheck = false;

//ДЛЯ ФАЙЛА С РЕЗУЛЬТАТОМ
vector <string> forbiddenWords;
vector <string> fileName;
vector <string> filePath;
vector <int> fileSize;
vector <int> amountOfWords;

SearchForWords::SearchForWords(void)
{
    ptr = this;
    hDialog = NULL;
}

SearchForWords::~SearchForWords(void){}

void SearchForWords::Cls_OnClose(HWND hwnd)
{
    ReleaseMutex(hMutex);
    EndDialog(hwnd, 0);
}


//Инициализация 
BOOL SearchForWords::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    hDialog = hwnd;
    hProgress1 = GetDlgItem(hDialog, IDC_PROGRESS1);
    // установка интервала для индикатора 
    SendMessage(hProgress1, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    // установка шага приращения  индикатора 
    SendMessage(hProgress1, PBM_SETSTEP, 45, 0);
    // установка текущей позиции индикатора
    SendMessage(hProgress1, PBM_SETPOS, 0, 0);
    // установка цвета фона индикатора
    SendMessage(hProgress1, PBM_SETBKCOLOR, 0, LPARAM(RGB(0, 0, 255)));
    // установка цвета заполняемых прямоугольников
    SendMessage(hProgress1, PBM_SETBARCOLOR, 0, LPARAM(RGB(255, 255, 0)));

    hEnterWords = GetDlgItem(hwnd, IDC_EDIT1);
    hAmountWords = GetDlgItem(hwnd, IDC_EDIT4);
    hShowRes = GetDlgItem(hwnd, IDC_EDIT3);

    hChooseFile = GetDlgItem(hwnd, IDC_BUTTON1);
    hContinue = GetDlgItem(hwnd, IDC_BUTTON2);
    hStarter = GetDlgItem(hwnd, IDC_BUTTON3);
    hStop = GetDlgItem(hwnd, IDC_BUTTON4);
    hExit = GetDlgItem(hwnd, IDC_BUTTON5);
   // EnableWindow(hStarter, FALSE);

    return TRUE;
}




DWORD WINAPI ProgressThread(LPVOID lpParam)
{
    for (int i = 0; i <= 100; ++i) {
        SendMessage(hProgress1, PBM_SETPOS, i, 0);
        Sleep(100);

        if (progressCheck) {
            while (progressCheck) {
                Sleep(100);
            }
            SendMessage(hProgress1, PBM_SETPOS, i, 0);
        }
    }
    return 0;
}


//Функция для записи информации в файл
void SaveInfo() {
    SetEvent(hEvent);
    ofstream outputFile("result.txt");
    Sleep(100);
    if (outputFile.is_open()) {

        for (size_t i = 0; i < fileName.size(); ++i) {

            outputFile << "File`s Name: " << fileName[i] << endl;
            outputFile << "File`s Size(in bytes): " << fileSize[i] << " bytes" << endl;
            outputFile << "File`s Path: " << filePath[i] << endl;
            outputFile << "Amount of forbiddenWords: " << amountOfWords[i] << endl;
            outputFile << endl;
        }

        outputFile.close();
    }
}

// Функция для получения информации о файле
void GetFileInfo(const string& Path, string& name, int& size) {
    name = Path.substr(Path.find_last_of("\\") + 1);

    ifstream file(Path, ios::binary | ios::ate);
    size = static_cast<int>(file.tellg());
}

// Функция для получения информации
void GetInfo(const string& Path) {
    string name;
    int size;
    GetFileInfo(Path, name, size);

    // Добавляем информацию о файле
    fileName.push_back(name);
    filePath.push_back(Path);
    fileSize.push_back(size);

}

DWORD WINAPI CountWords(LPVOID lpParam) {
    string filePath = static_cast<const char*>(lpParam);
    SearchForWords* findWords = (SearchForWords*)lpParam;
    ifstream file(filePath);
    int counter = 0;
    if (file.is_open()) {
        string word;
        counter = 0;
        while (file >> word) {
            if (find(forbiddenWords.begin(), forbiddenWords.end(), word) != forbiddenWords.end()) {
                ++totalWordCount;
                ++counter;
            }
        }
        amountOfWords.push_back(counter);
        file.close();
        GetInfo(filePath);
    }

    return 0;
}

DWORD WINAPI MainThread(LPVOID lpParam);

DWORD WINAPI InfoThread(LPVOID lpParam) {
    SaveInfo();
    return 0;
}


void HideWords() {

    size_t currentIndex = 0;
    wstring filePath = L"Q:\\ProcProg\\111111\\SearchForWords2\\SearchForWords\\folder\\hidden.txt";

    HANDLE hFile = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {

        DWORD bytesWritten;
        for (const auto& word : forbiddenWords) {
            string replacement = "******* ";
            currentIndex++;
            if (!WriteFile(hFile, replacement.c_str(), static_cast<DWORD>(replacement.size()), &bytesWritten, NULL)) {
                MessageBoxA(NULL, "Failed to create file", "Result", MB_OK | MB_ICONINFORMATION);
                CloseHandle(hFile);
                return;
            }
            if (currentIndex == forbiddenWords.size()){}
        }
        CloseHandle(hFile);
    }
    else{
        MessageBoxA(NULL, "Failed to create file", "Result", MB_OK | MB_ICONINFORMATION);
    }

}
// Функция для записи слов в файл
void WriteWordsToFile() {
    ofstream outputFile("statistic.txt", ios::app);

    if (outputFile.is_open()) {
        outputFile << "your words: ";
        for (const auto& word : forbiddenWords) {
            outputFile << word << " ";
        }
        outputFile << endl;
        outputFile.close();
    }
}

// Основная функция обработки файлов
DWORD WINAPI MainThread(LPVOID lpParam) {
    SearchForWords* ptr = (SearchForWords*)lpParam;

    HANDLE hInfoThread = CreateThread(NULL, 0, InfoThread, NULL, 0, NULL);
    string directoryPath = "Q:\\ProcProg\\111111\\SearchForWords2\\SearchForWords\\folder\\";
    for (int i = 1; i <= 3; ++i) {
        string filePath = directoryPath + to_string(i) + ".txt";
        HANDLE hThread = CreateThread(NULL, 0, CountWords, (LPVOID)filePath.c_str(), 0, NULL);
        hThreads[i - 1] = hThread;
        WaitForSingleObject(hThread, INFINITE);
    }

    WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);

    HANDLE hProgressThread = CreateThread(NULL, 0, ProgressThread, hProgress1, 0, NULL);
    WaitForSingleObject(hProgressThread, INFINITE);

    WaitForSingleObject(hEvent, INFINITE);
    Sleep(500);

    for (int i = 0; i < 3; ++i) {
        CloseHandle(hThreads[i]);
    }

    TCHAR resNum[100];
    wsprintf(resNum, TEXT("%d"), totalWordCount);
    SetWindowText(hAmountWords, resNum);
    
    HideWords();
    WriteWordsToFile();

    char buf[100];
    ifstream in("result.txt", ios::in | ios::binary);

    if (!in){
        MessageBoxA(hDialog, "Ошибка открытия файла!", "Ошибка", MB_OK | MB_ICONINFORMATION);
        return 1;
    }
    else
    {
        while (!in.eof()){
            in.getline(buf, 100);
            SendMessageA(hShowRes, EM_REPLACESEL, 0, (LPARAM)buf);
            SendMessageA(hShowRes, EM_REPLACESEL, 0, (LPARAM)"\r\n");
        }
    }

    in.close();

    return 0;
}


void SearchForWords::Cls_OnTimer(HWND hwnd, UINT id)
{
    SendMessage(hProgress1, PBM_STEPIT, 0, 0);
}

BOOL CALLBACK SearchForWords::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
        HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);

    case WM_COMMAND:
        //КНОПКА СТАРТ
        if (LOWORD(wParam) == IDC_BUTTON3) {
            EnableWindow(hStarter, TRUE);
            CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        }
        //КНОПКА СТОП
        if (LOWORD(wParam) == IDC_BUTTON4) {
            progressCheck = true;
        }
        //КНОПКА ПРОДОЛЖИТЬ
        if (LOWORD(wParam) == IDC_BUTTON2) {
            progressCheck = false;
        }
        //КНОПКА ВЫХОД
        if (LOWORD(wParam) == IDC_BUTTON5){
            ReleaseMutex(hMutex);
            EndDialog(hwnd, 0);
        }
        //КНОПКА ЗАГРУЗИТЬ ФАЙЛ
        if (LOWORD(wParam) == IDC_BUTTON1){
            TCHAR FullPath[MAX_PATH] = { 0 };
            OPENFILENAME open = { sizeof(OPENFILENAME) };
            open.hwndOwner = hwnd;
            open.lpstrFilter = TEXT("Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0");
            open.lpstrFile = FullPath;
            open.nMaxFile = MAX_PATH;
            open.lpstrInitialDir = TEXT("C:\\");
            open.Flags = OFN_CREATEPROMPT | OFN_PATHMUSTEXIST;

            if (GetOpenFileName(&open))
            {
                ifstream file(open.lpstrFile);
                if (file.is_open())
                {
                    string word;
                    while (file >> word)
                    {
                        forbiddenWords.push_back(word);
                    }
                    file.close();

                    string txt = "your array:\n";
                    for (const auto& word : forbiddenWords){
                        txt += word + "\r\n";
                    }

                    EnableWindow(hEnterWords, FALSE);
                    EnableWindow(hStarter, TRUE);
                }
                else{
                    MessageBox(hwnd, TEXT("Error opening the file!"), TEXT("Error"), MB_OK | MB_ICONERROR);
                }
            }
        }

        break;


    }
    int textLength = GetWindowTextLength(hEnterWords);

    if (textLength == 0){}
    else{
        vector<TCHAR> buffer(textLength + 1);
        GetWindowText(hEnterWords, buffer.data(), textLength + 1);

        // Преобразуем TCHAR в string
        string text(buffer.begin(), buffer.end());

        forbiddenWords.clear();

        istringstream stream(text);
        string word;

        while (stream >> word)
        {
            forbiddenWords.push_back(word);
        }

        EnableWindow(hStarter, TRUE);
    }

    return FALSE;
}