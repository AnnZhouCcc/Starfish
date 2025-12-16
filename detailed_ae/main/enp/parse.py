import os

seed_list = [1,2,3,4,5]
scheme_list = ["dringsu2","dringsu2e","leafspine","rrgsu2","df2augalg","dringecmp","dringwcmpecmp","dringfatpaths","dringfhi","dringracke1four"]
load_list = range(1,11)
sumfile = "fct_summary.txt"
with open(sumfile,'a') as fw:
    for j in scheme_list:
        if os.path.exists(f"outfiles/{j}_load{load_list[0]}_seed{seed_list[0]}.out"):
            for load in load_list:
                for seed in seed_list:
                    outfile = f"outfiles/{j}_load{load}_seed{seed}.out"
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
                    # print(outfile)
                    fw.write(f"prv1/{outfile}\t{sum(durationlist)/len(durationlist)}\t{durationlist[int(len(durationlist)*0.5)]}\t{durationlist[int(len(durationlist)*0.99)]}\t{durationlist[int(len(durationlist)*0.9999)]}\n")