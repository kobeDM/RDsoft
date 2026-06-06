# RDsoft
Softwares for NEWAGE radon detectors. DAQ with ALADM 2000 and analysis with ROOT framework.
Detail info. will be found here: https://ppwww.phys.sci.kobe-u.ac.jp/~newage/wiki/?RD-DAQ

```
$ cd ~/
$ git clone https://github.com/kobeDM/RDsoft
```

## Requires boost
```
$ sudo apt install libboost-dev
```

## DAQ
### Setup
Install adalm drivers following the REAEDME in 
https://github.com/kobeDM/adalm

If you do not want to build `RD-daq` and the `libm2k` dependent parts, disable it at CMake configure time.

```
$ cd RDsoft
$ mkdir build && cd build && cmake ../source -DBUILD_RDDAQ=OFF && make && make install
```

If you want to build `RD-daq` as well, use the default configure command.

```
$ cd RDsoft
$ mkdir build && cd build && cmake ../source && make && make install
```

### Run
```
$ cd ~RD?/data (?=1 or 2 or 3)
$ runRD-daq.py (daq runner, use with adalm) 
$ runRD-daq.py & (daq runner, use with adalm, run in BG from remote) 
$ RD-daqkiller.py (daq killer)

```


## Analysis
Here quick analysis will be performed by: 
```
$ cd ~RD?/data (?=1 or 2 or 3)
$ runRD-ana.py (analysis runner) 
```

If you want to exclude events from analysis by time range, prepare `config/RD-exclude.json` and pass it with `-x`.
Each event timestamp is read from the `#Ev.` header line in each `.dat` file, and you can list multiple excluded periods.

```json
{
	"excluded_periods": [
		{"start": "2026-05-01 12:00:00", "end": "2026-05-01 13:00:00"},
		{"start": "2026-05-02 09:30:00", "end": "2026-05-02 10:15:00"}
	]
}
```

Run analysis with the exclusion config:

```bash
$ runRD-ana.py -x /path/to/RDsoft/config/RD-exclude.json
```

## Analysis including influxdb update
Grafana monitoring will be performed by: 
```
# If you want to output a result plot, use ssh key authentication.
$ cd ~status/radon
$ nohup runRD-mon.py &(monitoring runner including auto analysis)
access http://10.37.0.216:3000/
check the energy spectrum on na16:~/RD?/ana/YYYYmmDD/rnrate.png
```

`runRD-mon.py` also uses the same exclusion config and skips events whose `#Ev.` header timestamp falls inside any excluded period.
