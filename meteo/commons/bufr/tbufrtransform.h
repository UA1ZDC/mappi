#ifndef TMETEO_COMMONS_BUFRTRANSFROM_H
#define TMETEO_COMMONS_BUFRTRANSFROM_H

#include <qmap.h>
#include <qhash.h>
#include <qdebug.h>
#include <meteo/commons/proto/surface_service.pb.h>

class BufrDescriptor;
class QDomDocument;
class QDomNode;
class QDomElement;


//! параметры для приведения bufr дескприптора к TMeteoData
struct BufrConvert {
  int descr; //!< новое значение дескриптора (-1, если нет)
};

struct BufrSequence {
  int number;      //!< порядковый номер последовательности
  meteo::surf::DataType stationType; //!< тип станции
  QList<QPair<uint, int> > descr; //!< последовательность дескрипторов с указанием замены номера
};

struct BufrExpand {
  int group; //!< номер группы (добавляется в качестве двух первых цифр номера дескриптора. -1, если нет)
  BufrSequence seq; //!< последовательность с указанием преобразования дескрипторов
};

struct SequenceType {
  int bufrCateg = -1;
  meteo::surf::DataType category = meteo::surf::kUnknownDataType;
  int prior = -1;
  SequenceType() {}
  SequenceType(int bc, meteo::surf::DataType c, int p): bufrCateg(bc), category(c), prior(p) {}

  bool operator<(const SequenceType& other) const {
    if (category < other.category) {
      return true;
    }
    if (bufrCateg < other.bufrCateg) {
      return true;
    }
    return false;
  }
};

//! для определения типа по категории bufr и номеру последовательности X (для B и D дескрипторов)
struct DescrSeq {
  int bufrCateg = -1;
  int X = -1;
  DescrSeq() {}
  DescrSeq(int categ, int x): bufrCateg(categ), X(x) {}
  bool operator<(const DescrSeq& other) const {
    if (bufrCateg < other.bufrCateg) {
      return true;
    }
    if (bufrCateg == other.bufrCateg && X < other.X) {
      return true;
    }
    return false;
  }
};

QDebug& operator<<(QDebug& out, const DescrSeq& d);


class TBufrTransform {
public:
  TBufrTransform();
  ~TBufrTransform();

  // const BufrSequence& findSequence(const QList<BufrDescriptor>& descr, int cnt, bool* ok, 
  // 				   int* number, meteo::surf::DataType* type) const;
  const BufrExpand bufrExpand(uint descr) const { return _dDescr.value(descr); }
  const QHash<uint, BufrConvert>& bufrConvert() { return _bDescr; }
  const QHash<uint, QMap<int, uint> >& bufrSignGroup() { return _signGroup; }

  meteo::surf::DataType findType(const QList<BufrDescriptor>& descr, int cnt, int bufrCateg) const;
  meteo::surf::DataType findSequenceType(const BufrDescriptor& descr, int bufrCateg) const;
  SequenceType findDescrType(const BufrDescriptor& descr, int bufrCateg, const QMap<DescrSeq, SequenceType>& descrType) const;
  
private:
  void init();
  void readSettings(QDomNode node);
  void readSequence(const QDomElement& node);
  void readDescrType(const QDomElement& pel, QMap<DescrSeq, SequenceType>* descrType);
  void readTransform(QDomNode node);
  void readSignGroup(QDomNode node);

  //bool equalSequence(const QList<BufrDescriptor>& descr, const BufrSequence& seq) const;
  
private:
  QList<BufrSequence> _sequence;    //!< известные последовательности принимаемых bufr
  QHash<uint, BufrExpand> _dDescr;  //!< список номеров для преобразования, таблица D
  QHash<uint, BufrConvert> _bDescr; //!< список номеров для преобразования, таблица B
  QHash<uint, QMap<int, uint> > _signGroup; //!< номер группы в зависимости от значения дескриптора (дескр, (номер бита,
					    //номер группы))

  QMap<uint, SequenceType> _dSeqType;
  QMap<DescrSeq, SequenceType> _dDescrType;
  QMap<DescrSeq, SequenceType> _bDescrType;
  
};

#endif
