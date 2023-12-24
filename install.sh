#!/bin/bash
apt-get update
apt-get install -y -q git cmake g++ libboost-all-dev libgmp-dev swig python3-numpy python3-mako python3-sphinx python3-lxml doxygen libfftw3-dev libsdl1.2-dev libgsl-dev libqwt-qt5-dev libqt5opengl5-dev python3-pyqt5 liblog4cpp5-dev libzmq3-dev python3-yaml python3-click python3-click-plugins python3-zmq python3-scipy python3-gi python3-gi-cairo gir1.2-gtk-3.0 libcodec2-dev libgsm1-dev libusb-1.0-0 libusb-1.0-0-dev libudev-dev
apt-get install -y -q pybind11-dev python3-matplotlib libsndfile1-dev python3-pip libsoapysdr-dev soapysdr-tools
pip install pygccxml
pip install pyqtgraph
pip install pygccxml
apt-get install -y -q libiio-dev libad9361-dev libspdlog-dev python3-packaging python3-jsonschema
apt remove swig
apt-get install -y -q autoconf automake build-essential ccache cmake cpufrequtils doxygen ethtool g++ git inetutils-tools libboost-all-dev libncurses5 libncurses5-dev libusb-1.0-0 libusb-1.0-0-dev libusb-dev python3-dev python3-mako python3-numpy python3-requests python3-scipy python3-setuptools python3-ruamel.yaml 
git clone https://github.com/UA1ZDC/UHD_Owl.git -b new-imager
cd UHD_Owl/host
mkdir build && cd build
cmake ../
make -j`nproc`
make install
cd $HOME
git clone --recursive https://github.com/gnuradio/volk.git
cd volk/
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DPYTHON_EXECUTABLE=/usr/bin/python3 ../
make -j`nproc`
make install
ldconfig
cd $HOME
git clone https://github.com/gnuradio/gnuradio.git
cd gnuradio
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DPYTHON_EXECUTABLE=/usr/bin/python3 ../
make -j`nproc`
make install
ldconfig
gnuradio-companion
apt-get install cmake xorg-dev libglu1-mesa-dev
git clone https://github.com/glfw/glfw
cd glfw
mkdir build && cd build
cmake ../ -DBUILD_SHARED_LIBS=true
make -j`nproc`
make install
ldconfig
cd $HOME
apt-get install -y -q intel-opencl-icd ocl-icd-dev ocl-icd-opencl-dev
git clone https://gitea.osmocom.org/sdr/gr-fosphor.git
cd gr-fosphor
mkdir build && cd build
cmake ..
make -j`nproc`
make install
ldconfig
volk_profile
apt-get install -y -q cpufrequtils
for ((i=0;i<$(nproc --all);i++)); do cpufreq-set -c $i -r -g performance; done
groupadd usrp
echo -e "\n@usrp - rtprio 99\n" >> /etc/security/limits.conf
cp /etc/sysctl.conf /etc/sysctl.conf_bak
echo -e "\n\n" >> /etc/sysctl.conf
echo -e "net.core.wmem_max=50000000\n" >> /etc/sysctl.conf
echo -e "net.core.rmem_max=50000000\n" >> /etc/sysctl.conf
echo -e "net.core.wmem_default=50000000\n" >> /etc/sysctl.conf
echo -e "net.core.rmem_default=50000000\n" >> /etc/sysctl.conf
/sbin/sysctl -p
echo -e "\n\n" >> /etc/dhcp/dhclient.conf
echo -e "default interface-mtu 1500;\n" >> /etc/dhcp/dhclient.conf
echo -e "supersede interface-mtu 1500;\n" >> /etc/dhcp/dhclient.conf
service networking restart
nano /etc/default/grub
update-grub
apt-get install -y -q dpdk dpdk-dev
#git clone -b develop --recursive http://gitlab.meteo.vka/pak_group/mappi.git  mappi
git clone --depth 1 --recurse-submodules --shallow-submodules -b develop http://https://github.com/UA1ZDC/mappi.git
apt-get install -y -q build-essential cmake g++ gcc graphicsmagick-libmagick-dev-compat libblas-dev libboost-python-dev     libcgal-dev libdlib-dev libgdal-dev libicu-dev libmagick-dev libnetcdf-dev libopenblas-dev libopenjp2-7-dev     libpam0g-dev libpng-dev libpoppler-qt5-dev libpq-dev libprotobuf-dev libqt5webchannel5-dev libqt5websockets5-dev     libqt5x11extras5-dev libsnappy-dev libssl-dev libtiff5-dev libwebp-dev locate make pkg-config postgis libqt5serialport5-dev     postgresql-server-dev-all protobuf-compiler python3-lxml python3-protobuf python3-pyqt5.qtwebsockets python3-dev     python-setuptools qt5-image-formats-plugins qt5-qmake qtbase5-dev qtbase5-dev-tools qtbase5-private-dev     qtdeclarative5-dev qtscript5-dev qttools5-dev qttools5-dev-tools qtwebengine5-dev libmagic-dev libprotoc-dev     libqt5gui5 libqt5xml5 libprotobuf23 libqt5webchannel5-dev libqt5websockets5-dev libpq-dev libfftw3-dev     libvolk2-dev libpng-dev libluajit-5.1-dev libnng-dev librtlsdr-dev libhackrf-dev libairspy-dev libairspyhf-dev     libglew-dev libglfw3-dev libzstd-dev libomp-dev ocl-icd-opencl-dev
cd mappi/satdump/
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
make -j`nproc`
make install
cd $HOME
ln -s /usr/lib/x86_64-linux-gnu/libboost_python39.so /usr/lib/x86_64-linux-gnu/libboost_python.so 
./build_mappi.sh
