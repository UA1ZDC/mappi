#ifndef METEO_COMMONS_RPC_PARSER_V1_H
#define METEO_COMMONS_RPC_PARSER_V1_H

#include <meteo/commons/rpc/parser.h>

namespace meteo {
namespace rpc {

constexpr char RPCMSGSTART_v1[] = "rpcmsgstart";
constexpr char RPCMSGEND_v1[]   = "rpcmsgend";

class MethodPack;

class Parser_v1 : public Parser
{
  public:
    Parser_v1();
    ~Parser_v1();
    virtual bool parseMessage( MethodPack* msg );

  private:
    bool searchStart();
    bool searchSize();
    bool recvMessage();
    bool searchEnd();

    DISALLOW_EVIL_CONSTRUCTORS(Parser_v1);
};

}
}

#endif
