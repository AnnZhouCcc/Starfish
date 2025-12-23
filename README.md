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
python3 parse.py # Note that this command may take a few minutes to run.
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
python3 parse.py # Note that this command may take a few minutes to run
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
python3 parse.py # Note that this command may take a few minutes to run
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
python3 parse.py # Note that this command may take a few minutes to run
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
python3 parse.py # Note that this command may take a few minutes to run
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

Claim: Starfish performs well across all percentiles. Note that the current caption of Figure 15 on paper is outdated; we will fix it in the revised paper.

Success metric: Generate `fig15.pdf` and it should match `expected_fig15.pdf`.

In this subsection, we do not generate new output files. If you have done Full Run in [Main](#main) UNV & ENP, plot with the following command:
```bash
python3 plot.py
```
Else if you have used Quick Run in [Main](#main) UNV & ENP, proceed with the following commands:
```bash
tar -xvJf outfilesdir.tar.xz # Untar the output files
python3 plot2.py
```
You should expect to see `fig15.pdf`, which should match `expected_fig15.pdf`.


### Failure

Claim: Starfish is resilient to random link failures, outperforming leaf-spine and approaching failure-aware performance.

Success metric: We have one subsection for each of the two traffic traces. In each of the subsection, generate `fig16_xx.pdf` and it should match the corresponding subfigure in `expected_fig16.pdf`.


#### UNV

1. Untar path weight files:
```bash
cd detailed_ae/failure/unv/
tar -xvJf pwfilesdir.tar.xz
```
2. Run experiments:
```bash
mkdir outfiles
cd ../
python3 pararun.py --conf unv/run.conf --worker 50 # It takes a few minutes to run this command.
```
3. Parse output files & plot:
```bash
python3 parse.py
python3 plot.py
```
You should expect to see `fig16_unv.pdf`, which should match the first subfigure in `../expected_fig16.pdf`.


#### ENP

1. Untar path weight files:
```bash
cd detailed_ae/failure/enp/
tar -xvJf pwfilesdir.tar.xz
```
2. Run experiments:
```bash
mkdir outfiles
cd ../
python3 pararun.py --conf enp/run.conf --worker 50 # It takes a few minutes to run this command. The occasional assertion failed is fine.
```
3. Parse output files & plot:
```bash
python3 parse.py
python3 plot.py
```
You should expect to see `fig16_enp.pdf`, which should match the second subfigure in `../expected_fig16.pdf`.


### Scale

Claim: Starfish performs well at small scale, when we keep the ring size constant when adding switches (Figure 17a). However, Starfish’s performance deteriorates at larger scale when the ring size has to increase to accommodate even more switches (Figure 17b).

Success metric: Generate `fig17a.pdf` and `fig17b.pdf`, and they should match `expected_fig17a.pdf` and `expected_fig17b.pdf` respectively.

1. Prepare netpath files:
```bash
cd detailed_ae/evalscalenetpathfiles/
tar -xvJf twolargedringnetpath.tar.xz
```
2. Generate traffic files:
```bash
cd ../scale/
mkdir cmfiles
python3 generate_trafficfiles.py
```
3. Prepare path weight files: 
```bash
tar -xvJf pwfilesdir.tar.xz
```
4. Run experiments:
```bash
mkdir outfiles
python3 pararun.py --conf run.conf --worker 100 # It takes about 10 minutes to run this commnd.
```
5. Parse output files & plot:
```bash
python3 parse.py
python3 plot.py
```
You should expect to see `fig17a.pdf` and `fig17b.pdf`, which should match `expected_fig17a.pdf` and `expected_fig17b.pdf` respectively.


### Bottleneck

Claim: Starfish’s performance gains are more pronounced with higher rack oversubscription and hence higher rack congestion.

Success metric: Generate `fig18.pdf` and it should match `expected_fig18.pdf`.

1. Run experiments and parse outputs:

**Full Run**
> **Caution:** It takes 35+ minutes to finish all experiments with 100 cores.
```bash
cd detailed_ae/bottleneck/
mkdir outfiles
python3 pararun.py --conf run.conf --worker 100
python3 parse.py
```
**Quick Run**
```bash
cd detailed_ae/bottleneck/
mkdir outfiles
python3 pararun.py --conf quickrun.conf --worker 100
cp expected_fct_summary.txt fct_summary.txt
```
2. Plot:
```bash
python3 plot.py
```
You should expect to see `fig18.pdf`, which should match `expected_fig18.pdf`.
