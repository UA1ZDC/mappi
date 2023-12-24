-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: python-cryptography
Binary: python3-cryptography, python-cryptography-doc
Architecture: any all
Version: 3.2.1-1
Maintainer: Tristan Seligmann <mithrandi@debian.org>
Uploaders:  Debian Python Modules Team <python-modules-team@lists.alioth.debian.org>, Sandro Tosi <morph@debian.org>,
Homepage: https://cryptography.io/
Standards-Version: 4.5.0
Vcs-Browser: https://salsa.debian.org/python-team/packages/python-cryptography
Vcs-Git: https://salsa.debian.org/python-team/packages/python-cryptography.git
Testsuite: autopkgtest
Testsuite-Triggers: python3-all, python3-cffi, python3-cryptography-vectors, python3-hypothesis, python3-iso8601, python3-pretend, python3-pyasn1-modules, python3-pytest, python3-tz
Build-Depends: debhelper-compat (= 13), dh-sequence-python3, dh-sequence-sphinxdoc <!nodoc>, dpkg-dev (>= 1.17.14), libssl-dev, python3-all-dev, python3-asn1crypto (>= 0.21.0~), python3-cffi (>= 1.7~), python3-cryptography-vectors (<< 3.2.2~) <!nocheck>, python3-cryptography-vectors (>= 3.2.1~) <!nocheck>, python3-doc <!nodoc>, python3-hypothesis <!nocheck>, python3-idna (>= 2.0~), python3-iso8601 <!nocheck>, python3-pretend <!nocheck>, python3-pytest (>= 2.9.0) <!nocheck>, python3-setuptools (>= 11.3), python3-six (>= 1.4.1~), python3-sphinx <!nodoc>, python3-sphinx-rtd-theme <!nodoc>, python3-tz <!nocheck>
Package-List:
 python-cryptography-doc deb doc optional arch=all profile=!nodoc
 python3-cryptography deb python optional arch=any
Checksums-Sha1:
 20708a4955dcf7e2bb53d05418273d2bc0f80ab4 540994 python-cryptography_3.2.1.orig.tar.gz
 91b2ab3b4f955fe49f0e2c91e62608912f51f4c7 488 python-cryptography_3.2.1.orig.tar.gz.asc
 8d0caea05cdb6bbe3f8c81322f0e50291cbbf397 11784 python-cryptography_3.2.1-1.debian.tar.xz
Checksums-Sha256:
 d3d5e10be0cf2a12214ddee45c6bd203dab435e3d83b4560c03066eda600bfe3 540994 python-cryptography_3.2.1.orig.tar.gz
 41cb889b5ecb9fe555667139bab7a28cd3e8911841a16b95953410c0406e417e 488 python-cryptography_3.2.1.orig.tar.gz.asc
 0c68541d2ade034952d1962ca062e832a0861bbf7d8cc673fea0a0635095cccd 11784 python-cryptography_3.2.1-1.debian.tar.xz
Files:
 906eb57df20bb8a60222a5196c38d843 540994 python-cryptography_3.2.1.orig.tar.gz
 049acbc45710fc9663002369ca7e8495 488 python-cryptography_3.2.1.orig.tar.gz.asc
 136ce715d442cee24913d17898189f83 11784 python-cryptography_3.2.1-1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQGTBAEBCgB9FiEEXAZWhXVRbQoz/6ejwImQ+x9jeJMFAl+eul9fFIAAAAAALgAo
aXNzdWVyLWZwckBub3RhdGlvbnMub3BlbnBncC5maWZ0aGhvcnNlbWFuLm5ldDVD
MDY1Njg1NzU1MTZEMEEzM0ZGQTdBM0MwODk5MEZCMUY2Mzc4OTMACgkQwImQ+x9j
eJM2FQf/bkezRI9yfNNIK6RVgfDKbqlKOtVRCURjxZp35kJSggdTiwvrY6CKfzif
LTSwhUpl57BQFgvZKEDVuNpcLfA5EK1dhs5BvM7XY7KUhRnx30gbkc8W8fRPN4JF
R2o5Qp7CjaxIxyLrkBJ/JR9MANUuiBf9O3+Um0ViCuEjd9ta2gVMdr+8xdyVcLQb
4VuDp3a96jaJVFNs7sqz8aMXp33KCxTXH9k2h12Be6+i621tKnImoRMkJbfs3zYE
aYo1Rg5yJRmSNDMHuKSYuHMnMSWMZWm82lqYWYev9SXUJzbn4Qz8TvcSrYu9GvqV
uBy5LdveivmQ2f6m0lDFheQCDb3vRQ==
=wnXK
-----END PGP SIGNATURE-----
