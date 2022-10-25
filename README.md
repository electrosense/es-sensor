
# orfs-sensor 

Sensing software used in [OpenRFSense](https://github.com/openrfsense) nodes, originally forked from [es-sensor](https://github.com/electrosense/es-sensor).

This package will be pulled and built by the OpenRFSense [image builder](https://github.com/openrfsense/image), but build instructions are also included below.

### Dependencies

Install the following packages on Debian 11+:

```shell
$ sudo apt install git cmake librtlsdr-dev librtlsdr0 libliquid2d libliquid-dev liblzma-dev liblzma5 libusb-1.0-0-dev fftw-dev libssl-dev libjson-c5 libjson-c-dev zlib1g-dev zlib1g
```

or, on Arch-based distros:
```shell
$ sudo pacman -S git cmake rtl-sdr liquid-dsp xz libusb libusb-compat fftw openssl json-c zlib
```

More details on Apache Avro are available on [the Avro website](http://avro.apache.org). Releases may be downloaded from your package manager of choice:

```shell
$ sudo apt install libavro-dev libavro23
```

or (an AUR helper is needed):
```shell
$ sudo yay -S avro-c
```

### Compiling

Clone and build the repo (CMake is needed):

```shell
$ git clone https://github.com/openrfsense/orfs-sensor
$ mkdir build && cd build
$ cmake ..
```

To create a `.deb` package use:

```shell
$ cpack .
```

### Running

Store PSD measurements to a CSV file. The output contains time, center frequency and PSD information.

```
./orfs_sensor <sensor_id> 24000000 1700000000 -z PSD -s 2400000 -u /tmp/psd_data.csv
```

---

Record IQ data for 10 seconds to a file. Depending on the output format you like you must specify the samples format 
just right before the file name.

**FLOAT**: The output contains I/Q samples as a sequence of 32-bit float values 
(little endian). IQ is normalized between -1 and 1, therefore it can be fed straight to gqrx or gnuradio as raw file.

```
./orfs_sensor <sensor_id> 102000000 102000000 -z IQ -s 2400000 -t 10 -u FLOAT:/tmp/iq_data.raw
```

**BYTE**: The output contains I/Q samples as a sequence of 8-bit unsigned values 
(little endian). IQ is in the range of [0, 255], therefore it is fully compatible 
with the rtl-sdr suite. This option is quite convenient to use on a RPi0.

```
./orfs_sensor <sensor_id> 102000000 102000000 -z IQ -s 2400000 -t 10 -u BYTE:/tmp/iq_data.raw
```