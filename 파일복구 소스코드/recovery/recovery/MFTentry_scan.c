#include <windows.h>
#include <stdio.h>
#include "ntfs.h"
#include "mft.h"
#include <math.h>

HANDLE OpenFilesystem(LPCWSTR volumePath);  //파일시스템에 대한 핸들을 받아오는 함수
BOOL ReadFilesystem(HANDLE hVolume, U8 *buffer, DWORD dwBytesRead); //받아온 핸들을 통해 파일시스템 파일을 여는 함수
void Print_BPB(NTFS_BPB* bpb);              //BPB의 기본적인 정보들을 초기화하고 출력하는 함수
void Print_MFT(MFTEntry *mft);              //MFT entry의 기본적인 정보들을 초기화하고 출력하는 함수
void Process_Hexdata(U8* buffer, DWORD dwBytesRead);    //BPB의 16진수 데이터를 출력하는 함수(디버깅용)
BOOL ReadMFTEntry(HANDLE hVolume, U32 StartOfMFToffset, MFTEntry *mft);  //부트섹터에서 MFT의 시작 섹터로 이동하는 함수

int main()
{
    LPCWSTR volumePath = L"\\\\.\\C:";
    U8 buffer[512];
    DWORD dwBytesRead;
    HANDLE hVolume;
    NTFS_BPB *bpb;
    MFTEntry mft;
    U32 StartOfMFToffset;
    U32 MFTcount=0;
    hVolume = OpenFilesystem(volumePath); //파일 시스템 열기
    dwBytesRead = ReadFilesystem(hVolume, buffer, sizeof(buffer));
    printf("Boot Sector Data\n");
    Process_Hexdata(buffer, dwBytesRead);

    bpb = (NTFS_BPB*)buffer;
    Print_BPB(bpb);

    StartOfMFToffset = bpb->BytePerSec*bpb->SecPerClus*bpb->StartOfMFT;

    while(1)
    {
        
        printf("*******The %dth MFT Entry********\n\n", MFTcount);
        
        if(!ReadMFTEntry(hVolume, StartOfMFToffset, &mft))  break;
        Print_MFT(&mft);
        StartOfMFToffset += 1024;
        MFTcount++;
    }

    getchar();
    return 0;
}

//파일 시스템에 대한 핸들러를 받아오는 함수
HANDLE OpenFilesystem(LPCWSTR volumePath)
{
    HANDLE hVolume;
    hVolume = CreateFileW(volumePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hVolume == INVALID_HANDLE_VALUE) {
        printf("Failed to open volume. Error code: %d\n", GetLastError());
        return hVolume;
    }
}

//받아온 핸들러를 통해 파일시스템 파일을 여는 함수
BOOL ReadFilesystem(HANDLE hVolume, U8 *buffer, DWORD buffersize)
{
    DWORD dwBytesRead;
    if(!ReadFile(hVolume, buffer, buffersize, &dwBytesRead, NULL))
    {
        printf("Failed, Error code: %d\n", GetLastError());
        
        CloseHandle(hVolume);
        return FALSE;
    }   //파일 시스템 읽어오기
    else    return dwBytesRead;
}

//BPB의 기본적인 정보들을 초기화하고 출력하기 위한 함수
void Print_BPB(NTFS_BPB* bpb) 
{

    //MFT 사이즈가 0x80 이상이면 음수이므로 2의 보수로 변환하여 계산
    int MFTsize = bpb->SizeOfMFTEntry;
    MFTsize = pow(2, (-1)*MFTsize);
    printf("*******BPB_information*******\n");
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
    printf("Signature: %02X\n\n", bpb->Signature);
}

//MFT의 기본적인 정보들을 초기화하고 출력하기 위한 함수
void Print_MFT(MFTEntry *mft)
{
    printf("MFT Entry Data\n");
    printf("$LogFile Sequence Number: %u\n", mft->Header.LSN);
    printf("Sequence Number: %d\n", mft->Header.SequenceNum);
    printf("Hard Link Count: %d\n", mft->Header.LinkCnt);
    printf("Offset to File First Attribute: 0x%x\n", mft->Header.OffsetFirstAttr);
    printf("Flag: 0x%x, %s\n", mft->Header.Flags, mft->Header.Flags == 0? "Deleted~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" : mft->Header.Flags == 1? "File":"Directory");
    printf("Real Size of MFT Entry: %d\n", mft->Header.UsedSizeOfEntry);
    printf("Allocated Size to MFT Entry: %d\n", mft->Header.AllocSizeOfEntry);
    printf("File Reference to Base Entry: 0x%x\n", mft->Header.FileRefer);
    printf("Next Attribute ID: 0x%x\n\n", mft->Header.NextAttrId);
}

//부트섹터 위치에서 MFT의 시작 위치로 이동하기 위한 함수
BOOL ReadMFTEntry(HANDLE hVolume, U32 StartOfMFToffset, MFTEntry *mft)
{
    U8 MFT_Entry[1024];
    LARGE_INTEGER MFToffset;
    
    MFToffset.QuadPart = (LONGLONG)StartOfMFToffset;

    if(SetFilePointer(hVolume, MFToffset.LowPart, &MFToffset.HighPart, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        printf("Failed to move %u, Error code: %d\n", StartOfMFToffset, GetLastError());
        return FALSE;
    }
    else
    {
        ReadFilesystem(hVolume, MFT_Entry, sizeof(MFT_Entry));
        memcpy(mft, MFT_Entry, sizeof(MFT_Entry));
        // printf("\n\n\nMFT Entry Hex digit\n");            //디버깅용 데이터 출력
        // for (DWORD i = 0; i < sizeof(MFT_Entry); i++) 
        // {
        //     printf("%02X ", MFT_Entry[i]);
        //     if ((i + 1) % 16 == 0) 
        //     {
        //         printf("\n");
        //     }
        // }
        // return TRUE;
    }
}

//boot sector 데이터를 16진수로 출력해주는 함수(디버깅용)
void Process_Hexdata(U8* buffer, DWORD dwBytesRead) 
{
    for (DWORD i = 0; i < dwBytesRead; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
