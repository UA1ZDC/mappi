#ifndef METEO_COMMONS_ALPHANUM_CLIWAREMSG_H
#define METEO_COMMONS_ALPHANUM_CLIWAREMSG_H

//! Раскодирование данных ВНИИГМИ-МЦД
/*!
  \file   cliwaremsg.h
  \brief  Раскодирование 


*/

#include <qmap.h>
#include <qdatetime.h>
#include <QJsonDocument>
#include <QVariantList>
#include <QJsonArray>
#include <QDomNode>
#include <commons/meteo_data/tmeteodescr.h>

class TMeteoData;


namespace meteo {
namespace anc {

  class XmlLineDecoder;
  
  class CliwareMsg {
  public:
    struct DescrDefinition {
      QString descrname; //!< название дескритора, можно сразу в TMeteoData
      QString andescr;   //!< название в andescriptors.xml, необходимо преобразование
      QDomNode node;     //!< Вся строка xml с описанием
      DescrDefinition() {}
      DescrDefinition(const QString& dname, const QString& andname, QDomNode anode):
	descrname(dname), andescr(andname), node(anode) {}
    };
    
    
    CliwareMsg();
    virtual ~CliwareMsg();
    
    int parse(const QMap<QString, QString>& type, const QJsonDocument& data);
    bool hasNext();
    bool decodeNext();

    QString code()     { return _code; }
    int category()     { return _category; }
    TMeteoData* data() { return _data; }
   
  protected:
    virtual void dataReady() {}

  private:
    int getCategory(const QString& code);
    bool addCliValue(const QString& name, const QString& cliVal);
    bool decode(const QVariantList& abbrev, const QVariantList& msg);

    void readConfig();
    void readCliType(QDomNode node);
    void readDescriptors(QDomNode node);
    
  private:

    //QDateTime _dt;    //!< Дата/время формирования сообщения
    TMeteoData* _data = nullptr;
    QString _code; //! Название кодовой формы
    int _category; //!< тип данных

    int _msgIdx = 0;
    QJsonArray _msgList;
    QVariantList _cliNames;

    QMap<QString, uint> _types;
    QMap<QString, DescrDefinition> _descr;
    XmlLineDecoder* _lineDecoder = nullptr; //!< Сохранение метеоданных в соответствии с параметрами в xml строке
  };
    

}
}



#endif
