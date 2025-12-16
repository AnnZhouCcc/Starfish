import matplotlib.pyplot as plt
import numpy as np

i = "cluster_b"
load_list = range(3,31,3)
seed_list = [1,2,3,4,5]
scheme_list = ["dringsu2","dringsu2e","leafspine","rrgsu2","df2augalg","dringecmp","dringwcmpecmp","dringfatpaths","dringfhi","dringracke1four"]
plottitledict = {"unv1":"UNV","prv1":"ENP","cluster_a":"DB","cluster_b":"WS","cluster_c":"HD"}
plotlabeldict = {"dringsu2":"Starfish","dringsu2e":"Starfish (Oblivious)*","leafspine":"Leaf-Spine*","df2augalg":"Dragonfly","rrgsu2":"Jellyfish","dringecmp":"DRing+SRT","dringwcmpecmp":"DRing+WCMP*","dringfatpaths":"DRing+FatPaths","dringfhi":"DRing+VLB","dringracke1four":"DRing+SMORE"}
plotxlimdict = {"unv1":None,"prv1":9,"cluster_a":25,"cluster_b":35,"cluster_c":40}
plotmarkers = ['o', 's', '^', 'v', 'D', 'P', '*', 'X', 'h', '1']
plotcolordict = {"dringsu2":"#f35b04","dringsu2e":"#f7b801","leafspine":"#07f49e","df2augalg":"#9683ec","rrgsu2":"#5d16a6","dringecmp":"#4cc9f0","dringwcmpecmp":"#4895ef","dringfatpaths":"#4361ee","dringfhi":"#3f37c9","dringracke1four":"#3a0ca3"}
plotxticksdict = {"unv1":[0,2,4,6,8,10],"prv1":[0,2,4,6,8],"cluster_a":[0,5,10,15,20,25],"cluster_b":[0,5,10,15,20,25,30,35],"cluster_c":[0,10,20,30,40]}

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

markerlist = ['o','x']
for isc,scheme in enumerate(scheme_list):
    fct_list = list()
    for load in load_list:
        fct_list_allseeds = list()
        for seed in seed_list:
            outfile = f"{i}/outfiles/{scheme}_load{load}_seed{seed}.out"
            p99 = plotdatadict[i][outfile]
            fct_list_allseeds.append(p99)
        fct_list.append(fct_list_allseeds)
    plt.errorbar(load_list, [sum(x)/len(x) for x in fct_list], yerr=[[sum(x)/len(x)-min(x) for x in fct_list],[max(x)-sum(x)/len(x) for x in fct_list]], alpha=0.9, markersize=10, marker=plotmarkers[isc], color=plotcolordict[scheme], capsize=4, label=plotlabeldict[scheme])

plt.xlabel("Load (%)", fontsize=24)
plt.ylabel("99ile FCT (ms)", fontsize=24)
plt.xlim(0,plotxlimdict[cm])
plt.ylim(0,50)
plt.xticks(plotxticksdict[cm])
plt.tick_params(axis='both', labelsize=20)
plt.title(plottitledict[cm], fontsize=24)
plt.grid(True, linestyle='--', alpha=0.7)
plt.legend(loc='upper center', bbox_to_anchor=(0.5, 1.45), ncol=5, fontsize=22)
plt.savefig(f"fig14_ws.pdf", dpi=600, bbox_inches='tight')