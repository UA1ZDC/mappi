#ifndef METEO_COMMONS_MSGPARSER_TLGPARSER_H
#define METEO_COMMONS_MSGPARSER_TLGPARSER_H

#include <qstring.h>
#include <qbytearray.h>
#include <qmap.h>

#include <cross-commons/app/tsigslot.h>

namespace meteo
{
namespace tlg
{
class Message;
class MessageNew;
}
}

namespace meteo
{

class TlgMsg;

class TlgParser
{
  enum TlgType
  {
    WRMM        = 0,
    WAMM        = 1,
    GRMM        = 2,
    GAMM        = 3,
    GCMM        = 4,
    UNKTLG
  };
  public:
    TlgParser( const QByteArray& arr, bool splitstuckedmessages = false );
    TlgParser( const QByteArray& arr, const QString& rulename, bool splitstuckedmessages = false );
    ~TlgParser();

    void setData( const QByteArray& arr );
    void appendData( const QByteArray& arr ) { arr_ += arr; }
    void appendData( const char* arr, int len ) { arr_.append( arr, len ); }

    bool parseNextMessage(tlg::MessageNew* newmsg = 0 );

    QList<QString> counternames() const { return counter_.keys(); }
    int count( const QString& name ) const { return counter_[name]; }

    QByteArray tlg() const ;
    const QByteArray& startline() const ;
    const QByteArray& header() const ;
    const QByteArray& message() const ;
    const QByteArray& end() const ;

    sigslot::signal0<>& brokenTlg() { return brokenTlg_; }

  private:
    QByteArray arr_;
    TlgMsg* tlgmsg_;
    QMap< QString, int > counter_;
    sigslot::signal0<> brokenTlg_;
    QString rulename_;

    void fillMessageNew( tlg::MessageNew* msg ) const ;
    void fillBbb( tlg::MessageNew* msg ) const ;
};

}

#endif
