#!/usr/bin/python3
import subprocess
modules=['bin/RD-daq', 'runRD-daq.py', 'runRD-backup.py']
killpids=[]
for i in range(len(modules)):
    print("kill ",modules[i])
    ps="ps -aux | grep -v \' grep \' | grep -v \' emacs \'| grep "+modules[i] 
    process = (subprocess.Popen(ps, stdout=subprocess.PIPE,
                                shell=True).communicate()[0]).decode('utf-8')
    pl=process.split("\n")
    for j in range(len(pl)-1):
        pll=pl[j].split()
        print(pll)
        killpids.append(pll[1])
print(killpids)
for i in range(len(killpids)-1):
    kill="kill -KILL "+killpids[i]
    subprocess.run(kill,shell=True)
    

    
