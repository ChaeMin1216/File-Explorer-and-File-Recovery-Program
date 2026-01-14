#include "fileexplorer.h"

int isFileOrDir()
{
    if (fd.attrib & _A_SUBDIR)
        return 0; // 디렉터리면 0 반환
    else
        return 1; // 그밖의 경우는 "존재하는 파일"이기에 1 반환
}

void SearchFileByName(char directory[], const char *target_file_name)
{
    intptr_t handle;
    int check = 0;
    char directory_backup[_MAX_PATH];
    
    strcat(directory, "\\");        
    strcpy(directory_backup, directory);
    strcat(directory, "*");         

    if ((handle = _findfirst(directory, &fd)) == -1)
    {
        return;
    }
    
    while (_findnext(handle, &fd) == 0)
    {
        char current_path[_MAX_PATH];
        strcpy(current_path, directory_backup);
        strcat(current_path, fd.name);

        check = isFileOrDir();

        if (check == 0 && fd.name[0] != '.')
        {
            SearchFileByName(current_path, target_file_name);  // 하위 디렉토리 검색 재귀함수
        }
        else if (check == 1 && strstr(fd.name, target_file_name) != NULL)
        {
            printf("Found file: %s\n", current_path);
        }
    }
    _findclose(handle);
}

// void SearchFileByName(char directory[], const char *target_file_name)
// {
//     intptr_t handle;
//     int check = 0;
//     char directory_backup[_MAX_PATH];

//     // 디렉토리 백업
//     strcpy(directory_backup, directory);

//     strcat(directory, "\\*"); // 경로에 파일 및 디렉토리를 모두 가져옴

//     if ((handle = _findfirst(directory, &fd)) == -1)
//     {
//         return;
//     }
    
//     while (_findnext(handle, &fd) == 0)
//     {
//         char current_path[_MAX_PATH];

//         // 이전 디렉토리 백업 사용
//         strcpy(current_path, directory_backup);
//         strcat(current_path, "\\");
//         strcat(current_path, fd.name);

//         check = isFileOrDir();

//         if (check == 0 && fd.name[0] != '.')
//         {
//             SearchFileByName(current_path, target_file_name);  // 하위 디렉토리 검색 재귀함수
//         }
//         else if (check == 1 && strstr(fd.name, target_file_name) != NULL)
//         {
//             printf("Found file: %s\n", current_path);
//         }
//     }
//     _findclose(handle);
// }
