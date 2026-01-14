#include <windows.h>
#include <stdio.h>
#include "ntfs.h"
#include <math.h>

void ProcessFileMetadata(U8* buffer, U16 dwBytesRead);
void Print_BPB(NTFS_BPB* bpb);

int main() {
    HANDLE hVolume;
    U16 dwBytesRead;
    U8 buffer[512]; // 부트 섹터를 읽어오기 위한 버퍼
    NTFS_BPB* bpb;

    // 파일 시스템을 열기 위한 경로 지정 (예: \\.\C:)
    LPCWSTR volumePath = L"\\\\.\\C:";  // wchar_t*

    // 파일 시스템 열기
    hVolume = CreateFileW(volumePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hVolume == INVALID_HANDLE_VALUE) {
        printf("Failed to open volume. Error code: %d\n", GetLastError());
        return 1;
    }

    // 부트 섹터 읽기
    if (!ReadFile(hVolume, buffer, sizeof(buffer), &dwBytesRead, NULL)) {
        printf("Failed, Error code: %d\n", GetLastError());
        CloseHandle(hVolume);
        return 1;
    }

    // 부트 섹터 데이터 처리
    printf("Boot sector data:\n");
    printf("-----------------\n");
    ProcessFileMetadata(buffer, dwBytesRead);

    // BPB 데이터 출력
    bpb = (NTFS_BPB*)buffer;
    Print_BPB(bpb);

    // 파일 시스템 닫기
    CloseHandle(hVolume);
    getchar();

    return 0;
}

void ProcessFileMetadata(U8* buffer, U16 dwBytesRead) {
    // 각 부트 섹터 데이터를 확인하기 위해 테스트 출력
    for (DWORD i = 0; i < dwBytesRead; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void Print_BPB(NTFS_BPB* bpb) {

    //MFT 사이즈가 0x80 이상이면 음수이므로 2의 보수로 변환하여 계산
    int MFTsize = bpb->SizeOfMFTEntry;
    MFTsize = pow(2, (-1)*MFTsize);

    printf("Jump Boot Code: %02X %02X %02X\n", bpb->JmpBoot[0], bpb->JmpBoot[1], bpb->JmpBoot[2]);
    printf("OEM Name: %.8s\n", bpb->OEMName);
    printf("Byte Per Sector: %u\n", bpb->BytePerSec);
    printf("Sector Per Cluster: %u\n", bpb->SecPerClus);
    printf("Reserved Sectors: %u\n", bpb->RsvdSecCnt);
    printf("Media: %02X\n", bpb->Media);
    printf("Total Sector: %llu\n", bpb->TotalSector);
    printf("Start of MFT: %llu\n", bpb->StartOfMFT);
    printf("Start of MFTMirr: %llu\n", bpb->StartOfMFTMirr);
    printf("Size of MFT Entry: %u\n", MFTsize);
    printf("Size of Index Record: %d\n", bpb->SizeOfIndexRecord);
    printf("Serial Number: %llX\n", bpb->SerialNumber);
    printf("Signature: %02X\n", bpb->Signature);
}
