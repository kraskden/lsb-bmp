#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"

int main()
{
    const char *path = "test.bmp";
    FILE *f;
    if (!(f = fopen(path, "rb")))
        return 1;
    BmpFileInfo bmpFileInfo;
    if (!fread(&bmpFileInfo, sizeof (bmpFileInfo), 1, f))
        return 1;
    if (bmpFileInfo.id != BMP_ID)
        return 1;
    fseek(f, bmpFileInfo.offBits, SEEK_SET);
    return 0;
}
