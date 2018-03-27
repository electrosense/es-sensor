
# es-sensor 2.0


## Requirements

avro-c-1.7.7 

```
$ 
```

## Compile

```
$ git clone $REPO
$ cmake .
$ make 
```

## Run

* Storage the measurements to a file

```
./es_sensor 24000000 1600000000 -u /tmp/measurements.csv
```

* Send the spectrum measurements to the backend 

```
./es_sensor 24000000 1600000000 -n collector.electrosense.org:5000#certs/CA-Cert.pem#certs/Sensor-SSL-Cert.pem#certs/Sensor-SSL-SK.pem
```