import random
import numpy as np

# from makec2s.ipynb
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


# DRing
# From eval_scale_larger_supernode
stime = 144 # ms
bw = 1342176000 # B per second
seed_list = [1,2,3,4,5]
topologytype = 2
os = 1
nintervals = 8
cm = "prv1"
load = 2
swlist = range(40,121,20)
dringserverlist = [752,1680,2992,4672,6720]

for isw,sw in enumerate(swlist):
  # set parameters
  topologyfile = f"../evalscaletopologyfiles/dring_{sw}_{int(sw*0.8)}.edgelist"
  serverfile = f"../evalserverfiles/dring_{dringserverlist[isw]}_{sw}_{int(sw*0.8)}.sv"
  npfile = f"../evalscalenetpathfiles/netpath_dring_{sw}_{int(sw*0.8)}_su2.np"
  nhosts = dringserverlist[isw]
  nswitches = sw
  k = int(sw*0.8)
  with open(f"{topologyfile}", 'r') as f:
      nlines = sum(1 for _ in f)
  nlinks = nlines * 2
  
  # generate connection_matrices file (1)
  unv1bytes = 0
  unv1file = f'../rawtrafficfiles/{cm}'
  maxinterval = 0
  with open(unv1file, 'r') as f:
      lines = f.readlines()
      for line in lines:
          tokens = line.split(',')
          # 0,32,31,10500
          # interval,fromserver,toserver,bytes
          unv1bytes += int(tokens[3])
          maxinterval = max(maxinterval, int(tokens[0]))
  print(f'unv1bytes {unv1bytes}, maxinterval {maxinterval}, fullload {bw * stime * nlinks / 1000}, ratio {(bw * stime * nlinks / 1000) / unv1bytes}')

  # generate connection_matrices file (2)
  random.seed(0)
  totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
  mult = totalbytes / unv1bytes
  actualbytes = 0
  cmfile = f'cmfiles/dring_sw{sw}_load{load}.cm'
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

# From eval_scale_larger_ring
stime = 144 # ms
bw = 1342176000 # B per second
seed_list = [1,2,3,4,5]
topologytype = 2
os = 1
nintervals = 8
cm = "prv1"
load = 2
swlist = [140,160] # range(120,201,20)
degree = 96
dringswpersn = 10

for isw,sw in enumerate(swlist):
  # set parameters
  topologyfile = f"../evalscaletopologyfiles/dring_{sw}_{degree}.edgelist"
  serverfile = f"../evalserverfiles/dring_{sw*56}_{sw}_{degree}.sv"
  npfile = f"../evalscalenetpathfiles/netpath_dring_{sw}_{degree}_su2.np"
  nhosts = sw*56
  nswitches = sw
  k = degree
  with open(f"{topologyfile}", 'r') as f:
      nlines = sum(1 for _ in f)
  nlinks = nlines * 2
  
  # generate connection_matrices file (1)
  unv1bytes = 0
  unv1file = f'../rawtrafficfiles/{cm}'
  maxinterval = 0
  with open(unv1file, 'r') as f:
      lines = f.readlines()
      for line in lines:
          tokens = line.split(',')
          # 0,32,31,10500
          # interval,fromserver,toserver,bytes
          unv1bytes += int(tokens[3])
          maxinterval = max(maxinterval, int(tokens[0]))
  print(f'unv1bytes {unv1bytes}, maxinterval {maxinterval}, fullload {bw * stime * nlinks / 1000}, ratio {(bw * stime * nlinks / 1000) / unv1bytes}')

  # generate connection_matrices file (2)
  random.seed(0)
  totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
  mult = totalbytes / unv1bytes
  actualbytes = 0
  cmfile = f'cmfiles/dring_sw{sw}_load{load}.cm'
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
# From eval_scale_larger_supernode
stime = 144 # ms
bw = 1342176000 # B per second
seed_list = [1,2,3,4,5]
topologytype = 2
os = 1
nintervals = 8
cm = "prv1"
load = 2
swlist = range(40,121,20)

for isw,sw in enumerate(swlist):
  # set parameters
  topologyfile = f"../evalscaletopologyfiles/rrg_{sw}_{int(sw*0.8)}.edgelist"
  serverfile = f"../evalserverfiles/rrg_{(sw//5)*(sw//5)*12}_{sw}_{int(sw*0.8)}.sv"
  npfile = f"../evalscalenetpathfiles/netpath_rrg_{sw}_{int(sw*0.8)}_su2.np"
  nhosts = (sw//5)*(sw//5)*12
  nswitches = sw
  k = int(sw*0.8)
  with open(f"{topologyfile}", 'r') as f:
      nlines = sum(1 for _ in f)
  nlinks = nlines * 2

  # generate connection_matrices file (1)
  unv1bytes = 0
  unv1file = f'../rawtrafficfiles/{cm}'
  maxinterval = 0
  with open(unv1file, 'r') as f:
      lines = f.readlines()
      for line in lines:
          tokens = line.split(',')
          # 0,32,31,10500
          # interval,fromserver,toserver,bytes
          unv1bytes += int(tokens[3])
          maxinterval = max(maxinterval, int(tokens[0]))
  print(f'unv1bytes {unv1bytes}, maxinterval {maxinterval}, fullload {bw * stime * nlinks / 1000}, ratio {(bw * stime * nlinks / 1000) / unv1bytes}')

  # generate connection_matrices file (2)
  random.seed(0)
  totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
  mult = totalbytes / unv1bytes
  actualbytes = 0
  cmfile = f'cmfiles/rrg_sw{sw}_load{load}.cm'
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

# From eval_scale_larger_ring
stime = 144 # ms
bw = 1342176000 # B per second
seed_list = [1,2,3,4,5]
topologytype = 2
os = 1
nintervals = 8
cm = "prv1"
load = 2
swlist = [140,160]
degree = 96
dringswpersn = 10

for isw,sw in enumerate(swlist):
  # set parameters
  topologyfile = f"../evalscaletopologyfiles/rrg_{sw}_{degree}.edgelist"
  serverfile = f"../evalserverfiles/rrg_{sw*56}_{sw}_{degree}.sv"
  npfile = f"../evalscalenetpathfiles/netpath_rrg_{sw}_{degree}_su2.np"
  nhosts = sw*56
  nswitches = sw
  k = degree
  with open(f"{topologyfile}", 'r') as f:
      nlines = sum(1 for _ in f)
  nlinks = nlines * 2

  # generate connection_matrices file (1)
  unv1bytes = 0
  unv1file = f'../rawtrafficfiles/{cm}'
  maxinterval = 0
  with open(unv1file, 'r') as f:
      lines = f.readlines()
      for line in lines:
          tokens = line.split(',')
          # 0,32,31,10500
          # interval,fromserver,toserver,bytes
          unv1bytes += int(tokens[3])
          maxinterval = max(maxinterval, int(tokens[0]))
  print(f'unv1bytes {unv1bytes}, maxinterval {maxinterval}, fullload {bw * stime * nlinks / 1000}, ratio {(bw * stime * nlinks / 1000) / unv1bytes}')

  # generate connection_matrices file (2)
  random.seed(0)
  totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
  mult = totalbytes / unv1bytes
  actualbytes = 0
  cmfile = f'cmfiles/rrg_sw{sw}_load{load}.cm'
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


# Leafspine
# From eval_scale_larger_supernode
stime = 144 # ms
bw = 1342176000 # B per second
seed_list = [1,2,3,4,5]
topologytype = 1
os = 1
nintervals = 1
cm = "prv1"
load = 2
swlist = range(40,121,20)

for isw,sw in enumerate(swlist):
  # set parameters
  topologyfile = f"../evalscaletopologyfiles/ls_{sw}_{int(sw*0.8)}.edgelist"
  # serverfile = f"evalserverfiles/ls_{(numsw//5)*(numsw//5)*12}_{sw}_{int(sw*0.8)}.sv"
  npfile = f"../evalscalenetpathfiles/netpath_ls_{sw}_{int(sw*0.8)}_ecmp.np"
  nhosts = (sw//5)*(sw//5)*12
  nswitches = sw
  k = int(sw*0.8)
  with open(f"{topologyfile}", 'r') as f:
      nlines = sum(1 for _ in f)
  nlinks = nlines * 2
  
  # generate connection_matrices file (1)
  unv1bytes = 0
  unv1file = f'../rawtrafficfiles/{cm}'
  maxinterval = 0
  with open(unv1file, 'r') as f:
      lines = f.readlines()
      for line in lines:
          tokens = line.split(',')
          # 0,32,31,10500
          # interval,fromserver,toserver,bytes
          unv1bytes += int(tokens[3])
          maxinterval = max(maxinterval, int(tokens[0]))
  print(f'unv1bytes {unv1bytes}, maxinterval {maxinterval}, fullload {bw * stime * nlinks / 1000}, ratio {(bw * stime * nlinks / 1000) / unv1bytes}')

  # generate connection_matrices file (2)
  random.seed(0)
  totalbytes = bw * stime / 1000 * nlinks * load / 100  # B
  mult = totalbytes / unv1bytes
  actualbytes = 0
  cmfile = f'cmfiles/ls_sw{sw}_load{load}.cm'
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

# From eval_scale_larger_ring: N.A.