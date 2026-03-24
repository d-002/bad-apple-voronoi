from common import checks, main

checks()

def read_func(path):
    with open(path, 'rb') as f:
        data = f.read()

    return data

def transform_func(data):
    return data

def write_func(path, data):
    with open(path, 'wb') as f:
        f.write(data)

main(read_func, transform_func, write_func)
