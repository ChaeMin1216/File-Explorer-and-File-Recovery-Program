#include "Func_Set.h"

#define MAX_LINE_LENGTH 512
#define MAX_CLUSTERS 256

// D_Data 구조체 선언
// typedef struct
// {
//     wchar_t *File_Name;  // 파일 이름
//     U64 Sector_Num;      // 섹터 번호
//     U64 File_Size;       // 파일 크기
//     long long Cluster_Offset[256];  // 클러스터 위치
//     U64 Cluster_Length[256];        // 클러스터 길이
// } D_Data;

/*
    *******************************
    함수 get_row_data 을 사용할 경우
    wprintf 값들은 제거하기
    *******************************
*/


// D_Data 구조체 초기화 함수
void initialize_d_data(D_Data *d_data) {
    // File_Name 초기화
    d_data->File_Name = NULL;

    // Sector_Num 초기화
    d_data->Sector_Num = 0;

    // File_Size 초기화
    d_data->File_Size = 0;

    // Cluster_Offset, Cluster_Length 초기화
    for (int i = 0; i < MAX_CLUSTERS; i++) {
        d_data->Cluster_Offset[i] = 0;  // 클러스터 오프셋 초기화
        d_data->Cluster_Length[i] = 0;  // 클러스터 길이 초기화
    }
}

// '|\n' 구분자를 기준으로 한 줄씩 읽는 함수
wchar_t *fgetws_custom(wchar_t *buffer, int n, FILE *stream) {
    wchar_t *result = buffer;
    wint_t wc;
    int i = 0;

    while (i < n - 1) {  // n-1 까지만 읽기
        wc = fgetwc(stream);  // 파일로부터 wide character 읽기

        if (wc == WEOF) {  // 파일 끝에 도달한 경우
            if (i == 0)  // 아무 문자도 읽지 않았으면 NULL 반환
                return NULL;
            break;
        }

        buffer[i++] = wc;  // 버퍼에 문자를 저장

        // '|'를 만났고 그 다음 문자가 '\n'인지 확인
        if (wc == L'|') {
            wc = fgetwc(stream);  // 다음 문자를 읽음
            if (wc == L'\n') {    // 다음 문자가 '\n'이면 종료
                break;
            } else if (wc == WEOF) {
                break;  // 파일 끝이면 종료
            } else {
                buffer[i++] = wc;  // 다른 문자라면 계속 읽기
            }
        }
    }

    buffer[i] = L'\0';  // 널 종단 문자 추가
    return result;  // 버퍼 반환
}

// 특정 행의 데이터를 가져오는 함수
void get_row_data(const wchar_t *file_name, long long target_row) {

    FILE *file = _wfopen(file_name, L"r, ccs=UTF-16LE"); // UTF-16LE로 인코딩된 파일 읽기
    if (file == NULL) {
        perror("cannot open file");
        return;
    }

    wchar_t line[MAX_LINE_LENGTH];      // 한 줄씩 처리할 버퍼 line 선언
    long long current_row = 0;          // 현재 처리 중인 행 번호
    D_Data d_data; // D_Data 구조체 선언
    initialize_d_data(&d_data); // 구조체 초기화

    // 파일을 한 줄씩 읽어가며 해당 행을 찾음
    while (fgetws_custom(line, sizeof(line) / sizeof(wchar_t), file)) {
        current_row++;      // 다음 행 처리

        if (current_row == target_row) {
            wchar_t *token;
            int cluster_index = 0; // 클러스터 관련 배열 인덱스

            // 첫 번째 열: File_Name 저장
            token = wcstok(line, L",");         // 버퍼 line에 존재하는 한 줄에서 구분자 , 을 이용해서 구분
            if (token) {
                d_data.File_Name = _wcsdup(token);  // 메모리 할당 및 문자열 복사
                wprintf(L"File Name: %ls\n", d_data.File_Name);     
            }

            // 두 번째 열: Sector_Num 저장
            token = wcstok(NULL, L",");         // 버퍼 line 에서 이미 구분된 값(null)을 제외하고 다음 문자에서 구분자 , 을 이용해서 구분
            if (token) {
                d_data.Sector_Num = wcstoull(token, NULL, 10);
                wprintf(L"Sector Num: %llu\n", d_data.Sector_Num);
            }

            // 세 번째 열: File_Size 저장
            token = wcstok(NULL, L",");
            if (token) {
                d_data.File_Size = wcstoull(token, NULL, 10);
                wprintf(L"File Size: %llu\n", d_data.File_Size);
            }

            // 네 번째 열 이후의 값 처리
            while (cluster_index < MAX_CLUSTERS) {
                token = wcstok(NULL, L",|");  // ',' 또는 '|' 기준으로 토큰화   (csv 파일 상에서 가장 마지막 Cluster_Offset의 값 뒤에 | 가 나오기 때문)
                if (token) {
                    wchar_t *colon_pos = wcschr(token, L':');  // ':' 위치 찾기 (csv 파일 상에서 Cluster_Offset의 값이 12345:1 형식으로 나오기 때문)

                    if (colon_pos) {
                        // ':' 이전 값은 Cluster_Offset
                        *colon_pos = L'\0';  // ':'을 널 문자로 교체
                        d_data.Cluster_Offset[cluster_index] = wcstoll(token, NULL, 10);

                        // ':' 이후 값은 Cluster_Length
                        d_data.Cluster_Length[cluster_index] = wcstoull(colon_pos + 1, NULL, 10);

                        wprintf(L"Cluster Offset[%d]: %lld, Cluster Length[%d]: %llu\n",
                                cluster_index, d_data.Cluster_Offset[cluster_index],
                                cluster_index, d_data.Cluster_Length[cluster_index]);
                        cluster_index++;  // 다음 배열 인덱스로 이동
                    }
                } else {
                    break;  // 값이 없으면 반복문 종료
                }
            }

            wprintf(L"\n");

            // 구조체 초기화
            initialize_d_data(&d_data);

            break; // 원하는 행을 찾았으므로 루프 종료
        }
    }

    if (current_row < target_row) {
        printf("The specified row doesn't exist.\n");
    }

    fclose(file);
}

int main() {
    const wchar_t *csv_file = L"./test.csv"; // CSV 파일 이름
    long long target_row = 0;                // 가져오려는 행 번호 (헤더는 제외한 데이터 기준)
    
    while (1) {
        wprintf(L"Enter row number: ");
        scanf("%lld", &target_row); // 행 번호 입력 받기
        get_row_data(csv_file, target_row);  // 해당 행의 데이터를 출력
    }

    return 0;
}