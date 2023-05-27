#include "tbufr.h"
#include "tbufrlist.h"
#include "tbufrtransform.h"

//TODO Отдельным набором идут флаги наличия данных (031031) и флаги качества (033yyy)
//04-07 приращения и сдвиги
//дескриптор и данные с задержкой
//94.5.4.3 приращение и дескриптор повторения
//ассоциированные поля могут добавляться
//не все операторы поддерживаются

#include <cross-commons/debug/tmap.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/tprototext.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/control/tmeteocontrol.h>
#include <meteo/commons/proto/state.pb.h>

#include <qlist.h>

#include <new>
#include <math.h>

#ifdef WIN32
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define SIZEOCT 8
#include <qfileinfo.h>

#include <qdebug.h>

#define OCT(a) int((unsigned char)a)
#define NOCT(a) int((a-1)/SIZEOCT)+1

#define GETBIT(a,n) (a>>(SIZEOCT-n)&1)

#define CONF_FILE MnCommon::etcPath() + "bufr.conf"
#define UNKNOWN_BUFR_SEQ_FILE MnCommon::varPath("meteo") + "/bufr_unknown.txt"

static int nachaloS4=33;
static int NO_REPLICATIONS= 6;

static int START_SECT1= 8;
static int SECTION5_LENGHT =4;
static int COUNT_BIT = 6;
  

using namespace meteo;

TBufr::TBufr()
{
  _transf = new TBufrTransform;
  _codec = QTextCodec::codecForLocale();

  meteo::bufr::DecodingTypes prop;
  
  QFile file(CONF_FILE);
  if ( !file.open(QIODevice::ReadOnly) ) {
    error_log << QObject::tr("Ошибка при загрузке конфигурации '%1'").arg(CONF_FILE);
  } else {
    QString text = QString::fromUtf8(file.readAll());
    file.close();
    
    if ( !TProtoText::fillProto(text, &prop) ) {
      error_log << QObject::tr("Ошибка в структуре файла конфигурации");
    }
  }
  for (auto type : prop.type()) {
    _procTypes.append(surf::DataType(type));
  }
  
  _stat.set(bufr::kMsgKind, bufr::kReceivedUnk, meteo::app::OperationState_NORM,
        QObject::tr("Принято пустых телеграмм"));
  _stat.set(bufr::kMsgKind, bufr::kReceived, meteo::app::OperationState_WARN,
        QObject::tr("Принято сообщений BUFR"));
  _stat.set(bufr::kMsgKind, bufr::kDecoded, meteo::app::OperationState_WARN,
        QObject::tr("Раскодировано сообщений BUFR"));
  _stat.set(bufr::kMsgUnk, bufr::kErrStructure, meteo::app::OperationState_NORM,
        QObject::tr("Принято пустых сообщений BUFR"));
  _stat.set(bufr::kMsgKind, bufr::kErrStructure, meteo::app::OperationState_NORM,
        QObject::tr("Ошибка структуры сообщения BUFR"));


  QList<bufr::StatType> stype;
  stype << bufr::kDecoded << bufr::kReceived;

  for (auto category : prop.category()) {
    if (category.process()) {
      _stat.set(meteo::bufr::StatKind(category.num()), stype, meteo::app::OperationState_WARN,
		QString::fromStdString(category.name()));
      _procCateg.append(category.num());
    }
  }
  
  _stat.set(bufr::kMsgUnk, bufr::kReceived, meteo::app::OperationState_WARN,
        QObject::tr("Другие категории"));


  _stat.set(bufr::kMsgKind, bufr::kDecodedSubs, meteo::app::OperationState_WARN,
        QObject::tr("Извлечено сводок"));
}

TBufr::~TBufr()
{
  clearBM();
  if (_transf) delete _transf;
  _transf = 0;
}

//! Очистка внутренних параметров
void TBufr::clearBM() {
  if (_bufrList) {
    _bufrList->clearDelete();
    delete _bufrList;
    _bufrList=0;
  }
  _subs.clear();
  _chop.clear();
  _type = bufr::kTransfTypeUnk;
  _seqNumber = -1;
  _dataType = meteo::surf::kUnknownDataType;
}

bool TBufr::setCodec(const QByteArray& name)
{
  if (QTextCodec::availableCodecs().contains(name)) {
    debug_log << QObject::tr("Указанный кодек не поддерживается");
    return false;
  }
  _codec = QTextCodec::codecForName(name);
  return true;
}

//! Раскодирование BUFR
/*!
  \param filename название файла, содержащего сводки в формате BUFR
  \return количество нераскодированных сообщений (для которых последовательности известны)
*/
int TBufr::decode( const QString &filename )
{
  QFile fl(filename);
  if (!fl.open(QIODevice::ReadOnly)) {
    debug_log << QObject::tr("Ошибка открытия файла '%1'").arg(filename);
    return 0;
  }
  _bufAll = fl.readAll();
  fl.close();

  return decode();
}

//! Раскодирование BUFR
/*!
  \param type для совместимости, здесь не используется
  \param ba массив байт, содержащий сводки в формате BUFR
  \return  количество нераскодированных сообщений (для которых последовательности известны)
*/
int TBufr::decode(const QMap<QString, QString>& type, const QByteArray& ba)
{
  Q_UNUSED(type);
  _bufAll = ba;

  return decode();
}

//! раскодирование данных в буффере _bufAll
int TBufr::decode() //[private]
{
  if (_bufAll.size() < 15) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Не верный размер =") << _bufAll.size() << "idPtkpp=" << sourceId();
#endif
    _stat.add(bufr::kMsgKind, bufr::kReceivedUnk, meteo::app::OperationState_WARN);
    return 0;
  }
  int count_bufr_messages = _bufAll.count("BUFR");

  if ( 0 == count_bufr_messages) {
    _stat.add(bufr::kMsgKind, bufr::kReceivedUnk, meteo::app::OperationState_WARN);
    debug_log << QObject::tr("В телеграмме нет сообщений BUFR") << sourceId();
    return 0;
  }

  _stat.add(bufr::kMsgKind, bufr::kReceived, meteo::app::OperationState_NORM, count_bufr_messages );


#ifdef DEBUG_BUFR_PARSER
  debug_log<<"found  "<<count_bufr_messages <<"BUFR messages";
#endif

  int loaded = loadBufr();

  _stat.add(bufr::kMsgKind, bufr::kDecoded, meteo::app::OperationState_NORM, loaded);

  if (loaded == 0) {
    return -1;
  }

  return loaded;
}

//! Для вызова из просмотра ГМИ (результат не в БД, а в result, контроль качества)
int TBufr::decode(const QByteArray& ba, QList<TMeteoData>* result, QString* error, const QDateTime& dt)
{
  _bufAll = ba;

  if (_bufAll.size() < 15) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Не верный размер =") << _bufAll.size() << "idPtkpp=" << sourceId();
#endif
    *error = QObject::tr("Пустое сообщение");
    return 0;
  }

  int count_bufr_messages = _bufAll.count("BUFR");
  if ( 0 == count_bufr_messages) {
    *error = QObject::tr("Пустое сообщение");
    return 0;
  }

  int loaded = loadBufr(result, error, dt);


  return loaded;
}

//////////////////////////////////////////

//! Раскодирование загруженных данных
/*!
  \return  количество загруженных BUFR
*/
int TBufr::loadBufr()
{
  int messages_loaded=0;
  _stpos =0;
  while ( -1 < (_stpos = _bufAll.indexOf("BUFR", _stpos))) {
    if ( (_stpos + getLength(_bufAll, _stpos+4) ) > _bufAll.size() ) {
      _stpos += 4;
#ifdef PARSE_LOG
      debug_log << QObject::tr("Ошибка структуры BUFR") << sourceId();
#endif
      _stat.add(bufr::kMsgKind, bufr::kErrStructure, meteo::app::OperationState_WARN);
      continue;
    }
    if ( !controlBufr() ) {
#ifdef PARSE_LOG
      debug_log << QObject::tr("Ошибка структуры BUFR") << sourceId();
#endif
      _stat.add(bufr::kMsgKind, bufr::kErrStructure, meteo::app::OperationState_WARN);
      _stpos += 4 ;
      continue;
    }
    //var(stpos);
    int res;
    if ( (res = getOneBufr()) != NO_ERRORS ) {
      _stpos += 4 ;
      if (res == EMPTY_BUFR) {
    _stat.add(bufr::kMsgUnk, bufr::kReceivedUnk, meteo::app::OperationState_WARN);
    //messages_loaded ++;
      } else if (res == OSH_BUFR_STRUCTURE) {
    _stat.add(bufr::kMsgKind, bufr::kErrStructure, meteo::app::OperationState_WARN);
      }
      //  else if (res != NOT_PROCESSED) {
      // 	debug_log << QObject::tr("Ошибка структуры BUFR") << sourceId();
      // }
      continue;
    }

    _stat.add(meteo::bufr::StatKind(_bufrMsg.s1.type_data), bufr::kDecoded, meteo::app::OperationState_NORM);
    _stat.add(bufr::kMsgKind, bufr::kDecodedSubs, meteo::app::OperationState_NORM, _bufrMsg.s3.kol_subsets);
    dataReady();

    messages_loaded++;
    _stpos += 4 ;

  }

#ifdef DEBUG_BUFR_PARSER
  debug_log<<"loaded "<<messages_loaded <<"BUFR messages";
#endif

  return messages_loaded;
}

int TBufr::loadBufr(QList<TMeteoData>* result, QString* error, const QDateTime& dt)
{
  int messages_loaded=0;
  _stpos =0;
  while ( -1 < (_stpos = _bufAll.indexOf("BUFR", _stpos))) {
    if ( (_stpos + getLength(_bufAll, _stpos+4) ) > _bufAll.size() ) {
      _stpos += 4;
      continue;
    }
    if ( !controlBufr() ) {
#ifdef PARSE_LOG
      debug_log << QObject::tr("Ошибка структуры BUFR");
#endif
      *error = QObject::tr("Ошибка структуры BUFR");
      _stpos += 4 ;
      continue;
    }
    //var(stpos);
    int res;
    if ( (res = getOneBufr()) != NO_ERRORS ) {
      _stpos += 4 ;
      if (res == EMPTY_BUFR) {
    messages_loaded ++;
      }
      continue;
    }

    QList<TMeteoData> meteodata;
    meteo::bufr::TransfType bt;
    if (_type == bufr::kRadarMap) {
      fill(&meteodata, &bt, true);
    } else {
      fill(&meteodata, &bt, false);
    }

    clearBM();

    for (int i = 0; i < meteodata.size(); i++) {
      meteodata[i].setDateTime(dt);
      uint mask = control::LIMIT_CTRL | control::CONSISTENCY_CTRL;
      if (!TMeteoControl::instance()->control(mask, &meteodata[i])) {
    *error = QObject::tr("Ошибка контроля");
      }

      // QByteArray buf;
      // meteodata.at(i) >> buf;
      result->append(meteodata.at(i));
    }

    messages_loaded++;

    _stpos += 4 ;

  }

  return messages_loaded;
}

//! Раскодирование одной сводки
/*!
  \return 0 - если всё хорошо
*/
int TBufr::getOneBufr()
{
  clearBM();

  int length = getLength(_bufAll, _stpos+4);
  if (length == 0) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Ошибка структуры") << sourceId();
#endif
    return OSH_BUFR_STRUCTURE;
  }

  QByteArray buf = _bufAll.mid(_stpos, length);
  //  var(length);

  _bufrMsg.s1.bufr_edition_number = OCT( buf[7] );
  _bufrMsg.sect[0] = buf.mid(0, START_SECT1);

  int fullLength = START_SECT1;

  for (int i=1; i < 5; i++ ) {
    if ( i==2 &&  !isSect2Exist(buf) ) {
      _bufrMsg.sect[i].clear();
      continue;
    }
    int curLength = getLength(buf, fullLength);

    if ( fullLength > length || curLength == 0 ) {
      clearBM();
#ifdef PARSE_LOG
      debug_log << QObject::tr("Ошибка структуры") << sourceId();
#endif
      return OSH_BUFR_STRUCTURE;
    }
    _bufrMsg.sect[i] = buf.mid(fullLength, curLength );
    fullLength += curLength;
  }

  fullLength += SECTION5_LENGHT;

  if ( fullLength != length ) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Ошибка структуры") << sourceId();
#endif
    clearBM();
    return OSH_BUFR_STRUCTURE;
  }

  return readSections(isSect2Exist(buf));
}

//! Разбор разделов BUFR
/*!
  \param sect2Exist наличие/отсутствие 2 раздела
  \return
*/
int TBufr::readSections(bool sect2Exist)
{
  int res=NO_ERRORS;

  if ( !section1(_bufrMsg.s1, _bufrMsg.sect[1])) {
    clearBM();
    return OSH_BUFR_STRUCTURE;
  }

  if (sect2Exist) {
    section2( _bufrMsg );
  }

  res = section3 (_bufrMsg.s3, _bufrMsg.sect[3] );
  if (res != NO_ERRORS) {
    return res;
  }

  if (_procCateg.contains(_bufrMsg.s1.type_data)) {
    _stat.add(meteo::bufr::StatKind(_bufrMsg.s1.type_data), bufr::kReceived, meteo::app::OperationState_NORM);
  } else {
    _stat.add(bufr::kMsgUnk, bufr::kReceived, meteo::app::OperationState_NORM);
    //debug_log << QObject::tr("Unk category %1, id = %2").arg(_bufrMsg.s1.type_data).arg(sourceId());
  }

  res = section4();

  return res;
}

//! Разбор раздела указателя
/*!
  \param s1 заполняемые параметры из радела
  \param buf массив байт с BUFR
  \return false - в случае ошибок
*/
bool TBufr::section1 ( Section1 &s1,const QByteArray &buf )
{
  switch (_bufrMsg.s1.bufr_edition_number) {
  case 3:
  case 2:
    s1.subcenter = OCT( buf[4] );
    s1.center    = OCT( buf[5] );
    s1.modify_number=OCT ( buf[6] );
    s1.type_data = OCT( buf[8] );
    s1.subtype_data    = OCT( buf[9] );
    s1.table_version   = OCT( buf[10] );
    s1.addtable_version= OCT ( buf[11] );
    s1.year  = OCT( buf[12] );
    s1.month = OCT( buf[13] );
    s1.day   = OCT( buf[14] );
    s1.hour  = OCT( buf[15] );
    s1.minute= OCT( buf[16] );
    s1.sec=0;
    break;
  case 4:
    s1.center    = OCT( buf[4] )*256 + OCT( buf[5] );
    s1.subcenter = (OCT( buf[6] ) <<8 ) + OCT( buf[7] );
    s1.modify_number = OCT( buf[8] );
    s1.type_data     = OCT( buf[10] );
    s1.subtype_data  = OCT( buf[11] );
    s1.table_version = OCT( buf[13] );
    s1.addtable_version=OCT ( buf[14] );
    s1.year  = OCT( buf[15] )*256 + OCT( buf[16] );
    s1.month = OCT( buf[17] );
    s1.day   = OCT( buf[18] );
    s1.hour  = OCT( buf[19] );
    s1.minute =OCT( buf[20] );
    s1.sec    =OCT( buf[21] );
    break;
  default: {
    debug_log<<QObject::tr("Версия bufr %1 не поддерживается").arg(_bufrMsg.s1.bufr_edition_number);
    return false;
  }

  }

  if (_bufrMsg.s1.year < 2000) {
    _bufrMsg.s1.year += 2000;
  }

#ifdef DEBUG_BUFR_PARSER
  debug_log<< "category" << s1.type_data;
#endif

  return true;
}

//! Раздел для локального использования, пропускаем
void TBufr::section2 ( BufrMessage & ) {
#ifdef DEBUG_BUFR_PARSER
  info_log << QObject::tr("В BUFR содержится раздел для локального использования")
       << "center =" << _bufrMsg.s1.center << _bufrMsg.s1.subcenter
       << "edition =" << _bufrMsg.s1.bufr_edition_number
       << "table =" << _bufrMsg.s1.table_version
       << "local =" << _bufrMsg.s1.addtable_version
       << sourceId();
#endif
}

//! Разбор раздела описания данных
/*!
  \param s3 заполняемые параметры из радела
  \param buf массив байт с BUFR
  \return 0 - успешно
*/
int TBufr::section3 ( Section3 &s3,const QByteArray &buf )
{
  s3.kol_subsets=OCT ( buf[4] ) *256+OCT ( buf[5] );
  s3.type_dan=OCT ( buf[6] ) >>7;
  s3.is_compressed= ( OCT ( buf[6] ) &64 ) >>6;
  s3.kol_descr= ( buf.size() - 7 ) /2;
  s3.descr.clear();

  // var(_bufrMsg.s1.type_data);

  for (int numSub = 0; numSub < _bufrMsg.s3.kol_subsets; numSub++) {
    for (int i=0, k=0; k<s3.kol_descr; k++, i=i+2 ) {
      s3.descr.append(BufrDescriptor (  (OCT ( buf[7+i] ))>>6, (OCT ( buf[7+i] ))&63, OCT ( buf[8+i] )) );
      if (numSub == 0) {
#ifdef DEBUG_BUFR_PARSER
    BufrDescriptor tmp = BufrDescriptor(  (OCT ( buf[7+i] ))>>6, (OCT ( buf[7+i] ))&63, OCT ( buf[8+i] ));
    debug_log << tmp.tableIndexStr();
    //debug_log <<((OCT ( buf[7+i] ))>>6) << ((OCT ( buf[7+i] ))&63) << OCT ( buf[8+i] );
#endif
      }
    }
  }

  if (s3.descr.size() == 0 || s3.kol_subsets ==0 || s3.descr.count() == 0 ) {
    //warning_log << QObject::tr("Пустая сводка");
    return EMPTY_BUFR;
  }

  return NO_ERRORS;
}

//! Разбор раздела данных
/*!
  \return 0 - успешно
*/
int TBufr::section4()
{
  int res=NO_ERRORS;

  if (!_tables.load(_bufrMsg.s1.bufr_edition_number, _bufrMsg.s1.center, _bufrMsg.s1.subcenter,
           _bufrMsg.s1.table_version, _bufrMsg.s1.addtable_version)) {
    return -1;
  }

  _bufrList=new TBufrList(0);

#ifdef DEBUG_BUFR_PARSER
  var(_bufrMsg.s3.kol_descr);
  var(_bufrMsg.s3.kol_subsets);
  var(_bufrMsg.s3.is_compressed);
#endif


  bool ok = fillBufrList();
  // var(ok);
  if (!ok) {
#ifdef DEBUG_BUFR_PARSER
    debug_log << ok;
    var("unknown");
#endif
    //saveUnknown();
    return NOT_PROCESSED;
  }
  // if (_bufrMsg.s1.type_data == 6) {
  //   checkRadarIfUnknown();
  // }


  if ( _bufrMsg.s3.is_compressed ) {
    if (!readCompressData()) res = -1;
  } else {
    if (!readUncompressData()) res = -1;
  }

#ifdef DEBUG_BUFR_PARSER
  debug_log << "data ready res=" << res;
#endif

  return res;
}

//! Выделение целочисленного значения из раздела данных
/*!
  \param astart смещение от начала данных, бит
  \param awidth размер, бит
  \param ret_val возвращаемое значение
  \return true - в случае успеха
*/
bool TBufr::getDataS4( int astart, int awidth, int &ret_val )
{
  return getBitData( _bufrMsg.sect[4], astart, awidth, ret_val );
}

//! Выделение строкового значения из раздела данных
/*!
  \param astart смещение от начала данных, бит
  \param awidth размер, бит
  \param ret_val возвращаемое значение
  \return true - в случае успеха
*/
bool TBufr::getDataS4( int astart, int awidth, QString& res )
{
  int cnt = awidth/8;
  int ret_val;
  bool eow = false;
  res = "";

  QByteArray ba;

  for (int i=0; i< cnt; i++) {
    if (!getBitData ( _bufrMsg.sect[4], astart +i*8, 8, ret_val )) return false;
    if (ret_val == 0) {
      eow = true;
    }
    if (ret_val != 255 && !eow) {
      ba += ret_val;
    }
  }

  res = _codec->toUnicode(ba);
  res = res.trimmed();

  return true;
}

//! Выделение значения из битового массива данных
/*!
  \param abuf    массив данных
  \param astart  смещение от начала данных, бит
  \param awidth  размер, бит
  \param ret_val возвращаемое значение
  \return true - в случае успеха
*/
bool TBufr::getBitData(const QByteArray &abuf, int astart, int awidth, int &ret_val)
{
  //считаем от 1 !

  ret_val=0;
  if ( (astart+awidth)-1 >  abuf.size()*SIZEOCT ) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Ошибка структуры данных: превышение размера при обращении к данным") << sourceId();
#endif
#ifdef DEBUG_BUFR_PARSER
    debug_log << "astart+awidth > abuf.size()*SIZEOCT"
          << (astart+awidth) << abuf.size()*SIZEOCT;
#endif

    return false;
  }

  for (int i=astart; i < astart+awidth; i++ ) {
    int no = NOCT( i );
    int nb = (i - SIZEOCT*( no-1 ));
    int h  = OCT(abuf.at(no-1));
    int z  = ( 1<< ( astart+awidth-i-1 ) ) * GETBIT(h, nb);
    ret_val+=z;
  }

  return true;
}

//! Для отладки. Сохранение неизвестных наборов BUFR
void TBufr::saveUnknown()
{
  //для отладки. сохраняем в файл, чтоб сохранить неизвестные последовательности
  QFile file(UNKNOWN_BUFR_SEQ_FILE);
  if (file.open(QIODevice::Append | QIODevice::Text)) {
    QTextStream out(&file);
    out << "source " << sourceId() << '\n';
    out << "category " << _bufrMsg.s1.type_data << '\n';
    out << "start\n";
    for (int i = 0; i < _bufrMsg.s3.kol_descr; i++) {
      out << _bufrMsg.s3.descr.at(i).tableIndex() << '\n';
    }
    out << "end\n";
    file.close();
  }
}



//! Раскодировка несжатых данных
/*!
  \return true - в случае успеха
*/
bool TBufr::readUncompressData()
{
  int res = 0;

  QVector<TBufrList*> curSub(1);
  curSub[0] = _bufrList;
  uint64_t dataPos = nachaloS4;

  //  var(dataPos);

  while (curSub[0]) {
    curSub[0]->setChop(_chop);
    _chop.setLocWidth = 0;
    QString curIdx = curSub[0]->descr()->tableIndexStr();

    // var(curIdx);
    // var(dataPos);

    switch (curSub[0]->descr()->F) {
    case 0:
      if (!unCompressedData(curSub[0], &dataPos)) {
    int index = 0;
    _bufrList->findIndex(&index);

    debug_log << QObject::tr("Ошибка чтения дескриптора или данных таблицы B") << curIdx
          << "center =" << _bufrMsg.s1.center << _bufrMsg.s1.subcenter
          << "edition =" << _bufrMsg.s1.bufr_edition_number
          << "table =" << _bufrMsg.s1.table_version
          << "local =" << _bufrMsg.s1.addtable_version
          << "station=" << index
          << sourceId();
    return false;
      }

      break;
    case 1:
      res=replication(curSub, &dataPos);
      if ( res != NO_ERRORS && res != NO_REPLICATIONS ) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Ошибка обработки дескриптора повторения") << sourceId() << "edition =" << _bufrMsg.s1.bufr_edition_number;
#endif
    return false;
      }
      break;
    case 2:
      if (0 != dataOperators(curSub, &dataPos)) {
#ifdef PARSE_LOG
        debug_log << QObject::tr("Ошибка обработки дескриптора оператора") << curSub[0]->descr()->tableIndexStr();
#endif
        return false;
      }
      break;
    case 3:
      if (0 != expandDescr(curSub)) {
    return false;
      }
      continue;
      break;
    default:
      return false;
    }

    if (curSub[0]->next() && curSub[0]->next()->subNum() != curSub[0]->subNum()) {
      _chop.clear();
    }

    curSub[0] = curSub[0]->next();

  }

  return true;
}

//! Раскодировка сжатых данных
/*!
  \return true - в случае успеха
*/
bool TBufr::readCompressData()
{
  int res = 0;

  QVector<TBufrList*> curSub = _subs;
  if (_subs.size() == 1) {
    _subs.append(0);
  }

  uint64_t dataPos = nachaloS4;

#ifdef DEBUG_BUFR_PARSER
  // var(_bufrMsg.s3.kol_descr);
  // var(curSub[0]);
  // var(_subs[1]);
#endif

  while (curSub[0] != _subs[1]) {

    for (int numSub = 0; numSub < _bufrMsg.s3.kol_subsets; numSub++) {
      curSub[numSub]->setChop(_chop);
    }
    _chop.setLocWidth = 0;
    QString curIdx = curSub[0]->descr()->tableIndexStr();

    switch ( curSub[0]->descr()->F ) {
    case 0:
      if (!compressedData(curSub, &dataPos)) {
	int index = 0;
	_bufrList->findIndex(&index);
	debug_log << QObject::tr("Ошибка чтения дескриптора или данных таблицы B") << curIdx
		  << "center =" << _bufrMsg.s1.center << _bufrMsg.s1.subcenter
		  << "edition =" << _bufrMsg.s1.bufr_edition_number
		  << "table =" << _bufrMsg.s1.table_version
		  << "local =" << _bufrMsg.s1.addtable_version
		  << "station=" << index;
	return false;
      }
           
      break;
    case 1:
      res=replication(curSub, &dataPos);
      if ( res != NO_ERRORS && res != NO_REPLICATIONS ) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Ошибка обработки дескриптора повторения") << sourceId();
#endif
    return false;
      }
      //continue;
      break;
    case 2:
      if (0 != dataOperators(curSub, &dataPos)) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Ошибка обработки дескриптора оператора") << curSub[0]->descr()->tableIndexStr();
#endif
    return false;
      }
      break;
    case 3:
      if (0 != expandDescr(curSub)) {
    return false;
      }
      continue;
      break;
    default:
      return false;
    }
    for (int numSub = 0; numSub < curSub.size(); numSub++) {
      curSub[numSub] = curSub[numSub]->next();
    }
    // var(curSub[0]);
    // var(_subs[1]);
  }

  return true;
}

//! Проверка является значение пропущенным (все биты установленны на '1')
/*!
  \param adata значение
  \param awidth размер, бит
  \return true - если значение считается пропущенным
*/
bool TBufr::isDataMissing ( int64_t adata, int awidth )
{
  return ( adata == ((2 << (awidth-1)) - 1) );
}


//! Извлечение значения, соответствующего дескриптору элемента в несжатых данных
/*!
  \param vs возвращаемый заполненный элемент
  \param pos смещение от начала данных до извлекаемого значения, бит
  \return true - в случае успеха
*/
bool TBufr::unCompressedData(TBufrList* vs, uint64_t* pos)
{
  int ret_val;
  QString ret_str;

  vs->set(_tables.table_b().value(vs->descr()->tableIndex()));
  if (vs->descr()->tableIndex() == 0) {
    return false;
  }

  if (vs->nBit() == 0) {
    return true;
  }

  if (vs->aBit() != 0) {
    if ( !getDataS4 ( *pos, vs->aBit(), ret_val ) ) {
      return false;
    }
    *pos += vs->aBit();
    vs->setAssoc(ret_val);
  }

  if (vs->isSymbol()) {
    if ( !getDataS4 ( *pos ,vs->nBit(),ret_str ) ) {
      return false;
    }
    *pos += vs->nBit();
    vs->setData(ret_str);
  } else {
    if ( !getDataS4 ( *pos, vs->nBit(), ret_val ) ) {
      return false;
    }
    *pos += vs->nBit();
    vs->setData(ret_val);
  }

  return true;
}

//! Обработка дескриптора повторения
/*!
  \param curSub указатель на дескриптор повторения в BUFR-списке
  \param pos смещение от начала данных до извлекаемого значения, бит
  \return 0 - если всё хорошо
*/
int TBufr::replication(QVector<TBufrList*>& curSub, uint64_t* pos)
{
  int res = NO_REPLICATIONS;
  TBufrList *vs = curSub[0];

  if (vs->isRetry()) return res;
  if (vs->descr()->F != 1) return res;

  if (vs->descr()->Y != 0) {
    res = simpleReplication(curSub);
  } else {
    res=delayedReplication(curSub, pos);
  }

  return res;
}

//! Обработка дескриптора повторения с задержкой
/*!
  \param curSub указатель на дескриптор в BUFR-списке
  \param pos смещение от начала данных до извлекаемого значения, бит
  \return 0 - если всё хорошо
*/
int TBufr::delayedReplication(QVector<TBufrList*>& curSub, uint64_t* pos)
{
  int kol_repl_descriptors=curSub[0]->descr()->X;

  if (kol_repl_descriptors == 0) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Ошибка дескриптора повторения. Количество дескрипторов, которое требуется повторить = 0");
#endif
    return -1;
  }

  int repl_descriptors_time = 0;

  for (int numSub = 0; numSub < curSub.size(); numSub++) {
    curSub[numSub]->setRetry();
    curSub[numSub] = curSub[numSub]->next();
    curSub[numSub]->setRetry();
    curSub[numSub]->set(_tables.table_b().value(curSub[numSub]->descr()->tableIndex()));
  }

  if ( !getDataS4 (*pos, curSub[0]->nBit(), repl_descriptors_time) ) {
    return OSH_BUFR_STRUCTURE;
  }


  (*pos) += curSub[0]->nBit();
  if (_bufrMsg.s3.is_compressed) {
    (*pos) += COUNT_BIT;
  }

  // var(repl_descriptors_time);

  for (int numSub = 0; numSub < curSub.size(); numSub++) {
    curSub[numSub]->setData(repl_descriptors_time, '0');
  }

  if (repl_descriptors_time == 0) {
    for (int numSub = 0; numSub < curSub.size(); numSub++) {
      if (! curSub[numSub]->del(curSub[numSub]->next(), kol_repl_descriptors)) {
        return -1;
      }
    }
    return 0;
  }

  for (int numSub = 0; numSub < curSub.size(); numSub++) {
    if (!curSub[numSub]->next()->setReplicationFlag(kol_repl_descriptors)) {
      return -1;
    }
  }

  if (repl_descriptors_time == 1) {
    return 0;
  }

  TBufrList* vs = curSub[0]->next();

#ifdef DEBUG_BUFR_PARSER
  // var(curSub[0]->descr()->tableIndexStr());
  // var(repl_descriptors_time);
  // var(kol_repl_descriptors);
#endif

  int err = 0;

  if (curSub[0]->descr()->Y == 11 || curSub[0]->descr()->Y == 12) {
    err =  delayReplicationData(kol_repl_descriptors, repl_descriptors_time, curSub, pos);
  } else {
    for (int numSub = 0; numSub < curSub.size(); numSub++) {
      //  var(numSub);
      vs = curSub[numSub];
      for ( int j=0;j<repl_descriptors_time-1;j++ ) {
    //  var(j);
    //      curSub[numSub]->insert(vs, mas_repl, vs->group());
    if (0 == vs->next()) {
#ifdef PARSE_LOG
      debug_log << QObject::tr("Требуется повторение %1 дескрипторов, тогда как в закодированном BUFR их %2").arg(repl_descriptors_time).arg(j + 1);
#endif
      return -1;
    }
    curSub[numSub]->insert(vs, vs->next(), kol_repl_descriptors);
      }
    }
  }

  return err;
}

//! Обработка повторения дескриптора и данных с задержкой
/*!
  \param descrCnt Количество дескрипторов
  \param replTimes Количество повторов
  \param curSub  указатель на дескриптор повторения в BUFR-списке
  \param pos  смещение от начала данных до извлекаемого значения, бит
  \return  - если всё хорошо
*/
int TBufr::delayReplicationData(int descrCnt, int replTimes, QVector<TBufrList*>& curSub, uint64_t* pos)
{
  if (_bufrMsg.s3.is_compressed) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Релизация только для несжатых данных");
#endif
    return -1;
  }

  TBufrList* prev = curSub[0];
  curSub[0] = curSub[0]->next(); //пропускаем дескриптор с коэффициентом повторения

  for (int num = 0; num < descrCnt; num++) {
    curSub[0]->setChop(_chop);
    _chop.setLocWidth = 0;
    QString curIdx = curSub[0]->descr()->tableIndexStr();

    switch (curSub[0]->descr()->F) {
    case 0: {
      if (!unCompressedData(curSub[0], pos)) {
    int index = 0;
    _bufrList->findIndex(&index);
    debug_log << QObject::tr("Ошибка чтения дескриптора или данных таблицы B") << curIdx
          << "center =" << _bufrMsg.s1.center << _bufrMsg.s1.subcenter
          << "edition =" << _bufrMsg.s1.bufr_edition_number
          << "table =" << _bufrMsg.s1.table_version
          << "local =" << _bufrMsg.s1.addtable_version
          << "station=" << index;
    return -1;
      }

      for ( int j = 0; j < replTimes-1; j++ ) {
    curSub[0]->insert(prev, prev->next(), 1, true);
      }

      //curSub[0] = curSub[0]->copy(curSub[0], replTimes-1, true);
    }
      break;
    case 2:
      if (0 != dataOperators(curSub, pos)) {
#ifdef PARSE_LOG
        debug_log << QObject::tr("Ошибка обработки дескриптора оператора") << curSub[0]->descr()->tableIndexStr();
#endif
        return -1;
      }
      break;
    default:
#ifdef PARSE_LOG
      debug_log <<  QObject::tr("Коэффициент повторения дескриптора и данных с задержкой должен использоваться для единичного элемента\n");
#endif
      return -1;
    }

    if (curSub[0]->next() && curSub[0]->next()->subNum() != curSub[0]->subNum()) {
      _chop.clear();
    }

    prev = curSub[0];
    if ( num != descrCnt-1) {
      if (0 == curSub[0]->next()) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Требуется повторение %1 дескрипторов, тогда как в закодированном BUFR их %2").arg(descrCnt).arg(num + 1);
#endif
    return -1;
      }
      curSub[0] = curSub[0]->next();
    }
  }

  return 0;
}

//! Обработка дескриптора повторения без задержки
/*!
  \param curSub указатель на дескриптор в BUFR-списке
  \param pos смещение от начала данных до извлекаемого значения, бит
  \return 0 - если всё хорошо
*/
int TBufr::simpleReplication(QVector<TBufrList*>& curSub)
{
  int kol_repl_descriptors = curSub[0]->descr()->X;
  int repl_descriptors_time = curSub[0]->descr()->Y;

  for (int numSub = 0; numSub < curSub.size(); numSub++) {
    curSub[numSub]->setRetry();
  }

  if (repl_descriptors_time == 0) {
    for (int numSub = 0; numSub < curSub.size(); numSub++) {
      curSub[numSub]->del(curSub[numSub]->next(), kol_repl_descriptors);
    }
    return 0;
  }

  for (int numSub = 0; numSub < curSub.size(); numSub++) {
    if (! curSub[numSub]->next()->setReplicationFlag(kol_repl_descriptors)) {
      return -1;
    }
  }

  if (repl_descriptors_time == 1) {
    return 0;
  }

  //QVector<int> mas_repl;
  TBufrList* vs = curSub[0]->next();

  // for ( int i=0;i<kol_repl_descriptors;i++ ) {
  //   mas_repl.append(vs->descr()->tableIndex());
  //   vs = vs->next();
  // }

  for (int numSub = 0; numSub < curSub.size(); numSub++) {
    vs = curSub[numSub];
    for ( int j=0;j<repl_descriptors_time-1;j++ ) {
      //curSub[numSub]->insert( vs, mas_repl, vs->group());
      if (0 == vs->next()) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Требуется повторение %1 дескрипторов, тогда как в закодированном BUFR их %2").arg(repl_descriptors_time).arg(j + 1);
#endif
    return -1;
      }
      curSub[numSub]->insert(vs, vs->next(), kol_repl_descriptors);
    }
  }

  return 0;
}


//! Обработка дескриптора оператора
/*!
  \param curSub указатель на дескриптор в BUFR-списке
  \param pos смещение от начала данных до извлекаемого значения, бит
  \return 0 - если всё хорошо
*/
int TBufr::dataOperators (QVector<TBufrList*>& curSub, uint64_t* pos)
{
  int sign,ret_val,res = NO_ERRORS;
  int Y = curSub[0]->descr()->Y;

  //  debug_log<<"F"<<2<<"X"<< curSub[0]->descr()->X<<"Y"<<Y;

  switch (curSub[0]->descr()->X) {
  case 1:
    if (Y==0) { _chop.addWidth = 0; }
    else {      _chop.addWidth = Y-128; }
    break;
  case 2:
    if (Y==0) { _chop.addScale = 0; }
    else {      _chop.addScale = Y-128; }
    break;
  case 3:
    if (Y == 255 || Y == 0) {
      _chop.setRefval=0;
      break;
    }
    if (!getDataS4(*pos, 1, sign)) {
      res= OSH_BUFR_STRUCTURE;
      break;
    }
    if (!getDataS4( (*pos) +1, Y-1, ret_val)) {
      res= OSH_BUFR_STRUCTURE;
      break;
    }
    *pos += Y;
    if (sign) _chop.setRefval = -ret_val;
    else _chop.setRefval = ret_val;
    break;
  case 4:
    if (Y==0) {
      _chop.aswidth = 0;
      //      chop.as_significance = 0;
      break;
    }
    _chop.aswidth = Y;


    break;
  case 5: {
    if (Y==0) {
      _chop.setCharWidth = 0;
      break;
    }
    _chop.setCharWidth = Y*8;

    QString ret_str;
    if ( !getDataS4 ( *pos, Y*8, ret_str) ) {
      return OSH_BUFR_STRUCTURE;
    }
    *pos += Y*8;
    for (int numSub = 0; numSub < curSub.size(); numSub++) {
      curSub[numSub]->setSymbol();
      curSub[numSub]->calcWidth();
      curSub[numSub]->setData(ret_str);
    }

  }
    break;
  case 6:
    _chop.setLocWidth = Y;

    break;
  case 7:
    _chop.addScale = Y;
    _chop.mulRef = pow(10, Y);
    _chop.addWidth = (10*Y + 2) / 3;
    break;
  case 8:
    _chop.setCharWidth = Y*8;
    break;
  case 21:
    _chop.noDataCnt = Y;
    break;
  case 22:
  case 23:
  case 24:
  case 25:
  case 32:
  case 35:
  case 36:
  case 37:
    break;
  default: {
#ifdef PARSE_LOG
    debug_log<<QObject::tr("Оператор X=%1 не поддерживается").arg(curSub[0]->descr()->X);
#endif
    return OSH_BUFR_STRUCTURE;
  }
  }

  return res;
}

//! Извлечение значения, соответствующего дескриптору элемента в сжатых данных
/*!
  \param curSub возвращаемый заполненный элемент в каждом наборе
  \param pos смещение от начала данных до извлекаемых значений, бит
  \return true - в случае успеха
*/
bool TBufr::compressedData(QVector<TBufrList*>& curSub, uint64_t* pos)
{
  for (int numSub = 0; numSub < curSub.size(); numSub++) {
    curSub[numSub]->set(_tables.table_b().value(curSub[0]->descr()->tableIndex()));
  }

  if (curSub[0]->descr()->tableIndex() == 0) {
    return false;
  }

  if (curSub[0]->nBit() == 0) {
    return true;
  }

  if (curSub[0]->aBit() != 0) {
    getCompressDigit(curSub, pos, true);
  }

  if (curSub[0]->isSymbol()) {
    return getCompressStr(curSub, pos);
  }

  return getCompressDigit(curSub, pos, false);
}

//! Извлечение строкового значения, соответствующего дескриптору элемента в сжатых данных
/*!
  \param curSub возвращаемый заполненный элемент в каждом наборе
  \param pos смещение от начала данных до извлекаемых значений, бит
  \return true - в случае успеха
*/
bool TBufr::getCompressStr(QVector<TBufrList*>& curSub, uint64_t* pos)
{
  QString min_str, ret_str;
  int obs_width = 0;
  int nb = curSub[0]->nBit();

  //  var(curSub[0]->descr()->tableIndexStr());
  //debug_log<<(*pos)<<curSub[0]->isSymbol();

  if ( !getDataS4(*pos, nb, min_str) ) {
    return false;
  }
  //var(min_str);

  (*pos) += nb;

  if ( !getDataS4 (*pos, COUNT_BIT, obs_width) ) {
    return false;
  }
  (*pos) += COUNT_BIT;
  //  var(obs_width);

  for (int subNum = 0; subNum < curSub.size(); subNum++) {
    if (obs_width != 0) {
      if ( !getDataS4 (*pos, obs_width*8, ret_str) ) {
    return false;
      }
      //	var(ret_str);
      curSub[subNum]->setData (ret_str);
    } else {
      curSub[subNum]->setData (min_str);
    }
    (*pos) += obs_width * 8;
  }

  return true;
}

//! Извлечение численного значения, соответствующего дескриптору элемента в сжатых данных
/*!
  \param curSub возвращаемый заполненный элемент в каждом наборе
  \param pos смещение от начала данных до извлекаемых значений, бит
  \return true - в случае успеха
*/
bool TBufr::getCompressDigit(QVector<TBufrList*>& curSub, uint64_t* pos, bool readAssoc)
{
  int min_val = 0, ret_val = 0;
  int obs_width = 0;
  int nb = curSub[0]->nBit();

  //  var(curSub[0]->descr()->tableIndexStr());
  //debug_log<<(*pos)<<curSub[0]->isSymbol();


  if ( !getDataS4(*pos, nb, min_val) ) {
    return false;
  }
  //var(min_val);

  (*pos) += nb;

  if ( !getDataS4 (*pos, COUNT_BIT, obs_width) ) {
    return false;
  }
  (*pos) += COUNT_BIT;
  //  var(obs_width);

  for (int subNum = 0; subNum < curSub.size(); subNum++) {
    char qual='0';
    if (obs_width != 0) {
      if ( !getDataS4 (*pos, obs_width, ret_val) ) {
    return false;
      }
      if (isDataMissing(ret_val, obs_width)) {
    qual = '1';
      }
      //	debug_log<<subNum<<ret_val;
    } else {
      if (isDataMissing(min_val, nb)) {
    qual = '1';
      }
    }
    if (readAssoc) {
      curSub[subNum]->setAssoc(ret_val+min_val);
    } else {
      curSub[subNum]->setData(ret_val+min_val, qual);
    }
    //      var(curSub[subNum]->descr()->tableIndexStr());

    // var(curSub[subNum]);
    // var(curSub[subNum]->next());
    // curSub[subNum] = curSub[subNum]->next();
    (*pos) += obs_width;
  }

  return true;
}



//! Обработка дескриптора последовательности
/*!
  \param curSub указатель на дескриптор в BUFR-списке
  \return 0 - если всё хорошо
*/
int TBufr::expandDescr(QVector<TBufrList*>& curSub)
{
  QVector<int> dep_vec = _tables.table_d().value(curSub[0]->descr()->tableIndex());
  if (dep_vec.size() < 1) {
    int index = 0;
    _bufrList->findIndex(&index);

    debug_log << QObject::tr("Нет дескриптора в таблице D") << curSub[0]->descr()->tableIndexStr()
          << "center =" << _bufrMsg.s1.center << _bufrMsg.s1.subcenter
          << "edition =" << _bufrMsg.s1.bufr_edition_number
          << "table =" << _bufrMsg.s1.table_version
          << "local =" << _bufrMsg.s1.addtable_version
          << "station=" << index;
    return OSH_BUFR_STRUCTURE;
  }

  for (int numSub = 0; numSub < curSub.size(); numSub++) {
    curSub[numSub]->replace(curSub[numSub], dep_vec, _transf->bufrExpand(curSub[numSub]->descr()->tableIndex()));
  }
  return 0;
}

//! Получение размера BUFR
/*!
  \param abuf1 массив с данными
  \param st смещение до начала BUFR данных
  \return размер BUFR
*/
int TBufr::getLength ( const QByteArray &abuf1,int st )
{
  if (st + 2 >= abuf1.size()) {
    return 0;
  }
  return OCT(abuf1[st])*256*256 + OCT(abuf1[st+1])*256 + OCT(abuf1[st+2]);
}

//! Проверка наличия 2 раздела
/*!
  \param abuf массив с BUFR
  \return true - есть 2 раздел, false - нет
*/
bool TBufr::isSect2Exist ( const QByteArray & abuf)
{
  if (_bufrMsg.s1.bufr_edition_number == 4) {
    return ((uchar)abuf[START_SECT1+9] == 0x80);
  }

  return ( (uchar)abuf[START_SECT1+7] == 0x80 );
}

//! Проверка размера BUFR
/*!
  \return true - всё хорошо
*/
bool TBufr::controlBufr()
{
  int f = _stpos + getLength(_bufAll, _stpos+4) - 4; //ожидаемая концовка
  if (_bufAll.mid(f, 4) == QByteArray("7777")) {
    return true;
  }
  return false;
}

//! Вызывается при готовности данных. Здесь для отладочной информации.
void  TBufr::dataReady()
{
  // trc;
  // var(_type);
  _bufrList->print();

  return;
}

//! Преобразование данных BUFR в TMeteoData
/*!
  \param data результирующий массив
  \param type способ заполнения
  \param onlyHeader без данных карты, для радиолокации
*/
void TBufr::fill(QList<TMeteoData>* data, meteo::bufr::TransfType* type, bool onlyHeader /* = false */)
{
  if (nullptr == data || nullptr == type) return;
  //  _bufrList->print();
  *type = _type;

  // var(_type);
  if (_type == bufr::kTransfTypeUnk) {
    return;
  }

  if (_type == bufr::kMeteoData) {
    _bufrList->fill(*data, _transf->bufrConvert(), _dataType);
  } else {
    _bufrList->fillRadar(*data, _transf->bufrConvert(), onlyHeader);
    if (data->size() == 0) {
      debug_log << "Empty meteo from radar id=" << sourceId();
    }
    for (int i=0; i < data->size(); i++) {
      QDateTime dt = TMeteoDescriptor::instance()->dateTime(data->at(i));
      if (!dt.isValid()) {
	dt = dateTime();
	(*data)[i].setDateTime(dt);
      }
      if (!data->at(i).hasParam(TMeteoDescriptor::instance()->descriptor("iii"))) {
	QString shortn = TMeteoDescriptor::instance()->stationShortName(data->at(i));
	bool iok;
	int stationIndex = shortn.toInt(&iok);
	if (iok) {
	  (*data)[i].set(TMeteoDescriptor::instance()->descriptor("II"),
			 TMeteoParam("", int(stationIndex / 1000), control::RIGHT));
	  (*data)[i].set(TMeteoDescriptor::instance()->descriptor("iii"),
			 TMeteoParam("", stationIndex % 1000, control::RIGHT));
	}
      }
    }
  }
  
  for (int i=0; i < data->size(); i++) {
    int fullIndex = TMeteoDescriptor::instance()->station(data->at(i), -1);
    if (fullIndex > 0) {
      (*data)[i].set(TMeteoDescriptor::instance()->descriptor("station_index"),
             TMeteoParam("", fullIndex, control::RIGHT));
    }
    int buoy = TMeteoDescriptor::instance()->buoy(data->at(i), -1);
    if (buoy != -1) {
      (*data)[i].set(TMeteoDescriptor::instance()->descriptor("buoy"),
          TMeteoParam("", buoy, control::RIGHT));
    }

    (*data)[i].add(TMeteoDescriptor::instance()->descriptor("category"),
		   "",  _dataType, control::RIGHT);


    int mountainLev = TMeteoDescriptor::instance()->mountainLevel((*data)[i]);
    if (BAD_METEO_ELEMENT_VAL != mountainLev) {
      (*data)[i].set(TMeteoDescriptor::instance()->descriptor("ims"),
		     TMeteoParam("", 1, control::RIGHT));
    }

    int sat = (*data)[i].getValue(TMeteoDescriptor::instance()->descriptor("sat"), BAD_METEO_ELEMENT_VAL, false);
    if (BAD_METEO_ELEMENT_VAL != sat) {
      (*data)[i].set(TMeteoDescriptor::instance()->descriptor("sat"),
		     TMeteoParam(_tables.table_c5().value(sat), sat, control::RIGHT));
    }    

    int instr = (*data)[i].getValue(TMeteoDescriptor::instance()->descriptor("instr"), BAD_METEO_ELEMENT_VAL, false);
    if (BAD_METEO_ELEMENT_VAL != sat) {
      (*data)[i].set(TMeteoDescriptor::instance()->descriptor("instr"),
		     TMeteoParam(_tables.table_c5().value(instr), instr, control::RIGHT));
    }    

    
    TMeteoDescriptor::instance()->fillTypeLevel(&(*data)[i]);
  }
}

//! Удаление "плохих" данных (без идентификации или даты/времении)
/*!
  \param data массив с данными
*/
void TBufr::removeBad(QList<TMeteoData>& data)
{
#ifdef DEBUG_BUFR_PARSER
  trc;
  debug_log << data.count();
#endif

  bool ok = false;

  QMutableListIterator<TMeteoData> it(data);
  while (it.hasNext()) {
    ok = true;
    TMeteoData& cur = it.next();
    if (cur.count() <= 1) {
      ok = false;
    } else {
      QDateTime dt = TMeteoDescriptor::instance()->dateTime(cur);
      if (!dt.isValid()) {
    ok = false;
#ifdef PARSE_LOG
    debug_log << QObject::tr("Ошибка даты в данных");
#endif
      }
    }

    if (!ok) {
      it.remove();
    }
 }
}

//! Получение даты/времени формирования BUFR
/*!
  \return дата/время из 1 раздела
*/
QDateTime TBufr::dateTime()
{
  QDate date(_bufrMsg.s1.year, _bufrMsg.s1.month, _bufrMsg.s1.day);
  QTime time(_bufrMsg.s1.hour, _bufrMsg.s1.minute, _bufrMsg.s1.sec);

#ifdef DEBUG_BUFR_PARSER
  debug_log << "date="<< _bufrMsg.s1.year << _bufrMsg.s1.month << _bufrMsg.s1.day;
  debug_log << "time=" << _bufrMsg.s1.hour << _bufrMsg.s1.minute << _bufrMsg.s1.sec;
#endif
  return QDateTime(date, time);
}

//! без определения типа данных
bool TBufr::fillSimple()
{
  if ( _bufrMsg.s3.is_compressed ) {
    _subs.resize(_bufrMsg.s3.kol_subsets);

    for (int numSub = 0; numSub < _bufrMsg.s3.kol_subsets; numSub++) {
      for (int k=0; k< _bufrMsg.s3.kol_descr; k++) {
    _bufrList->add(_bufrMsg.s3.descr[k], numSub, _bufrMsg.s3.descr[k].tableIndex(), -1);
    if (k == 0) {
      _subs[numSub] = _bufrList->last();
    }
      }
    }

  } else {

    for (int numSub = 0; numSub < _bufrMsg.s3.kol_subsets; numSub++) {
      for (int k=0; k<_bufrMsg.s3.kol_descr; k++) {
    _bufrList->add(_bufrMsg.s3.descr[k], numSub, _bufrMsg.s3.descr[k].tableIndex(), -1);
      }
    }
  }

  _type = bufr::kMeteoData;

  return true;
}

//! с определением типа данных
bool TBufr::fillSequence()
{
  meteo::surf::DataType category = _transf->findType(_bufrMsg.s3.descr, _bufrMsg.s3.kol_descr, _bufrMsg.s1.type_data);

  if (category > meteo::surf::kUnknownDataType ||
      ! _procTypes.contains(category)) {
    debug_log << "not processed categ="  << _bufrMsg.s1.type_data << category << sourceId();
    return false;
  }

  if (category == meteo::surf::kUnknownDataType) {
#ifdef PARSE_LOG
    debug_log << QObject::tr("Не определён тип данных bufrCateg=%1 source=%2").arg(_bufrMsg.s1.type_data).arg(sourceId());
    for (int idx = 0; idx <  _bufrMsg.s3.kol_descr; idx++) {
      debug_log << _bufrMsg.s3.descr.at(idx).tableIndexStr();
    }
#endif

    return false;
  }

    // for (int idx = 0; idx <  _bufrMsg.s3.kol_descr; idx++) {
    //   debug_log << _bufrMsg.s3.descr.at(idx).tableIndexStr();
    // }
    // var(category);
    // var(_bufrMsg.s1.type_data);
  // if (category == meteo::surf::kAirplaneType) {
  //   debug_log << "not processed categ (airplane)=" << _bufrMsg.s1.type_data << sourceId();
  //   return false;
  // }

#ifdef DEBUG_BUFR_PARSER
  var(_bufrMsg.s1.type_data);
  var(category);
#endif

  if ( _bufrMsg.s3.is_compressed ) {
    _subs.resize(_bufrMsg.s3.kol_subsets);
    
    for (int numSub = 0; numSub < _bufrMsg.s3.kol_subsets; numSub++) {
      for (int k=0; k< _bufrMsg.s3.kol_descr; k++) {
	_bufrList->add(_bufrMsg.s3.descr[k], numSub, _bufrMsg.s3.descr[k].tableIndex(), -1);
	if (k == 0) {
	  _subs[numSub] = _bufrList->last();
	}
      }
    }
    
  } else {

    for (int numSub = 0; numSub < _bufrMsg.s3.kol_subsets; numSub++) {
      for (int k=0; k<_bufrMsg.s3.kol_descr; k++) {
	_bufrList->add(_bufrMsg.s3.descr[k], numSub, _bufrMsg.s3.descr[k].tableIndex(), -1);
      }
    }
  }

  _dataType = category;

  return true;
}

bool TBufr::fillBufrList()
{
  bool ok = false;
  //#ifdef DEBUG_BUFR_PARSER
  // var(_bufrMsg.s1.type_data);
  //#endif
  
  if (!_procCateg.contains(_bufrMsg.s1.type_data)) {
    debug_log << "not processed categ=" << _bufrMsg.s1.type_data << sourceId();
    // fillSimple(); //TODO для отладки
    // ok = true;
    return false;
  }
  
  ok = fillSequence();

  _type = bufr::kMeteoData;
  if (_dataType == meteo::surf::kRadarMapType) {
    _type = bufr::kRadarMap;
  }
  
  return ok;
}
