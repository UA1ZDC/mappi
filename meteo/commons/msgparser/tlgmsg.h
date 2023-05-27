#ifndef METEO_COMMONS_MSGPARSER_TLGMSG_H
#define METEO_COMMONS_MSGPARSER_TLGMSG_H

#include <qbytearray.h>
#include <qmap.h>

#include <cross-commons/app/tsigslot.h>
#include <meteo/commons/proto/msgparser.pb.h>

#include "tmsgformat.h"

namespace meteo
{

class TlgMsg
{
  static const int CHUNKSIZE;
  static const int TWOCHUNKSIZE;
  static const int TLGMAXSIZE;
  static const int TWOTLGMAXSIZE;
  public:
    enum ParseState
    {
      SEARCH_START_LINE         = 0,
      SEARCH_HEADER             = 1,
      TYPE_DETECT               = 2,
      SEARCH_BINARYSIZE         = 3,
      SEARCH_BINARYPART         = 4,
      SEARCH_BINARYCHUNK        = 5,
      SEARCH_BINARYEND          = 6,
      SEARCH_END                = 7,
      MESSAGE_PARSED            = 8,
      UNKNOWN_TLGSTATE
    };
    enum TlgType
    {
      BINTLG    = 0,
      TEXTLG    = 1,
      UNKTLG
    };

    TlgMsg();
    TlgMsg( const QString& rulename );

    void setStuckedSplit( bool fl );
    bool stuckedSplit() const { return splitstuckedmessages_; }

    void reset();

    ParseState state() const { return state_; }
    TlgType type() const { return type_; }

    const QByteArray& startline() const { return startline_; }
    const QByteArray& header() const { return header_; }
    const QByteArray& message() const { return message_; }
    const QByteArray& end() const { return end_; }
    const msgformat::TMsgFormat::MsgRule& msgrule() const { return msgrule_; }
    const msgformat::TMsgFormat::Section& startRule() const { return startrule_; }
    const msgformat::TMsgFormat::Section& headerRule() const { return headerule_; }

    QByteArray tlg() const ;

    ParseState parseArray( QByteArray* arr );

    const QString& msgtype() const { return msgtype_; }

    int arrpos() const { return arrpos_; }
    int64_t binsize() const { return binsize_; }
    bool toolittlearray() const { return toolittlearray_; }

    static void saveUnparsed( const QByteArray& arr );
  private:
    ParseState state_;
    TlgType type_;
    QString msgtype_;

    msgformat::TMsgFormat::MsgRule msgrule_;
    msgformat::TMsgFormat::BinRule binrule_;

    QByteArray startline_;
    QByteArray header_;
    QByteArray message_;
    QByteArray end_;
    int64_t binsize_;
    int arrpos_;
    msgformat::TMsgFormat::Section startrule_;
    msgformat::TMsgFormat::Section headerule_;

//    QList<msgformat::TMsgFormat::MsgRule> msgformat_;
    QMap< QString, msgformat::TMsgFormat::MsgRule> msgformat_;
    QMap< QString, msgformat::TMsgFormat::BinRule> binformat_;
    QString rulename_;
    bool toolittlearray_;
    bool splitstuckedmessages_; //!< извлекать склеенные грибы и бафры в одном сообщении, как отдельные сообщения

    void loadMsgFormat();

    bool parseStartLine( QByteArray* arr );
    bool parseStartLine( const msgformat::TMsgFormat::MsgRule& rule, QByteArray* arr );

    bool parseHeader( QByteArray* arr );
    bool typeDetectByT1();
    bool typeDetect( QByteArray* arr );
    bool parseBinSize( QByteArray* arr );
    bool parseBinPart( QByteArray* arr );
    bool parseBinChunk( QByteArray* arr );
    bool parseEnd( QByteArray* arr );
};

}

#endif
