# Starfish: A Topology-Routing Co-Design for Small-Scale Data Centers

This repository contains the source code and instructions for artifact evaluation for our NSDI'26 paper Starfish: A Topology-Routing Co-Design for Small-Scale Data Centers. 

We are submitting this artifact for all three badges: Artifact Available, Artifact Functional, and Results Reproduced.


## Getting Started Instructions

Make the executables:
```bash
cd src/emp/ && make clean && make
cd datacentre/ && make clean && make
```


## Detailed Instructions

### General setup

Make the executables:
```bash
cd src/emp/ && make clean && make
cd datacentre/ && make clean && make
```

Set up files to be used later:
```bash
cd evalnetpathfiles
tar -xvJf netpath_fhi_dring.tar.xz
mv netpath_fhi_dring.txt netpath_dring_80_64_fhi.np

tar -xvJf rawtrafficfiles.tar.xz
```


### Main

#### UNV

Figure 14, Figure 15, `experiments/nsdi26fall/eval_main/unv1/`, `detailed_ae/main/unv/`

1. Generate traffic files:
```bash
mkdir cmfiles
python3 generate_trafficfiles.py
```

2. Untar path weight files: `tar -xvJf pwfilesdir.tar.xz`

3. Run experiments (this takes about 45 minutes to complete on a 100-core machine, can skip):
```bash
mkdir outfiles
python3 pararun.py --conf unv/run.conf --worker 100
```

4. Plot
If skipped the previous step of running experiments: `cp expected_fct_summary.fxt fct_summary.fxt`
Or (this command `parse.py` would take a few minutes to finish):
```bash
python3 parse.py
python3 plot.py
```


prv/

clustera/

clusterb/

clusterc/