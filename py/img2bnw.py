from common import checks, main
from PIL import Image

def read_func(path):
    image = Image.open(path).convert('RGB')
    old_w, old_h = image.size
    pixels = image.get_flattened_data()

    # optionally crop the video, here I use a Bad apple video in 4:3 but stored
    # as 16:9
    old_ratio, new_ratio = old_w / old_h, 4 / 3
    if new_ratio < old_ratio:
        w = int(old_w * new_ratio / old_ratio)
        h = old_h
        dx = (old_w - w) // 2
        dy = 0
    else:
        w = old_w
        h = int(old_h * new_ratio / old_ratio)
        dx = 0
        dy = (old_h - h) // 2

    buf = bytearray(5 + w * h // 8)
    buf[0] = w >> 8
    buf[1] = w & 0xff
    buf[2] = h >> 8
    buf[3] = h & 0xff

    i = 0
    while i < w * h:
        n = 0
        for _ in range(8):
            y, x = divmod(i, w)
            if i < w * h:
                i_ = (y + dy) * old_w + (x + dx)
                value = int(pixels[i_][0] >= 128)
            else:
                value = 0
            n = (n << 1) + value

            i += 1

        buf[i // 8 + 4] = n

    return buf

def transform_func(data):
    return data

def write_func(path, data):
    with open(path, 'wb') as f:
        f.write(data)

if __name__ == '__main__':
    checks()

    main(read_func, transform_func, write_func)
