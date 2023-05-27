#include <boost/python.hpp>
#include <commons/obanal/tfield.h>
#include <qbytearray.h>

using namespace boost::python;
using namespace obanal;

TField field;

bool isEmptyWrap() { return field.isEmpty(); }

bool fromBufferWrap(const char* data, int size) {
  return field.fromBuffer(new QByteArray(data, size));
}

double pointValueWrap(double la, double lo) {
  bool ok;
  double value = field.pointValue(meteo::GeoPoint::fromDegree(la, lo), &ok);
  if( true == ok ){
    return value;
  }
  else{
    return -9999;
  }
}

object isEmpty()
{
   return boost::python::make_function(isEmptyWrap);
}

object fromBuffer(const char* /*data*/, int /*size*/)
{
  return boost::python::make_function( static_cast< bool (*)(const char*, int) >( fromBufferWrap ));
}

object pointValue(double /*la*/, double /*lo*/)
{
  return boost::python::make_function( static_cast< double (*)(double, double) >( pointValueWrap ));
}

BOOST_PYTHON_MODULE(libfieldwrap)
{
    def("isEmpty", isEmpty);
    def("fromBuffer", fromBuffer);
    def("pointValue", pointValue);
}
