#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <windows.h>
#include "fileexplorer.h"
#include "capacity.h"


#define NUM_THREADS 2

DWORD WINAPI Searchthread(LPVOID lpParam) 
{
    char* path = (char*)lpParam;
    FileSearch(path);
    return 0;
}

int main()
{
    char path[100] = {'\0'};
    scanf("%s", path);

    HANDLE threads[NUM_THREADS];    //thread를 위한 핸들
    DWORD threadIds[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads[i] = CreateThread(NULL, 0, Searchthread, path, 0, &threadIds[i]);
        if (threads[i] == NULL) 
        {
            fprintf(stderr, "Error creating thread %d\n", i);
            return 1;
        }
    }
    

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    for (int i = 0; i < NUM_THREADS; ++i) {
        CloseHandle(threads[i]);
    }

    return 0;
}

