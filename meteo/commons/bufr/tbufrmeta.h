#ifndef BUFR_META_DATA_H
#define BUFR_META_DATA_H

#include <qstring.h>
#include <qhash.h>
#include <qmap.h>
#include <qvector.h>
#include <float.h>

#define SYMBOL_UNIT "CCITTIA5"

//#define DEBUG_BUFR_PARSER
//#define PARSE_LOG

enum TBufrErrors
{
  NO_ERRORS = 0,
  NO_FILE,
  FILE_STATS_ERROR,
  NOT_BINARY,
  NOT_TEXT,
  OSH_BUFR_STRUCTURE,
  OSH_TABLE_B_STRUCTURE,
  NO_INDEX_FOUND,
  EMPTY_BUFR,
  NOT_PROCESSED
};

//! операторы BUFR
struct ChangeOperator {
  short addWidth;   	 //!< Изменение длины данных
  short addScale;   	 //!< Изменение масштаба данных
  short setRefval;  	 //!< Изменение начала отсчёта (установка)
  short mulRef;     	 //!< Изменение начала отсчёта (умножение)
  short aswidth;    	 //!< Добавить ассоциированное поле
  //short addCharWidth;  //!< Назначить символы
  short setLocWidth;     //!< Обозначить длину данных для немедленно следующего локального дескриптора
  short setCharWidth;    //!< Изменить длину поля MKKTT MA5 
  short noDataCnt;       //!< Данные отсутствуют
  //int adwidth; //������ ���������� (��������� � ������ 2) ������, �������� � 2 05 ���

  void clear() {
    addWidth = 0;
    addScale = 0;
    setRefval = 0;
    mulRef = 1;
    aswidth = 0;
    // addCharWidth = 0;
    setLocWidth  = 0;
    setCharWidth = 0;
    noDataCnt = 0;
  }

  ChangeOperator() {
    clear();
  }

  ChangeOperator(const ChangeOperator& other) = default; // {
  //   addWidth  = other.addWidth;
  //   addScale  = other.addScale;
  //   setRefval = other.setRefval;
  //   mulRef  = other.mulRef;
  //   aswidth = other.aswidth;
  //   setLocWidth  = other.setLocWidth;
  //   setCharWidth = other.setCharWidth;
  //   noDataCnt = other.noDataCnt;
  // }

};


//! Дескриптор в виде F X Y
struct BufrDescriptor {
  uint8_t F;
  uint8_t X;
  ushort Y;
  int tableIndex() const {
    return F*100000+X*1000+Y;
  }
  
  QString tableIndexStr() const  {
    return  QString::number(tableIndex()).rightJustified(6,'0');
  }
  BufrDescriptor() {
    F=0;
    X=0;
    Y=0;
  }
  BufrDescriptor(int af, int ax, int ay) {
    F= af;
    X= ax;
    Y= ay;
  }
  BufrDescriptor(int aindex) {
    setIndex(aindex);
  }
  BufrDescriptor(const BufrDescriptor& other) = default; // {
  //   F= other.F;
  //   X= other.X;
  //   Y= other.Y;
  // }
  
  void setIndex(int aindex){
    F=aindex/100000;
    X= ( aindex-F*100000 ) /1000;
    Y=aindex-F*100000-X*1000;
  }
};

//! Описатель данных (таблица B)
struct DataDescriptor {
  QString name_elem;		//!< Название элемента
  QString unit;			//!< Единицы измерения
  int scale;			//!< Масштаб
  int ref_value;		//!< Начало отсчёта
  ushort width;			//!< Длина данных, бит
  BufrDescriptor descriptor;	//!< Дескриптор
  ~DataDescriptor() {
  }
  
  DataDescriptor() {
    scale=0;
    ref_value=0;
    width=0;
  }
};

//! Данные, соответствующие одному дескриптору
struct BufrData 
{
  float fdata;			//!< Значение величины, если численное
  char quality;			//!< Показатель качества
  QString sdata;		//!< Значение величины, если строковое
  uchar assoc; 			//!< Ассоциированное поле

  void set(float afdata, char qual='0') {
    fdata = afdata;
    quality = qual;
  }

  void set(const QString& str, char qual='0') {
    sdata = str;
    quality = qual;
  }
  
  void setAssoc(uchar aassoc) {
    assoc = aassoc;
  }

  void clear() {
    fdata=FLT_MAX;
    quality = '1';
    assoc = 255;
    sdata = QString();
  }

  BufrData() {
    clear();
  }
};

//! Метаданные BUFR (таблицы B и D)
class BufrMetaData {
public:  
  bool load(int edition, int center, int subcenter, int mastertable, int localtable);

  const QHash<int, DataDescriptor > &table_b() {return _table_b;};
  const QHash<int, QVector<int> > &table_d()   {return _table_d;};
  const QMap<int, QString > &table_c8()   {return _table_c8;};
  const QMap<int, QString > &table_c5()   {return _table_c5;};
  
private:  
  bool loadTableB(const QString &filename);
  bool loadTableD(const QString &filename);
  bool loadTableC5(const QString &filename);
  bool loadTableC8(const QString &filename);
  bool loadTableDNewVers(QStringList lines);

  void getVersName(int edition, int subcenter, int center, int mastertable, int localtable, char alpha, QStringList& tableList);
  QString findTableFile(int edition, int center, int subcenter, int mastertable, int localtable, char tableSym);

  int setTableB(const QStringList &list);
  bool loadTextBuffer(const QString &filename,  QStringList* list);

private:
  
  QString _tableB;		//!< Название загруженной таблицы B
  QString _tableD;		//!< Название загруженной таблицы D
  QHash<int,DataDescriptor > _table_b; //!< Таблица B
  QHash<int,QVector<int> > _table_d;   //!< Таблица D
  QMap<int, QString> _table_c5;   //!< Таблица C5
  QMap<int, QString> _table_c8;   //!< Таблица C8

};

#endif
