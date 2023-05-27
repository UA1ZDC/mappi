#include <boost/python.hpp>

#include <qbytearray.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/parser.h>
#include <meteo/commons/rpc/methodpack.h>
#include <meteo/commons/rpc/transport.h>
#include <meteo/commons/rpc/tcpsocket.h>

std::string id( meteo::rpc::MethodPack& pack )
{
  return std::string( pack.id().data(), pack.id().size() );
}

void setId( meteo::rpc::MethodPack& pack, const std::string& id )
{
  pack.setId( QByteArray(id.data(), id.size() ) );
}

bool stub( meteo::rpc::MethodPack& pack )
{
  return pack.stub();
}

void setStub( meteo::rpc::MethodPack& pack, bool stub )
{
  pack.setStub(stub);
}

bool failed( meteo::rpc::MethodPack& pack )
{
  return pack.failed();
}

void setFailed( meteo::rpc::MethodPack& pack, bool f )
{
  pack.setFailed(f);
}

std::string service( meteo::rpc::MethodPack& pack )
{
  return std::string( pack.service().data(), pack.service().size() );
}

void setService( meteo::rpc::MethodPack& pack, const std::string& s )
{
  pack.setService( QByteArray( s.data(), s.size() ) );
}

std::string method( meteo::rpc::MethodPack& pack )
{
  return std::string( pack.method().data(), pack.method().size() );
}

void setMethod( meteo::rpc::MethodPack& pack, const std::string& m )
{
  pack.setMethod( QByteArray( m.data(), m.size() ) );
}

PyObject* message( meteo::rpc::MethodPack& pack )
{
  return PyBytes_FromStringAndSize(pack.message().data(), pack.message().size());
}

void setMessage( meteo::rpc::MethodPack& pack, const std::string& m )
{
  pack.setMessage( QByteArray( m.data(), m.size() ) );
}

std::string comment( meteo::rpc::MethodPack& pack )
{
  return std::string( pack.comment().data(), pack.comment().size() );
}

void appendData( meteo::rpc::Parser& parser, const std::string& data, int size )
{
  QByteArray arr;
  arr.setRawData( data.c_str(), size );
  parser.appendData(&arr);
}

int64_t msgsize( meteo::rpc::Parser& parser )
{
  if ( 0 == parser.msgsize() ) {
    return 0;
  }
  return parser.msgsize() + meteo::rpc::kStartSz + meteo::rpc::kSizeSz + meteo::rpc::kEndSz;
}

PyObject* data( meteo::rpc::MethodPack& pack )
{
  QByteArray arr( pack.size(), '\0' );
  pack.getData(&arr);
  return PyBytes_FromStringAndSize(arr.data(), arr.size());
}

int64_t size( meteo::rpc::MethodPack& pack )
{
  return pack.size();
}

bool parseMessage( meteo::rpc::Parser& parser, meteo::rpc::MethodPack& pack )
{
  return parser.parseMessage(&pack);
}

std::string hostByAddress( const std::string& address )
{
  QString host;
  int32_t port;
  meteo::rpc::TcpSocket::parseAddress(address,&host, &port);
  return host.toStdString();
}

int32_t portByAddress(const std::string&  address )
{
  QString host;
  int32_t port;
  meteo::rpc::TcpSocket::parseAddress(address,&host, &port);
  return port;
}


BOOST_PYTHON_MODULE(librpcpywrap)
{
  boost::python::class_<meteo::rpc::MethodPack>( "MethodPack" )
    .def( "id", id )
    .def( "setId", setId, boost::python::args("id") )
    .def( "stub", stub )
    .def( "setStub", setStub, boost::python::args("stub") )
    .def( "failed", failed )
    .def( "setFailed", setFailed, boost::python::args("failed") )
    .def( "service", service )
    .def( "setService", setService, boost::python::args("service") )
    .def( "method", method )
    .def( "setMethod", setMethod, boost::python::args("method") )
    .def( "message", message )
    .def( "setMessage", setMessage, boost::python::args("message") )
    .def( "data", data )
    .def( "size", size )
  ;
  boost::python::class_<meteo::rpc::Parser, boost::noncopyable>( "RpcParser" )
    .def( "appendData", appendData, boost::python::args( "data", "size" ) )
    .def( "parseMessage", parseMessage, boost::python::args( "pack" ) )
    .def( "msgsize", msgsize )
  ;
   boost::python::def( "hostByAddress", hostByAddress);
   boost::python::def( "portByAddress", portByAddress);
}
