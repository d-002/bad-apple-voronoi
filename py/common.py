import os
import sys
import time
from threading import Thread
from multiprocessing import Manager, Pool

def checks():
    global force, source, destination, list_source, list_destination

    force = False
    while '-f' in sys.argv:
        force = True
        sys.argv.remove('-f')
    if len(sys.argv) != 3:
        print('Wrong arguments.')
        exit(1)

    source, destination = sys.argv[1:3]
    if not os.path.isdir(source) or not os.path.isdir(destination):
        print('Source or destination are not valid directories.')
        exit(1)

    list_source = set(os.listdir(source))
    list_destination = set(os.listdir(destination))

def worker(*args, **kwargs):
    try:
        shared_value, source, destination, read_func, transform_func, \
                write_func, files = args
        for file in files:
            data_source = read_func(os.path.join(source, file))
            data_destination = transform_func(data_source)
            write_func(os.path.join(destination, file), data_destination)

            lock.acquire()
            shared_value.value += 1
            lock.release()

    except Exception as e:
        print('Exception in worker:', file=sys.stderr)
        print(e, file=sys.stderr)

def share_lock(l):
    global lock
    lock = l

def progress_bar(n, shared_value):
    start = time.time()

    while not done:
        time.sleep(.5)

        spent = time.time() - start
        i = shared_value.value
        eta = spent * (n / (i + 1) - 1)
        min_s, sec_s = divmod(round(spent), 60)
        min_e, sec_e = divmod(round(eta), 60)

        size = 50
        if n == 0:
            min_e = sec_e = 0
            prop = 1
        else:
            prop = i / n
        count = round(prop * size)
        print(f'Progress: [{'=' * count}{' ' * (size - count)}] ' \
                f'{round(prop * 100)}%, spent: {min_s:02}:{sec_s:02}, ' \
                f'eta: {min_e:02}:{sec_e:02}', end='\r')

    print(f'\n{sys.argv[0]} is done.')

def main(read_func, transform_func, write_func):
    global done
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

        if add or force:
            todo.add(file)

    manager = Manager()
    lock = manager.Lock()
    shared_value = manager.Value('i', 0)

    todo = list(todo)
    n_workers = max(2, min(os.process_cpu_count(), len(todo)))
    if not n_workers:
        print(f'{sys.argv[0]}: nothing to do.')
        return

    print(f'Found {len(todo)} files to convert, splitting in '\
          f'{n_workers} workers.')

    increment = int(len(todo) / (n_workers - 1))
    ranges = [(i * increment, min((i + 1) * increment, len(todo)))
               for i in range(n_workers)]

    done = False
    Thread(target=progress_bar, args=(len(todo), shared_value)).start()
    pool = Pool(n_workers, initializer=share_lock, initargs=(lock,))
    for a, b in ranges:
        pool.apply_async(worker, (shared_value, source, destination, read_func,
                                  transform_func, write_func, todo[a:b]))
    try:
        pool.close()
        pool.join()
    except Exception as e:
        print('Exception while waiting for workers:', file=sys.stderr)
        print(e, file=sys.stderr)

    done = True
