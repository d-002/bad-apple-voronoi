#include <stdio.h>
#include <stdlib.h>

#include "image/loadimage.h"

int main(void)
{
    struct image img = loadResizedImage("sample_image.png", 100, 100);
    printf("(%d %d), %p\n", img.w, img.h, (void *)img.pixels);
    free(img.pixels);
}
