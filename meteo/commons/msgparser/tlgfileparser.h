#ifndef METEO_COMMONS_MSGPARSER_TLGFILEPARSER_H
#define METEO_COMMONS_MSGPARSER_TLGFILEPARSER_H

#include <qbytearray.h>
#include <qdatastream.h>
#include <qstring.h>
#include <qfile.h>
#include <qlist.h>

#include "tmsgformat.h"

class TMsgParser
{
  enum ParseState
  {
    SEARCH_START_LINE = 0,
    SEARCH_HEADER,
    SEARCH_MESSAGE,
    SEARCH_END_LINE
  };
  public:
    static const int CHUNKSIZE;
    static const int TWOCHUNKSIZE;

  public:
    TMsgParser( const QString& filename, bool unk );
    TMsgParser();
    ~TMsgParser();

    ParseState state() const { return state_; } 

    bool setTelegramFile( const QString& filename );
    void resetTelegramFile();

    void setUnk( bool unk ) { unk_ = unk; }
    bool parseNextMessageInFile();
    const QString& dayhourmincap() const { return dayhourmincap_; }
    const QString& binfilename() const { return binfilename_; }

    const QByteArray& msgbuf() const { return msgbuf_; }
    const QString& type() const { return type_; }

  private:
    QByteArray msgbuf_;
    QString type_;
    ParseState state_;
    QFile tlgfile_;
    QFile unknowndatafile_;
    QDataStream unknowndatastream_;
    QMap< QString, msgformat::TMsgFormat::MsgRule> msgformat_;
    QList<msgformat::TMsgFormat::BinRule> binformat_;
    QString dayhourmincap_;
    bool unk_;
    QString binfilename_;

    void loadMsgFormat();

    msgformat::TMsgFormat::MsgRule parseStartLine( bool* ok );
    bool parseHeader( const msgformat::TMsgFormat::MsgRule& rule );
    bool parseMessage( const msgformat::TMsgFormat::MsgRule& rule );
    bool processMessage( QIODevice* source, const msgformat::TMsgFormat::BinRule& rule, int64_t size );
    bool processMessage( QIODevice* source, const QString& endline );

    int64_t readNextPortionForSearchStartLine( QByteArray* arr, int64_t readbytes );
  
  static int64_t readFirstBuf( QByteArray* buf, QIODevice* source, int64_t size );
  static int64_t readNextBuf( QByteArray* buf, QIODevice* source, int64_t size );
};

#endif
