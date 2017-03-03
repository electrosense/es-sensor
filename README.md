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

    
#### Running
Example:
```sh
$ ./run_cpu_sensor 24000000 1766000000
```