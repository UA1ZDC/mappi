#!/usr/bin/python3

import psycopg2
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

  def get_table_column_names( self, db, scheme, table ):
    pgcon = psycopg2.connect( user        =   self.db_user,
                              host        =   self.db_host,
                              port        =   self.db_port,
                              database    =   db )
    query = 'SELECT column_name, data_type  \
         FROM information_schema.columns WHERE table_schema=%s and table_name=%s;'
    cur = pgcon.cursor()
    cur.execute( query, ( scheme, table ) )
    records = cur.fetchall()
    return records

  def dump(self):
    try:
      json_file = open(self.file_name)
    except:
      print('Не удалось открыть файл конфигурации')
      return
    scenario = json.load(json_file)

    docfile = open( './tables_doc.txt', 'w' )

    for d in scenario['database']:
      print('Экспорт ' + d['name'] + ' старт...' )
      docfile.write('Наименование базы данных: ' + d['name'] + '\n' )
      dbname = d['name']
      for s in d['scheme']:
        scname = s['name']
        print(' ->  Схема ' + s['name'] )
        docfile.write('     Наименование схемы: ' + s['name'] + '\n' )
        tables = s.get('table', None)
        if tables is None:
          continue;
        for t in tables:
          print('       ->  Таблица ' + t['name'] )
          docfile.write('       ->  Наименование таблицф ' + t['name'] + '\n' )
          columns = self.get_table_column_names( dbname, s['name'], t['name'] )
          docfile.write('\tНаименование колонки\t\t\t|\tТип данных\n')
          for c in columns:
            docfile.write('\t' + c[0] + '\t\t' + '|\t' + c[1] + '\n')
#          self.dump_table_to_file( dbname, scname, t )
    print('Дамп БД ' + d['name'] + ' успешно!' )

def main(argv):
  dumper = Dumper()
  dumper.parseInput(argv)
  dumper.dump()



if __name__ == "__main__":
  main(argv)
