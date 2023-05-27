#ifndef METEO_COMMONS_RPC_PARSER_H
#define METEO_COMMONS_RPC_PARSER_H

#include <qbytearray.h>

#include <cross-commons/includes/tcommoninclude.h>

namespace meteo {
namespace rpc {

class MethodPack;

class Parser
{
  protected:
    enum State {
      kStartSearch      = 0,
      kSizeSearch       = 1,
      kMessageRecv      = 2,
      kEndSearch        = 3,
      kUnknownState     = 4
    };
  public:
    Parser();
    virtual ~Parser();

    void resetState() { state_ = kStartSearch; }

    void appendData( QByteArray* arr ) { recvbuf_.append(*arr); }
    void appendData( const char* data, int64_t size ) { recvbuf_.append( data, size ); }
    virtual bool parseMessage( MethodPack* msg );
    int64_t msgsize() const { return msgsize_; }

  protected:
    State state_ = kStartSearch;
    int64_t msgsize_ = 0;

    QByteArray recvbuf_;
    MethodPack* message_ = nullptr;

  private:
    bool searchStart();
    bool searchSize();
    bool recvMessage();
    bool searchEnd();

    DISALLOW_EVIL_CONSTRUCTORS(Parser);
};

}
}

#endif
