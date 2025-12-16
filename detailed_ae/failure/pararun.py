import multiprocessing as mp
import subprocess as sp
import argparse
from tqdm import tqdm
import time
from psutil import virtual_memory

pbar = tqdm(smoothing=0, ncols=80)

def run_thread(conf, safety_margin):
    # Wait until enough memory is available
    while virtual_memory().available <= safety_margin:
        print(f"[{conf.strip()}] Waiting, insufficient memory {virtual_memory().available}")
        time.sleep(10)

    print(f"[{conf.strip()}] Starting with {virtual_memory().available} available memory")
    proc = sp.Popen(conf, shell=True)
    proc.wait()

def pbar_update(*a):
    pbar.update()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--conf')
    parser.add_argument('--worker', type=int, default=(mp.cpu_count()*0.9))
    parser.add_argument('--safety', type=float, default=10e9)  # 10 GB
    args = parser.parse_args()

    with open(args.conf, 'r') as f:
        confs = f.readlines()

    pbar.reset(total=len(confs))

    pool = mp.Pool(processes=args.worker)
    for conf in confs:
        pool.apply_async(run_thread, (conf, args.safety), callback=pbar_update)

    pool.close()
    pool.join()
    pbar.close()
