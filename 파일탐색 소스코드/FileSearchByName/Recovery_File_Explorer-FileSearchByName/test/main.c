#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include "fileexplorer.h"
#include "capacity.h"

#define NUM_THREADS 2 // 생성할 스레드 수를 2로 정의

// 스레드가 실행할 함수
DWORD WINAPI Searchthread(LPVOID lpParam) 
{
    char* path = (char*)lpParam; // lpParam을 문자열 경로로 캐스팅
    search_execute(path); // search_execute 함수를 호출하여 경로에서 파일을 검색
    return 0; // 스레드가 성공적으로 종료됨을 반환
}

int main() {
    int total_capacity = showTotalCapacity() + 10; // 총 용량을 가져와서 10을 더함
    
    if (total_capacity >= 10) // 총 용량이 10 이상이면
    {       
        HANDLE threads[NUM_THREADS]; // 스레드 핸들을 저장할 배열
        DWORD threadIds[NUM_THREADS]; // 스레드 ID를 저장할 배열
        char drive[100][4] = {'\0'}; // 드라이브 경로를 저장할 배열 초기화
        int drive_count = 0; // 드라이브 개수를 저장할 변수 초기화
        ListDrives(drive, &drive_count); // 사용 가능한 드라이브 목록을 가져옴

        char path[100][4] = {'\0'}; // 검색할 경로를 저장할 배열 초기화
        char target_file_name[100]; // 찾고자 하는 파일명을 저장할 배열
        printf("Enter the filename to search for: "); // 사용자에게 파일명을 입력받도록 요청
        scanf("%s", target_file_name); // 파일명을 입력받아 target_file_name에 저장

        // 각 드라이브에서 파일을 검색
        for (int i = 0; i < drive_count; i++)
        {
            strcpy(path[i], drive[i]); // 드라이브 경로를 검색 경로에 복사

            SearchFileByName(path[i], target_file_name); // 해당 경로에서 파일을 검색
            int j = i + 1;
            path[j][0] = NULL; // 다음 경로를 NULL로 초기화
        }
        
        // 스레드를 생성하여 Searchthread 함수를 실행
        for (int i = 0; i < NUM_THREADS; ++i) 
        {
            threads[i] = CreateThread(NULL, 0, Searchthread, path, 0, &threadIds[i]); // 스레드 생성
            if (threads[i] == NULL) // 스레드 생성에 실패하면
            {
                fprintf(stderr, "Error creating thread %d\n", i); // 오류 메시지 출력
                return 1; // 프로그램 종료
            }
        }
        
        WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE); // 모든 스레드가 종료될 때까지 대기
    }

    ending(); // 프로그램 종료 전 마무리 작업
    return 0; // 프로그램 성공적으로 종료
}


int showTotalCapacity() {
    ULARGE_INTEGER totalSpace = {0};

    DWORD driveMask = GetLogicalDrives();
    if (driveMask == 0) {
        printf("GetLogicalDrives() failed with error code: %lu\n", GetLastError());
        return 0;
    }

    for (char driveLetter = 'A'; driveLetter <= 'Z'; ++driveLetter) {
        if (driveMask & (1 << (driveLetter - 'A'))) {
            ULARGE_INTEGER freeBytesAvailableToCaller;
            ULARGE_INTEGER totalNumberOfBytes;
            ULARGE_INTEGER totalNumberOfFreeBytes;

            char rootPath[4];
            sprintf(rootPath, "%c:\\", driveLetter);

            if (GetDiskFreeSpaceEx(rootPath, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
                totalSpace.QuadPart += totalNumberOfBytes.QuadPart;
            } else {
                printf("Error getting disk space information for drive %c:\\\n", driveLetter);
            }
        }
    }

    // Calculate total capacity in terabytes
    int totalTB = totalSpace.QuadPart / (1024.0 * 1024 * 1024 * 1024);

    return totalTB;
}


void search_execute()
{   
    char drive[100][4] = {'\0'};  // 드라이브 경로를 저장할 배열
    int drive_count = 0;
    ListDrives(drive, &drive_count);    //drive 배열에 현재 컴퓨터 드라이브 이름 저장

    char path[100][4] = {'\0'};    // 검색할 경로 NULL로 초기화
    char target_file_name[100]; // 찾고자 하는 파일명
    printf("Enter the filename to search for: ");
    scanf("%s", target_file_name); // 찾고자 하는 파일명 입력받기

    for (int i = 0; i < drive_count; i++)   //드라이브 갯수대로 for문
    {
        strcpy(path[i], drive[i]);      // 현재 컴퓨터 드라이브를 path 배열에 저장

        SearchFileByName(path[i], target_file_name);    // 찾고자 하는 파일명을 path 배열에 저장된 드라이브에서 파일 검색
        int j = i+1;    
        path[j][0] = NULL;  // 현재 path의 다음 배열
    }
    ending();   // 엔딩

    return 0;
}




// int showTotalCapacity() {
//     ULARGE_INTEGER totalSpace = {0};
//     ULARGE_INTEGER totalFreeSpace = {0};

//     DWORD driveMask = GetLogicalDrives();
//     if (driveMask == 0) {
//         printf("GetLogicalDrives() failed with error code: %lu\n", GetLastError());
//         return 0;
//     }

//     for (char driveLetter = 'A'; driveLetter <= 'Z'; ++driveLetter) {
//         if (driveMask & (1 << (driveLetter - 'A'))) {
//             ULARGE_INTEGER freeBytesAvailableToCaller;
//             ULARGE_INTEGER totalNumberOfBytes;
//             ULARGE_INTEGER totalNumberOfFreeBytes;

//             char rootPath[4];
//             sprintf(rootPath, "%c:\\", driveLetter);

//             if (GetDiskFreeSpaceEx(rootPath, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
//                 totalSpace.QuadPart += totalNumberOfBytes.QuadPart;
//             } else {
//                 printf("Error getting disk space information for drive %c:\\\n", driveLetter);
//             }
//         }
//     }

//     int totalGB = (int)(totalSpace.QuadPart / (1024 * 1024 * 1024 * 1024));

//     return totalGB;
// }



int isFileOrDir()
{
    if (fd.attrib & _A_SUBDIR)
        return 0; // 디렉터리면 0 반환
    else
        return 1; // 그밖의 경우는 "존재하는 파일"이기에 1 반환
}

void SearchFileByName(char directory[], const char *target_file_name)
{
    intptr_t handle;
    int check = 0;
    char directory_backup[_MAX_PATH];
    
    strcat(directory, "\\");        
    strcpy(directory_backup, directory);
    strcat(directory, "*");         

    if ((handle = _findfirst(directory, &fd)) == -1)
    {
        return;
    }
    
    while (_findnext(handle, &fd) == 0)
    {
        char current_path[_MAX_PATH];
        strcpy(current_path, directory_backup);
        strcat(current_path, fd.name);

        check = isFileOrDir();

        if (check == 0 && fd.name[0] != '.')
        {
            SearchFileByName(current_path, target_file_name);  // 하위 디렉토리 검색 재귀함수
        }
        else if (check == 1 && strstr(fd.name, target_file_name) != NULL)
        {
            printf("Found file: %s\n", current_path);
        }
    }
    _findclose(handle);
}


void ending() {
    char input;
    
    printf("Press 'q' to quit.\n");

    do {
        input = getchar();  // 사용자 입력 받기
    } while (input != 'q');

    printf("Program is ending...\n");
}



void ListDrives(char (*drive)[4], int *drive_count)
{
    DWORD drive_mask = GetLogicalDrives();
    if (drive_mask == 0)
    {
        printf("GetLogicalDrives() failed with error code: %lu\n", GetLastError());
        return;
    }

    int count = 0;
    for (char drive_letter = 'A'; drive_letter <= 'Z'; ++drive_letter)
    {
        if (drive_mask & (1 << (drive_letter - 'A')))
        {
            snprintf(drive[count], 4, "%c:\\", drive_letter);
            count++;
        }
    }
    *drive_count = count;
}