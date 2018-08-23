
# es-sensor 

Sensing software used in ElectroSense nodes (http://electrosense.org). 

If you are looking for an image ready to run on a RaspberryPi go [here](https://electrosense.org/app.html#!/join)

## Dependencies

* Install the following packages

```
$  sudo apt install git-core cmake librtlsdr-dev librtlsdr0 \
    libliquid1d libliquid-dev liblzma-dev liblzma5 libssl1.0-dev \ 
    libusb-1.0-0-dev librtlsdr fftw-dev libssl-dev
```

* More details to Apache Avro are available at http://avro.apache.org. Releases may be downloaded from Apache mirror at http://www.apache.org/dyn/closer.cgi/avro/.

```
$ wget http://mirror.switch.ch/mirror/apache/dist/avro/avro-1.7.7/c/avro-c-1.7.7.tar.gz
$ tar -zxf avro-c-1.7.7.tar.gz && rm avro-c-1.7.7.tar.gz
$ cd avro-c-1.7.7/
$ mkdir build && cd build/
$ cmake ../ -DCMAKE_INSTALL_PREFIX=$PREFIX \ -DCMAKE_BUILD_TYPE=RelWithDebInfo
$ make
$ sudo make install
```

## Compile

```
$ git clone https://github.com/electrosense/es-sensor 
$ cmake .
$ make 
```

## Run

* Write measurements into a file:

```
./es_sensor 24000000 1700000000 -u /tmp/measurements.csv
```

