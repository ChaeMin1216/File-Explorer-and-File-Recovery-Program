#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include "capacity.h"
#include "fileexplorer.h"
#define MAX_DRIVES 100

typedef struct {
    char drive[100][4];
    char target_file_name[100];
} SearchParams;

DWORD WINAPI SearchFile(LPVOID lpParam) {
    SearchParams *params = (SearchParams *)lpParam;
    SearchFileByName(params->drive, params->target_file_name);
    return 0;
}

int main() {
    big_capacity_search_execute();
    return 0;
}
/*
    10TB 이상의 용량을 가진 컴퓨터에서 실행시킬 경우
    파티션의 개수대로 멀티스레드를 생성
    각각의 스레드에서 SearchFileByName함수(검색함수) 실행
    각 드라이브에서 원하는 파일명을 가진 파일 검색
*/

void drive_list(char *drive, int *drive_count)
{
    char drive[MAX_DRIVES][4] = {'\0'};  // 드라이브 경로를 저장할 배열
    int drive_count = 0;
    ListDrives(drive, &drive_count);    // 현재 컴퓨터 드라이브 이름 저장
    
}



void big_capacity_search_execute() {
    // char drive[MAX_DRIVES][4] = {'\0'};  // 드라이브 경로를 저장할 배열
    // int drive_count = 0;
    // ListDrives(drive, &drive_count);    // 현재 컴퓨터 드라이브 이름 저장
    // printf("Found drives:\n");
    // for (int i = 0; i < drive_count; ++i) {
    //     printf("%s\n", drive[i]);
    // }
    
    char drive[MAX_DRIVES][4] = drive_list(drive);

    char target_file_name[100]; // 찾고자 하는 파일명
    printf("Enter the filename to search for: ");
    scanf("%s", target_file_name); // 찾고자 하는 파일명 입력받기

    HANDLE threads[MAX_DRIVES];
    SearchParams params[MAX_DRIVES];

    for (int i = 0; i < drive_count; i++) {
        strcpy(params[i].drive, drive[i]);
        strcpy(params[i].target_file_name, target_file_name);
        
        threads[i] = CreateThread(
            NULL,                       // 기본 보안 속성
            0,                          // 기본 스택 크기
            SearchFile,                 // 스레드 시작 함수
            &params[i],                 // 매개변수
            0,                          // 기본 생성 플래그
            NULL                        // 스레드 ID (사용하지 않음)
        );

        if (threads[i] == NULL) {
            printf("스레드 생성 실패\n");
            return;
        }
    }

    // 모든 스레드가 종료될 때까지 대기
    WaitForMultipleObjects(drive_count, threads, TRUE, INFINITE);

    // 스레드 핸들 닫기
    for (int i = 0; i < drive_count; i++) {
        CloseHandle(threads[i]);
    }

    ending();   // 엔딩
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
    if (fd.attrib & _A_SUBDIR)  //파일 속성을 확인해서 디렉터리인지 확인
        return 0; // 디렉터리면 0 반환
    else
        return 1; // 그밖의 경우는 "존재하는 파일"이기에 1 반환
}
// 주어진 디렉터리에서 지정된 파일명을 검색하는 함수
void SearchFileByName(char directory[], const char *target_file_name)
{
    intptr_t handle;            // 파일 탐색을 위한 핸들
    int check = 0;              // 디렉터리인지 파일인지 확인
    char directory_backup[_MAX_PATH]; //재귀하기 위한 백업 경로
    
    strcat(directory, "\\");                // "\" 추가
    strcpy(directory_backup, directory);    // 백업
    strcat(directory, "*");                 // 파일명 지정하여 파일을 검색할 수 있도록 설정

    // 현재 주소 내 디렉터리의 첫 번째 파일 탐색
    if ((handle = _findfirst(directory, &fd)) == -1)    // 디렉터리가 비어있거나 에러 발생 시 함수 종료
    {
        return;
    }
    
    // 디렉터리 내의 모든 파일 및 디렉터리에 대한 탐색
    while (_findnext(handle, &fd) == 0)     //findnext 함수가 존재하면
    {
        char current_path[_MAX_PATH];
        strcpy(current_path, directory_backup);     //현재 경로를 백업경로로 복사
        strcat(current_path, fd.name);              //현재 파일 경로 추가

        check = isFileOrDir();                      //파일인지 디렉터리인지 확인하는 변수

        if (check == 0 && fd.name[0] != '.')           //디렉터리면서 숨김파일이 아닐때
        {
            SearchFileByName(current_path, target_file_name);  // 하위 디렉토리 재귀로 검색
        }
        else if (check == 1 && strstr(fd.name, target_file_name) != NULL)   //파일일때
        {
            printf("Found file: %s\n", current_path);                   // 파일을 출력
        }
    }
    _findclose(handle);         // 파일 검색 완료 후 핸들을 닫음
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

