#define U8 unsigned char
#define S8 char
#define U16 unsigned short
#define U32 unsigned int
#define S32 int
#define U64 unsigned long long

#pragma pack(1)
typedef struct _MFT_Header {
    U8 Signature[4];
    U16 OffsetFixUpArray;   //fixup 배열 오프셋
    U16 CountFixUpArray;    //fixup 배열 항목수
    U64 LSN;                //$LogFile에 존재하는 해당 파일 트랜잭션 위치
    U16 SequenceNum;        //순서 번호로 MFT Entry 생성 후 할당/해제 시 1씩 증가
    U16 LinkCnt;            //해당 MFT Entry에 연결된 하드 링크
    U16 OffsetFirstAttr;    //해당 MFT Entry의 첫 번째 속성 오프셋
    U16 Flags;              //0x0001 = 사용중, 0x0002 = 디렉터리
    U32 UsedSizeOfEntry;    //실제 크기
    U32 AllocSizeOfEntry;   //MFT Entry에 할당된 크기(1024)
    U64 FileRefer;          //non base일때 base Entry 주소
    U16 NextAttrId;         //다음 속성 ID
    U16 AlignTo4Bboundary;
    U32	NumberOfThisMFTEntry;

} MFT_Header;
#pragma pack()

typedef union _MFTEntry {
    MFT_Header Header;
    U8 Data[1024];
} MFTEntry;


#pragma pack(1)
typedef struct _Resident {
    U32 AttrSize;           //속성 길이
    U16 AttrOffset;         //속성값 오프셋
    U8 IndexedFlag;         //1이면 인덱스된 속성
    U8 Padding;             //미사용
} Resident;
#pragma pack()


#pragma pack(1)
typedef struct _Non_Resident {
    U64 StartVCNOfRunList;  //속성 내용이 담긴 런 리스트 시작 VCN
    U64 EndVCNOfRunList;    //속성 내용이 담긴 런 리스트 끝 VCN
    U16 RunListOffset;      //런 리스트 시작 위치
    U16 CompressUnitSize;   //압축된 경우 단위
    U32 Unused;
    U64 AllocSizeOfAttrContent; //속성 내용에 할당된 클러스터 크기
    U64 ActualSizeOfAttrContent;//속성 내용의 실제 크기
    U64 InitialSizeAttrContent; //속성 내용의 초기화된 크기
} Non_Resident;
#pragma pack()


#pragma pack(1)
typedef struct _Common_Header {
    U32 AttrTypeID;         //속성 ID
    U32 Length;             //헤더를 포함한 길이
    U8 NonResidentFlag;     //0x00: resident, 0x01: non-resident
    U8 NameLength;          //속성 이름 길이
    U16 NameOffset;         //속성 이름이 저장된 위치
    U16 Flags;              //0x0001: 압축, 0x4000: 암호화, 0x8000: Sparse
    U16 AttrID;             //고유 식별자
    union{
        Resident Res;
        Non_Resident NonRes;
    };
} Common_Header;
#pragma pack()


#pragma pack(1)
typedef struct _Attr_FILENAME {     //0x30
    U64 FileReferOfParantDIR;
    U64 CreateTime;
    U64 ModifyTime;
    U64 MFTmodifyTime;
    U64 AccessTime;
    U64 AllocSize;
    U64 UsedSize;
    U32 Flag;
    U32 ReparseValue;
    U8 NameLength;
    U8 Namespace;
    wchar_t FileName[256];
} Attr_FILENAME;
#pragma pack()


#pragma pack(1)
typedef struct _Attr_STANDARDINFO { //0x10
    U64 CreateTime;
    U64 ModifyTime;
    U64 MFTmodifyTime;
    U64 AccessTime;
    U32 Flag;
    U32 MaxVersion; //파일 버전 최대값
    U32 Version;    //파일 버전
    U32 ClassID;    //인덱스된 클래스 ID
    U32 OwnerID;    //파일 소유자 ID
    U32 SecurityID; 
    U64 Quata;
    U64 USN;
} Attr_StANDARDINFO;
#pragma pack()


#pragma pack(1)
typedef struct _ATTR_LIST_ENTRY {   //0x20
    U32 Type;
    U16 EntryLen;
    U8 NameLength;
    U8 NameOffset;
    U64 StartVCN;
    U64 BaseMFTAddr;
    U16 AttrID;
    wchar_t AttrName;
} ListEntry;
#pragma pack()

// typedef struct _Cluster_Run {
//     U8 ClusterRunLength; //한 개의 클러스터런 길이
//     BYTE ClusterOffset[20]; 
//     U8 ClusterSize[20]; 
//     U8 ClusterRUN[20]; //클러스터런 저장할 변수
// } Cluster_Run;

typedef struct {
    long long Offset; // 클러스터의 상대적인 오프셋
    U64 Length; // 클러스터의 길이
} ClusterRun;

typedef struct
{
    wchar_t *File_Name;
    U64 Sector_Num; //MFT Entry가 존재하는 섹터위치
    U64 File_Size;
    long long Cluster_Offset[256];  //실제 데이터가 저장된 클러스터의 위치
    U64 Cluster_Length[256];    //클러스터 런별 데이터 크기(클러스터 단위)
} D_Data;