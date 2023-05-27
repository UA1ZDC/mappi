#include <boost/python.hpp>

#include <qdatastream.h>

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>

using namespace boost::python;
using namespace control;

void fromArray(TMeteoData& tmData,const char* data, int size)
{
  QByteArray ba(data,size);
  QDataStream in(ba);
  in >> tmData;
}

// 
// Перегруженная функция, которая на вход принимает байты
// 
void fromArrayObject(TMeteoData& tmData,object data)
{
  PyObject* py_object = data.ptr();
  Py_buffer* py_buffer = new Py_buffer();
  
  PyObject_GetBuffer( py_object, py_buffer, PyBUF_SIMPLE );
  const char* cxx_buf = (const char*)(py_buffer->buf);
  int size = (int)py_buffer->len;

  QByteArray ba(cxx_buf,size);
  QDataStream in(ba);
  in >> tmData;
}

float getStationIndex(TMeteoData& tmData)
{
  return tmData.getValue(7,-1);
}

std::string getStation( TMeteoData& md )
{
  int indx = TMeteoDescriptor::instance()->station(md);
  std::string indexstr = QObject::tr("Не известно").toStdString();
  if ( BAD_METEO_ELEMENT_VAL != indx ) {
    indexstr = QObject::tr("%1").arg( indx, 5, 10, QChar('0') ).toStdString();
  }
  else if ( true == md.hasParam("CCCC") ) {
    indexstr = md.meteoParam("CCCC").code().toStdString();
  }
  return indexstr;
}

std::string getHHMM( TMeteoData& md )
{
  return TMeteoDescriptor::instance()->dateTime(md).toString("hh::mm").toUtf8().constData();
}

const char* getDateTime(TMeteoData& tmData)
{
    return TMeteoDescriptor::instance()->dateTime(tmData).toString(Qt::ISODate).toUtf8().constData();
}

float getParamValue(TMeteoData& tmData, std::string str)
{
    return tmData.getValue(TMeteoDescriptor::instance()->descriptor(QString::fromStdString(str)),-9999);
}

const TMeteoParam& meteoParam(TMeteoData& tmData, const char* name)
{
    return tmData.meteoParam(QString(name));
}

boost::python::list meteoParamList(TMeteoData& tmData, const char* name)
{
  boost::python::list list;
  auto map = tmData.meteoParamList(QString(name));
  boost::python::object surf = boost::python::import("surface_pb2");
  for ( auto& param : map ) {
    if ( true == param.isInvalid() ) {
      continue;
    }
    boost::python::object proto = surf.attr("MeteoParamProto")(); //Вызов конструктора объекта PYTHON
    proto.attr("code") = param.code().toStdString();
    proto.attr("value") = param.value();
    proto.attr("quality") = param.quality();
    proto.attr("descrname") = name;
    list.append(proto);
  }
  return list;
}

bool hasParam(TMeteoData& tmData, const char* name)
{
    return tmData.hasParam(TMeteoDescriptor::instance()->descriptor(QString(name)));
}

void printData(TMeteoData& tmData)
{
  tmData.printData();
}

std::string toString( TMeteoData& md ) {
  return md.toString().toStdString();
}

std::string code(TMeteoParam& param)
{
  return param.code().toStdString();
}

boost::python::list meteoParamListValues( TMeteoData& md, const char* name)
{
  boost::python::list list;
  auto map = md.meteoParamList(QString(name));
  for ( auto& param : map ) {
    if ( true == param.isInvalid() ) {
      continue;
    }
    if ( true == list.contains(param.value()) ) {
      continue;
    }
    list.append(param.value());
  }
  return list;
}

BOOST_PYTHON_MODULE(libmeteodatawrap)
{
  class_<TMeteoData, boost::noncopyable>( "TMeteoData" )
    .def("fromArray",fromArray,args("data","size"))
    .def("fromArrayObject",fromArrayObject,args("data"))
    .def("getStationIndex", getStationIndex)
    .def("getStation", getStation)
    .def("getHHMM", getHHMM)
    .def("getDateTime",getDateTime)
    .def("getParamValue", getParamValue)
    .def("meteoParam",meteoParam,args("name"),return_value_policy<reference_existing_object>())
    .def("meteoParamList",meteoParamList,args("name"))
    .def("meteoParamListValues", meteoParamListValues, args("name"))
    .def("hasParam",hasParam,args("name"))
    .def("printData", printData)
    .def("toString", toString)
    ;
  class_<TMeteoParam, boost::noncopyable>( "TMeteoParam")
    .def(init<const char*,float,QualityControl>(args("code","value","quality")))
    .def("value", &TMeteoParam::value)
    .def("isInvalid", &TMeteoParam::isInvalid)
    .def("code", code)
  ;
  enum_<QualityControl>("QualityControl")
    .value("RIGHT",RIGHT)
    .value("HAND_CORRECTED",HAND_CORRECTED)
    .value("AUTO_CORRECTED",AUTO_CORRECTED)
    .value("ABSENT_CORRECTED",ABSENT_CORRECTED)
    .value("DOUBTFUL_CORRECTED",DOUBTFUL_CORRECTED)
    .value("MISTAKEN_CORRECTED",MISTAKEN_CORRECTED)
    .value("HAND_CORR_DOUBTFUL",HAND_CORR_DOUBTFUL)
    .value("HAND_CORR_MISTAKEN",HAND_CORR_MISTAKEN)
    .value("HAND_CORR_ABSENT",HAND_CORR_ABSENT)
    .value("NO_CONTROL",NO_CONTROL)
    .value("SPECIAL_VALUE",SPECIAL_VALUE)
    .value("DOUBTFUL",DOUBTFUL)
    .value("MISTAKEN",MISTAKEN)
    .value("DOUBTFUL_STATION",DOUBTFUL_STATION)
    .value("NO_OBSERVE",NO_OBSERVE)
  ;
}
