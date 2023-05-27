#include "tbufrmeta.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <qstringlist.h>
#include <qvector.h>
#include <qfile.h>
#include <qdir.h>
#include <QRegularExpression>


#define TABLE_DIR MnCommon::varPath("meteo") + "/bufrtables/"
#define TABLE_C5_NAME "Common_C05_latest.txt"
#define TABLE_C8_NAME "Common_C08_latest.txt"

//! Загрузка таблиц BUFR
/*! 
  \param edition     Номер издания
  \param center      Идентификация центра — поставщика/производителя продукции
  \param subcenter   Идентификация подцентра 
  \param mastertable Номер версии эталонной таблицы
  \param localtable  Номер версии местных таблиц,таблиц
  \return true в случае успешной загрузки
*/
bool BufrMetaData::load(int edition, int center, int subcenter, int mastertable, int localtable)
{
  // var(edition);
  // var(center);
  // var(subcenter);
  // var(mastertable);
  // var(localtable);

  if (center == 255) center = 0;
  if (subcenter == 255) subcenter = 0;

  QString tableFile = findTableFile(edition, center, subcenter, mastertable, localtable, 'B');
  if (tableFile.isEmpty()) {
    return false;
  }

  if (_tableB != tableFile) {
    if (!loadTableB(tableFile)) {
      error_log<<QObject::tr("Ошибка чтения таблицы B %1").arg(tableFile);
      _tableB = QString();
      return false;
    }
    
    _tableB = tableFile;
  }

  //---

  tableFile = findTableFile(edition, center, subcenter, mastertable, localtable, 'D');
  if (tableFile.isEmpty()) {
    return false;
  }

  if (_tableD != tableFile) {
    if (!loadTableD(tableFile)) {
      error_log<<QObject::tr("Ошибка чтения таблицы D %1").arg(tableFile);
      _tableD = QString();
      return false;
    }
    
    _tableD = tableFile;
  }

  //---

  loadTableC5(TABLE_DIR + TABLE_C5_NAME);
  loadTableC8(TABLE_DIR + TABLE_C8_NAME);
  
  return true;
}

//! Поиск подходящего файла с таблицей
/*! 
  \param edition     Номер издания
  \param center      Идентификация центра — поставщика/производителя продукции
  \param subcenter   Идентификация подцентра 
  \param mastertable Номер версии эталонной таблицы
  \param localtable  Номер версии местных таблиц,таблиц  
  \param tableSym    Обозначение таблицы 'B' и 'D'
  \return Название файла с таблицей
*/
QString BufrMetaData::findTableFile(int edition, int center, int subcenter, int mastertable, int localtable, char tableSym)
{
  QDir dir(TABLE_DIR);
  QString tableName = QString();
  QStringList tableList;
  //  tableList.append(QString(tableSym) + "_latest.txt");
  getVersName(edition, subcenter, center, mastertable, localtable, tableSym, tableList);
  getVersName(edition, 0, 0, mastertable, 0, tableSym, tableList);//wmo table
  tableList.append(QString(tableSym) + "_latest.txt");

  //var(tableList);

  for (int i=0; i< tableList.count(); i++) {
    if ( dir.exists(tableList[i])) {
      tableName = tableList[i];
      break;
    }
  }

  if (tableName.isEmpty()) {
    error_log<<QObject::tr("Нет таблицы %5 для: Центр=%1 Подцентр=%2 Эталонная таблица=%3 Местая таблица=%4").
      arg(center).arg(subcenter).arg(mastertable).arg(localtable).arg(tableSym);
    return QString();
  }

#ifdef DEBUG_BUFR_PARSER
  var(tableName);
#endif

  return dir.absoluteFilePath(tableName);
}

//! Формирование имён файлов, соответствующих версии передаваемого сообщения
/*! 
  \param edition     Номер издания
  \param center      Идентификация центра — поставщика/производителя продукции
  \param subcenter   Идентификация подцентра 
  \param mastertable Номер версии эталонной таблицы
  \param localtable  Номер версии местных таблиц,таблиц  
  \param alpha       Обозначение таблицы 'B' и 'D'
  \param tableList   Список подходящих имён файлов
*/
void BufrMetaData::getVersName(int edition, int subcenter, int center, int mastertable, int localtable, char alpha, QStringList& tableList)
{
  QString tableName;

  switch (edition) {
  case 4: case 3:
    tableName.sprintf("%c00%03d%04d%04d%03d%03d.txt", alpha, 0, subcenter, center, mastertable, localtable);
    tableList.append(tableName); //подходят имена для 4 версии (в теории)
    tableName.sprintf("%c00000%03d%03d%02d%02d.txt", alpha, 0, center, mastertable, localtable);
    tableList.append(tableName); //подходят имена для 3,4 версии
 case 2:
    tableName.sprintf("%c%05d%02d%02d.txt", alpha, center, mastertable, localtable);
    tableList.append(tableName); //подходят имена для 2,3,4 версии
    break;
  default:
    error_log<<QObject::tr("Версия bufr %1 не поддерживается").arg(edition);
  }
}

//! Загрузка таблицы D из файла
/*! 
  \param filename название файла
  \return true - в случае успешной загрузки
*/
bool BufrMetaData::loadTableD ( const QString &filename ) 
{
  QStringList list;
  if(!loadTextBuffer (filename,&list )) {
    return false;
  }
  _table_d.clear();

  if (list.count() != 0 && list.at(0).contains("\"No\",\"Category\"")) {
    return loadTableDNewVers(list);
  }

  QRegExp rx( "\\s*(\\d+)\\s*(\\d+)\\s*(\\d+).*" );

  QStringList::iterator it;
  QStringList capt_list;
  
  QVector<int> list_depend;
  for(it = list.begin();it< list.end(); ++it){
    (*it).replace('D', '3').replace('B', '0').replace('R', '1').replace('C', '2');
    list_depend.clear();
    rx.setPattern("\\s*(\\d+)\\s*(\\d+)\\s*(\\d+).*" );
    int pos = (*it).trimmed().indexOf(rx,0);
    if(pos != 0) {continue;}
    
    if(rx.capturedTexts().count() != 4)
      continue;

    capt_list = rx.capturedTexts();
    bool ok1,ok2,ok3;
    int a1 = capt_list.at(1).toInt(&ok1);
    int a2 = capt_list.at(2).toInt(&ok2);
    int a3 = capt_list.at(3).toInt(&ok3);

    if(!ok1||!ok2||!ok3) {
      continue;
    }
    list_depend.append(a3);

    rx.setPattern("\\s+(\\d+)\\s*");

    for (int i=0; i<a2-1; i++) {
      ++it;
      (*it).replace('D', '3').replace('B', '0').replace('R', '1').replace('C', '2');
      (*it).indexOf(rx,0);

      QStringList list_vs = rx.capturedTexts();
      if(list_vs.count() != 2) {
        return false;
      }
      bool ok;
      a3 = list_vs.at(1).toInt(&ok);
      if(!ok) {
	return false;
      }
      list_depend.append(a3);
    }
    _table_d.insert(a1,list_depend);
  }  
  return true;
}

bool BufrMetaData::loadTableDNewVers(QStringList lines)
{
  QString d_descr;
  QVector<int> d_list;
  
  for (int idx = 1; idx < lines.count(); idx++) {
    QString one = lines.at(idx);
    while (one.contains(",,")) {
      one.replace(",,", ",\"\",");
    }
    QStringList line = one.split("\",\"");
    QString curDescr;
    QString curList;

    if (line.size() == 10) {
      curDescr = line.at(2);
      curList = line.at(5);
    } else {
      var(line.size());
      return false;
    }
  
    if (d_descr != curDescr && !d_descr.isEmpty()) {
      _table_d.insert(d_descr.toInt(), d_list);
      d_list.clear();
    }

    d_descr = curDescr;
    d_list.append(curList.toInt());
  }

  _table_d.insert(d_descr.toInt(), d_list);

  return true;
}


 
// bool BufrMetaData::loadTableDNewVers(QStringList lines)
// {
//    QRegExp rx("\\s*[\\d.]+,\"\\d\\d\",\"(?:.(?!\",))*.\",\"(\\d{6})\",(?:\"(?:.(?!\",))*.\")?,(?:\"(?:.(?!\",))*.\")?,\"(\\d{6})\".*");
//   QString d_descr;
//   QVector<int> d_list;
  
//   for (int idx = 1; idx < lines.count(); idx++) {
//     int pos = lines.at(idx).indexOf(rx);
//     if(pos < 0) {
//       var(pos);
//       return false;
//     }
   
//     if (d_descr != rx.cap(1) && !d_descr.isEmpty()) {
//       _table_d.insert(d_descr.toInt(), d_list);
//       d_list.clear();
//     }

//     d_descr = rx.cap(1);
//     d_list.append(rx.cap(2).toInt());
    
//   }

//   _table_d.insert(d_descr.toInt(), d_list);

//   var(_table_d.size());
//   QList<int> keys = _table_d.keys();
//   qSort(keys);
//   var(keys);


//   return true;
// }

//! Загрузка таблицы B из файла
/*! 
  \param filename название файла
  \return true - в случае успешной загрузки
*/ 
bool BufrMetaData::loadTableB ( const QString &filename ) 
{
  //  var(filename);
  QStringList list;
  _table_b.clear();
  if(!loadTextBuffer (filename,&list )) {
    return false;
  }

  QString d;
  if (list.count() != 0 && list.at(0).contains("\"No\",\"ClassNo\"")) {
    d = "\\s*[\\d.]+,\"\\d\\d\",\"(?:.(?!\",))*.\",\"(\\d{6})\",\"((?:.(?!\",))*.)\",(?:\"(?:.(?!\",))*.\")?,\"((?:.(?!\",))*.)\",\"\\s*(-?\\s*\\d+)\\s*\",\"\\s*(-?\\s*\\d+)\\s*\",\"\\s*(-?\\s*\\d+\\s*).*";
    //list.removeFirst();
    for (int idx = 1; idx < list.count(); idx++) {
      QString one = list.at(idx);
      while (one.contains(",,")) {
	one.replace(",,", ",\"\",");
      }
      QStringList line = one.split("\",\"");
      line.removeAt(4);
      if(NO_ERRORS != setTableB(line.mid(1, 7))) {
	return false;
      }
    }
    

  } else {
    d = "\\s*(\\d{6})\\s+(.{64})\\s*(.{25})\\s*(-?\\s*\\d+)\\s*(-?\\s*\\d+)\\s*(-?\\s*\\d+).*";
    QRegExp rx(d);
    QStringListIterator it(list);
    int pos=0;
    while(it.hasNext()) {
      pos = it.next().indexOf(rx,pos);
      if(pos < 0) {
	//var(pos);
	return true;
      }
      //    var(rx.cap());
      if(NO_ERRORS != setTableB(rx.capturedTexts())) {
	return false;
      }
    }

  }


    
  return true;
}


//! Парсинг строки таблицы B
/*! 
  \param list строка таблицы
  \return NO_ERRORS при отсуствии ошибок
*/
int BufrMetaData::setTableB (const QStringList &list ) 
{
  static DataDescriptor data_descr;
 
  bool ok = true;

  switch(list.count()){
    case  10:
    case  7:
      {
        data_descr.width = list.at(6).simplified().remove(' ').toInt(&ok) ;
        if(!ok) break;
        data_descr.ref_value = list.at(5).simplified().remove(' ').toInt(&ok) ;
        if(!ok) break;
        data_descr.scale = list.at(4).simplified().remove(' ').toInt(&ok) ;
        if(!ok) break;
        data_descr.unit = list.at(3).simplified();
        data_descr.name_elem = list.at(2).simplified();
        int idescript = list.at(1).simplified().remove(' ').toInt(&ok) ;
        if(!ok) break;
        data_descr.descriptor.setIndex(idescript );
      }
      break;
    default:
      ok = false;
  }
  if ( false == ok ) {
    return OSH_BUFR_STRUCTURE;
  }
  _table_b.insert(data_descr.descriptor.tableIndex(),data_descr);
  return NO_ERRORS;
}

//! Загрузка файла построчно
/*! 
  \param filename Название файла
  \param list     Считанные строки
  \return true - в случае успешной загрузки
*/
bool BufrMetaData::loadTextBuffer(const QString &filename,  QStringList* list)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    error_log << QObject::tr("Ошибка открытия файла") <<filename;
    return false;
  }
  
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine();
    list->append(line);
  }

  return true;
}


//! Загрузка таблицы B из файла
/*! 
  \param filename название файла
  \return true - в случае успешной загрузки
*/ 
bool BufrMetaData::loadTableC5( const QString &filename ) 
{
  //  var(filename);
  QStringList lines;
  _table_c5.clear();
  if(!loadTextBuffer (filename, &lines)) {
    return false;
  }

  QRegularExpression rx("\\s*[\\d.]+,\"\\d{3}\",\"(\\d{1,4})\",\"\\d{1,5}\",\"((?!\").*)\",\".*\"");
  QRegularExpressionMatch rmatch;
  
  for (int idx = 1; idx < lines.count(); idx++) {
    rmatch = rx.match(lines.at(idx));
    if (rmatch.hasMatch()) {
      int num = rmatch.captured(1).toInt();
      QString sat = rmatch.captured(2);
      _table_c5.insert(num, sat);
    }
  }
       
  return true;
}

//! Загрузка таблицы B из файла
/*! 
  \param filename название файла
  \return true - в случае успешной загрузки
*/ 
bool BufrMetaData::loadTableC8( const QString &filename ) 
{
  //  var(filename);
  QStringList lines;
  _table_c8.clear();
  if(!loadTextBuffer (filename, &lines)) {
    return false;
  }

  QRegularExpression rx("\\s*[\\d.]+,\"(\\d{1,4})\",\"(?:(?!\").)*\",\"(?:(?!\").)*\",\"((?:(?!\").)*)\",\".*\"");
  QRegularExpressionMatch rmatch;
  
  for (int idx = 1; idx < lines.count(); idx++) {
    rmatch = rx.match(lines.at(idx));
    if (rmatch.hasMatch()) {
      int num = rmatch.captured(1).toInt();
      QString sat = rmatch.captured(2);
      _table_c8.insert(num, sat);
    }
  }
       
  return true;
}
