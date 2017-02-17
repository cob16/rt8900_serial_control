#FT8900 Controller
Provides serial control for the YAESU FT-8900R Transceiver.

##Build and Install

For ubuntu but can be adapted for other distribution's / OS
```bash
sudo apt install cmake git build-essential
git clone <this repo url> rt8900c
cd rt8900c
cmake .
make
```

##Run tests
```bash
sudo apt install check
cmake .
make test
```

##Credits
CmakeLists build files taken from [this example](https://github.com/kaizouman/gtest-cmake-example)