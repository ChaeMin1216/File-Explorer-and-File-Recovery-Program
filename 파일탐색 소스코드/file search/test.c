#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char* inputcut(const char* a) 
{
    int length = 0;

    // 문자열의 길이 계산
    while (a[length] != '\0')
    {
        length++;
    }
    // 문자열을 저장할 동적 메모리 할당
    char* result = (char*)malloc((length + 1) * sizeof(char)); // NULL 문자를 위해 +1
    // 동적 메모리 할당 실패 확인
    if (result == NULL)
    {
        return NULL; // 실패 시 NULL 반환
    }
    // 문자열 복사
    for (int i = 0; i < length; i++)
    {
        result[i] = a[i];
    }
    // NULL 문자 추가
    result[length] = '\0';
    return result; // 결과 반환
    free(result);
}



int main(void) {
    char path[100] = {'\0'};
    scanf("%s", &path);
    printf("%s", inputcut(path));
}