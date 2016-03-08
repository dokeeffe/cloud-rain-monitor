#!/bin/bash
mkdir build
cd build
rm * -rf
cmake -DCMAKE_INSTALL_PREFIX=/usr ../cloud-rain-monitor/
sudo make install
