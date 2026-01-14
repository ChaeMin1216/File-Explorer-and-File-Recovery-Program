#include "Func_Set.h"

#define MAX_LINE_LENGTH 512
#define MAX_CLUSTERS 256
#define MAX_OFFSET_LENGTH 64  // 클러스터 오프셋의 최대 길이
#define MAX_BUFFER_SIZE 512


// Target_File 구조체 선언
typedef struct {
    wchar_t File_Name[MAX_LINE_LENGTH];  // 파일 이름
    U64 Sector_Num;                      // 섹터 번호
    wchar_t Cluster_Offset[MAX_CLUSTERS][MAX_OFFSET_LENGTH];  // 클러스터 오프셋 (문자열로 저장)
    U64 Cluster_Length[MAX_CLUSTERS];    // 클러스터 길이
} Target_File;

// Target_File 구조체 초기화 함수
void initialize_target_file(Target_File *target) {
    // File_Name 초기화
    wcscpy(target->File_Name, L"");

    // Sector_Num 초기화
    target->Sector_Num = 0;

    // Cluster_Offset, Cluster_Length 초기화
    for (int i = 0; i < MAX_CLUSTERS; i++) {
        wcscpy(target->Cluster_Offset[i], L"");  // 문자열 초기화
        target->Cluster_Length[i] = 0;
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

    FILE *file = _wfopen(file_name, L"r, ccs=UTF-16LE"); // UTF-8로 인코딩된 파일 읽기
    if (file == NULL) {
        perror("cannot open file");
        return;
    }

    wchar_t line[MAX_LINE_LENGTH];
    long long current_row = 0;
    Target_File target_file; // Target_File 구조체 선언
    initialize_target_file(&target_file); // 구조체 초기화

    // 파일을 한 줄씩 읽어가며 해당 행을 찾음
    while (fgetws_custom(line, sizeof(line) / sizeof(wchar_t), file)) {
        current_row++;

        if (current_row == target_row) {
            wchar_t *token;
            int cluster_index = 0; // 클러스터 관련 배열 인덱스

            // 첫 번째 열: File_Name 저장
            token = wcstok(line, L",");
            if (token) {
                wcscpy(target_file.File_Name, token);
                wprintf(L"File Name: %ls\n", target_file.File_Name);
            }

            // 두 번째 열: Sector_Num 저장
            token = wcstok(NULL, L",");
            if (token) {
                target_file.Sector_Num = wcstoull(token, NULL, 10);
                wprintf(L"Sector Num: %llu\n", target_file.Sector_Num);
            }

            // 세 번째 열: Cluster_Length 저장
            token = wcstok(NULL, L",");
            if (token) {
                target_file.Cluster_Length[0] = wcstoull(token, NULL, 10);
                wprintf(L"Cluster Length[0]: %llu\n", target_file.Cluster_Length[0]);
            }

            // 네 번째 열 이후의 값: Cluster_Offset 배열에 문자열로 저장
            while (cluster_index < MAX_CLUSTERS) {
                token = wcstok(NULL, L",");
                if (token) {
                    // Cluster_Offset 배열에 전체 문자열로 저장
                    wcscpy(target_file.Cluster_Offset[cluster_index], token);
                    wprintf(L"Cluster Offset[%d]: %ls\n", cluster_index, target_file.Cluster_Offset[cluster_index]);
                    cluster_index++;  // 다음 배열 인덱스로 이동
                } else {
                    break;  // 값이 없으면 반복문 종료
                }
            }

            wprintf(L"\n");

            // 구조체 초기화
            initialize_target_file(&target_file);

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
