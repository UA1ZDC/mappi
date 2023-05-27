#include <boost/python.hpp>
#include <meteo/commons/zond/zond.h>

using namespace boost::python;
using namespace zond;

void fromArray(Zond& zond,const char* data, int size)
{
    QByteArray ba(data,size);
    QDataStream in(ba);
    in >> zond;
}

// 
// Перегруженная функция, которая на вход принимает байты
// 
void fromArrayObject(Zond& zond,object data)
{
  PyObject* py_object = data.ptr();
  Py_buffer* py_buffer = new Py_buffer();
  
  PyObject_GetBuffer( py_object, py_buffer, PyBUF_SIMPLE );
  const char* cxx_buf = (const char*)(py_buffer->buf);
  int size = (int)py_buffer->len;

  QByteArray ba(cxx_buf,size);
  QDataStream in(ba);
  in >> zond;
  // 
  // FIX ME надо чистить память от new Py_buffer(); - надо ли?
  // 
}

std::string dateTime(Zond& zond)
{
    return zond.dateTime().toString("dd-MM-yyyy hh:mm").toStdString();
}

bool getUrPoLevelType(Zond& zond, int level_type, zond::Uroven* ur)
{
  return zond.getUrPoLevelType(level_type, ur);
}

bool getUrz(Zond& zond, zond::Uroven* ur)
{
  return zond.getUrz(ur);
}

float getAverWindD(Zond& zond, float h1, float h2)
{
  float dd = 0;
  float ff = 0;
  if (!zond.getAverWindDF(h1, h2, &dd, &ff)) {
    return -9999.0;
  }
  return dd;
}

float getAverWindF(Zond& zond, float h1, float h2)
{
  float dd = 0;
  float ff = 0;
  if (!zond.getAverWindDF(h1, h2, &dd, &ff)) {
    return -9999.0;
  }
  return ff;
}

double oprHpoP(Zond& zond, double p)
{

  return zond.oprHpoP(p);
}

boost::python::dict urovenList(Zond& zond)
{
  QMap<zond::Level,zond::Uroven>& zLvl = const_cast< QMap<zond::Level,zond::Uroven>&>(zond.urovenList());
  boost::python::dict zDict;
  foreach (float key, zLvl.keys()) {
    zDict[key] = boost::ref(zLvl[key]);
  }
  return zDict;
}

boost::python::list SA81_P_float( float h)
{
  float dh = h;
  float dt = 0;
  float dp = 0;
  zond::SA81_P(dh, &dt, &dp);
  float t = dt;
  float p = dp;
  boost::python::list list;
  list.append(t);
  list.append(p);
  return list;
}

float oprTvirt_float(float T, float P)
{
  double dt = T;
  double dp = P;
  return zond::oprTvirt(dt, dp);
}

boost::python::tuple oprSrParamPoPH_float(Zond& zond, ValueType type_par, ValueType type_ur, float ph1, float ph2)
{
  float sr_param = 0;
  bool ok = zond.oprSrParamPoPH(type_par, type_ur, ph1, ph2, &sr_param);
  return make_tuple(ok, sr_param);
}

void copy(Zond& zond, Zond to_copy)
{
  zond = to_copy;
}

double alt(Zond& zond)
{
  return zond.coord().alt();
}

const char* toString(Zond& zond)
{
  return zond.toString().toUtf8().data();
}

BOOST_PYTHON_MODULE(libzondwrap)
{
  class_<Zond>( "Zond" )
    .def("fromArray",fromArray,args("data","size"))
    .def("fromArrayObject",fromArrayObject,args("data"))
    .def("getUrPoH",&Zond::getUrPoH,args("height","level"))
    .def("getUrPoP",&Zond::getUrPoP,args("pressure","level"))
    .def("dateTime",dateTime)
    .def("setDataNoPreobr",static_cast< bool (Zond::*) (const TMeteoData&) > (&Zond::setDataNoPreobr), args("meteodata"))
    .def("show", &Zond::print)
    .def("toString", toString )
    .def("DF2UV", &Zond::DF2UV)
    .def("test", &Zond::test)
    .def("getUrPoLevelType", getUrPoLevelType, args("level_type", "level"))
    .def("getUrz", getUrz, args("level"))
    .def("getAverWindD", getAverWindD, args("h1", "h2"))
    .def("getAverWindF", getAverWindF, args("h1", "h2"))
    .def("oprHpoP", oprHpoP, args("p"))
    .def("preobr", &Zond::preobr_for_wrap)
    .def("urovenList", urovenList)
    .def("oprSrParamPoPH", oprSrParamPoPH_float)
    .def("copy", copy, args("copy"))
    .def("haveData", &Zond::haveData)
    .def("alt", alt)
    //FIXME можно заменить?  .def("setData", static_cast< void (Zond::*) (int, int, const Uroven&) > (&Zond::setData), args("level_type", "level", "uroven"));
  .def("setData", static_cast< void (Zond::*) (int, float, const Uroven&) > (&Zond::setData), args("level_type", "level", "uroven"));

  def("SA81_P", SA81_P_float);
  def("oprTvirt", oprTvirt_float);

  class_<Uroven>( "Uroven" )
    .def("value", &Uroven::value, args("ValueType"))
    .def("quality", &Uroven::quality, args("ValueType"))
    .def("set", &Uroven::set, args("ValueType, value, quality"))
    .def("isGood", &Uroven::isGood, args("ValueType"))
    .def("show", &Uroven::print)
  ;

  enum_<ValueType>("ValueType")
    .value("UR_P", UR_P)
    .value("UR_H", UR_H)
    .value("UR_T", UR_T)
    .value("UR_Td", UR_Td)
    .value("UR_D", UR_D)
    .value("UR_dd", UR_dd)
    .value("UR_ff", UR_ff)
    .value("UR_u", UR_u)
    .value("UR_v", UR_v)
  ;
}
