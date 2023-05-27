#ifndef TMETEO_COMMONS_BUFR_H
#define TMETEO_COMMONS_BUFR_H

#include <meteo/commons/bufr/tbufrmeta.h>
#include <meteo/commons/bufr/tbufrtransform.h>
#include <meteo/commons/services/decoders/tservicestat.h>
#include <meteo/commons/proto/bufr.pb.h>

#include <QString>
#include <QStringList>
#include <QFile>
#include <qdatetime.h>
#include <qtextcodec.h>

#include <math.h>
#include <stdint.h>

namespace meteo {
  namespace app {
    class AppState_OperationStatus;
  }
}

class QTextCodec;

class TBufrList;
class TBufrTransform;
class TMeteoData;

namespace meteo {
  namespace bufr {
    //! Состояние раскодировщика BUFR
    enum StatType {
      kReceivedUnk,     //!< Принятно, но это не BUFR
      kReceived,	//!< Количество полученных BUFR
      kDecoded,		//!< Количество раскодированных BUFR
      kDecodedSubs,     //!< Количество раскодированных сводок
      kErrStructure     //!< Ошибка структуры BUFR
    };

    enum StatKind {
      kMsgKind = -1, //!< Все категории
      kMsgUnk  = 256 //!< Не обрабатываемая категория
    };

    //! Тип данных BUFR, для указания к чему их можно преобразовать
    enum TransfType {
      kTransfTypeUnk =  -1,
      kMeteoData = 0, //!< набор пар дескриптор-значение (TMeteoData)
      kRadarMap  = 1  //!< данные радара на карте
    };
    

  }
}

//! Раздел 3 BUFR
struct Section3 {
  int kol_subsets;		//!< количество поднаборов
  int type_dan;			//!< тип данных (наблюдения, другие)
  int is_compressed;		//!< сжатые или нет
  int kol_descr;		//!< количество дескрипторов
  QList<BufrDescriptor> descr;	//!< список дескрипторов
};

//! Раздел 1 BUFR
struct Section1 {
  int bufr_edition_number;	//!< Номер издания BUFR
  int center;			//!< Идентификация центра — поставщика/производителя продукции
  int subcenter;		//!< Идентификация подцентра
  int modify_number;		//!< Последовательный номер обновления
  int type_data;		//!< Категория данных
  int subtype_data;		//!< Международная подкатегория данных
  int table_version;		//!< Номер версии эталонной таблицы
  int addtable_version;		//!< Номер версии местных таблиц,таблиц
  int year;			//!< Год
  int month;			//!< Месяц
  int day;			//!< День
  int hour;			//!< Час
  int minute;			//!< Минута
  int sec;			//!< Секунда
};

//! Сообщение BUFR
struct BufrMessage {
  QByteArray sect[5];		//!< Двоичные данные из раздела данных
  Section1 s1;			//!< Раздел 1
  Section3 s3;			//!< Раздел 3
  void clear() {
    s3.descr.clear();
  }
};

//! Декодирование данных в формате BUFR
class TBufr
{
public:
  TBufr();
  virtual ~TBufr();

  int decode(const QMap<QString, QString>& type, const QByteArray& ba);
  // int decode(const QByteArray& ba, QList<QByteArray>* result, QString* error, const QDateTime& dt = QDateTime()); 
  int decode(const QByteArray& ba, QList<TMeteoData>* result, QString* error, const QDateTime& dt = QDateTime()); 
  int decode(const QString &filename);

  meteo::bufr::TransfType type() { return _type; }

  void fill(QList<TMeteoData>* data, meteo::bufr::TransfType* type, bool onlyHeader = false);
  void removeBad(QList<TMeteoData>& data);
  QDateTime dateTime();

  const service::Status<meteo::bufr::StatKind, meteo::bufr::StatType>& status() const { return _stat; }

  int seqNumber() { return _seqNumber; }

  void setCodec(QTextCodec* c) { _codec = c; }
  bool setCodec(const QByteArray& name);

protected:
  virtual void dataReady();
  virtual QString sourceId() { return ""; }
  void clearBM();

private:

  int loadBufr();
  int loadBufr(QList<TMeteoData>* result, QString* error, const QDateTime& dt);
  int decode();
  
  bool controlBufr();
  int getLength (const QByteArray &,int );
  int getOneBufr();

  int readSections(bool sect2Exist);
  bool isSect2Exist ( const QByteArray & );
  bool section1 ( Section1 &s1,const QByteArray & );
  void section2 ( BufrMessage & );
  int section3 ( Section3 &s3,const QByteArray & );
  int section4();

  bool getDataS4 ( int ast,int awidth,int & );
  bool getDataS4 ( int astart,int awidth, QString& res );
  bool getBitData ( const QByteArray &abuf,int astart,int awidth,int &ret_val );

  bool isDataMissing (int64_t adata, int awidth );
  int isBitOrderCompress ( TBufrList *&vs );

  bool getCompressDigit(QVector<TBufrList*>& curSub, uint64_t* pos, bool readAssoc);
  bool getCompressStr(QVector<TBufrList*>& curSub, uint64_t* pos);
  bool compressedData(QVector<TBufrList*>& curSub, uint64_t* pos);
  bool readCompressData();
  bool readUncompressData();
  int replication(QVector<TBufrList*>& curSub, uint64_t* pos);
  int simpleReplication(QVector<TBufrList*>& curSub) ;
  int delayedReplication(QVector<TBufrList*>& curSub, uint64_t* pos);
  int delayReplicationData(int descrCnt, int replTimes, QVector<TBufrList*>& curSub, uint64_t* pos);
  int expandDescr(QVector<TBufrList*>& curSub);
  int dataOperators(QVector<TBufrList*>& curSub, uint64_t* pos);
  bool unCompressedData(TBufrList* vs, uint64_t* pos);

  bool fillBufrList();
  bool fillSimple();
  bool fillSequence();

  void saveUnknown();
private:

  BufrMessage _bufrMsg; //!< Обрабатываемое сообщение из _bufAll;
  QByteArray _bufAll;   //!< Двоичные данные, содержащие BUFR-сообщения

  TBufrList* _bufrList = nullptr;      //!< Список дескрипторов BUFR
  QVector<TBufrList*> _subs; //!< Ссылки на те места table_list, где начинаются подсекции
  ChangeOperator _chop;      //!< Текущие операторы
  int _stpos = 0;                //!< Текущая позиция в двоичных данных
  BufrMetaData _tables;      //!< Таблицы BUFR

  TBufrTransform* _transf;   //!< Класс для преобразования к виду TMeteoData
  QList<int> _procCateg; //!< Обрабатываемые категории
  QList<meteo::surf::DataType> _procTypes; //!< Обрабатываемые типы данных
  
  meteo::bufr::TransfType _type = meteo::bufr::kTransfTypeUnk; //!< Тип данных BUFR, для указания к чему их можно преобразовать
  int _seqNumber = -1; //!< номер текущей последовательности
  meteo::surf::DataType _dataType;

  service::Status<meteo::bufr::StatKind, meteo::bufr::StatType> _stat;
  
  QTextCodec* _codec; //!< Кодек для преобразования символов
};

#endif

