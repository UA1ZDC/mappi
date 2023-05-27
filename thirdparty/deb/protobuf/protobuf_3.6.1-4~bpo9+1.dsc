-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: protobuf
Binary: ruby-google-protobuf, libprotobuf17, libprotobuf-lite17, libprotobuf-dev, libprotoc17, libprotoc-dev, protobuf-compiler, python-protobuf, python3-protobuf, libprotobuf-java
Architecture: linux-any all
Version: 3.6.1-4~bpo9+1
Maintainer: Laszlo Boszormenyi (GCS) <gcs@debian.org>
Homepage: https://github.com/google/protobuf/
Standards-Version: 4.2.1
Testsuite: autopkgtest
Testsuite-Triggers: build-essential, default-jdk, make, pkg-config, python, zlib1g-dev
Build-Depends: debhelper (>= 11), zlib1g-dev, libgmock-dev, libgtest-dev, dh-python, python-all:any, libpython-all-dev, python3-all:any, libpython3-all-dev, python-setuptools, python-six, python3-setuptools, python3-six, xmlto, unzip, rake-compiler, gem2deb
Build-Depends-Indep: ant, default-jdk, maven-repo-helper
Package-List:
 libprotobuf-dev deb libdevel optional arch=linux-any
 libprotobuf-java deb java optional arch=all
 libprotobuf-lite17 deb libs optional arch=linux-any
 libprotobuf17 deb libs optional arch=linux-any
 libprotoc-dev deb libdevel optional arch=linux-any
 libprotoc17 deb libs optional arch=linux-any
 protobuf-compiler deb devel optional arch=linux-any
 python-protobuf deb python optional arch=linux-any
 python3-protobuf deb python optional arch=linux-any
 ruby-google-protobuf deb ruby optional arch=linux-any
Checksums-Sha1:
 9238da8596b18b22b73327508a4f4e1e9848083e 4485582 protobuf_3.6.1.orig.tar.gz
 050da87fdc6463c10309b80a0d708e12c87d17ef 23548 protobuf_3.6.1-4~bpo9+1.debian.tar.xz
Checksums-Sha256:
 3d4e589d81b2006ca603c1ab712c9715a76227293032d05b26fca603f90b3f5b 4485582 protobuf_3.6.1.orig.tar.gz
 d70f364c218385643a0e80b6da20cd50fa58070174105cd505c4227286738b20 23548 protobuf_3.6.1-4~bpo9+1.debian.tar.xz
Files:
 e8ce2659ea4f5df1a1e0dbd107dd61d9 4485582 protobuf_3.6.1.orig.tar.gz
 1e7d8982924d83d723d96613206afb38 23548 protobuf_3.6.1-4~bpo9+1.debian.tar.xz
Ruby-Versions: all

-----BEGIN PGP SIGNATURE-----

iQIzBAEBCgAdFiEE/OFtr184QaN6dPMgC3aSB2Kmt4UFAlvRbSQACgkQC3aSB2Km
t4W9GQ//UinWC+HeCRyyfZn1to9i5VH6lYZBZMih47hPehJdSuvrXkNokBlVL/U3
a7e3+E35QvEMK10knHt8K/cmrAuOmkTWwq1t/38ZT5zD61CZceltJQYKilRJdFbZ
1PsRRf1b2l9yB/fTe/pQXL8eJRug8Hbrv/9FtHcjQNg+cSrsuo6+x5y7WYqmnw6y
KH/fYB1Adk2NvI7K8YPjYcmzARXoo3etKr/2AdFmMWj2rdg0Ghjd41AAmeD0dFW1
wW90SXKGFmjHDJukzf+dosaKMf/h5o+9RT8RvO8ZBedb5iRcYwbDwPCzSQn5Gl/p
qu3EYj4G1i8qPpLlNgTTbRn3BHWkjeeP9AvRHIlK+xF0HmteDHX4hqMBJjQHwlUx
UTgc3NSa8LbiFxFuhgPZfWFlBuD8ROlfRamw6c2BG8TJrKsEBsO5We+WdSf421qR
tU9D7pF0kU3uOfrZd/bNCiwDe59NxutnfjV43k3f7tPK8de2mQ8is4vt0ZjYE5VP
OZct9z1DdUPM51kEAWGQmuophhq0w0QImOL4JZtAar3H8lpCNQ5Sc0BLlwtECBXa
1GquD0FcIMDEvhrf0wFQEYGrI+jDydPKEbaOhpBVzCfL6iSfNlnXvtNajZrOV2m2
01K16Avu+0XJjnOst4aPZjF3xOg8sRx7ZIoL/rEwt12ZCms3nOE=
=9WX0
-----END PGP SIGNATURE-----
