import os

fname = "enp"
prevfname = "prv1"
nfaillinks_list_dict = dict()
nfaillinks_list_dict["leafspine_before"] = [40,81,122,163,204]
nfaillinks_list_dict["dringsu2_before"] = [42,85,127,170,213]
nfaillinks_list_dict["dringsu2_after"] = [42,85,127,170,213]
fseed_list = range(10)
scheme_list = ["leafspine_before","dringsu2_before","dringsu2_after"]
failpct_list = [0,2,4,6,8,10]
load = 2
seed = 1

sumfile = "fct_summary.txt"
with open(sumfile,'a') as fw:
    for scheme in scheme_list:
        for nfaillinks in nfaillinks_list_dict[scheme]:
            for fseed in fseed_list:
                outfile = f"outfiles/{scheme}_nfaillinks{nfaillinks}_fseed{fseed}.out"
                prevoutfile = f"{prevfname}/outfiles/{scheme}_nfaillinks{nfaillinks}_fseed{fseed}.out"
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
                    
                    fw.write(f"{prevoutfile}\t{sum(durationlist)/len(durationlist)}\t{durationlist[int(len(durationlist)*0.5)]}\t{durationlist[int(len(durationlist)*0.99)]}\t{durationlist[int(len(durationlist)*0.9999)]}\n")