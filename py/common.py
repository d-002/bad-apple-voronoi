import os
import sys
import time

def checks():
    global source, destination, list_source, list_destination

    if len(sys.argv) != 3:
        print('Wrong arguments.')
        exit(1)

    source, destination = sys.argv[1:3]
    if not os.path.isdir(source) or not os.path.isdir(destination):
        print('Source or destination are not valid directories.')
        exit(1)

    list_source = set(os.listdir(source))
    list_destination = set(os.listdir(destination))

def main(read_func, transform_func, write_func):
    todo = set()

    for file in list_destination:
        if file not in list_source:
            os.remove(os.path.join(destination, file))
    for file in list_source:
        add = False
        if file in list_destination:
            add = os.path.getmtime(os.path.join(source, file)) > \
                    os.path.getmtime(os.path.join(destination, file))
        else:
            add = True

        if add:
            todo.add(file)

    print(f'Found {len(todo)} files to convert.')
    start = time.time()
    for i, file in enumerate(todo):
        data_source = read_func(os.path.join(source, file))
        data_destination = transform_func(data_source)
        write_func(os.path.join(destination, file), data_destination)

        spent = time.time() - start
        eta = spent * (len(todo) / (i + 1) - 1)
        min_s, sec_s = divmod(round(spent), 60)
        min_e, sec_e = divmod(round(eta), 60)

        size = 50
        prop = i / len(todo)
        count = round(prop * size)
        print(f'Progress: [{'=' * count}{' ' * (size - count)}] ' \
                f'{round(prop * 100)}%, spent: {min_s:02}:{sec_s:02}, ' \
                f'eta: {min_e:02}:{sec_e:02}', end='\r')
    print()

    print(f'{sys.argv[0]} is done.')
