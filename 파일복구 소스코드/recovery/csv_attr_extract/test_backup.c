#include "Func_Set.h"

#define MAX_LINE_LENGTH 512

// 특정 행의 데이터를 가져오는 함수
void get_row_data(const wchar_t *file_name, long long target_row) {

    setlocale(LC_ALL, "ko_KR.UTF-8"); // UTF-8 로케일 설정

    FILE *file = _wfopen(file_name, L"r, ccs=UTF-8"); // UTF-8로 인코딩된 파일 읽기
    if (file == NULL) {
        perror("cannot open file");
        return;
    }

    wchar_t line[MAX_LINE_LENGTH];
    long long current_row = 0;
    printf("current_row : %lld\n", current_row);
    // 파일을 한 줄씩 읽어가며 해당 행을 찾음
    while (fgetws(line, sizeof(line) / sizeof(wchar_t), file)) {
        current_row++;
        printf("current_row : %lld\n", current_row);
        // target_row와 current_row가 일치하면 해당 행의 데이터를 파싱
        if (current_row == target_row) {
            wchar_t *token;

            // 첫 번째 열: file_name
            token = wcstok(line, L",");
            wprintf(L"File Name: %ls\n", token);
            
            // 두 번째 열: MFT Size
            token = wcstok(NULL, L",");
            if (token) {
                wprintf(L"MFT Size: %ls\n", token);
            }
            else {
                wprintf(L"MFT Size: (null)\n");
                printf("current_row : %lld\n", current_row);
            }
            
            // 세 번째 열: File Size
            token = wcstok(NULL, L",");
            if (token) {
                wprintf(L"File Size: %ls\n", token);
            } else {
                wprintf(L"File Size: (null)\n");
                printf("current_row : %lld\n", current_row);
            }

            // 네 번째 열: Cluster Run
            token = wcstok(NULL, L",");
            if (token) {
                wprintf(L"Cluster Run: %ls", token);
            } else {
                wprintf(L"Cluster Run: (null)\n");
            }

            // 추가 열 출력 반복문
            while (token != NULL) {
                token = wcstok(NULL, L",");
                if (token && wcslen(token) > 0) {  // 값이 있으면 출력
                    wprintf(L", %ls", token);
                } else {  // 값이 없으면 반복문 종료
                    break;
                }
            }
            printf("current_row : %lld\n", current_row);
            wprintf(L"\n");

            break; // 원하는 행을 찾았으므로 루프 종료
        }
        printf("current_row : %lld\n", current_row);
    }

    if (current_row < target_row) {
        printf("The specified row doesn't exist.\n");
    }

    fclose(file);

}


int main() {
    setlocale(LC_ALL, "ko_KR.UTF-8"); // UTF-8 로케일 설정

    const wchar_t *csv_file = L"./test.csv"; // CSV 파일 이름
    long long target_row = 0;                // 가져오려는 행 번호 (헤더는 제외한 데이터 기준)
    
    while (1) {
        wprintf(L"Enter row number: ");
        scanf("%lld", &target_row); // 행 번호 입력 받기
        get_row_data(csv_file, target_row);  // 해당 행의 데이터를 출력
    }

    return 0;
}
