#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include "ntfs.h"
#include "mft.h"
#define save_path L".\\test.csv"
void start(LPCWSTR volumePath);                                         // 함수의 순차적인 실행을 위한 함수 정의
HANDLE Open_FileSystem(LPCWSTR volumePath);                             // 파일시스템 여는 함수
U32 Read_BPB(HANDLE hVolume, NTFS_BPB *bpb);                            // BPB영역의 데이터를 읽어옴
void Print_BPB(NTFS_BPB *bpb);                                          // BPB의 기본 정보들을 초기화해서 보여줌
MFTEntry GET_$MFT(HANDLE hVolume, U32 StartOfMFToffset, MFTEntry $mft); //$MFT 파일 따로 저장하기 위한 함수
void Print_MFT(MFTEntry *mft);                                          // MFT Entry의 기본 헤더 정보를 출력
void Identify_$MFT_DataAttr(MFTEntry mft, U64 segMFTOffset[], int segMFTSize[]);                                   // 속성값 식별함수
int Find$MFTSegments(MFTEntry mft, U16 nextAttr, U16 attrSIZE, U64 segMFTOffset[], int segMFTSize[]);          // 분할된 MFT영역 위치 식별
void Read_segMFT(HANDLE hVolume, U32 secpercluster, U64 segMFTOffset[], int segMFTSize[]);
wchar_t* Return_name(MFTEntry* mft);
U16 Attr_offset(MFTEntry mft, U16 attr_offset);
void saveToFile(wchar_t* file_name, long long sector_num, U64 segMFTOffset[], int segMFTSize[]);
void ending();

int main()
{
    setlocale(LC_CTYPE, "");
    LPCWSTR volumePath = L"\\\\.\\C:"; // 드라이브 경로 지정
    // wchar_t volumePath[MAX_PATH];
    // wchar_t driveLetter;
    // wscanf(L"%1c", &driveLetter);
    // swprintf(volumePath, MAX_PATH, L"\\\\.\\%1c:", driveLetter);
    start(volumePath);
    ending();
    Sleep(10000); // 디버깅용

    return 0;
}

void start(LPCWSTR volumePath)
{
    U64 $MFT_segMFTOffset[20];
    int $MFT_segMFTSize[20];
    HANDLE hVolume;       // 파일시스템 핸들 받아올 변수
    NTFS_BPB bpb;         // bpb 영역을 초기화하기 위한 변수
    MFTEntry $mft;        // $MFT 파일을 저장할 변수 ->MFT영역에 대한 정보가 담겨있음
    U32 StartOfMFToffset; // bpb에서 MFT영역 시작 오프셋을 저장할 변수

    hVolume = Open_FileSystem(volumePath);            // 파일시스템 열음
    StartOfMFToffset = Read_BPB(hVolume, &bpb);       // bpb영역 읽어와서 오프셋을 저장함
    $mft = GET_$MFT(hVolume, StartOfMFToffset, $mft); // 너무 복잡해서 $MFT파일을 읽기위한 함수를 따로 만듬

    Identify_$MFT_DataAttr($mft, $MFT_segMFTOffset, $MFT_segMFTSize);

    // Return_name(&$mft);
    Read_segMFT(hVolume, bpb.SecPerClus, $MFT_segMFTOffset, $MFT_segMFTSize);
    // wprintf(L"%ls", Return_name(&$mft));
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
    // Return_name(mft);
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

void Identify_$MFT_DataAttr(MFTEntry mft, U64 segMFTOffset[], int segMFTSize[])
{
    U16 attr_offset = mft.Header.OffsetFirstAttr;
    U16 attrSIZE;
    U16 temp;

    while(mft.Data[attr_offset] != 0xFF)
    {
        attrSIZE = 0x0;
        temp = attr_offset;
        attr_offset = Attr_offset(mft, attr_offset);    //다음 속성 위치 반환받음
        for(int i=0; i<4; i++)
        attrSIZE += mft.Data[attr_offset+4+i]<<8*i;

        if (mft.Data[attr_offset + 8] == 0x01 && mft.Data[attr_offset] == 0x80)
        {
            // printf("attr_offset: %X, attrSIZE: %X\n", attr_offset, attrSIZE);
            Find$MFTSegments(mft, attr_offset, attrSIZE, segMFTOffset, segMFTSize);
        }
        // attrSIZE = attr_offset - temp;  //반환받은 다음 속성 위치 - 기존 속성 위치        
    }
}

int Find$MFTSegments(MFTEntry mft, U16 nextAttr, U16 attrSIZE, U64 segMFTOffset[], int segMFTSize[])
{   
    // printf("attrSize: %d, nextAttr: %x", attrSIZE, nextAttr);
    // printf("Cluster Run used\n\n");

    U8 runListOffset = mft.Data[nextAttr + 32];
    U8 *clusterRunList = (U8 *)malloc(attrSIZE - runListOffset);
    if (clusterRunList == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    U8 clusterOffsetCnt;
    U8 clusterSizeCnt;
    U8 startOffset = 0;
    U8 endOffset = 0;
    int count = 0;
    int i = -1;
    int OffsetClusterRun = 0;

    Cluster_Run *cluster = (Cluster_Run *)malloc(sizeof(Cluster_Run) * 128);  // 128개 클러스터 할당 (수정필요할 수도 있음)
    if (cluster == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(clusterRunList);
        return 1;
    }

    for (int i = 0; i < attrSIZE - runListOffset; i++)                  //데이터속성 크기에서 클러스터런 시작 위치를 뺌(전체 클러스터런 크기 식별을 위함)
    {
        clusterRunList[i] = mft.Data[nextAttr + runListOffset + i];
        // printf("%02X ", clusterRunList[i]);
    }

    while (clusterRunList[startOffset] != 0x00)
    {
        int index = 0;
        clusterOffsetCnt = clusterRunList[startOffset] >> 4;
        clusterSizeCnt = clusterRunList[startOffset] & 0x0F;
        endOffset = startOffset + clusterOffsetCnt + clusterSizeCnt;
        // printf("\n%dth Cluster Run Data\n", count);

        for (int i = startOffset; i <= endOffset; i++)
        {
            cluster[count].ClusterRUN[index] = clusterRunList[i];
            // printf("TEST: %X", cluster[count].ClusterRUN[index]);
            index++;
        }
        // printf("\nTEST: %X\n", cluster[count].ClusterRUN);
        while (i < count)
        {
            int SizeOfClusterRun = 0;

            for (int j = 0; j < clusterSizeCnt; j++)
            {
                cluster[count].ClusterSize[j] = cluster[count].ClusterRUN[j + 1];
                SizeOfClusterRun += cluster[count].ClusterSize[j] << 8 * j;
            }

            segMFTSize[count] = SizeOfClusterRun;

            // printf("\t Cluster Offset: ");
            if (cluster[count].ClusterRUN[clusterOffsetCnt + clusterSizeCnt] < 0x80)
            {
                for (int j = 0; j < clusterOffsetCnt; j++)
                {
                    cluster[count].ClusterOffset[j] = cluster[count].ClusterRUN[clusterSizeCnt + 1 + j];
                    // printf("%02X", cluster[count].ClusterOffset[j]);
                    OffsetClusterRun += cluster[count].ClusterOffset[j] << 8 * j;
                }
                // printf("OffsetClusterRun: %X\n", OffsetClusterRun);
            }
            else
            {
                int temp = 0x0;
                unsigned long long mask = (1ULL << clusterOffsetCnt * 8) - 1;

                for (int j = 0; j < clusterOffsetCnt; j++)
                {
                    cluster[count].ClusterOffset[j] = cluster[count].ClusterRUN[clusterSizeCnt + 1 + j];
                    temp += cluster[count].ClusterOffset[j] << 8 * j;
                    
                }
                // printf("\ntemp: %X\n", temp);
                OffsetClusterRun -= (~temp + 1) & mask;
                temp = 0x0;
                // printf("OffsetClusterRun: %d\n", OffsetClusterRun);
            }

            segMFTOffset[count] = OffsetClusterRun;
            // printf("\nReal_segMFT: %llu\n",Real_segMFTOffset[count]);
            i++;
        }
        printf("Cluster Run %d - Size: %d, Offset: %x\n", count, segMFTSize[count], segMFTOffset[count]);

        startOffset = endOffset + 1;
        count++;
    }

    // 메모리 해제
    free(clusterRunList);
    free(cluster);

    return 0;
}


void Read_segMFT(HANDLE hVolume, U32 secpercluster, U64 segMFTOffset[], int segMFTSize[])
{
    int i = 0;
    U8 mftEntry[1024];
    MFTEntry *entry;
    DWORD dwbytes;
    LARGE_INTEGER liDistanceToMove;
    U64 mft_count = 0;
    MFTEntry mft;
    wchar_t *filename;

    while (segMFTOffset[i] != 0)
    {
        U64 count = 0;
        liDistanceToMove.QuadPart = segMFTOffset[i] * secpercluster * 512; // 바이트단위
        printf("\n<<<<<<<<<<<<<<<<  %dth Cluster Num: %llX  >>>>>>>>>>>>>>>\n", (i+1), segMFTOffset[i] * secpercluster * 512);

        for (count; count < segMFTSize[i] * secpercluster / 2; count++)
        {
            U64 segMFTOffset[20];
            int segMFTSize[20];
            for(int i = 0; i <20; i++)
            {
                segMFTOffset[i] = 0;
                segMFTSize[i] = 0;
            }
            LARGE_INTEGER add;
            add.QuadPart = liDistanceToMove.QuadPart + count * 1024; // 바이트 단위
            SetFilePointerEx(hVolume, add, NULL, FILE_BEGIN);
            ReadFile(hVolume, mftEntry, 1024, &dwbytes, NULL);

            entry = (MFTEntry *)mftEntry;
            // printf("%uth MFT Entry\nSector NUM: %lld\n", mft_count, add.QuadPart / 512);
            
            // printf("\n\t\t%d\t%d\n", entry->Header.Flags, entry->Header.Flags & 0x0001);

            if ((entry->Header.Flags & 0x0001) == 0)
            {
                // printf("%llu th MFT Entry\nSector NUM: %lld\n", mft_count, add.QuadPart / 512);
                // if(entry->Header.Flags == 0x0003)
                // printf("<<<<<<<<<<<<\tMFT Entry Data\t>>>>>>>>>>>>\n");
                if(entry->Header.OffsetFirstAttr != 0x0)
                {   
                    Identify_$MFT_DataAttr(*entry, segMFTOffset, segMFTSize);
                    
                    filename = Return_name(entry);
                    if (filename==NULL)
                        continue;
                    // wprintf(L"\nFILE NAME: %ls\n", filename);
                    saveToFile(filename, add.QuadPart/512, segMFTOffset, segMFTSize);

                }
                // Print_MFT(entry);
                
            }
            ++mft_count;
        }
        i++;

        printf("\n");
    }
}

wchar_t* Return_name(MFTEntry *mft) {
    U16 attr_offset = mft->Header.OffsetFirstAttr; 
    U16 attrSIZE;
    U16 temp;
    wchar_t *last_file_name = NULL;  // 가장 마지막 0x30 속성의 파일 이름을 저장할 변수

    if (attr_offset == 0x00)
        return 0;

    while (mft->Data[attr_offset] != 0xFF) 
    {
        temp = attr_offset;
        attr_offset = Attr_offset(*mft, attr_offset); 
        attrSIZE = attr_offset - temp; // 다음 속성 위치 - 기존 속성 위치

        // 속성 유형이 파일 이름(0x30)인 경우
        if (mft->Data[temp] == 0x30) 
        {
            // 마지막으로 발견된 0x30 속성의 파일 이름을 저장
            U8 *buffer = (U8 *)malloc(attrSIZE);
            if (buffer == NULL) {
                // 메모리 할당 실패 처리
                return NULL;
            }

            // 속성 데이터 복사
            memcpy(buffer, &mft->Data[temp + 0x18], attrSIZE - 0x18);  

            // 파일 이름 추출
            wchar_t *file_name = (wchar_t *)(buffer + 66); 
           
            // 이전에 저장된 이름이 있다면 해제
            if (last_file_name != NULL) {
                free(last_file_name);
            }

            // 현재 파일 이름을 복사하여 last_file_name에 저장
            last_file_name = wcsdup(file_name); 
            free(buffer);
        }
    }

    return last_file_name; // 가장 마지막 0x30 속성의 파일 이름을 반환
}



// 내가 수정한 부분
// wchar_t* Return_name(MFTEntry *mft) {
//     U16 attr_offset = mft->Header.OffsetFirstAttr; 
//     U16 attrSIZE;
//     U16 temp;

//     if(attr_offset == 0x00)
//         return 0;
    
//     while (mft->Data[attr_offset] != 0xFF) 
//     {
//         temp = attr_offset;
//         attr_offset = Attr_offset(*mft, attr_offset); 
//         attrSIZE = attr_offset - temp; // 다음 속성 위치 - 기존 속성 위치

//         // 속성 유형이 파일 이름인 경우
//         if (mft->Data[temp] == 0x30) 
//         {
//             U8 *buffer = (U8 *)malloc(attrSIZE);
//             if (buffer == NULL) {
//                 // 메모리 할당 실패 처리
//                 return NULL;
//             }
            
//             // 속성 데이터 복사
//             memcpy(buffer, &mft->Data[temp + 0x18], attrSIZE - 0x18);  
            
//             // 파일 이름 추출
//             wchar_t *file_name = (wchar_t *)(buffer + 66); 
           
//             // 결과 반환 전에 메모리 해제
//             wchar_t *result = wcsdup(file_name); // 파일 이름을 복사하여 반환
//             free(buffer);
//             return result;
//         }
//     }

//     return "null"; // 파일 이름 속성이 없는 경우
// }
// wchar_t Return_name(MFTEntry mft)
// {
//     U16 attr_offset = mft.Header.OffsetFirstAttr;
//     U16 attrSIZE;
//     U16 temp;

//     while(mft.Data[attr_offset] != 0xFF)
//     {
//         temp = attr_offset;
//         attr_offset = Attr_offset(mft, attr_offset);    //다음 속성 위치 반환받음

//         attrSIZE = attr_offset - temp;  //반환받은 다음 속성 위치 - 기존 속성 위치

//         if (mft.Data[attr_offset] == 0x30)  //filename 속성이면
//         {
//             U8 buffer[attrSIZE];
//             memcpy(buffer, &mft.Data[attr_offset + 0x18], attrSIZE - 0x18);

//             wchar_t *file_name = (wchar_t *)(buffer + 66);
//             size_t length = buffer[64] * 2;
//             size_t num_chars = length / sizeof(wchar_t);

//             // wprintf(L"%ls", file_name);
//             return file_name;
//         }
//     }
// }

U16 Attr_offset(MFTEntry mft, U16 attr_offset) // 시작할 속성의 offset받음 => 다음 속성 위치 반환
{
    // U16 attr = mft.Header.OffsetFirstAttr;
    U16 attrSize[4];
    // U16 nextAttr = attr;

    for (int i = 0; i < 4; i++)
    {
        attrSize[3 - i] = mft.Data[attr_offset + 4 + i]; // 입력받은 offset의 속성의 크기를 저장
    }

    U32 attrSIZE = (attrSize[0] << 24) | (attrSize[1] << 16) | (attrSize[2] << 8) | attrSize[3];

    U16 next_offset = attrSIZE + attr_offset;

    return next_offset;
}

void saveToFile(wchar_t* file_name, long long sector_num, U64 segMFTOffset[], int segMFTSize[])
{
    
    FILE* file = _wfopen(save_path, L"a");  // UTF-16LE로 파일을 엽니다., ccs=UTF-16LE

    // 파일 열기 실패 시
    if (file == NULL) {
        wprintf(L"Failed to open file: %ls\n", save_path);
        Sleep(1000);
        return;
    }

    // 파일에 데이터 쓰기
    if (file_name[0] != '\0') {  // 파일 이름이 비어있지 않은 경우
        fwprintf(file, L"\n%lls , %llu ,", file_name, sector_num);  // CSV 형식으로 저장
        int i = 0;
        while(segMFTOffset[i] != 0)
        {
            fwprintf(file, L" %llu | ", segMFTOffset[i]);
            i++;
        }
        fwprintf(file, L" , ");
        i = 0;
        while(segMFTSize[i] != 0)
        {
            fwprintf(file, L" %d | ", segMFTSize[i]);
            i++;
        }
    } else {  // 파일 이름이 비어있는 경우
        fwprintf(file, L", %llu\n", sector_num);  // 파일 이름이 비어있다면 첫 번째 칸을 비워둠
    }

    // 파일 닫기
    fclose(file);

    // wprintf(L"Data appended successfully to %ls\n", save_path);
}

//숨김속성             SetFileAttributesW(full_path, FILE_ATTRIBUTE_HIDDEN);

void ending() {
    char input;
    
    printf("Press 'q' to quit.\n");

    do {
        input = getchar();  // 사용자 입력 받기
    } 
    while (input != 'q');
    printf("Program is ending...\n");
    
    return;
}
