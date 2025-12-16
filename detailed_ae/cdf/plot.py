import matplotlib.pyplot as plt
import numpy as np
import os


# load data
datadict = dict()
for i in ["unv","enp"]:
    for j in ["dringsu2","dringsu2e","leafspine","dringfhi"]:
        outfile = f"{i}/outfiles/{j}_load3_seed1.out"
        if os.path.exists(outfile):
            durationlist = list()
            sizelist = list()
            startlist = list()
            with open(outfile,'r') as fr:
                lines = fr.readlines()
                for line in lines:
                    tokens = line.split()
                    if tokens[0] == "FCT":
                        durationlist.append(float(tokens[2]))
                        sizelist.append(int(tokens[1]))
                        startlist.append(float(tokens[3]))
            durationlist.sort()
            datadict[outfile] = [durationlist,sizelist,startlist]
                        


# plot
plottitledict = {"unv1":"UNV","prv1":"ENP","cluster_a":"DB","cluster_b":"WS","cluster_c":"HD"}
plotlabeldict = {"dringsu2":"Starfish","dringsu2e":"Starfish (Oblivious)","leafspine":"Leaf-Spine","df2augalg":"Dragonfly","rrgsu2":"Jellyfish","dringecmp":"DRing+ECMP","dringwcmpecmp":"DRing+WCMP","dringfatpaths":"DRing+FatPaths","dringfhi":"DRing+VLB","dringracke1four":"DRing+SMORE"}
plotcolordict = {"dringsu2":"#f35b04","dringsu2e":"#f7b801","leafspine":"#07f49e","df2augalg":"#9683ec","rrgsu2":"#5d16a6","dringecmp":"#4cc9f0","dringwcmpecmp":"#4895ef","dringfatpaths":"#4361ee","dringfhi":"#3f37c9","dringracke1four":"#3a0ca3"}


nr = 2
nc = 2
fig, axs =plt.subplots(nr, nc, figsize=(nc*5, nr*3),sharey='row')
for icm,cm in enumerate(["unv1","prv1"]):
    for isc,scheme in enumerate(["dringsu2","dringsu2e","leafspine","dringfhi"]):
        outfile = f"{cm}/outfiles/{scheme}_load3_seed1.out"
        # print(outfile)
        if scheme == "dringsu2e":
            durationlist = dringsu2edatadict[outfile][0] # [durationlist,sizelist,startlist]
        else:
            if cm=="unv1":
                durationlist = unv1datadict[outfile][0] # [durationlist,sizelist,startlist]
            elif cm=="prv1":
                durationlist = prv1datadict[outfile][0]
        sorted_data = np.sort(durationlist)
        cdf = np.arange(1, len(sorted_data)+1) / len(sorted_data)

        if icm==0:
            axs[0,icm].plot(sorted_data, cdf, alpha=0.95, color=plotcolordict[scheme], label=plotlabeldict[scheme])
        else:
            axs[0,icm].plot(sorted_data, cdf, alpha=0.95, color=plotcolordict[scheme])
        axs[1,icm].plot(sorted_data, cdf, alpha=0.95, color=plotcolordict[scheme])

    axs[0,icm].set_xlim(-1,50)
    axs[0,icm].set_title(f"{plottitledict[cm]}",fontsize=24)
    axs[0,icm].set_xticks([0,10,20,30,40,50])
    axs[0,icm].tick_params(axis='both', labelsize=20)
    axs[0,icm].grid(True, linestyle='--', alpha=0.7)

    axs[1,icm].set_xlim(-0.01,0.5)
    axs[1,icm].set_xlabel("FCT (ms)", fontsize=24)
    # axs[1,icm].set_title(f"{cm} (Zoomed-In)",fontsize=24)
    axs[1,icm].set_xticks([0,0.1,0.2,0.3,0.4,0.5])
    axs[1,icm].tick_params(axis='both', labelsize=20)
    axs[1,icm].grid(True, linestyle='--', alpha=0.7)

axs[0,0].set_ylabel("CDF", fontsize=24)
axs[1,0].set_ylabel("CDF", fontsize=24)
    
fig.legend(loc='upper center', bbox_to_anchor=(0.5, 1.15), ncol=2, fontsize=22)
plt.savefig(f"eval_main_allp.pdf", dpi=600, bbox_inches='tight')