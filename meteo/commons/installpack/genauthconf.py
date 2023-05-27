#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# Утитита для генерации конфигурационного настроек авторизации СПО
#

from argparse import ArgumentParser
from authconf_pb2 import AuthConf, kAuthALD, kAuthPAM
from google.protobuf import  text_format

if __name__ == '__main__':
  parser = ArgumentParser()
  parser.add_argument('prj_dir')
  typeParser = parser.add_subparsers(title="type", description="Тип авторизации", dest="type" )
  aldParser = typeParser.add_parser('ald')
  aldParser.add_argument('keytab', )
  aldParser.add_argument('domain')
  aldParser.add_argument('service')
  aldParser.add_argument('admin')
  aldParser.add_argument('password')

  pamParser = typeParser.add_parser('pam')

  args = parser.parse_args()
  prj_dir = args.prj_dir

  conf = AuthConf()
  if args.type == "pam":
    conf.auth_provider = kAuthPAM
  else:
    conf.auth_provider = kAuthALD
    conf.ald_keytab = args.keytab
    conf.ald_domain = args.domain
    conf.ald_service_name = args.service
    conf.ald_admin = args.admin
    conf.ald_password = args.password

  with open('{}/etc/meteo/os-auth.conf'.format(prj_dir), 'w+' ) as confFile:
    config = text_format.MessageToString(conf)
    confFile.write(config)
