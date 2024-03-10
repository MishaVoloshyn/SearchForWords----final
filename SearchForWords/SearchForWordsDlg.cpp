#include "SearchForWords.h"

SearchForWords* SearchForWords::ptr = NULL;
HWND hDialog, hEnterWords, hShowRes, hAmountWords, hChooseFile, hStarter, hStop, hExit, hContinue, hProgress1;
HANDLE hMutex;

HANDLE hThreads[3];
HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

//СЧЕТЧИК СЛОВ
int forbiddenWordsCounter = 0;
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

void SearchForWords::Cls_OnTimer(HWND hwnd, UINT id)
{
    SendMessage(hProgress1, PBM_STEPIT, 0, 0);
}

//Инициализация 
BOOL SearchForWords::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    hDialog = hwnd;
    hProgress1 = GetDlgItem(hDialog, IDC_PROGRESS1);
    // установка интервала для индикатора 
    SendMessage(hProgress1, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    // установка шага приращения  индикатора 
    SendMessage(hProgress1, PBM_SETSTEP, 20, 0);
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


void SearchForWords::WriteInfo()
{
    SetEvent(hEvent);
    ofstream outputFile("result.txt");

    if (outputFile.is_open()) {
        
        for (size_t i = 0; i < fileName.size(); ++i) {
            Sleep(100);
            outputFile << "File`s name: " << fileName[i] << endl;
            outputFile << "File`s size (in bytes): " << fileSize[i] << " bytes" << endl;
            outputFile << "Forbidden Words: " << amountOfWords[i] << endl;
            outputFile << "File`s path: " << filePath[i] << endl;
            outputFile << endl;
        }
        outputFile.close();
    }
}

void SearchForWords::GetFileInfo(const string& filePath, string& fileName, int& fileSize)
{
    fileName = filePath.substr(filePath.find_last_of("\\") + 1);

    ifstream file(filePath, ios::binary | ios::ate);
    fileSize = static_cast<int>(file.tellg());
}

void SearchForWords::GetInfo(const string& path)
{
    string name;
    int size;
    GetFileInfo(path, name, size);
    fileName.push_back(name);
    filePath.push_back(path);
    fileSize.push_back(size);
}

DWORD WINAPI CountForbWords(LPVOID lpParam)
{
    string filePath = static_cast<const char*>(lpParam);
    SearchForWords* searchForWords = (SearchForWords*)lpParam;
    ifstream file(filePath);
    int counter = 0;
    if (file.is_open()) {
        string word;
        counter = 0;
        while (file >> word) {
            if (find(forbiddenWords.begin(), forbiddenWords.end(), word) != forbiddenWords.end()) {
                ++forbiddenWordsCounter;
                ++counter;
            }
        }
        amountOfWords.push_back(counter);
        file.close();


        // Получаем статистику о файле
        searchForWords->GetInfo(filePath);
    }

    return 0;
}

DWORD WINAPI ResultThread(LPVOID lpParam)
{
    SearchForWords* ptr = static_cast<SearchForWords*>(lpParam);

    ptr->WriteInfo();
    return 0;
}

DWORD WINAPI ProgressThread(LPVOID lpParam)
{

    for (int i = 0; i <= 100; ++i) {
        SendMessage(hProgress1, PBM_SETPOS, i, 0);
        Sleep(200);
        if (progressCheck) {
            while (progressCheck) {
                Sleep(100);
            }
            SendMessage(hProgress1, PBM_SETPOS, i, 0);
        }
    }
    return 0;
}

void ShowAmount() {
    //Вывод кол-ва запрещенных слов в Edit
    TCHAR buff[10];
    wsprintf(buff, TEXT("%d"), forbiddenWordsCounter);
    SetWindowText(hAmountWords, buff);

    char buf[100];
    ifstream in("result.txt", ios::in | ios::binary);

    if (!in) {
        MessageBoxA(hDialog, "Ошибка открытия файла!", "Ошибка", MB_OK | MB_ICONINFORMATION);
    }
    else {
        while (!in.eof()) {
            in.getline(buf, 100);
            SendMessageA(hShowRes, EM_REPLACESEL, 0, (LPARAM)buf);
            SendMessageA(hShowRes, EM_REPLACESEL, 0, (LPARAM)"\r\n");
        }
    }

    in.close();
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
    SearchForWords* filePtr = reinterpret_cast<SearchForWords*>(lpParam);
    HANDLE hResultThread = CreateThread(NULL, 0, ResultThread, NULL, 0, NULL);

    //количество файлов - 3

    //Путь к папке с файлами
    string path = "C:\\Users\\Admin\\source\\repos\\SearchForWords\\SearchForWords\\folder\\";


    for (int i = 1; i <= 3; ++i) {
        string filePath = path + to_string(i) + ".txt";

        // create threads for files
        HANDLE hThread = CreateThread(NULL, 0, CountForbWords, (LPVOID)filePath.c_str(), 0, NULL);

        // save thread`s handle in array
        hThreads[i - 1] = hThread;
        // waiting for thread to complete
        WaitForSingleObject(hThread, INFINITE);
    }

    WaitForMultipleObjects(3, hThreads, TRUE, INFINITE);
    
    // Поток для прогресс бара
    HANDLE hProgress1Thread = CreateThread(NULL, 0, ProgressThread, hProgress1, 0, NULL);
    // Ожидание индикатора на 100
    WaitForSingleObject(hProgress1Thread, INFINITE);
    for (int i = 1; i <= 3; ++i) {
        CloseHandle(hThreads[i]);
    }

    WaitForSingleObject(hEvent, INFINITE);
    Sleep(300);

    //Вывод кол-ва запрещенных слов в Edit
    ShowAmount();

    ofstream outputFile("result.txt", ios::app);

    if (outputFile.is_open()) {
        outputFile << "your words: \n";
        for (const auto& word : forbiddenWords) {
            outputFile << word << " ";
        }
        outputFile << endl;
        outputFile.close();
    }

    //замена слов на *******
    ofstream out("C:\\Users\\Admin\\source\\repos\\SearchForWords\\SearchForWords\\hidden.txt");

    if (out.is_open()) {
        for (const auto& word : forbiddenWords) {
            out << "******* ";
        }

        out.close();
    }
    else {
        MessageBoxA(NULL, "Не удалось открыть файл", "Ошибка", MB_OK | MB_ICONINFORMATION);
    }

    return 0;
}

BOOL CALLBACK SearchForWords::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
        HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON3)
        {
            CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        }
        if (LOWORD(wParam) == IDC_BUTTON4)
        {
            progressCheck = true;

        }
        if (LOWORD(wParam) == IDC_BUTTON2)
        {
            progressCheck = false;

        }
        if (LOWORD(wParam) == IDC_BUTTON5)
        {
            ReleaseMutex(hMutex);
            EndDialog(hwnd, 0);
        }
        if (LOWORD(wParam) == IDC_BUTTON1)
        {
            TCHAR FullPath[MAX_PATH] = { 0 };
            OPENFILENAME open = { sizeof(OPENFILENAME) };
            open.hwndOwner = hwnd;
            open.lpstrFilter = TEXT("Text Files(*.txt)\0*.txt\0All Files(*.*)\0*.*\0");
            open.lpstrFile = FullPath;
            open.nMaxFile = MAX_PATH;
            open.lpstrInitialDir = TEXT("C:\\");
            open.Flags = OFN_CREATEPROMPT | OFN_PATHMUSTEXIST;

            if (GetOpenFileName(&open)){
                ifstream file(open.lpstrFile);
                if (file.is_open()){
                    string word;
                    while (file >> word)
                    {
                        forbiddenWords.push_back(word);
                    }
                    file.close();


                    EnableWindow(hEnterWords, FALSE);
                    EnableWindow(hChooseFile, FALSE);
                    
                    MessageBox(hwnd, TEXT("Файл получен!"), TEXT("Работа с файлом"), MB_OK | MB_ICONINFORMATION);

                }
                else{
                    MessageBox(hwnd, TEXT("Не удалось открыть файл!"), TEXT("Ошибка"), MB_OK | MB_ICONERROR);
                }
            }
        }
        break;
    }

    int textLength = GetWindowTextLength(hEnterWords) +1;

    if (textLength != 0){
        EnableWindow(hStarter, TRUE);
        vector<TCHAR> temp(textLength);
        GetWindowText(hEnterWords, temp.data(), textLength);
        string str(temp.begin(), temp.end());

        istringstream stream(str);
        string word;

        while (stream >> word){
            forbiddenWords.push_back(word);
        }
    }
    else{
        EnableWindow(hStarter, FALSE);
    }

    return FALSE;
}