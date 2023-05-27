#!/usr/bin/python3

import json
import pathlib
import subprocess
import os
import platform
import sys
from sys import argv
import argparse

class Dumper():

  def __init__(self):
    self.file_name = 'scheme_list.json'
    self.db_user='postgres'
    self.db_host='127.0.0.1'
    self.db_port='5432'
    self.dump_dir = 'dump/'
    self.saveMac = False

  def parseInput(self, argv):
    parser = argparse.ArgumentParser(description= str("Создание дампа postgresql"))
    parser.add_argument('--file_name', '-fn', type=str, help=str("Файл с конфигурацией дампирования"), default = 'scheme_list.json' )
    parser.add_argument('--user', '-u', type=str, help=str("Пользователь в БД"), default = 'postgres' )
    parser.add_argument('--host_name', '-hn', type=str, help=str("Хост с БД"), default = '127.0.0.1' )
    parser.add_argument('--port', '-p', type=str, help=str("Порт БД"), default = '5432' )
    parser.add_argument('--dump_dir', '-d', type=str, help=str("Директория с дампом"), default = 'dump/' )
    parser.add_argument('--save_mac', '-s', type=bool, help=str("Сохранение мандатных меток"), default = False )

    args = parser.parse_args()

    self.file_name = args.file_name
    self.db_user= args.user
    self.db_host= args.host_name
    self.db_port= args.port
    self.dump_dir = args.dump_dir
    self.saveMac = args.save_mac

  def checkAstra(self, params):
    if 'astra' in platform.version() and not self.saveMac:
      params.append('--disable-macs')
      params.append('--no-security-labels')
    return params

  def dump_create_extensions(self, db ):
    filename = self.dump_dir + '/' + db + '/' + 'extensions.sql'
    params = [ 'pg_dump', '-h', self.db_host,
                        '-p', self.db_port,
                        '-U', self.db_user,
                        '-d', db,
                        '--format=p',
                        '-N', '*',
                        '-T', '*',
                        '-f', filename
            ]
    params = self.checkAstra(params)
    subprocess.run( params )

  def dump_scheme_without_tables(self, db, scheme ):
    filename = self.dump_dir + '/' + db + '/' + scheme + '/functions.sql'
    params = [ 'pg_dump', '-h', self.db_host,
                        '-p', self.db_port,
                        '-U', self.db_user,
                        '-d', db,
                        '--format=p',
                        '--schema='+scheme,
                        '-T', scheme +'.*',
                        '-f', filename
            ]
    params = self.checkAstra(params)
    subprocess.run( params )

  def dump_table_to_file(self, db, scheme, table ):
    tablename = table['name']
    if 'true' == table['scheme']:
      filename = self.dump_dir + '/' + db+'/'+scheme + '/' + tablename + '_scheme.sql'
      params = [ 'pg_dump', '-h', self.db_host,
                          '-p', self.db_port,
                          '-U', self.db_user,
                          '-d', db,
                          "-s",
                          '--schema='+scheme,
                          '-t', scheme + '.' + tablename, '-t', scheme + '.' + tablename + '*seq',
                          '--format=p',
                          '-f', filename
          ]
      params = self.checkAstra(params)
      subprocess.run( params )

    if 'true' == table['data'] and True == table.get('override_data', True):
      filename = self.dump_dir + '/' + db+'/'+scheme + '/' + tablename + '_data.sql'
      params = [ 'pg_dump', '-h', self.db_host,
                          '-p', self.db_port,
                          '-U', self.db_user,
                          '-d', db,
                          '-a',
                          '--column-inserts',
                          '--schema='+scheme,
                          '-t', scheme + '.' + tablename, '-t',
                          '--format=p',
                          '-f', filename
          ]
      params = self.checkAstra(params)
      subprocess.run( params )

  def dump(self):
    try:
      json_file = open(self.file_name)
    except:
      print('Не удалось открыть файл конфигурации')
      return
    scenario = json.load(json_file)
    for d in scenario['database']:
        print('Дамп БД ' + d['name'] + ' старт...' )
        dbname = d['name']
        pathlib.Path(self.dump_dir + '/' + dbname).mkdir( parents=True, exist_ok=True )
        self.dump_create_extensions(dbname)
        for s in d['scheme']:
          scname = s['name']
          print(' ->  Схема ' + s['name'] )
          pathlib.Path( self.dump_dir + '/' + dbname + '/' + scname ).mkdir( parents=True, exist_ok=True )
          self.dump_scheme_without_tables( dbname, scname )
          tables = s.get('table', None)
          if tables is None:
            continue;
          for t in tables:
            print('       ->  Таблица ' + t['name'] )
            self.dump_table_to_file( dbname, scname, t )
    print('Дамп БД ' + d['name'] + ' успешно!' )

def main(argv):
  dumper = Dumper()
  dumper.parseInput(argv)
  dumper.dump()



if __name__ == "__main__":
  main(argv)
