#ifndef HEADER_H
# define HEADER_H

#pragma warning ( disable : 4996 )  // _findfirst, _findnext 함수가 안전하지 않으므로 경고문 무시
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <Windows.h>

struct _finddata_t fd;  // 파일 검색에 사용됨. 파일 및 디렉터리 정보를 저장함

#endif