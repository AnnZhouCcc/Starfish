# Starfish: A Topology-Routing Co-Design for Small-Scale Data Centers

This repository contains the source code and instructions for artifact evaluation for our NSDI'26 paper Starfish: A Topology-Routing Co-Design for Small-Scale Data Centers. The following codes are run with `g++ 11.4.0` and `Python 3.10.12`.

We are submitting this artifact for all three badges: Artifact Available, Artifact Functional, and Results Reproduced.


## Getting Started Instructions

1. Run the following command to install dependencies:
```bash
pip3 install -r requirements.txt
```
2. Make sure that line 4 `BASEDIR=/path/to/your/dir/Starfish/src/emp` in `Starfish/src/emp/datacentre/Makefile` is set properly to the actual path. Be careful that we are changing the `Makefile` under `src/emp/datacentre/`, but _not_ the one under `src/emp/`.
3. Run the following commands to make the executable:
```bash
cd src/emp/ && make clean && make
cd datacentre/
make clean
make
```
4. Run the following command to run a test experiment:
```bash
cd ../../../getting_started/
python3 pararun.py --conf run.conf --worker 1
```
5. Expect the experiment to finish quickly and also expect an `outfile.out`. The content should match the content in `expected_outfile.out`.


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
cd detailed_ae/main/unv/
mkdir cmfiles
python3 generate_trafficfiles.py
```

2. Untar path weight files: `tar -xvJf pwfilesdir.tar.xz`

3. Run experiments (this takes about 45 minutes to complete on a 100-core machine, can skip; best if run, cdf uses raw data):
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


#### ENP

Figure 14, Figure 15, `experiments/nsdi26fall/eval_main/prv1/`, `detailed_ae/main/enp/`

1. Generate traffic files:
```bash
cd detailed_ae/main/enp/
mkdir cmfiles
python3 generate_trafficfiles.py
```

2. Untar path weight files: `tar -xvJf pwfilesdir.tar.xz`

3. Run experiments (this takes about 40 minutes to complete on a 100-core machine, can skip; best if run, cdf uses raw data):
```bash
mkdir outfiles
python3 pararun.py --conf enp/run.conf --worker 100
```

4. Plot
If skipped the previous step of running experiments: `cp expected_fct_summary.fxt fct_summary.fxt`
Or (this command `parse.py` would take a few minutes to finish):
```bash
python3 parse.py
python3 plot.py
```


#### DB

Figure 14, Figure 15, `experiments/nsdi26fall/eval_main/cluster_a/`, `detailed_ae/main/db/`

1. Generate traffic files:
```bash
cd detailed_ae/main/db/
mkdir cmfiles
python3 generate_trafficfiles.py
```

2. Untar path weight files: `./restore_pwfiles.sh`

3. Run experiments (7G-338G 9:57am-4:46pm please be cautious about running this experiment with many cores. memories may explode. this takes about 40 minutes to complete on a 100-core machine, can skip):
```bash
mkdir outfiles
python3 pararun.py --conf db/run.conf --worker 30
```

4. Plot
If skipped the previous step of running experiments: `cp expected_fct_summary.fxt fct_summary.fxt`
Or (this command `parse.py` would take a few minutes to finish):
```bash
python3 parse.py
python3 plot.py
```


#### WS

Figure 14, Figure 15, `experiments/nsdi26fall/eval_main/cluster_b/`, `detailed_ae/main/ws/`

1. Generate traffic files:
```bash
cd detailed_ae/main/ws/
mkdir cmfiles
python3 generate_trafficfiles.py
```

2. Untar path weight files: `./restore_pwfiles.sh`

3. Run experiments (?G- ?am- please be cautious about running this experiment with many cores. memories may explode. this takes about 40 minutes to complete on a 100-core machine, can skip):
```bash
mkdir outfiles
python3 pararun.py --conf ws/run.conf --worker 30
```

4. Plot
If skipped the previous step of running experiments: `cp expected_fct_summary.fxt fct_summary.fxt`
Or (this command `parse.py` would take a few minutes to finish):
```bash
python3 parse.py
python3 plot.py
```


#### HD

Figure 14, Figure 15, `experiments/nsdi26fall/eval_main/cluster_c/`, `detailed_ae/main/hd/`

1. Generate traffic files:
```bash
cd detailed_ae/main/hd/
mkdir cmfiles
python3 generate_trafficfiles.py
```

2. Untar path weight files: `./restore_pwfiles.sh`

3. Run experiments (?G- ?am- please be cautious about running this experiment with many cores. memories may explode. this takes about 40 minutes to complete on a 100-core machine, can skip):
```bash
mkdir outfiles
python3 pararun.py --conf hd/run.conf --worker 30
```

4. Plot
If skipped the previous step of running experiments: `cp expected_fct_summary.fxt fct_summary.fxt`
Or (this command `parse.py` would take a few minutes to finish):
```bash
python3 parse.py
python3 plot.py
```


### CDF

We make use of the data generated in the Main section. do not generate new data here.
`python3 plot.py`
or if not run before:
```bash
tar -xvJf outfilesdir.tar.xz
python3 plot2.py
```


### Failure

#### UNV

Figure 16, `experiments/nsdi26fall/eval_failure_link/unv1/`, `detailed_ae/failure/unv/`

1. Do not generate new traffic files; reuse the ones in Main
2. Untar path weight files:
```bash
cd detailed_ae/failure/unv/
tar -xvJf pwfilesdir.tar.xz
```
3. Run experiments
dringsu2, dringsu2after, leafspine
3:55pm-4:00pm
```bash
mkdir outfiles
python3 pararun.py --conf unv/run.conf --worker 50
```
4. Parse & plot
either `cp expected_fct_summary.txt fct_summary.txt`
or
```bash
python3 parse.py
python3 plot.py
```


#### ENP

Figure 16, `experiments/nsdi26fall/eval_failure_link/prv1/`, `detailed_ae/failure/enp/`

1. Do not generate new traffic files; reuse the ones in Main
2. Untar path weight files:
```bash
cd detailed_ae/failure/enp/
tar -xvJf pwfilesdir.tar.xz
```
3. Run experiments
dringsu2, dringsu2after, leafspine
4:58pm-5:02pm
The occasional assertion failed should be fine; they are associated with the loggers
```bash
mkdir outfiles
python3 pararun.py --conf enp/run.conf --worker 100
```
4. Parse & plot
either `cp expected_fct_summary.txt fct_summary.txt`
or
```bash
python3 parse.py
python3 plot.py
```


### Scale

Figure 17a, `experiments/nsdi26fall/eval_scale_larger_supernode/`, `detailed_ae/scale/`, constant ring size
Figure 17b, `experiments/nsdi26fall/eval_scale_larger_ring/`, `detailed_ae/scale/`, constant supernode size

1. Generate traffic files (needed here because topology size is changing)
```bash
cd detailed_ae/evalscalenetpathfiles/
tar -xvJf twolargedringnetpath.tar.xz
cd detailed_ae/scale/
mkdir cmfiles
python3 generate_trafficfiles.py
```
2. Untar path weight files: `tar -xvJf pwfilesdir.tar.xz`
3. Run experiments
8 minutes
```bash
mkdir outfiles
python3 pararun.py --conf run.conf --worker 100
```
4. Parse & plot
`cp expected_fct_summary.txt fct_summary.txt`
or
`python3 parse.py`
`python3 plot.py`


### Bottleneck

Figure 18, `experiments/nsdi26fall/eval_bottleneck_oversub/`, `detailed_ae/bottleneck/`

1. Generate traffic files: we do not generate new traffic files

2. Untar path weight files: we do not untar new path weight files

3. Run experiments (~31 minutes)
```bash
mkdir outfiles
python3 pararun.py --conf run.conf --worker 100
```
4. Parse & plot
`cp expected_fct_summary.txt fct_summary.txt`
or
`python3 parse.py`
`python3 plot.py`
