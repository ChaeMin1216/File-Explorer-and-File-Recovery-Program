#include <stdlib.h>
#include <stdio.h>

void ending() {
    char input;
    
    printf("Press 'q' to quit.\n");

    do {
        input = getchar();  // 사용자 입력 받기
    } while (input != 'q');

    printf("Program is ending...\n");
}