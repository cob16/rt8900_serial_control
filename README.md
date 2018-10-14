# FT8900 Controller
![Build badge](https://api.travis-ci.org/cob16/rt8900_serial_control.svg?branch=master)

This application and included library provide computer control for the YAESU FT-8900r radio by pretending to be the detached control head of the radio. 
This is done by attaching a serial device to the connecting serial control lines. 

## Features
Currency this application can be used to set and get both VFO's:
- Frequency
- Volume
- Squelch
- Transmission Power
- Push-to-talk
- "Main" status
- Busy status 

## Usage
```
Usage: rt8900c [OPTION...] <serial port path>
Provides serial control for the YAESU FT-8900R Transceiver.

  -d, --dtr-on               Use the DTR pin of the serial connection as a
                             power button for the rig. (REQUIRES compatible
                             hardware)
      --hard-emulation       Exactly emulates the radio head instead of being
                             lazy_sending (worse performance, no observed
                             benefit, only useful for debugging)
  -v, --verbose[=LEVEL]      Produce verbose output add a number to select
                             level (1 = ERROR, 2= WARNING, 3=INFO, 4=ERROR,
                             5=DEBUG) output default is 'warning'.
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <cormac.brady@hotmail.co.uk>.
```
During runtime type help to get a description of available commands 

## Build and Install
For ubuntu but can be adapted for other distribution's and OS
```bash
sudo apt install cmake git build-essential 
git clone <this repo url> rt8900c
cd rt8900c
cmake .
make
```

## Run tests
```bash
cmake .
make
make test
```
Or for verbose output
```bash
./test/test_librt8900/test_librt8900
```

## Doxygen
```bash
sudo apt install doxygen
doxygen Doxyfile
```
Current HTML version is also hosted at [cormacbrady.info/rt8900_docs](https://cormacbrady.info/rt8900_docs/)

##Credits
- CmakeLists build files taken from [this example](https://github.com/kaizouman/gtest-cmake-example)
