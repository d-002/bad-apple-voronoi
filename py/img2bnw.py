from common import checks, main
from PIL import Image

def read_func(path):
    image = Image.open(path).convert('RGB')
    w, h = image.size
    pixels = image.get_flattened_data()
    buf = bytearray(5 + w * h // 8)
    buf[0] = w >> 8
    buf[1] = w & 0xff
    buf[2] = h >> 8
    buf[3] = h & 0xff

    i = 0
    while i < w * h:
        n = 0
        for _ in range(8):
            i += 1
            if i < w * h:
                value = int(pixels[i][0] >= 128)
            else:
                value = 0
            n = (n << 1) + value

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
