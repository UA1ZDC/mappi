#/bin/sh -e

VERSION=$2
TAR=../dlib_$VERSION.orig.tar.gz
NEWTAR=../dlib_$VERSION.orig.tar.bz2
DIR=dlib-$VERSION
mkdir -p $DIR

# Unpack ready fo re-packing
tar -xzf $TAR -C $DIR --strip-components=1

# Remove included sources for external libraries (libjpeg, libpng, zlib)
rm -Rf $DIR/dlib/external

# Repack
rm $TAR
GZIP=--best tar -cjf $NEWTAR $DIR

rm -rf $DIR

