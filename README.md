es-sensor
==============

# Installation
The following installation instructions are currently only tested for Debian-based Linux operating systems (such as Ubuntu or Raspbian).

## Dependencies

```sh
$ sudo apt-get install libusb-1.0-0-dev librtlsdr fftw-dev libssl-dev
```

## Compilation

```sh
$ cmake ..
$ make
```  

#### Running
Example:
```sh
$ ./run_cpu_sensor 24000000 1766000000
```
