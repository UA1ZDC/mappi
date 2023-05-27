# mappi
Software for automated collection and processing of satellite hydrometeorological information.

apt-get update &&\
    apt-get install -y -q \
    build-essential cmake g++ gcc graphicsmagick-libmagick-dev-compat libblas-dev libboost-python-dev \
    libcgal-dev libdlib-dev libgdal-dev libicu-dev libmagick-dev libnetcdf-dev libopenblas-dev libopenjp2-7-dev \
    libpam0g-dev libpng-dev libpoppler-qt5-dev libpq-dev libprotobuf-dev libqt5webchannel5-dev libqt5websockets5-dev \
    libqt5x11extras5-dev libsnappy-dev libssl-dev libtiff5-dev libwebp-dev locate make pkg-config libqt5serialport5-dev \
    postgresql-server-dev-all protobuf-compiler python3-lxml python3-protobuf python3-pyqt5.qtwebsockets python-dev \
    python-setuptools qt5-image-formats-plugins qt5-qmake qtbase5-dev qtbase5-dev-tools qtbase5-private-dev \
    qtdeclarative5-dev qtscript5-dev qttools5-dev qttools5-dev-tools qtwebengine5-dev libmagic-dev libprotoc-dev \
    libqt5gui5 libqt5xml5 libprotobuf17 libqt5webchannel5-dev libqt5websockets5-dev libpq-dev
    
Then build ./satdump and install
