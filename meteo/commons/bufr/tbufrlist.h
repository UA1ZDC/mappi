#ifndef TTABLELIST_H
#define TTABLELIST_H

#include "tbufrmeta.h"
#include <commons/meteo_data/meteo_data.h>

#include <qvector.h>

class TMeteoData;
class BufrExpand;
class BufrConvert;


//! Раскодированный BUFR
/*! 
  Представляет собой список, каждый элемент которого содержит дескриптор BUFR, описание величины, ему
  соответстующей и саму величину  
*/
class TBufrList  {
public:
  TBufrList(uint subNum);
  TBufrList(const TBufrList& vs); //копия описания
  ~TBufrList();

  void add ( const BufrDescriptor &descr, uint subNum, int meteoDescr, int group);
  bool insert(TBufrList* parent, TBufrList* first, uint cnt, bool withData = false);
  bool replace(TBufrList* parent, QVector<int>& atable_index, const BufrExpand& exp);
  TBufrList* copy(TBufrList* item, uint cnt, bool withData = false);
  bool del( TBufrList *vs, uint cnt);

  void set (const DataDescriptor &adescr );
  void set ( int atable_index );

  void clearDelete();


  TBufrList *first();
  TBufrList *last();
  TBufrList *prev()  { return _prev; }
  TBufrList *next()  { return _next; }

  BufrDescriptor *descr() { return & _bufrDescr.descriptor; }
  const DataDescriptor& bufrDescr() { return _bufrDescr; }
  int group() { return _group; }

  void setChop(const ChangeOperator& ch) { _chop = ch; }
  const ChangeOperator& getChop()        { return _chop; }

  int nBit()    { return _w; }
  int aBit()    { return _wAssoc; }
  uint subNum() { return _subNum; }

  void setData (int adata, char);
  void setData (int adata);
  void setData(const QString& adata);
  void setAssoc(int assoc) { _data.assoc = assoc; }
  

  bool isRetry() const  { return _isRetry; }
  void setRetry() { _isRetry = true; }

  void setSymbol() { _bufrDescr.unit = SYMBOL_UNIT; }
  bool isSymbol() const  { return (_bufrDescr.unit == SYMBOL_UNIT || _bufrDescr.unit == "CCITT IA5" ); }

  bool setReplicationFlag(int cnt);

  void calcWidth();


  void print();
  // void fill(QList<TMeteoData>& dataList, const QHash<uint, BufrConvert>& bDescr, 
  // 	    const QHash<uint, QMap<int, uint> >& signGroup);
  void fillRadar(QList<TMeteoData>& dataList, const QHash<uint, BufrConvert>& bDescr, bool onlyHeader);
  void fill(QList<TMeteoData>& dataList, const QHash<uint, BufrConvert>& bDescr, int category);

  void findIndex(int* index);

private:
  bool add ( int atable_index, uint numSub, int descr = -1, int group = 0);
  //  bool insert(TBufrList* parent, int atable_index);
  
  //  int table_index() { return   _bufrDescr.descriptor.tableIndex(); }
  
  void set (const BufrDescriptor &adescr );
  void clear();

  bool isIdentDataDescr(const BufrDescriptor* descr) const;
  //  bool isMeteoParamDescr(const BufrDescriptor* descr) const;
  bool isMeteoDataDescr(const BufrDescriptor* descr) const;
  bool isReplacedDescr(TBufrList* start, descr_t descr, const QList<descr_t>& childsIdent) const;
  bool isCanceledDescr(const BufrDescriptor* descr) const;
  bool isMapData(const BufrDescriptor* descr, const DataDescriptor& bufrDescr, float value, bool* isEmpty) const;
  int  linkedDescrCount(TBufrList* vs) const;
  bool isNeedIsolate(const BufrDescriptor* descr, QList<descr_t> childsIdent) const;

  void setValue(const TBufrList* vs, QString* strVal, float* val, control::QualityControl* qual) const;
  void convertBDescr(const QHash<uint, BufrConvert>& bDescr, TBufrList* vs);
  // void setCurrentSignGroup(const QHash<uint, QMap<int, uint> >& signGroup,  const TBufrList* vs, 
  // 			   QList<int>* curSignGroup) const;
  
private:
  ChangeOperator _chop;		//!< Оператор BUFR
  ushort _subNum; 		//!< Номер поднабора (в сжатых данных)
  bool _isRetry;		//!< true - если дескриптор повторения
  bool _isSymbol;		//!< true - если строковое (перенести в BufrData?)

  bool _isReplStart;
  int  _isReplEnd;
  
  uint8_t _group;      		//!< Группа для создания дескриптора при сохранении в TMeteoData
  int _meteoDescr; 		//!< Дескриптор для сохранения в TMeteoData (без группы)
  
  BufrData _data;		//!< Данные
  DataDescriptor _bufrDescr;	//!< Дескриптор
  
  ushort _w;   			//!< Количество бит данных без ассоциированного поля
  ushort _wAssoc; 			//!< Количество бит ассоц-го поля
  
  TBufrList *_prev;		//!< Следующий элемент в списке
  TBufrList *_next;		//!< Предыдущий элемент в списке
};

#endif
