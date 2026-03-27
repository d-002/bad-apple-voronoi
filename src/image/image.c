#include "image.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "logger/logger.h"
#include "utils/utils.h"

#define BUF_SIZE 64

enum error_code image_load(const char *path, struct image *image)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        logerror("Failed to open file for reading: '%s'", path);
        return IO_ERROR;
    }

    char size_buf[4];
    if (read(fd, size_buf, 4) != 4)
        goto read_err;
    image->w = ((unsigned char)size_buf[0] << 8) + (unsigned char)size_buf[1];
    image->h = ((unsigned char)size_buf[2] << 8) + (unsigned char)size_buf[3];
    image->size = image->w * image->h;
    image->pixels = calloc(image->size / 8 + 1, sizeof(uint8_t));

    char buf[BUF_SIZE];
    ssize_t count;
    size_t i = 0;
    while ((count = read(fd, buf, BUF_SIZE)) != 0)
    {
        if (count < 0)
            goto read_err;

        if (i + count > image->size / 8 + 1)
        {
            logerror("Specified image size was too small");
            free(image->pixels);
            close(fd);
            return IO_ERROR;
        }

        memcpy(image->pixels + i * sizeof(uint8_t), buf,
               count * sizeof(uint8_t));
        i += count;
    }

    if (i * 8 < image->size)
        logwarn(
            "Expected %ld pixels but only got %ld for image of size (%dx%d)",
            image->size, i, image->w, image->h);

    close(fd);
    return SUCCESS;

read_err:
    free(image->pixels);
    close(fd);
    logerror("Failed to read from file: '%s'", path);
    return IO_ERROR;
}

enum error_code image_save(const struct image *image, const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        logerror("Failed to open file for writing: '%s'", path);
        return IO_ERROR;
    }

    char size_buf[4] = { image->w >> 8, image->w, image->h >> 8, image->h };
    if (write(fd, size_buf, 4) != 4)
        goto write_err;

    size_t size = image->size / 8 + 1;
    size_t i = 0;
    while (i < size)
    {
        size_t remaining = MIN2(size - i, BUF_SIZE);
        ssize_t count = write(fd, image->pixels + i, remaining);
        if (count < 0)
            goto write_err;

        i += count;
    }

    close(fd);
    return SUCCESS;

write_err:
    close(fd);
    logerror("Failed to write to file: '%s'", path);
    return IO_ERROR;
}
