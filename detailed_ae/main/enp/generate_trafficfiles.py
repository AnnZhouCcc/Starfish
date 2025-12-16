import random
import numpy as np


def genflowbytes():
    np.random.seed(0)
    
    mean_bytes = 100.0 * 1024
    shape = 1.05
    scale = mean_bytes * (shape - 1)/shape

    x = np.random.exponential(scale=1.0/shape)
    flowbytes = int(scale * np.exp(x))
    return flowbytes

def adjustbytesbymtu(flowbytes):
  mss = 1500
  return mss * ((flowbytes+mss-1)//mss)

large_flow_threshold = 10 * 1024 * 1024


stime = 144 # ms
bw = 1342176000 # B per second
seed_list = [1,2,3,4,5]
nswitches = 80
os = 1
k = 64


#######
# UNV #
#######
unv1file = '../../rawtrafficfiles/prv1'
load_list = range(1,11)

unv1bytes = 0
maxinterval = 0
with open(unv1file, 'r') as f:
    lines = f.readlines()
    for line in lines:
        tokens = line.split(',')
        # 0,32,31,10500
        # interval,fromserver,toserver,bytes
        unv1bytes += int(tokens[3])
        maxinterval = max(maxinterval, int(tokens[0]))
# print(f'unv1bytes {unv1bytes}, maxinterval {maxinterval}, fullload {bw * stime * nlinks / 1000}, ratio {(bw * stime * nlinks / 1000) / unv1bytes}')
print(f'unv1bytes {unv1bytes}, maxinterval {maxinterval}')


# DRing
nlinks = 2132 # 1066*2, uni-directional
nhosts = 2988
topologytype = 2
nintervals = 8

random.seed(0)
for load in load_list:
    totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
    mult = totalbytes / unv1bytes
    actualbytes = 0
    cmfile = f'cmfiles/dring_load{load}.cm'
    with open(cmfile, 'w') as fw:
        with open(unv1file, 'r') as fr:
            lines = fr.readlines()
            iline = 0
            while actualbytes < totalbytes:
                line = lines[iline]
                tokens = line.split(',')
                interval = int(tokens[0])
                fromserver = int(tokens[1])
                toserver = int(tokens[2])
                multbytes = int(tokens[3])

                if fromserver >= nhosts or toserver >= nhosts:
                    iline += 1
                    if iline >= len(lines):
                        iline = 0
                        if mult-1>0:
                            mult = mult-1
                    continue

                if mult >= 1 or (random.random() < mult):
                    multbytes = adjustbytesbymtu(multbytes)
    
                    # generate random start time
                    start_time_ms = random.uniform(0, stime//(maxinterval+1)) + interval * (stime//(maxinterval+1))

                    fw.write(f'{fromserver},{toserver},{int(multbytes)},{start_time_ms:.4f}\n')
                    actualbytes += int(multbytes)

                iline += 1
                if iline >= len(lines):
                    iline = 0
                    if mult-1>0:
                        mult = mult-1

                    # print(f'actualbytes {actualbytes}, totalbytes {totalbytes}, mult {mult}', end='\r')

    print(f'load {load}%, totalbytes {totalbytes}, unv1bytes {unv1bytes}, mult {mult}, actualbytes {actualbytes}')


# LS
nlinks = 2048
nhosts = 3072
topologytype = 1
nintervals = 1

random.seed(0)
for load in load_list:
    totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
    mult = totalbytes / unv1bytes
    actualbytes = 0
    cmfile = f'cmfiles/leafspine_load{load}.cm'
    with open(cmfile, 'w') as fw:
        with open(unv1file, 'r') as fr:
            lines = fr.readlines()
            iline = 0
            while actualbytes < totalbytes:
                line = lines[iline]
                tokens = line.split(',')
                interval = int(tokens[0])
                fromserver = int(tokens[1])
                toserver = int(tokens[2])
                multbytes = int(tokens[3])

                if fromserver >= nhosts or toserver >= nhosts:
                    iline += 1
                    if iline >= len(lines):
                        iline = 0
                        if mult-1>0:
                            mult = mult-1
                    continue

                if mult >= 1 or (random.random() < mult):
                    multbytes = adjustbytesbymtu(multbytes)
    
                    # generate random start time
                    start_time_ms = random.uniform(0, stime//(maxinterval+1)) + interval * (stime//(maxinterval+1))

                    fw.write(f'{fromserver},{toserver},{int(multbytes)},{start_time_ms:.4f}\n')
                    actualbytes += int(multbytes)

                iline += 1
                if iline >= len(lines):
                    iline = 0
                    if mult-1>0:
                        mult = mult-1

                    # print(f'actualbytes {actualbytes}, totalbytes {totalbytes}, mult {mult}', end='\r')

    print(f'load {load}%, totalbytes {totalbytes}, unv1bytes {unv1bytes}, mult {mult}, actualbytes {actualbytes}')


# RRG
nlinks = 2048 # 1024*2, uni-directional
nhosts = 3072
topologytype = 2
nintervals = 8

random.seed(0)
for load in load_list:
    totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
    mult = totalbytes / unv1bytes
    actualbytes = 0
    cmfile = f'cmfiles/rrg_load{load}.cm'
    with open(cmfile, 'w') as fw:
        with open(unv1file, 'r') as fr:
            lines = fr.readlines()
            iline = 0
            while actualbytes < totalbytes:
                line = lines[iline]
                tokens = line.split(',')
                interval = int(tokens[0])
                fromserver = int(tokens[1])
                toserver = int(tokens[2])
                multbytes = int(tokens[3])

                if fromserver >= nhosts or toserver >= nhosts:
                    iline += 1
                    if iline >= len(lines):
                        iline = 0
                        if mult-1>0:
                            mult = mult-1
                    continue

                if mult >= 1 or (random.random() < mult):
                    multbytes = adjustbytesbymtu(multbytes)
    
                    # generate random start time
                    start_time_ms = random.uniform(0, stime//(maxinterval+1)) + interval * (stime//(maxinterval+1))

                    fw.write(f'{fromserver},{toserver},{int(multbytes)},{start_time_ms:.4f}\n')
                    actualbytes += int(multbytes)

                iline += 1
                if iline >= len(lines):
                    iline = 0
                    if mult-1>0:
                        mult = mult-1

                    # print(f'actualbytes {actualbytes}, totalbytes {totalbytes}, mult {mult}', end='\r')

    print(f'load {load}%, totalbytes {totalbytes}, unv1bytes {unv1bytes}, mult {mult}, actualbytes {actualbytes}')


# DF2
p = 40
a = 2
h = 19
g = a*h+1
nlinks = a*(a-1)*g + g*(g-1) # uni-directional
nhosts = a*p*g
topologytype = 3
nintervals = 8

random.seed(0)
for load in load_list:
    totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
    mult = totalbytes / unv1bytes
    actualbytes = 0
    cmfile = f'cmfiles/df2_load{load}.cm'
    with open(cmfile, 'w') as fw:
        with open(unv1file, 'r') as fr:
            lines = fr.readlines()
            iline = 0
            while actualbytes < totalbytes:
                line = lines[iline]
                tokens = line.split(',')
                interval = int(tokens[0])
                fromserver = int(tokens[1])
                toserver = int(tokens[2])
                multbytes = int(tokens[3])

                if fromserver >= nhosts or toserver >= nhosts:
                    iline += 1
                    if iline >= len(lines):
                        iline = 0
                        if mult-1>0:
                            mult = mult-1
                    continue

                if mult >= 1 or (random.random() < mult):
                    multbytes = adjustbytesbymtu(multbytes)
    
                    # generate random start time
                    start_time_ms = random.uniform(0, stime//(maxinterval+1)) + interval * (stime//(maxinterval+1))

                    fw.write(f'{fromserver},{toserver},{int(multbytes)},{start_time_ms:.4f}\n')
                    actualbytes += int(multbytes)

                iline += 1
                if iline >= len(lines):
                    iline = 0
                    if mult-1>0:
                        mult = mult-1

                    # print(f'actualbytes {actualbytes}, totalbytes {totalbytes}, mult {mult}', end='\r')

    print(f'load {load}%, totalbytes {totalbytes}, unv1bytes {unv1bytes}, mult {mult}, actualbytes {actualbytes}')
