#ifndef DISK_CAPACITY_H
#define DISK_CAPACITY_H

#include <stdio.h>
#include <windows.h>

// 드라이브의 용량 정보를 출력하는 함수
void showDriveCapacity(char drive) {
    ULARGE_INTEGER totalBytes;
    ULARGE_INTEGER totalGigabytes;

    // 드라이브의 루트 디렉터리에 대한 경로 생성
    char rootPath[MAX_PATH];
    sprintf(rootPath, "%c:\\", drive);

    // GetDiskFreeSpaceEx 함수를 사용하여 디스크 공간 정보 가져오기
    if (GetDiskFreeSpaceEx(rootPath, NULL, &totalBytes, NULL)) {
        // 바이트 단위를 GB 단위로 변환
        totalGigabytes.QuadPart = totalBytes.QuadPart / (1024 * 1024 * 1024);

        // 정보 출력
        printf("Drive %c:\n", drive);
        printf("Total disk space: %llu GB\n", totalGigabytes.QuadPart);
    } else {
        printf("Error getting disk space information for drive %c\n", drive);
    }
}

#endif /* DISK_CAPACITY_H */
