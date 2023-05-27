#ifndef METEO_ALPHANUM_REGEXPDECODE_H
#define METEO_ALPHANUM_REGEXPDECODE_H

#include <qstring.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qmap.h>


class TMeteoData;

class QDomNode;
class QDomElement;
class QByteArray;
class QRegExp;
class QDomNodeList;
class QDomDocument;


namespace meteo {
namespace anc {

class XmlLineDecoder;
  
//! Раскодирование метеоданных в буквенно-цифровом виде при помощи регулярных выражений
class RegExpDecode {
public:
  RegExpDecode();
  virtual ~RegExpDecode();
  
  //  virtual bool decode(const QString& code, const QByteArray& report);
  virtual bool decode(int code, const QByteArray& report);
  
  void setCaption(const QString& caption) { _caption = caption; }
  
  //! установить дату/время сводок. replace = true - не использовать дату/время из сводок, false - из dt будут заполнены только нехватающие элементы
  void setDateTime(const QDateTime& dt, bool replace) { _dt = dt;  _replaceDt = replace; }
  void setStationName(const QString& station) { _stationName = station; }
  void setStationIdx(int idx) { _stationIdx = idx; }
  void setDiffUtc(int diffUtc) { _diffUtc = diffUtc; }

  TMeteoData* data() { return _data; }
  QString code() { return _code; }
  int category() { return _category; }

protected:
  
  virtual bool decode(const QByteArray &report, const QDomDocument &decoder) = 0;  
  virtual bool fillIdentification();

  bool decodeNode(const QDomNode& node, const QString& msg);  

  QString endWith() const { return _endWith; }

private:
  void init();
  void clear();
  

  bool decodeNode(const QList<QDomNode>& nodeDecodersList, const QRegExp& rexp, const QString& msg, bool addChild, int categ);
  bool setMeteoDataValue(const QDomNode& node, const QString& msg);
  bool getRexpFromFirstNode(const QDomNode& node, QRegExp& rexp);
  bool setRexpVariant(const QString& indicatorDescr, const QDomNodeList& rexpNodesList, QRegExp& rexp);
  void getDecodersList(const QDomNode& child, QList<QDomNode>* nodeDecodersList);
  QDomNode getVariantDecodersList(const QDomNodeList& nodes);
  
  bool setDataMonthYear();
  void convertMeteoParams();
  void convertCoords();

  QDateTime dateTime();
  TMeteoData* currentMeteoData(QDomElement elem);
  
private:
  TMeteoData* _data = nullptr; //!< Раскодированные данные
  QString _code;     //!< Текущая сводка
  int _category;     //!< Текущая категория данных
  QString _caption;  //!< Заголовок сообщения (чтоб найти его при ошибках)
  QDateTime _dt;     //!< Дата/время, если нет данных в сводке
  QString _stationName; //!< Название станции или поста
  int _stationIdx; //!< Номер станции или поста
  int _diffUtc;    //!< Разниа времени в телеграмме с UTC
  bool _replaceDt; //!< Использовать время _dt, а не из телеграммы
  QString _endWith; //!< Окончание куска, который надо парсить

  XmlLineDecoder* _lineDecoder = nullptr; //!< Сохранение метеоданных в соответствии с параметрами в xml строке
  // QMap<int, TMeteoData*> _childs; //!< Для связи 
  // QMap<int, TMeteoData*> _inchilds; //!< Для связи внутри последнего звена (тег Value)
};

    
} //anc
} //meteo

#endif
