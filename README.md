# RDsoft
Softwares for NEWAGE radon detectors. DAQ with ALADM 2000 and analysis with ROOT framework.
Detail info. will be found here: https://ppwww.phys.sci.kobe-u.ac.jp/~newage/wiki/?RD-DAQ


$ cd ~/
$ git clone https://github.com/kobeDM/RDsoft


## DAQ
### setup
Install adalm drivers following the REAEDME in 
https://github.com/kobeDM/adalm

```
$ cd RDsoft
$ mkdir build && cd build && cmake ../source && make && make install
```

### run
```
$ cd ~RD?/data (?=1 or 2 or 3)
$ runRD-daq.py (daq runner, use with adalm) 
$ runRD-daq.py & (daq runner, use with adalm, run in BG from remote) 
$ RD-daqkiller.py (daq killer) 
```


## analysis
Here quick analysis will be performed by: 
```
$ cd ~RD?/data (?=1 or 2 or 3)
$ runRD-ana.py (analysis runner) 
```
## analysis including influxdb update
Grafana monitoring will be performed by: 
```
$ cd ~status/radon
$ nohup runRD-mon.py &(monitoring runner including auto analysis)
access http://10.37.0.216:3000/
```
