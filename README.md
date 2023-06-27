# RDsoft
Softwares for NEWAGE radon detectors. DAQ with ALADM 2000 and anapysis with root.

$ cd ~/
$ git clone https://github.com/kobeDM/RDsoft

##requires boost
$ sudo apt install libboost-dev

## DAQ
### setup
Install adalm drivers following the readme in 
https://github.com/kobeDM/adalm

$ cd RDsoft
$ mkdir build && cd build && cmake ../source && make && sudo make install

### run
```
$ cd ~RD?/data (?=1 or 2 or 3)
$runRD-daq.py & (daq runner, use with adalm, run in BG from remote) 
$RD-daqkiller.py (daq killer) 
```


## analysis
$ cd ~RD?/ana (?=1 or 2 or 3)
$runRD-ana.py (analysis runner) 
