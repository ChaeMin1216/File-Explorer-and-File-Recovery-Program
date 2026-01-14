#include <stdio.h>
#include <string.h>
#include <Windows.h>

void ListDrives(char (*drive)[4], int *drive_count)
{
    DWORD drive_mask = GetLogicalDrives();
    if (drive_mask == 0)
    {
        printf("GetLogicalDrives() failed with error code: %lu\n", GetLastError());
        return;
    }

    int count = 0;
    for (char drive_letter = 'A'; drive_letter <= 'Z'; ++drive_letter)
    {
        if (drive_mask & (1 << (drive_letter - 'A')))
        {
            snprintf(drive[count], 4, "%c:\\", drive_letter);
            count++;
        }
    }
    *drive_count = count;
}