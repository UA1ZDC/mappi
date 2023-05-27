#ifndef METEO_COMMONS_ALPHANUM_WMOMSG_H
#define METEO_COMMONS_ALPHANUM_WMOMSG_H

//! Раскодирование телеграммы в международном формате
/*!
  \file   wmomsg.h
  \brief  Раскодирование телеграммы в международном формате
*/

#include <qdatetime.h>
#include <qpair.h>

template<class key, class value > class QMap;
class TMeteoData;

namespace meteo {
  namespace anc {
    class GphDecoder;
    class WmoDecoder;
    class TableDecoder;
  }
}

namespace meteo {
namespace anc {

  
  class WmoMsg {
  public:
    WmoMsg();
    virtual ~WmoMsg();

    int parse(const QMap<QString, QString>& type, const QByteArray& ba, const QDateTime& dt);
    bool hasNext();
    bool decodeNext();
    QString current();

    TMeteoData* data() { return _data; }
    QList<TMeteoData*> split();
    QString code();
    int category();

    void setCodec(QTextCodec* c) { _codec = c; }

  protected:
    virtual void dataReady() {}

  private:
    void parseGeophysics(int type, const QString& wstr, QList<QPair<int, QByteArray> >* msgList);
    bool parseGeoph(const QString& str, QList<QPair<int, QByteArray> >* msgList);
    bool parseIonfo(const QByteArray& abuf, QList<QPair<int, QByteArray> >* msgList);
    bool parseHydro(const QString& astr, QList<QPair<int, QByteArray> >* msgList);
    bool tlg(const QByteArray& abuf, QList<QPair<int, QByteArray> >* msgList);
    bool parseWithoutGroup(int type, const QByteArray& ba, QList<QPair<int, QByteArray> >* msgList);

    bool decodeWmo(int type, const QByteArray& ba);
    bool decodeGph(int type, const QByteArray& ba);

    int findGroup(QString* str, QString* firstline, int* type);
    int findGroupInAllMsg(const QString& str, QString* firstline, int* type);
    //    int findWmoFormatGroup(const QString& str, QString* firstline, int* type);
    int checkReportHeader(const QString& str, int type);

  private:
    QDateTime _dt;    //!< Дата/время формирования сообщения
    QString _caption; //!< Заголовок сводки

    QList<QPair<int, QByteArray> > _msgList;
    int _msgIdx = 0;

    TMeteoData* _data = nullptr;
    WmoDecoder* _wmoDecoder = nullptr;
 
    QTextCodec* _codec; //!< Кодек для преобразования символов
    QString _idPtkpp;
  };
    

}
}


#endif
