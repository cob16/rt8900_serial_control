#FT8900 Controller
Provides serial control for the YAESU FT-8900R Transceiver.

##Usage
```
Usage: rt8900c [OPTION...] <serial port path>
Provides serial control for the YAESU FT-8900R Transceiver.

      --hard-emulation       Exactly emulates the radio head head instead of
                             being lazy_sending (worse performance, no observed
                             benefit, only useful for debugging
                             
  -v, --verbose[=LEVEL]      Produce verbose output add a number to select
                             level (1 = ERROR, 2= WARNING, 3=INFO, 4=ERROR,
                             5=DEBUG) output default is 'warning'.
                             
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <cormac.brady@hotmai.co.uk>.
```

##Build and Install
For ubuntu but can be adapted for other distribution's and OS
```bash
sudo apt install libreadline-dev cmake git build-essential 
git clone <this repo url> rt8900c
cd rt8900c
cmake .
make
```

##Run tests
```bash
cmake .
make test

#for verbose output
./test/test_librt8900/test_librt8900
```

##Credits
- CmakeLists build files taken from [this example](https://github.com/kaizouman/gtest-cmake-example)
