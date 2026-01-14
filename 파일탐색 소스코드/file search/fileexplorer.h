#ifndef HEADER_H
# define HEADER_H

#pragma warning ( disable : 4996 )  // _findfirst, _findnext 함수가 안전하지 않으므로 경고문 무시
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <Windows.h>

struct _finddata_t fd;  // 파일 검색에 사용됨. 파일 및 디렉터리 정보를 저장함


int isFileOrDir()
{
    if (fd.attrib & _A_SUBDIR)
        return 0; // 디렉토리면 0 반환
    else
        return 1; // 그밖의 경우는 "존재하는 파일"이기에 1 반환
 
}
//_finddata_t 구조체의 attrib 속성을 사용하여 파일인지, 디렉터리인지 확인


void FileSearch(char file_path[])
{
    intptr_t handle;                // api 에서 검색을 하기 위한 handle 추가
    int check = 0;                  // 디렉터리인지 파일인지를 나타내는 변수
    char file_path2[_MAX_PATH];     // 원래 경로 백업용 (재귀에 사용)

    strcat(file_path, "\\");        // 경로지정을 위해 \\를 file_path에 추가
    strcpy(file_path2, file_path);  // file_path를 백업용에 저장
    strcat(file_path, "*");         // 와일드카드 문자인 * 을 이용해서 모든 파일을 나타냄

    if ((handle = _findfirst(file_path, &fd)) == -1)
    {
        printf("No such file or directory\n");
        return;
    }
    // findfirst를 통해 fd 구조체에서 파일의 정보를 검색함
    // 검색된 파일이 존재하지 않으면 -1을 출력하는 것을 이용
    
    while (_findnext(handle, &fd) == 0)
    {
        char file_pt[_MAX_PATH];        // 현재 검색된 것에 대한 경로를 저장하는 변수
        strcpy(file_pt, file_path2);    // 현재까지의 경로를 변수에 저장
        strcat(file_pt, fd.name);       // 현재 검색된 것의 이름을 file_pt에 저장

        check = isFileOrDir();          // 파일인지 디렉토리 인지 식별
    // findnext값이 0이면 다음 파일이나 디렉터리를 검색함. 
    // p.s -1값이 나온다면 검색할 파일이나 디렉터리가 없는 것

if (check == 0 && fd.name[0] != '.')
        // check에서 디렉터리라고 판단된 경우
        {
        FileSearch(file_pt);    //하위 디렉토리 검색 재귀함수
        }
        else if (check == 1 && fd.size != 0 && fd.name[0] != '.')
        // check에서 파일이라고 판단된 경우
        {
            printf("name : %s, size:%d\n", file_pt, fd.size);
        }
    }
    _findclose(handle);
}



#endif