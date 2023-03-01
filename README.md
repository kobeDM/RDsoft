# RDsoft
Softwares for NEWAGE radon detectors. DAQ with ALADM 2000 and anapysis with root.

$ cd ~/
$ git clone https://github.com/kobeDM/RDsoft


## DAQ
### setup
Install adalm drivers following the readme in 
https://github.com/kobeDM/adalm

$ cd RDsoft
$ mkdir build && cd build && cmake ../source && make && sudo make install

### run
```
$ cd ~RD?/data (?=1 or 2 or 3)
$runRD-daq.py (daq runner, use with adalm) 
```


## analysis

