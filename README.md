
# es-sensor 

Sensing software used in ElectroSense nodes (http://electrosense.org). 

If you are looking for an image ready to run on a RaspberryPi go [here](https://electrosense.org/app.html#!/join)

## Dependencies

* Install the following packages

```
$  sudo apt install git-core cmake librtlsdr-dev librtlsdr0 libliquid1d libliquid-dev liblzma-dev liblzma5 libssl1.0-dev libusb-1.0-0-dev librtlsdr fftw-dev libssl-dev
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

If you want to create debian package just type:

```
$ cpack .
```

## Run

<<<<<<< HEAD
* Write measurements into a file:
=======
* Storage PSD measurements to a file. The output contains time, center frequency and PSD information.

```
./es_sensor 24000000 1700000000 -z PSD -s 2400000 -u /tmp/psd_data.csv
```

* Storage IQ data for 10 seconds to a file. The output contains I/Q samples as a sequence of 32bits float values (little endian).
>>>>>>> devel

```
./es_sensor 806000000 806000000 -z IQ -s 2400000 -t 10 -u /tmp/iq_data.raw
```

