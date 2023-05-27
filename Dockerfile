FROM gcc:10.4-buster AS build
LABEL version="1.0"
LABEL description="Container made build qt project from source"
#Installing required packages and dependencies
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update &&\
    apt-get install -y -q \
    build-essential cmake g++ gcc graphicsmagick-libmagick-dev-compat libblas-dev libboost-python-dev \
    libcgal-dev libdlib-dev libgdal-dev libicu-dev libmagick-dev libnetcdf-dev libopenblas-dev libopenjp2-7-dev \
    libpam0g-dev libpng-dev libpoppler-qt5-dev libpq-dev libprotobuf-dev libqt5webchannel5-dev libqt5websockets5-dev \
    libqt5x11extras5-dev libsnappy-dev libssl-dev libtiff5-dev libwebp-dev locate make pkg-config libqt5serialport5-dev \
    postgresql-server-dev-all protobuf-compiler python3-lxml python3-protobuf python3-pyqt5.qtwebsockets python-dev \
    python-setuptools qt5-image-formats-plugins qt5-qmake qtbase5-dev qtbase5-dev-tools qtbase5-private-dev \
    qtdeclarative5-dev qtscript5-dev qttools5-dev qttools5-dev-tools qtwebengine5-dev libmagic-dev libprotoc-dev &&\
    apt-get clean &&\
    rm -rf /var/lib/apt/lists/*
#Geting mappi source
ARG src_path="/mappi_src"
ARG install_path="/opt/mappi"
WORKDIR $src_path
#COPY --from=cloner $src_path ./
COPY ./ ./
#Set enviroenment values and build
SHELL ["/bin/bash", "-c"]
ENV SRC="$src_path"
ENV BUILD_DIR="$install_path"
ENV QT_SELECT=qt5
RUN source ./env-bash && source ./build_tools/cmd_build.sh mappi
WORKDIR $install_path
RUN rm -rf $src_path

FROM debian:10 AS deploy
LABEL version="1.0"
LABEL description="Container made deploy and run qt project without sources"
#Installing required packages and dependencies
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update &&\
    apt-get install -y -q\
#TODO: Add dependencies
    libqt5gui5 libqt5xml5 libprotobuf17 libqt5webchannel5-dev libqt5websockets5-dev libpq-dev &&\
    apt-get clean &&\
    rm -rf /var/lib/apt/lists/*
#Copying mappi binaries
COPY --chmod=0755 ./thirdparty/multirun /bin/
ARG install_path="/opt/mappi"
WORKDIR $install_path
COPY --from=build $install_path ./
#Setting mappi env variables
ENV LD_LIBRARY_PATH="/usr/local/lib:$install_path/lib:"
#Running mappi
#TODO: Select binaries for start
#ENTRYPOINT ["multirun", "bin/mappi.app.manager"]
