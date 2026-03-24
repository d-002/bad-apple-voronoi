from common import checks, main
from PIL import Image
import io

checks()

def read_func(path):
    image = Image.open(path)
    w, h = image.size
    pixels = image.get_flattened_data()
    buffer = io.BytesIO()
    buffer.write(chr(w // 256).encode())
    buffer.write(chr(w % 256).encode())
    buffer.write(chr(h // 256).encode())
    buffer.write(chr(h % 256).encode())

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

        buffer.write(chr(n).encode())

    return buffer.getvalue()

def transform_func(data):
    return data

def write_func(path, data):
    with open(path, 'wb') as f:
        f.write(data)

main(read_func, transform_func, write_func)
