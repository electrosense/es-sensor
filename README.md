
# es-sensor 

Sensing software used in ElectroSense nodes (http://electrosense.org). 

If you are looking for an image ready to run on a RaspberryPi go [here](https://electrosense.org/app.html#!/join)

## Dependencies

* Install the following packages

```
$  sudo apt install git-core cmake librtlsdr-dev librtlsdr0 libliquid1d libliquid-dev liblzma-dev liblzma5 libusb-1.0-0-dev fftw-dev libssl-dev libjson-c3 libjson-c-dev zlib1g-dev zlib1g
```

* More details to Apache Avro are available at http://avro.apache.org. Releases may be downloaded from Apache mirror at http://www.apache.org/dyn/closer.cgi/avro/.

```
$ wget https://archive.apache.org/dist/avro/avro-1.7.7/c/avro-c-1.7.7.tar.gz
$ tar -zxf avro-c-1.7.7.tar.gz && rm avro-c-1.7.7.tar.gz
$ cd avro-c-1.7.7/
$ mkdir build && cd build/
$ cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
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

* Storage PSD measurements to a file. The output contains time, center frequency and PSD information.

```
./es_sensor 24000000 1700000000 -z PSD -s 2400000 -u /tmp/psd_data.csv
```

* Storage IQ data for 10 seconds to a file. Depending on the output format you like you must specify the samples format 
just right before the file name.

**FLOAT**: The output contains I/Q samples as a sequence of 32bits float values 
(little endian). IQ is normalized between -1 and 1, therefore it is compatible to use it as input for gqrx or gnuradio.

```
./es_sensor 102000000 102000000 -z IQ -s 2400000 -t 10 -u FLOAT:/tmp/iq_data.raw
```

**BYTE**: The output contains I/Q samples as a sequence of 8bits unsigned values 
(little endian). IQ is in the range of [0, 255], therefore it is full compatible 
with rtl-sdr suite. This option is quite convenient to use in RPi0. 

```
./es_sensor 102000000 102000000 -z IQ -s 2400000 -t 10 -u BYTE:/tmp/iq_data.raw
```