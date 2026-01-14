#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "ntfs.h"
#include "mft.h"

void start(LPCWSTR volumePath);                                         // 함수의 순차적인 실행을 위한 함수 정의
HANDLE Open_FileSystem(LPCWSTR volumePath);                             // 파일시스템 여는 함수
U32 Read_BPB(HANDLE hVolume, NTFS_BPB *bpb);                            // BPB영역의 데이터를 읽어옴
void Print_BPB(NTFS_BPB *bpb);                                          // BPB의 기본 정보들을 초기화해서 보여줌
MFTEntry GET_$MFT(HANDLE hVolume, U32 StartOfMFToffset, MFTEntry $mft); //$MFT 파일 따로 저장하기 위한 함수
void Print_MFT(MFTEntry *mft);                                          // MFT Entry의 기본 헤더 정보를 출력
void Identify_$MFTAttr(MFTEntry mft);                                   // 속성값 식별함수
int FindMFTSegments(MFTEntry mft, U16 nextAttr, U16 attrSIZE);          // 분할된 MFT영역 위치 식별
void Read_segMFT(HANDLE hVolume, U32 secpercluster, U64 segMFT[], int Real_segMFTSize[]);

U64 Real_segMFTOffset[20];
int Real_segMFTSize[20];

int main()
{
    LPCWSTR volumePath = L"\\\\.\\C:"; // 드라이브 경로 지정
    start(volumePath);
    Sleep(10000); // 디버깅용

    return 0;
}

void start(LPCWSTR volumePath)
{
    HANDLE hVolume;       // 파일시스템 핸들 받아올 변수
    NTFS_BPB bpb;         // bpb 영역을 초기화하기 위한 변수
    MFTEntry $mft;        // $MFT 파일을 저장할 변수 ->MFT영역에 대한 정보가 담겨있음
    U32 StartOfMFToffset; // bpb에서 MFT영역 시작 오프셋을 저장할 변수

    hVolume = Open_FileSystem(volumePath);            // 파일시스템 열음
    StartOfMFToffset = Read_BPB(hVolume, &bpb);       // bpb영역 읽어와서 오프셋을 저장함
    $mft = GET_$MFT(hVolume, StartOfMFToffset, $mft); // 너무 복잡해서 $MFT파일을 읽기위한 함수를 따로 만듬

    Identify_$MFTAttr($mft);
    Read_segMFT(hVolume, bpb.SecPerClus, Real_segMFTOffset, Real_segMFTSize);
    Sleep(100000);
}

HANDLE Open_FileSystem(LPCWSTR volumePath) // 볼륨 핸들받아오고 Read_BPB 호출
{
    HANDLE hVolume;

    hVolume = CreateFileW(volumePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

    if (hVolume == INVALID_HANDLE_VALUE) // 볼륨이 맞게 열렸는지 확인
    {
        printf("Failed to open Volume. \n Error Code: %d\n", GetLastError());
        Sleep(3000);
        exit(1);
    }
    return hVolume;
}

U32 Read_BPB(HANDLE hVolume, NTFS_BPB *bpb) // BPB 영역 읽어옴
{
    U8 buffer[512];
    DWORD dwBytesRead;
    U32 StartOfMFToffset;

    if (!ReadFile(hVolume, buffer, sizeof(buffer), &dwBytesRead, NULL))
    {
        printf("Failed to open Boot Sector.\n Error Code: %d\n", GetLastError());
        CloseHandle(hVolume);
    }

    *bpb = *(NTFS_BPB *)buffer;
    StartOfMFToffset = bpb->SecPerClus * bpb->StartOfMFT * bpb->BytePerSec;
    // Print_BPB(bpb);
    return StartOfMFToffset;
}

void Print_BPB(NTFS_BPB *bpb)
{
    int MFTsize = bpb->SizeOfMFTEntry;
    MFTsize = pow(2, (-1) * MFTsize);

    printf("<<<<<<<<<<<<\tBPB Data\t>>>>>>>>>>>>\n");
    printf("Jump Boot Code: %02X %02X %02X\n", bpb->JmpBoot[0], bpb->JmpBoot[1], bpb->JmpBoot[2]);
    printf("OEM Name: %.8s\n", bpb->OEMName);
    printf("Byte Per Sector: %u\n", bpb->BytePerSec);
    printf("Sector Per Cluster: %u\n", bpb->SecPerClus);
    printf("Reserved Sectors: %u\n", bpb->RsvdSecCnt);
    printf("Media: %02X\n", bpb->Media);
    printf("Total Sector: %llu\n", bpb->TotalSector);
    printf("Start of MFT: %llu\n", bpb->StartOfMFT);
    printf("Start of MFTMirr: %llu\n", bpb->StartOfMFTMirr);
    printf("Size of MFT Entry: %d\n", MFTsize);
    printf("Size of Index Record: %d\n", bpb->SizeOfIndexRecord);
    printf("Serial Number: %llX\n", bpb->SerialNumber);
    printf("Signature: %02X\n", bpb->Signature);
    printf("\n\n\n");
}

MFTEntry GET_$MFT(HANDLE hVolume, U32 StartOfMFToffset, MFTEntry $mft)
{
    LARGE_INTEGER MFToffset; // MFT영역 시작 위치 지정할 변수
    U8 $MFT[1024];           //$MFT파일 임시로 저장할 버퍼
    DWORD dwBytesRead;

    MFToffset.QuadPart = (LONGLONG)StartOfMFToffset;

    if (SetFilePointer(hVolume, MFToffset.LowPart, &MFToffset.HighPart, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        printf("Failed to get MFT data.\n Error code: %d\n", GetLastError());
        CloseHandle(hVolume);
    }

    else
    {
        ReadFile(hVolume, $MFT, 1024, &dwBytesRead, NULL);
        memcpy(&$mft, $MFT, 1024); // 읽어온 1024바이트만큼의 데이터를 저장해서 리턴해줌
        return $mft;
    }
}

void Print_MFT(MFTEntry *mft)
{
    printf("<<<<<<<<<<<<\tMFT Entry Data\t>>>>>>>>>>>>\n");
    printf("$LogFile Sequence Number: %u\n", mft->Header.LSN);
    printf("Sequence Number: %d\n", mft->Header.SequenceNum);
    printf("Hard Link Count: %d\n", mft->Header.LinkCnt);
    printf("Offset to File First Attribute: 0x%x\n", mft->Header.OffsetFirstAttr);
    printf("Flag: 0x%x, %s\n", mft->Header.Flags, mft->Header.Flags == 0 ? "Deleted" : mft->Header.Flags == 1 ? "File"
                                                                                                              : "Directory");
    printf("Real Size of MFT Entry: %d\n", mft->Header.UsedSizeOfEntry);
    printf("Allocated Size to MFT Entry: %d\n", mft->Header.AllocSizeOfEntry);
    printf("File Reference to Base Entry: 0x%x\n", mft->Header.FileRefer);
    printf("Next Attribute ID: 0x%x\n\n", mft->Header.NextAttrId);
    printf("\n");
}

void Identify_$MFTAttr(MFTEntry mft)
{
    //여가서부터
    U16 attr = mft.Header.OffsetFirstAttr;  //각 mftentry의 첫번째 속성 오프셋(통상 0x38)
    U16 attrSize[4];
    U16 nextAttr = attr;

    for (int i = 0; i < 4; i++)
    {
        attrSize[3 - i] = mft.Data[attr + 4 + i];
    }

    U32 attrSIZE = (attrSize[0] << 24) | (attrSize[1] << 16) | (attrSize[2] << 8) | attrSize[3];

    while (mft.Data[nextAttr] != 0xFF)
    {
        nextAttr = attrSIZE + nextAttr;
        attrSIZE = 0x0;

        for (int i = 0; i < 4; i++)
        {
            attrSize[3 - i] = mft.Data[nextAttr + 4 + i];
        }

        attrSIZE = (attrSize[0] << 24) | (attrSize[1] << 16) | (attrSize[2] << 8) | attrSize[3];

        // if(mft.Data[nextAttr] == 0x30) 여기까지 예상
        if (mft.Data[nextAttr + 8] == 0x01 && mft.Data[nextAttr] == 0x80)
            FindMFTSegments(mft, nextAttr, attrSIZE);
    }
}

int FindMFTSegments(MFTEntry mft, U16 nextAttr, U16 attrSIZE)
{
    // printf("Cluster Run used\n\n");
    // printf("\n\n%X, %X", nextAttr, attrSIZE);
    U8 runListOffset = mft.Data[nextAttr + 32];
    U8 clusterRunList[attrSIZE - runListOffset];
    U8 clusterOffsetCnt;
    U8 clusterSizeCnt;
    U8 startOffset = 0;
    U8 endOffset = 0;
    int count = 0;
    int i = -1;
    int OffsetClusterRun = 0;

    Cluster_Run *cluster = (Cluster_Run *)malloc(sizeof(Cluster_Run));
    if (cluster == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    for (int i = 0; i < attrSIZE - runListOffset; i++)
    {
        clusterRunList[i] = mft.Data[nextAttr + runListOffset + i];
    }

    while (clusterRunList[startOffset] != 0x00)
    {
        int index = 0;
        clusterOffsetCnt = clusterRunList[startOffset] >> 4;
        // printf("clusterOffset: %X\n", clusterOffsetCnt);

        clusterSizeCnt = clusterRunList[startOffset] & 0x0F;
        // printf("clusterSize: %X\n\n", clusterSizeCnt);

        endOffset = startOffset + clusterOffsetCnt + clusterSizeCnt;

        // printf("\n%dth Cluster Run Data\n", count);

        for (int i = startOffset; i <= endOffset; i++)
        {
            cluster[count].ClusterRUN[index] = clusterRunList[i];
            // printf("%02X ", cluster[count].ClusterRUN[index]);
            index++;
        }

        while (i < count)
        {
            // printf("Cluster Size: ");
            int SizeOfClusterRun = 0;

            for (int j = 0; j < clusterSizeCnt; j++)
            {
                cluster[count].ClusterSize[j] = cluster[count].ClusterRUN[j + 1];
                // printf("%02X ", cluster[count].ClusterSize[j]);
                SizeOfClusterRun += cluster[count].ClusterSize[j] << 8 * j;
            }

            Real_segMFTSize[count] = SizeOfClusterRun;

            // printf("\t Cluster Offset: ");
            if (cluster[count].ClusterRUN[clusterOffsetCnt + clusterSizeCnt] < 0x80)
            {
                for (int j = 0; j < clusterOffsetCnt; j++)
                {
                    cluster[count].ClusterOffset[j] = cluster[count].ClusterRUN[clusterSizeCnt + 1 + j];
                    // printf("%02X ", cluster[count].ClusterOffset[j]);
                    OffsetClusterRun += cluster[count].ClusterOffset[j] << 8 * j;
                }
            }
            else
            {
                int temp = 0x0;
                unsigned long long mask = (1ULL << clusterOffsetCnt * 8) - 1;

                for (int j = 0; j < clusterOffsetCnt; j++)
                {
                    cluster[count].ClusterOffset[j] = cluster[count].ClusterRUN[clusterSizeCnt + 1 + j];
                    // printf("%02X ", cluster[count].ClusterOffset[j]);
                    temp += cluster[count].ClusterOffset[j] << 8 * j;
                }

                OffsetClusterRun -= (~temp + 1) & mask;
            }
            Real_segMFTOffset[count] = OffsetClusterRun;
            i++;
        }

        startOffset = endOffset + 1;
        count++;
    }
    // if (cluster != NULL)
    // {
    //     free(cluster);
    //     cluster = NULL;
    // }
}

void Read_segMFT(HANDLE hVolume, U32 secpercluster, U64 segMFT[], int Real_segMFTSize[])
{
    int i = 0;
    U8 mftEntry[1024];
    MFTEntry *entry;
    DWORD dwbytes;
    LARGE_INTEGER liDistanceToMove;
    U32 mft_count = 0;
    while (Real_segMFTOffset[i] != 0)
    {

        U64 count = 0;
        liDistanceToMove.QuadPart = Real_segMFTOffset[i] * secpercluster * 512; //바이트단위
        printf("\n<<<<<<<<<<<<<<<< Cluster Num: %llX >>>>>>>>>>>>>>>\n", Real_segMFTOffset[i] * secpercluster * 512);

        for (count; count < Real_segMFTSize[i] * secpercluster / 2; count++)
        {
            LARGE_INTEGER add;
            add.QuadPart = liDistanceToMove.QuadPart + count * 1024; //바이트 단위
            SetFilePointerEx(hVolume, add, NULL, FILE_BEGIN);
            ReadFile(hVolume, mftEntry, 1024, &dwbytes, NULL);

            entry = (MFTEntry *)mftEntry;
            if (entry->Header.Flags == 0)
            {
                printf("%uth MFT Entry\nSector NUM: %lld\n", mft_count, add.QuadPart/512);
                Print_MFT(entry);
            }
            mft_count++;
        }
        i++;

        printf("\n\n");
    }
}
//파일 이름 저장해서 리턴해주는 함수 만들어야됨
//Read_segMFT 좀 더 수정해야됨
//속성을 찾아내는 연산 과정을 함수로 따로 만들고 분할된 MFT영역을 찾을때나 파일 이름을 찾을때 따로 호출하는 것이 나아보임
//Identify_$MFTAttr 함수를 분리
//ex) Acc_Attr(mft) 
//if(속성값 == 0x30)
//    if(filename == $MFT)
//    (0x80속성 오프셋을 반환해주며 FindSegMft 호출)