import os


seed_list = range(1,6)
scheme_list = ["dringsu2","rrgsu2"]
sw_list =  [120,140,160]

sumfile = "fct_summary.txt"
with open(sumfile,'a') as fw:
    for scheme in scheme_list:
        for sw in sw_list:
            for seed in seed_list:
                outfile = f"outfiles/{scheme}_sw{sw}_seed{seed}.out"
                # print(outfile)
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


seed_list = range(1,6)
scheme_list = ["dringsu2","rrgsu2","ls"]
sw_list = range(40,101,20)

sumfile = "fct_summary.txt"
with open(sumfile,'a') as fw:
    for scheme in scheme_list:
        for sw in sw_list:
            for seed in seed_list:
                outfile = f"outfiles/{scheme}_sw{sw}_seed{seed}.out"
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