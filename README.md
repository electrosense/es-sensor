RTL-Spec-Kafka
==============

# Installation
The following installation instructions are currently only tested for Debian-based Linux operating systems (such as Ubuntu or Raspbian).

## Dependencies
### libusb (Required)
libusb is a C library that gives applications easy access to USB devices on many different operating systems. RTL-Spec uses the library to interface the RTL-SDR USB dongle. More details to libusb are available at http://www.libusb.org.
```sh
$ sudo apt-get install libusb-1.0-0-dev
```

### librtlsdr (Required)
librtlsdr is a C library that turns RTL2832 based DVB-T dongles into SDR receivers. More details to librtlsdr are available at http://sdr.osmocom.org.
```sh
$ git clone git://git.osmocom.org/rtl-sdr.git
$ cd rtl-sdr/
$ mkdir build && cd build/
$ cmake ../ -DINSTALL_UDEV_RULES=ON
$ make
$ sudo su
$ make install
$ ldconfig
$ cat > /etc/modprobe.d/rtl-blacklist.conf << EOL
$ blacklist dvb_usb_rtl28xxu
$ blacklist rtl2832
$ blacklist rtl2830
$ EOL
$ rmmod dvb_usb_rtl28xxu rtl2832
$ exit
$ cd ../../
```

### fftw (Required)
fftw is a C library for computing the discrete Fourier transform (DFT). More details to fftw are available at http://www.fftw.org.
```sh
$ sudo apt-get install fftw-dev
```

### libssl-dev (Required)
```sh
$ sudo apt-get install libssl-dev
```

### Apache Avro (Optional)
More details to Apache Avro are available at http://avro.apache.org. Releases may be downloaded from Apache mirror at http://www.apache.org/dyn/closer.cgi/avro/.
```sh
$ wget http://mirror.switch.ch/mirror/apache/dist/avro/avro-1.7.7/c/avro-c-1.7.7.tar.gz
$ tar -zxf avro-c-1.7.7.tar.gz && rm avro-c-1.7.7.tar.gz
$ cd avro-c-1.7.7/
$ mkdir build && cd build/
$ cmake ../ \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo
$ make
$ make test
$ sudo make install
$ sudo ldconfig
$ cd ../src/
$ sudo cp -r avro/ /usr/local/include/
$ sudo cp avro.h /usr/local/include/
$ cd ../../
```

### Apache Kafka (Optional)
If available on your distribution (e.g. on Ubuntu), you can install Apache Kafka as follows:
```sh
sudo apt-get install librdkafka-dev
```
Otherwise (e.g. on Raspbian) follow the subsequent installation instructions.

More details to the Apache Kafka C/C++ client library are available at https://github.com/edenhill/librdkafka.
#### Updating gcc/g++
Installing librdkafka on certain architectures such as ARMv6 requires gcc/g++ version >= 4.7.2. The following instructions provide a guideline to upgrade gcc/g++ from 4.6 to 4.8.

Install gcc/g++-4.8, remove gcc/g++ alternative configurations (if any), and install alternatives:
```sh
$ sudo apt-get install gcc-4.8 g++-4.8
$ sudo update-alternatives --remove-all gcc
$ sudo update-alternatives --remove-all g++
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 20
$ sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 50
$ sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.6 20
$ sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 50
```
After setup, alternatives can be selected by:
```sh
$ sudo update-alternatives --config gcc
$ sudo update-alternatives --config g++
```
Verification that gcc/g++-4.8 is active can be done by:
```sh
$ gcc --version
$ g++ --version
```

#### Cloning
```sh
$ git clone https://github.com/edenhill/librdkafka.git
```
#### Building
```sh
$ cd librdkafka/
$ ./configure
$ make
$ sudo make install
$ sudo ldconfig
$ cd ../
```

## RTL-Spec-Kafka
#### Cloning
```sh
$ git clone https://github.com/pdamian/rtl-spec-kafka.git
```
#### Building
```sh
$ cd rtl-spec-kafka/
$ make collector CFLAGS="-O2 -DVERBOSE"
$ make sensor_cpu CFLAGS="-O2 -DVERBOSE"
```

```sh
    make <TARGET> [CFLAGS="<CFLAGS>"]
    <TARGET> = sensor_cpu | sensor_cpu_kafka | sensor_gpu | sensor_gpu_kafka | collector
    <CFLAGS> = [-O2] [-ggdb] [-DVERBOSE] [...]
```
    
#### Running
Example:
```sh
$ ./run_collector 5000
$ ./run_cpu_sensor 24000000 1766000000
```
Run with option *-h* for more help.
