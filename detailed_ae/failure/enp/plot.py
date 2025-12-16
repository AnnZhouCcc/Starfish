# plot from txt instead of pickle -- pickle files are too large
import matplotlib.pyplot as plt
import numpy as np


fname = "enp"
prevfname = "prv1"
nfaillinks_list_dict = dict()
nfaillinks_list_dict["leafspine_before"] = [40,81,122,163,204]
nfaillinks_list_dict["dringsu2_before"] = [42,85,127,170,213]
nfaillinks_list_dict["dringsu2_after"] = [42,85,127,170,213]
fseed_list = range(10)
scheme_list = ["dringsu2_before","leafspine_before","dringsu2_after"]
failpct_list = [0,2,4,6,8,10]
load = 2
seed = 1

plottitledict = {"unv1":"UNV","prv1":"ENP","cluster_a":"DB","cluster_b":"WS","cluster_c":"HD"}
plotlabeldict = {"dringsu2_after":"Starfish (Failure-Aware)","dringsu2_before":"Starfish","leafspine_before":"Leaf-Spine"}
plotmarkers = ['o', 's', '^', 'v', 'D', 'P', '*', 'X', 'h', '1']
plotcolordict = {"dringsu2_after":"#f35b04","leafspine_before":"#38a3a5","dringsu2_before":"#f7b801"}


plotdatadict = dict()
sumfile = "fct_summary.txt"
with open(sumfile,'r') as f:
    # cluster_a/outfiles/rrgsu2_load6_seed2.out	0.05765561435997601	0.0487378	0.169153	0.746218
    lines = f.readlines()
    for line in lines:
        tokens = line.split()
        outfile = tokens[0]
        outfiletokens = outfile.split('/')
        cm = outfiletokens[0]
        avg = float(tokens[1])
        p50 = float(tokens[2])
        p99 = float(tokens[3])
        p9999 = float(tokens[4])

        if cm not in plotdatadict:
            plotdatadict[cm] = dict()
        plotdatadict[cm][outfile] = p99

plotevalmaindatadict = dict()
sumfile = "../../main/enp/fct_summary.txt"
with open(sumfile,'r') as f:
    # cluster_a/outfiles/rrgsu2_load6_seed2.out	0.05765561435997601	0.0487378	0.169153	0.746218
    lines = f.readlines()
    for line in lines:
        tokens = line.split()
        outfile = tokens[0]
        outfiletokens = outfile.split('/')
        cm = outfiletokens[0]
        avg = float(tokens[1])
        p50 = float(tokens[2])
        p99 = float(tokens[3])
        p9999 = float(tokens[4])

        if cm not in plotevalmaindatadict:
            plotevalmaindatadict[cm] = dict()
        plotevalmaindatadict[cm][outfile] = p99


for isc,scheme in enumerate(scheme_list):
    fct_list = list()
    fct_list_allseeds = list()
    if "dring" in scheme:
        myscheme = "dringsu2"
    else:
        myscheme = "leafspine"
    outfile = f"{prevfname}/outfiles/{myscheme}_load{load}_seed{seed}.out"
    p99 = plotevalmaindatadict[prevfname][outfile]
    fct_list_allseeds.append(p99)
    fct_list.append(fct_list_allseeds)

    for nfaillinks in nfaillinks_list_dict[scheme]:
        fct_list_allseeds = list()
        for fseed in fseed_list:
            outfile = f"{prevfname}/outfiles/{scheme}_nfaillinks{nfaillinks}_fseed{fseed}.out"
            p99 = plotdatadict[prevfname][outfile]
            fct_list_allseeds.append(p99)
        fct_list.append(fct_list_allseeds)
    
    plt.errorbar(failpct_list, [sum(x)/len(x) for x in fct_list], yerr=[[sum(x)/len(x)-min(x) for x in fct_list],[max(x)-sum(x)/len(x) for x in fct_list]], marker=plotmarkers[isc], label=plotlabeldict[scheme],alpha=0.9, capsize=4, color=plotcolordict[scheme])

plt.xlabel("% of Links Failed", fontsize=24)
plt.title(plottitledict[cm], fontsize=24)
plt.tick_params(axis='both', labelsize=20)
plt.grid(True)
plt.xticks([0, 2, 4, 6, 8, 10])
# plt.ylabel("99ile FCT (ms)", fontsize=24)
# plt.yticks(range(7))
plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.45), ncol=2, fontsize=22)
plt.savefig(f"fig15_enp.pdf", dpi=600, bbox_inches='tight')