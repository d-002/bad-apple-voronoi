import os
import sys
import time
from typing import Any, Function
from threading import Thread
from multiprocessing import Manager, Pool

def checks() -> None:
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

def worker(*args, **kwargs) -> None:
    is_thread = kwargs.get('is_thread', True)
    try:
        shared_value, source, destination, read_func, transform_func, \
                write_func, files = args
        for file in files:
            data_source = read_func(os.path.join(source, file))
            data_destination = transform_func(data_source)
            write_func(os.path.join(destination, file), data_destination)

            if is_thread:
                lock.acquire()
                shared_value.value += 1
                lock.release()

    except Exception as e:
        print('Exception in worker:', file=sys.stderr)
        print(e, file=sys.stderr)

def share_lock(l) -> None:
    global lock
    lock = l

def progress_bar(i: int, n: int) -> None:
    spent = time.time() - start
    eta = spent * (n / (i + 1) - 1)
    hr_s, s = divmod(round(spent), 3600)
    min_s, sec_s = divmod(s, 60)
    hr_e, e = divmod(round(eta), 3600)
    min_e, sec_e = divmod(e, 60)

    size = 50
    if n == 0:
        min_e = sec_e = 0
        prop = 1
    else:
        prop = i / n
    count = round(prop * size)
    print(f'Progress: [{'=' * count}{' ' * (size - count)}] ' \
            f'{round(prop * 100):>3}%, spent: {hr_s}:{min_s:02}:{sec_s:02}, ' \
            f'eta: {hr_e}:{min_e:02}:{sec_e:02}', end='\n')

def progress_bar_loop(n: int, shared_value) -> None:
    global start
    start = time.time()

    while not done:
        # +1 because main thread will have completed one
        progress_bar(shared_value.value + main_thread_is_done, n)
        time.sleep(.5)

    print(f'\n{sys.argv[0]} is done.')

def main(read_func: Function[str, Any], transform_func: Function[Any, Any],
         write_func: Function[[str, Any], None]) -> None:
    global done, main_thread_is_done
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
    n_workers = min(os.process_cpu_count(), len(todo) - 1)
    if n_workers <= 0:
        print(f'{sys.argv[0]}: nothing to do.')
        return

    print(f'Found {len(todo)} files to convert, splitting in '\
          f'{n_workers} workers.')

    increment = len(todo) if n_workers < 2 else len(todo) / (n_workers - 1)
    ranges = [(int(i * increment), min(int((i + 1) * increment), len(todo)))
               for i in range(n_workers)]

    done = False
    pool = Pool(n_workers, initializer=share_lock, initargs=(lock,))
    Thread(target=progress_bar_loop, args=(len(todo), shared_value)).start()

    # run most of the work in asynchronous workers, but keep one in main thread
    for a, b in ranges[1:]:
        pool.apply_async(worker, (shared_value, source, destination, read_func,
                                  transform_func, write_func, todo[a:b]))
    a, b = ranges[0]
    main_thread_is_done = False
    worker(shared_value, source, destination, read_func, transform_func,
           write_func, todo[a:b], is_thread=False)
    main_thread_is_done = True

    try:
        pool.close()
        pool.join()
        print('joined')
        progress_bar(len(todo), len(todo))
    except Exception as e:
        print('Exception while waiting for workers:', file=sys.stderr)
        print(e, file=sys.stderr)

    done = True
