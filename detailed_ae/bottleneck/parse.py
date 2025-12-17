import os

load_list = range(1,11)
seed_list = [1,2,3,4,5]
oversub_list = [0.33,0.67,1.33,1.67]
scheme_list = ["dringsu2","leafspine"]

sumfile = "fct_summary.txt"
with open(sumfile,'a') as fw:
    for scheme in scheme_list:
        for oversub in oversub_list:
            for load in load_list:
                for seed in seed_list:
                    outfile = f"outfiles/{scheme}_oversub{oversub}_load{load}_seed{seed}.out"
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

                        fw.write(f"{outfile}\t{sum(durationlist)/len(durationlist)}\t{durationlist[int(len(durationlist)*0.5)]}\t{durationlist[int(len(durationlist)*0.99)]}\t{durationlist[int(len(durationlist)*0.9999)]}\n")
