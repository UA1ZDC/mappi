#ifndef METEO_COMMONS_MSGPARSER_MSGMETAINFO_H
#define METEO_COMMONS_MSGPARSER_MSGMETAINFO_H

#include <qdatetime.h>
#include <cross-commons/debug/tmap.h>

class TLog;

namespace meteo {
namespace tlg {
class MessageNew;
class Header;
} // tlg
} // meteo

namespace meteo {

//! Количество символов, которое считывается для получения значения элемента
static const TMap<QString,int> kLens = TMap<QString,int>()
<< QPair<QString,int>("{YYYY}", 4)
<< QPair<QString,int>("{YY}", 2)
<< QPair<QString,int>("{MM}", 2)
<< QPair<QString,int>("{DD}", 2)
<< QPair<QString,int>("{J}",  3)
<< QPair<QString,int>("{hh}", 2)
<< QPair<QString,int>("{mm}", 2)
<< QPair<QString,int>("{ss}", 2)
<< QPair<QString,int>("{ID}", 8)
<< QPair<QString,int>("{T1}", 1)
<< QPair<QString,int>("{T2}", 1)
<< QPair<QString,int>("{A1}", 1)
<< QPair<QString,int>("{A2}", 1)
<< QPair<QString,int>("{CCCC}", 4)
<< QPair<QString,int>("{ii}", 2)
<< QPair<QString,int>("{YYGGgg}", 6)
;

class MsgMetaInfo
{
  public:
    QString t1;
    QString t2;
    QString a1;
    QString a2;
    QString cccc;
    QString ii;
    QString yygggg;

    int year;
    int month;
    int day;
    int hh;
    int mm;

    QString templ;
    QString fileName;
    bool parseError_ = false;

  public:
    MsgMetaInfo() { clear(); }

    MsgMetaInfo(const ::meteo::tlg::MessageNew& msg);
    MsgMetaInfo(const QString& parseTemplate, const QString& filePath);
    bool parseError() { return parseError_; }

    void update(const ::meteo::tlg::MessageNew& msg);
    tlg::Header getHeader() const;
    QDateTime calcConvertedDt(const QDateTime& cur = QDateTime::currentDateTimeUtc()) const;

    void clear();
};

}

TLog& operator<< (TLog& log, const meteo::MsgMetaInfo& info);

#endif
