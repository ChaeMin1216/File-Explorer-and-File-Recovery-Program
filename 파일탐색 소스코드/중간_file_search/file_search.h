#ifndef __file_search_h__
#define __file_search_h__
#pragma warning(disable : 4996) // _findfirst, _findnext 함수가 안전하지 않으므로 경고문 무시
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <Windows.h>

struct _finddata_t fd; // 파일 검색에 사용됨. 파일 및 디렉터리 정보를 저장함

#define MAX_DRIVES 100
#define MAX_PATH_LENGTH 1024

typedef struct
{
    char drive[100][4];
    char target_file_name[100];
} SearchParams;

// C2371 오류를 해결하기 위한 함수 선정의
void SearchFileByName(char directory[], const char *target_file_name);
void big_capacity_search_execute();
int showTotalCapacity();
int isFileOrDir();
void ending();
void ListDrives(char (*drive)[4], int *drive_count);
void file_search();

static DWORD WINAPI SearchFile(LPVOID lpParam)
{
    SearchParams *params = (SearchParams *)lpParam;
    SearchFileByName(params->drive, params->target_file_name);
    return 0;
}

void file_search_start()
{
    int cap_size = showTotalCapacity();

    if (cap_size > 9)
    {
        big_capacity_search_execute();
        // file_search();
    }
    else
    {
        file_search();
        // big_capacity_search_execute();
    }
}

void file_search()
{
    char drive[MAX_DRIVES][4] = {'\0'}; // 드라이브 경로를 저장할 배열
    int drive_count = 0;
    ListDrives(drive, &drive_count); // 현재 컴퓨터 드라이브 이름 저장
    char path[100][4] = {'\0'};    // 검색할 경로 NULL로 초기화
    char target_file_name[100] = {'\0'}; // 찾고자 하는 파일명
    printf("Enter the filename to search for: ");
    scanf_s("%99s", target_file_name, sizeof(target_file_name));
    for (int i = 0; i < drive_count; i++)
    {
        strcpy(path[i], drive[i]);      // 현재 컴퓨터 드라이브를 path 배열에 저장
        SearchFileByName(path[i], target_file_name);    // 찾고자 하는 파일명을 path 배열에 저장된 드라이브에서 파일 검색
        int j = i+1;    
        path[j][0] = NULL;  // 현재 path의 다음 배열
    }
    ending();
}
/*
    10TB 이상의 용량을 가진 컴퓨터에서 실행시킬 경우
    파티션의 개수대로 멀티스레드를 생성
    각각의 스레드에서 SearchFileByName함수(검색함수) 실행
    각 드라이브에서 원하는 파일명을 가진 파일 검색
*/
void big_capacity_search_execute()
{
    char drive[MAX_DRIVES][4] = {'\0'}; // 드라이브 경로를 저장할 배열
    int drive_count = 0;
    ListDrives(drive, &drive_count); // 현재 컴퓨터 드라이브 이름 저장
    char target_file_name[100] = {'\0'}; // 찾고자 하는 파일명
    printf("Enter the filename to search for: ");
    scanf_s("%99s", target_file_name, sizeof(target_file_name));

    // 동적 메모리 할당
    HANDLE *threads = (HANDLE *)malloc(sizeof(HANDLE) * MAX_DRIVES);
    if (threads == NULL)
    {
        printf("스레드 핸들 메모리 할당 실패\n");
        return;
    }

    SearchParams *params = (SearchParams *)malloc(sizeof(SearchParams) * MAX_DRIVES);
    if (params == NULL)
    {
        printf("파라미터 메모리 할당 실패\n");
        free(threads);
        return;
    }

    for (int i = 0; i < drive_count; i++)
    {
        snprintf(params[i].drive, sizeof(params[i].drive), "%s", drive[i]);
        snprintf(params[i].target_file_name, sizeof(params[i].target_file_name), "%s", target_file_name);

        threads[i] = CreateThread(
            NULL,       // 기본 보안 속성
            0,          // 기본 스택 크기
            SearchFile, // 스레드 시작 함수
            &params[i], // 매개변수
            0,          // 기본 생성 플래그
            NULL        // 스레드 ID (사용하지 않음)
        );

        if (threads[i] == NULL)
        {
            printf("스레드 생성 실패\n");
            return;
        }
        else
        {
            printf("thread[%d]: started for drive %s\n", i, drive[i]);
        }
    }

    // 모든 스레드가 종료될 때까지 대기
    WaitForMultipleObjects(drive_count, threads, TRUE, INFINITE);

    // 스레드 핸들 닫기
    for (int i = 0; i < drive_count; i++)
    {
        CloseHandle(threads[i]);
    }
    // drive_count는 정해져있고
    // thread은 closehandle을 불러온 뒤 더이상 사용하지 않음

    free(params); // 할당한 메모리 해제
    free(threads);
    ending(); // 엔딩
}

int showTotalCapacity()
{
    ULARGE_INTEGER totalSpace = {0};
    DWORD driveMask = GetLogicalDrives();
    if (driveMask == 0)
    {
        printf("GetLogicalDrives() failed with error code: %lu\n", GetLastError());
        return 0;
    }

    for (char driveLetter = 'A'; driveLetter <= 'Z'; ++driveLetter)
    {
        if (driveMask & (1 << (driveLetter - 'A')))
        {
            ULARGE_INTEGER freeBytesAvailableToCaller;
            ULARGE_INTEGER totalNumberOfBytes;
            ULARGE_INTEGER totalNumberOfFreeBytes;

            char rootPath[4];
            sprintf(rootPath, "%c:\\", driveLetter);

            if (GetDiskFreeSpaceEx(rootPath, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes))
            {
                totalSpace.QuadPart += totalNumberOfBytes.QuadPart;
            }
            else
            {
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

int isFileOrDir(struct _finddata_t *fd)
{
    if (fd->attrib & _A_SUBDIR)
        return 0; // 디렉터리면 0 반환
    else
        return 1; // 그밖의 경우는 "존재하는 파일"이기에 1 반환
}

// 주어진 디렉터리에서 지정된 파일명을 검색하는 함수
void SearchFileByName(char directory[], const char *target_file_name)
{
    struct _finddata_t fd;                  // 파일 정보를 저장할 구조체
    intptr_t handle;                        // 파일 탐색을 위한 핸들
    int check = 0;                          // 디렉터리인지 파일인지 확인
    char directory_backup[MAX_PATH_LENGTH]; // 재귀하기 위한 백업 경로
    char current_path[MAX_PATH_LENGTH];     // 현재 경로를 저장할 변수 (충분히 큰 크기로 설정)

    // 디렉토리에 "\" 추가 (최대 길이를 고려하여 안전하게 처리)
    strncat(directory, "\\", MAX_PATH_LENGTH - strlen(directory) - 1);
    // 디렉터리 백업 (최대 길이를 고려하여 안전하게 처리)
    strncpy(directory_backup, directory, MAX_PATH_LENGTH - 1);
    directory_backup[MAX_PATH_LENGTH - 1] = '\0'; // 안전한 널 종료
    // 디렉터리에 "*" 추가 (최대 길이를 고려하여 안전하게 처리)
    strncat(directory, "*", MAX_PATH_LENGTH - strlen(directory) - 1);

    // 현재 주소 내 디렉터리의 첫 번째 파일 탐색
    if ((handle = _findfirst(directory, &fd)) == -1) // 디렉터리가 비어있거나 에러 발생 시 함수 종료
    {
        return;
    }

    // 디렉터리 내의 모든 파일 및 디렉터리에 대한 탐색
    while (_findnext(handle, &fd) == 0) // findnext 함수가 존재하면
    {
        snprintf(current_path, sizeof(current_path), "%s%s", directory_backup, fd.name); // 현재 파일 경로 추가

        check = isFileOrDir(&fd); // 파일인지 디렉터리인지 확인하는 변수

        if (check == 0 && fd.name[0] != '.') // 디렉터리면서 숨김파일이 아닐때
        {
            SearchFileByName(current_path, target_file_name); // 하위 디렉토리 재귀로 검색
        }
        else if (check == 1 && strstr(fd.name, target_file_name) != NULL) // 파일일때
        {
            // printf("test : %c\n", directory_backup[0]);
            printf("Found file: %s\n", current_path); // 파일을 출력
        }
    }
    _findclose(handle); // 파일 검색 완료 후 핸들을 닫음
}

void ending()
{
    char input;

    printf("Press 'q' to quit.\n");

    do
    {
        input = getchar(); // 사용자 입력 받기
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

#endif