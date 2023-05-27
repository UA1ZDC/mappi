#!/usr/bin/python3

import json
import pathlib
import subprocess
import sys
from sys import argv
import argparse
import  html.parser

import tempfile

EXIT_SUCCESS=0
EXIT_FAILTURE=1

class HtmlTableParser(html.parser.HTMLParser):

  row=0
  column=0
  data=dict()
  tag=None


  def handle_starttag(self, tag, attrs):
    self.tag = tag


  def handle_data(self, data):
    if self.tag is None:
      return

    if self.tag == 'tr':
      self.column = 0
      self.row += 1
      self.data[self.row] = dict()

    if self.tag == 'td':
      self.data[self.row] = self.data.get(self.row, dict())
      self.data[self.row][self.column] = data
      self.column += 1

  def handle_endtag(self, tag):
    self.tag = None



class Restorer():

  def __init__(self):
    self.file_name = 'scheme_list.json'
    self.db_user='postgres'
    self.db_host='127.0.0.1'
    self.pg_port = '5432'
    self.dump_dir = 'dump/'

    self.log_dir = tempfile.gettempdir()
    self.log_psqlfilename = 'PSQL.log'
    self.log_psqlerrfilename = 'Errors_PSQL.log'


  def parseInput(self, argv):
    parser = argparse.ArgumentParser(description= str("Восстановление БД из дампа postgresql"))
    parser.add_argument('--file_name', '-fn', type=str, help=str("Файл с конфигурацией восстановления"), default = 'scheme_list.json' )
    parser.add_argument('--user', '-u', type=str, help=str("Пользователь в БД"), default = 'postgres' )
    parser.add_argument('--host_name', '-hn', type=str, help=str("Хост с БД"), default = '127.0.0.1' )
    parser.add_argument('--dump_dir', '-d', type=str, help=str("Директория с дампом"), default = 'dump/' )
    parser.add_argument('--port', '-p', type=str, help=str("Порт с БД"), default = '5432' )

    args = parser.parse_args()

    self.file_name = args.file_name
    self.db_user= args.user
    self.db_host= args.host_name
    self.dump_dir = args.dump_dir
    self.pg_port = args.port


  def restore_by_file(self, db, path):
    if not pathlib.Path(path).is_file():
      print('Ошибка: Не существует файл \'' + path + '\'')
      return False

    args = ['psql',
            '--host', self.db_host,
            '--username', self.db_user,
            '--dbname', db,
            '--port', self.pg_port,
            '--file', path]
    psql_result = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout = psql_result.stdout.decode('utf-8')
    stderr = psql_result.stderr.decode('utf-8')
    self.logger_psql.write(stdout)
    self.error_logger_psql.write(stderr)

    if psql_result.returncode is not EXIT_SUCCESS:
      print(stdout)
      print(stderr)
      return False

    return True


  def restore_tables(self, d_name, sch_name, tables):
    for t in tables:
      t_name = t['name']
      if t['scheme'] == 'true':
        t_scheme_path = self.dump_dir + d_name + '/' + sch_name + '/' + t_name + '_scheme.sql'
        print(sch_name + ': Восстановление схемы таблицы ' + t_name + '...' )
        if self.restore_by_file(d_name, t_scheme_path) is False:
          return False


    for t in tables:
      t_name = t['name']
      if t['data'] == 'true':
        t_data_path = self.dump_dir + d_name + '/' + sch_name + '/' + t_name + '_data.sql'
        print(sch_name + ': Восстановление данных таблицы ' + t_name + '...' )
        if self.restore_by_file(d_name, t_data_path) is False:
          return False

  def isScemeExists(self, dbname, schemename):
    availableScemes = self.executePsqlCmd(sql='SELECT schema_name FROM information_schema.schemata', dbname=dbname)

    for index in availableScemes:
      scheme = availableScemes[index]
      if scheme[0] == schemename:
        return True
    return False

  def createScheme(self, dbname, schemename):
    sql = 'CREATE SCHEMA IF NOT EXISTS {}'.format(schemename)
    if self.executePsqlCmd(sql=sql, dbname=dbname) is None:
      return False
    return True

  def restore_schemes(self, db_name, schemes):

    for s in schemes:
      sch_name = s['name']
      if not self.createScheme(dbname=db_name, schemename=sch_name):
        print('Ошибка при создании схемы: {}/{}'.format(db_name, sch_name))
        return False

      print(db_name + ': Восстановление схемы ' + sch_name + '...' )
      funcs_path = self.dump_dir + db_name + '/' + sch_name + '/functions.sql'
      if not pathlib.Path(funcs_path).is_file():
        print('Ошибка: Не существует файл \'' + funcs_path + '\' для восстановления схемы ' + sch_name)
        return False

      if self.restore_by_file(db_name, funcs_path) is False:
        return False

      tables = s.get('table', None)
      if tables is None:
        continue

      if self.restore_tables(db_name, sch_name, tables) is False:
        return False


  def executePsqlCmd(self, sql, dbname=None):
    args = ['psql', '--html', '--tuples-only',
            '--host', self.db_host,
            '--port', self.pg_port,
            '--username', self.db_user,
            '--command', sql]

    if dbname is not None:
      args.append('--dbname')
      args.append(dbname)

    dbProcess = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    stdout = dbProcess.stdout.decode('utf-8')
    stderr = dbProcess.stderr.decode('utf-8')

    if dbProcess.returncode != 0:
      print(stderr)
      return None

    parser = HtmlTableParser()
    parser.feed(stdout)

    return parser.data


  def checkDatabaseExists(self, dbname):
    databaseData = self.executePsqlCmd(sql='\list', dbname='postgres')

    if databaseData is None:
      return False

    for index in databaseData:
      database = databaseData[index]

      if database.get(0, None) == dbname:
        return True

    return False


  def createDatabase(self, dbconf):

    dbname = dbconf['name']
    dbtemplate = dbconf.get('template', 'template1')

    sql = 'CREATE DATABASE {} TEMPLATE {}'.format(dbname, dbtemplate)

    if self.executePsqlCmd( sql=sql, dbname='postgres' ) is None:
      return False

    return True



  def restore(self):
    self.logger_psql = open(self.log_dir + '/' + self.log_psqlfilename, 'w')
    self.error_logger_psql = open(self.log_dir + '/' + self.log_psqlerrfilename, 'w')
    try:
      json_file = open(self.file_name)
    except:
      print('Не удалось открыть файл конфигурации')
      return
    scenario = json.load(json_file)
    print('Восстановление дампа БД...' )
    for d in scenario['database']:
      dbname = d['name']

      if self.checkDatabaseExists(dbname) is False:
        print('База данных не существует, создаю базу данных')
        if self.createDatabase(d) is False:
          print('Ошибка при создании базы данных {} '.format(dbname))
          return EXIT_FAILTURE
      else:
        print('База данных {} уже существует'.format(dbname))


      print('Восстановление расширений...' )
      extensions_path = 'dump/' + dbname + '/' + 'extensions.sql'
      if self.restore_by_file(dbname, extensions_path) is False:
        print('Восстановление расширений {} : ОШИБКА'.format(dbname))
        return EXIT_FAILTURE


      print('Восстановление схем...' )
      if self.restore_schemes(dbname, d['scheme']) is False:
        print('Восстановление схем: ОШИБКА' )
        return EXIT_FAILTURE

      print('Восстановление БД ' + dbname + ' завершено.' )

    self.logger_psql.close()
    self.error_logger_psql.close()
    print('Восстановление дампа завершено.' )
    print('Лог процесса восстановления дампа находится в ' + self.log_dir + '/' + self.log_psqlfilename )
    print('Лог ошибок процесса восстановления дампа находится в ' + self.log_dir + '/' + self.log_psqlerrfilename )
    return EXIT_SUCCESS


def main(argv):
  restorer = Restorer()
  restorer.parseInput(argv)
  return restorer.restore()


if __name__ == "__main__":
  main(argv)
