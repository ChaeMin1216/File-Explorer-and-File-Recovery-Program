#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ListDrives(char (*drive)[4], int *drive_count) {} //현재 저장장치 파티션을 문자열에 저장
void SearchFileByName(char directory[], const char *target_file_name) {} //문자열이 포함된 파일명을 가진 파일을 찾는 함수
void ending() {} // 프로그램 종료 방지
void showDriveCapacity(char drive) {} // 드라이브 용량 표기


int main()
{   
    char drive[100][4] = {'\0'};  // 드라이브 경로를 저장할 배열
    int drive_count = 0;
    ListDrives(drive, &drive_count);

    char path[100][4] = {'\0'};    // 검색할 경로 NULL로 초기화
    char target_file_name[100]; // 찾고자 하는 파일명
    printf("Enter the filename to search for: ");
    scanf("%s", target_file_name); // 찾고자 하는 파일명 입력받기

    for (int i = 0; i < drive_count; i++)
    {
        strcpy(path[i], drive[i]);

        SearchFileByName(path[i], target_file_name);
        int j = i+1;
        path[j][0] = NULL;
    }
    
    ending();

    return 0;
}
