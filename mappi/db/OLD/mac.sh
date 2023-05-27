#!/bin/sh

usermac -m 0:0 -c 0:0 mappi

#setfacl -m u:postgres:rx /etc/parsec/macdb
#setfacl -m u:postgres:rx /etc/parsec/capdb
#setfacl -d -m u:postgres:r /etc/parsec/macdb
#setfacl -d -m u:postgres:r /etc/parsec/capdb
#setfacl -R -m u:postgres:r /etc/parsec/macdb/*
#setfacl -R -m u:postgres:r /etc/parsec/capdb/*


usermac -m 0:0 -c 0:0 postgres

setfacl -d -m u:postgres:r /etc/parsec/macdb
setfacl -R -m u:postgres:r /etc/parsec/macdb
setfacl -m u:postgres:rx /etc/parsec/macdb
