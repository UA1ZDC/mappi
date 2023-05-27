#include "methodpack.h"

#include <cross-commons/debug/tlog.h>

namespace meteo {
namespace rpc {

static const int32_t kSzint = static_cast<int32_t>( sizeof(int32_t) );
static const int32_t kSzbool = static_cast<int32_t>( sizeof(bool) );

MethodPack::MethodPack( const QByteArray& id )
  : id_(id),
  failed_(false)
{
}

MethodPack::MethodPack()
  : failed_(false)
{
}

MethodPack::~MethodPack()
{
}

void MethodPack::clear()
{
  id_.clear();
  service_.clear();
  method_.clear();
  message_.clear();
}

void MethodPack::copyFrom( const MethodPack& pack )
{
  id_ = pack.id_;
  stub_ = pack.stub_;
  service_ = pack.service_;
  method_ = pack.method_;
  message_ = pack.message_;
}

int32_t MethodPack::size() const
{
  int32_t sz = kSzint;
  sz += id_.size();
  sz += kSzbool;
  sz += kSzint;
  sz += service_.size();
  sz += kSzint;
  sz += method_.size();
  sz += kSzint;
  sz += message_.size();
  sz += kSzint;
  sz += comment_.size();
  return sz;
}

bool MethodPack::getData( QByteArray* pack ) const
{
  char* d = pack->data();

  int32_t sz = id_.size();
  ::memcpy( d, &sz, kSzint );
  d += kSzint;
  ::memcpy( d, id_.data(), id_.size() );
  d += id_.size();

  ::memcpy( d, &stub_, kSzbool );
  d += kSzbool;

  sz = service_.size();
  ::memcpy( d, &sz, kSzint );
  d += kSzint;
  ::memcpy( d, service_.data(), service_.size() );
  d += service_.size();

  sz = method_.size();
  ::memcpy( d, &sz, kSzint );
  d += kSzint;
  ::memcpy( d, method_.data(), method_.size() );
  d += method_.size();

  sz = message_.size();
  ::memcpy( d, &sz, kSzint );
  d += kSzint;
  ::memcpy( d, message_.data(), message_.size() );
  d += message_.size();

  sz = comment_.size();
  ::memcpy( d, &sz, kSzint );
  d += kSzint;
  ::memcpy( d, comment_.data(), comment_.size() );

  return true;
}

bool MethodPack::setData( const QByteArray& d )
{
  int32_t pos = 0;
  int32_t dsize = d.size();
  int32_t sz = 0;

  if ( kSzint > dsize - pos ) {
    return false;
  }
  ::memcpy( &sz, d.data() + pos, kSzint );
  pos += kSzint;
  if ( sz > dsize - pos ) {
    return false;
  }
  id_ = QByteArray( d.data() + pos, sz );
  pos += sz;

  if ( kSzbool > dsize - pos ) {
    return false;
  }
  ::memcpy( &stub_, d.data() + pos, kSzbool );
  pos += kSzbool;

  if ( kSzint > dsize - pos ) {
    return false;
  }
  ::memcpy( &sz, d.data() + pos, kSzint );
  pos += kSzint;
  if ( sz > dsize - pos ) {
    return false;
  }
  service_ = QByteArray( d.data() + pos, sz );
  pos += sz;

  if ( kSzint > dsize - pos ) {
    return false;
  }
  ::memcpy( &sz, d.data() + pos, kSzint );
  pos += kSzint;
  if ( sz > dsize - pos ) {
    return false;
  }
  method_ = QByteArray( d.data() + pos, sz );
  pos += sz;

  if ( kSzint > dsize - pos ) {
    return false;
  }
  ::memcpy( &sz, d.data() + pos, kSzint );
  pos += kSzint;
  if ( sz > dsize - pos ) {
    return false;
  }
  message_ = QByteArray( d.data() + pos, sz );
  pos += sz;

  ::memcpy( &sz, d.data() + pos, kSzint );
  pos += kSzint;
  if ( sz > dsize - pos ) {
    return false;
  }
  comment_ = QByteArray( d.data() + pos, sz );
  pos += sz;

  return true;
}

}
}
