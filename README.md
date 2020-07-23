#### Build instruction.
```
mkdir $HOME/pyxis-gpio
cd $HOME/pyxis-gpio
git clone https://github.com/renetec-io/bcm2835.git
git clone https://github.com/renetec-io/pyxis-gpio.git
cd pyxis-gpio
mkdir build
cd build
cmake ../ -DBCM2835_SOURCE=../../bcm2835/
make
cpack
```
