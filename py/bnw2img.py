import struct
import numpy as np
from PIL import Image
from typing import Any

from common import checks, main

def read_func(path: str) -> Any:
    with open(path, 'rb') as f:
        data = f.read()

    return data

def transform_func(data: Any) -> Any:
    w, h = struct.unpack('>HH', data[:4])
    pixels = np.zeros((h, w, 3), dtype=np.uint8)

    i = 0
    while i * 8 < w * h:
        n = data[i + 4]

        for j in range(8):
            y, x = divmod(i * 8 + j, w)
            col = 255 if n & (1 << (7 - j)) else 0
            pixels[y][x] = [col, col, col]

        i += 1

    return Image.fromarray(pixels, 'RGB')

def write_func(path: str, data: Any) -> None:
    data.save(path)

if __name__ == '__main__':
    checks()

    main(read_func, transform_func, write_func)
