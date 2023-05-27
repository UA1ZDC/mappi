#ifndef METEO_COMMONS_RPC_METHODPACK_H
#define METEO_COMMONS_RPC_METHODPACK_H

#include <qbytearray.h>
#include <qstring.h>

namespace meteo {
namespace rpc {


static const QByteArray kRpcMsgStart("rpcv2msgstart");
static const QByteArray  kRpcMsgEnd("rpcmsgend");
static const int32_t kStartSz           = kRpcMsgStart.size();
static const int32_t kSizeSz            = sizeof(int64_t);
static const int32_t kEndSz             = kRpcMsgEnd.size();

class MethodPack
{
  public:
    MethodPack( const QByteArray& id );
    MethodPack();
    ~MethodPack();

    void setId( const QByteArray& id ) { id_ = id; }
    const QByteArray& id() const { return id_; }

    void setStub( bool s ) { stub_ = s; }
    bool stub() const { return stub_; }

    void setFailed( bool f ) { failed_ = f; }
    bool failed() const { return failed_; }

    void setService( const QByteArray& s ) { service_ = s; }
    const QByteArray& service() const { return service_; }

    void setMethod( const QByteArray& m ) { method_ = m; }
    const QByteArray& method() const { return method_; }

    void setMessage( const QByteArray& m ) { message_ = m; }
    const QByteArray& message() const { return message_; }
    QByteArray* mutableMessage() { return &message_; }

    void setComment( const QByteArray& c ) { comment_ = c; }
    const QByteArray& comment() const { return comment_; }

    void clear();
    void copyFrom( const MethodPack& pack );

    int32_t size() const ;
    bool  getData( QByteArray* data ) const ;
    bool setData( const QByteArray& data );

  private:
    QByteArray id_;
    bool stub_ = false;
    bool failed_ = false;
    QByteArray service_;
    QByteArray method_;
    QByteArray message_;
    QByteArray comment_;
};

}
}

#endif
