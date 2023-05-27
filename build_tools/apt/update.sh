PACKAGE_ROOT="dists/smolensk/"
RELEASE_FILE=${PACKAGE_ROOT}/Release
INRELEASE_FILE=${PACKAGE_ROOT}/InRelease

cd "$(dirname "$0")"
sudo rm -rf dists

mkdir -p ${PACKAGE_ROOT}/main/binary-amd64 
dpkg-scanpackages pool/main |  tee ${PACKAGE_ROOT}/main/binary-amd64/Packages 
dpkg-scanpackages pool/main |  gzip -c9 > ${PACKAGE_ROOT}/main/binary-amd64/Packages.gz 
dpkg-scanpackages pool/main |  bzip2 -c9 > ${PACKAGE_ROOT}/main/binary-amd64/Packages.bz2 

mkdir -p ${PACKAGE_ROOT}/contrib/binary-amd64 
dpkg-scanpackages pool/contrib |  tee ${PACKAGE_ROOT}/contrib/binary-amd64/Packages 
dpkg-scanpackages pool/contrib |  gzip -c9 > ${PACKAGE_ROOT}/contrib/binary-amd64/Packages.gz 
dpkg-scanpackages pool/contrib |  bzip2 -c9 > ${PACKAGE_ROOT}/contrib/binary-amd64/Packages.bz2 

mkdir -p ${PACKAGE_ROOT}/non-free/binary-amd64 
dpkg-scanpackages pool/non-free |  tee ${PACKAGE_ROOT}/non-free/binary-amd64/Packages 
dpkg-scanpackages pool/non-free |  gzip -c9 > ${PACKAGE_ROOT}/non-free/binary-amd64/Packages.gz 
dpkg-scanpackages pool/non-free |  bzip2 -c9 > ${PACKAGE_ROOT}/non-free/binary-amd64/Packages.bz2 

apt-ftparchive release ${PACKAGE_ROOT} -c=aptftp.conf >> ${RELEASE_FILE} 

gpg --batch --yes --clearsign -o ${INRELEASE_FILE} ${RELEASE_FILE}
gpg --batch --yes -abs -o dists/smolensk/Release.gpg dists/smolensk/Release

