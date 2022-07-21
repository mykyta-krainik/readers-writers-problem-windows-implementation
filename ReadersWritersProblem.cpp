#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <conio.h>

int numReadersRead = 0;
int numWritersInQueue = 0;
PCRITICAL_SECTION door;
PCRITICAL_SECTION dataBase;
PCRITICAL_SECTION print;

DWORD WINAPI reader(LPVOID id) {
    int readerId = *static_cast<int*>(id);

    srand((time(NULL) + readerId * 1000) * 100000);

    while (true) {
        Sleep(rand() % (3000 * (readerId + 1)) + 1000);

        EnterCriticalSection(door);

        if (numWritersInQueue) {
            LeaveCriticalSection(door);

            continue;
        }

        numReadersRead++;

        if (numReadersRead == 1) {
            EnterCriticalSection(dataBase);
        }

        LeaveCriticalSection(door);

        EnterCriticalSection(print);
        std::cout << "Reader " << readerId
            << " is going to read (total number of readers: "
            << numReadersRead << " )" << std::endl;
        LeaveCriticalSection(print);

        Sleep(rand() % (1000 * (readerId + 1)) + 1000);

        EnterCriticalSection(door);
        numReadersRead--;
        LeaveCriticalSection(door);

        EnterCriticalSection(print);
        std::cout << "Reader " << readerId
            << " has just stopped reading (total number of readers: "
            << numReadersRead << " )" << std::endl;
        LeaveCriticalSection(print);

        if (!numReadersRead) {
            LeaveCriticalSection(dataBase);
        }
    }

    return 0;
}

DWORD WINAPI writer(LPVOID id) {
    int writerId = *static_cast<int*>(id);

    srand((time(NULL) + writerId * 1000) * 100000);

    while (true) {
        Sleep(rand() % (10000 * (writerId + 1)) + 3000);

        EnterCriticalSection(door);
        numWritersInQueue = 1;
        LeaveCriticalSection(door);

        EnterCriticalSection(dataBase);

        numWritersInQueue = 0;

        Sleep(rand() % (500 * (writerId + 1)) + 500);

        std::cout << "Writer " << writerId
            << " is going to write" << std::endl;

        std::cout << "Writer " << writerId << " is writing" << std::endl;

        Sleep(rand() % (1000 * (writerId + 1)) + 1000);

        std::cout << "Writer " << writerId << " has just stopped writing" << std::endl;

        LeaveCriticalSection(dataBase);
    }

    return 0;
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    int numOfWriters = 0;
    int numOfReaders = 0;
    std::vector<HANDLE> readersThreads;
    std::vector<HANDLE> writersThreads;
    std::vector <DWORD> readersParam;
    std::vector <DWORD> writersParam;

    door = new CRITICAL_SECTION();
    dataBase = new CRITICAL_SECTION();
    print = new CRITICAL_SECTION();

    InitializeCriticalSection(door);
    InitializeCriticalSection(dataBase);
    InitializeCriticalSection(print);

    std::cout << "Enter a number of writers: ";
    std::cin >> numOfWriters;

    writersThreads.resize(numOfWriters);
    writersParam.resize(numOfWriters);

    std::cout << "Enter a number of readers: ";
    std::cin >> numOfReaders;
    std::cout << "\n\n";

    readersThreads.resize(numOfReaders);
    readersParam.resize(numOfReaders);

    for (int i = 0; i < numOfReaders; i++) {
        readersParam[i] = i;
        readersThreads[i] = CreateThread(NULL, 0, reader, &readersParam[i], 0, NULL);

        if (!readersThreads[i]) {
            std::cout << "Error" << std::endl;

            return -1;
        }
    }

    for (int i = 0; i < numOfWriters; i++) {
        writersParam[i] = i;
        writersThreads[i] = CreateThread(NULL, 0, writer, &writersParam[i], 0, NULL);

        if (!writersThreads[i]) {
            std::cout << "Error" << std::endl;
            
            return -1;
        }
    }

    _getch();

    for (HANDLE thread : readersThreads) {
        CloseHandle(thread);
    }

    for (HANDLE thread : writersThreads) {
        CloseHandle(thread);
    }

    DeleteCriticalSection(door);
    DeleteCriticalSection(dataBase);
    DeleteCriticalSection(print);

    return 0;
}
