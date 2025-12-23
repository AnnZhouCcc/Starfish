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

We provide instructions for the five major experiments that showcase the key results and major claims of our work. The table below summarizes the five experiments -- which figures they refer to, which directories contain the codes and which subsections describe the experiment details.

| Figure    | Directory   | Subsection                |
| --------- | ----------- | ------------------------- |
| Figure 14 | main/       | [Main](#main)             |
| Figure 15 | cdf/        | [CDF](#cdf)               |
| Figure 16 | failure/    | [Failure](#failure)       |
| Figure 17 | scale/      | [Scale](#scale)           |
| Figure 18 | bottleneck/ | [Bottleneck](#bottleneck) |

Setting up:
1. Make sure that you have completed all steps in [Getting Started Instructions](#getting-started-instructions).
2. Run the following commands to prepare the files that will be used later:
```bash
# Prepare netpath file for DRing FHI
cd detailed_ae/evalnetpathfiles/
tar -xvJf netpath_fhi_dring.tar.xz
mv netpath_fhi_dring.txt netpath_dring_80_64_fhi.np
# Prepare raw traffic files
cd ../
tar -xvJf rawtrafficfiles.tar.xz
```

Quick run: In some cases, one run of the experiment runs for 7+ hours and takes up 10+ GB of memory, and one experiment could contain up to 500 runs with different configurations. A single run of the experiment can only be run on one core and parallism only happens across multiple runs. As such, we will also provide `Quick Run` option for some experiments, where it runs a shorter version of the experiments to make sure experiments are functional and continues the figure plotting with data files provided to make sure results are correct.


### Main

Claim: Starfish delivers lower tail latency at comparable load and higher load at comparable tail latency,  outperforming baselines in the network load v.s. tail latency tradeoff across most traffic traces.

Success metric: We have one subsection for each of the five traffic traces. In each of the subsection, generate `fig14_xx.pdf` and it should match the corresponding subfigure in `expected_fig14.pdf`.


#### UNV

1. Generate traffic files:
```bash
cd detailed_ae/main/unv/
mkdir cmfiles
python3 generate_trafficfiles.py
```
2. Untar path weight files: 
```bash
tar -xvJf pwfilesdir.tar.xz
```
3. Create a directory for output files:
```bash
mkdir outfiles
```
4. Run experiments and parse outputs:

**Full Run**
> **Caution:** It takes 45+ minutes to finish all experiments with 100 cores.
```bash
cd ../
python3 pararun.py --conf unv/run.conf --worker 100
cd unv/
python3 parse.py
```
**Quick Run**
```bash
cd ../
python3 pararun.py --conf unv/quickrun.conf --worker 100
cd unv/
cp expected_fct_summary.fxt fct_summary.fxt
```
5. Plot:
```bash
python3 plot.py
```
You should expect to see `fig14_unv.pdf`, which should match the first subfigure in `../expected_fig14.pdf`.


#### ENP

1. Generate traffic files:
```bash
cd detailed_ae/main/enp/
mkdir cmfiles
python3 generate_trafficfiles.py
```
2. Untar path weight files: 
```bash
tar -xvJf pwfilesdir.tar.xz
```
3. Create a directory for output files:
```bash
mkdir outfiles
```
4. Run experiments and parse outputs:

**Full Run**
> **Caution:** It takes 40+ minutes to finish all experiments with 100 cores.
```bash
cd ../
python3 pararun.py --conf enp/run.conf --worker 100
cd enp/
python3 parse.py
```
**Quick Run**
```bash
cd ../
python3 pararun.py --conf enp/quickrun.conf --worker 100
cd enp/
cp expected_fct_summary.fxt fct_summary.fxt
```
5. Plot
```bash
python3 plot.py
```
You should expect to see `fig14_enp.pdf`, which should match the second subfigure in `../expected_fig14.pdf`.


#### DB

1. Generate traffic files:
```bash
cd detailed_ae/main/db/
mkdir cmfiles
python3 generate_trafficfiles.py
```
2. Untar path weight files: 
```bash
./restore_pwfiles.sh
tar -xvJf pwfilesdir.tar.xz
```
3. Create a directory for output files:
```bash
mkdir outfiles
```
4. Run experiments and parse outputs:

**Full Run**
> **Caution:** It takes 7+ hours and 330+ GB of memory to finish all experiments with 30 cores.
```bash
cd ../
python3 pararun.py --conf db/run.conf --worker 30
cd db/
python3 parse.py
```
**Quick Run**
```bash
cd ../
python3 pararun.py --conf db/quickrun.conf --worker 100
cd db/
cp expected_fct_summary.fxt fct_summary.fxt
```
5. Plot
```bash
python3 plot.py
```
You should expect to see `fig14_db.pdf`, which should match the third subfigure in `../expected_fig14.pdf`.


#### WS

1. Generate traffic files:
```bash
cd detailed_ae/main/ws/
mkdir cmfiles
python3 generate_trafficfiles.py
```
2. Untar path weight files: 
```bash
./restore_pwfiles.sh
tar -xvJf pwfilesdir.tar.xz
```
3. Create a directory for output files:
```bash
mkdir outfiles
```
4. Run experiments and parse outputs:

**Full Run**
> **Caution:** It takes multiple hours and hundreds of GB of memory to finish all experiments. Please watch out when running.
```bash
cd ../
python3 pararun.py --conf ws/run.conf --worker 30
cd ws/
python3 parse.py
```
**Quick Run**
```bash
cd ../
python3 pararun.py --conf ws/quickrun.conf --worker 100
cd ws/
cp expected_fct_summary.fxt fct_summary.fxt
```
5. Plot
```bash
python3 plot.py
```
You should expect to see `fig14_ws.pdf`, which should match the fourth subfigure in `../expected_fig14.pdf`.


#### HD

1. Generate traffic files:
```bash
cd detailed_ae/main/hd/
mkdir cmfiles
python3 generate_trafficfiles.py
```
2. Untar path weight files: 
```bash
./restore_pwfiles.sh
tar -xvJf pwfilesdir.tar.xz
```
3. Create a directory for output files:
```bash
mkdir outfiles
```
4. Run experiments and parse outputs:

**Full Run**
> **Caution:** It takes multiple hours and hundreds of GB of memory to finish all experiments. Please watch out when running.
```bash
cd ../
python3 pararun.py --conf hd/run.conf --worker 30
cd hd/
python3 parse.py
```
**Quick Run**
```bash
cd ../
python3 pararun.py --conf hd/quickrun.conf --worker 100
cd hd/
cp expected_fct_summary.fxt fct_summary.fxt
```
5. Plot
```bash
python3 plot.py
```
You should expect to see `fig14_hd.pdf`, which should match the last subfigure in `../expected_fig14.pdf`.


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
