#include <boost/python.hpp>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/global/weatherloader.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/proto/surface.pb.h>
#include <meteo/commons/punchrules/punchrules.h>

using namespace boost::python;

QHash<QString,::meteo::puanson::proto::CellRule>  templParams_;

std::string stdStringForParam(meteo::map::Puanson& p,const std::string& name)
{
  return p.stringForParam(QString::fromStdString(name)).toStdString();
}

bool setPunch(meteo::map::Puanson& p,const std::string &path)
{
  return p.setPunch( QString::fromStdString(path) );
}

bool setPunchById(meteo::map::Puanson& p,const std::string &id )
{
  return p.setPunchById( QString::fromStdString(id) );
}

void setParamValue(meteo::map::Puanson& p, const std::string& name, const TMeteoParam& param)
{
  p.setParamValue( QString::fromStdString( name ), param );
}

void loadPunch(const std::string& punch)
{
  meteo::map::WeatherLoader::instance()->loadPunchLibrary();
  meteo::puanson::proto::Puanson tmpl = meteo::map::WeatherLoader::instance()->punchlibrary().value(QObject::tr(punch.data()));
  for ( int i=0,isz=tmpl.rule_size(); i<isz; ++i ) {
    QString param = pbtools::toQString(tmpl.rule(i).id().name());
    templParams_[param] = tmpl.rule(i);
  }
}

std::string stringFromRuleValue( const char* desc, float value )
{
   return meteo::map::stringFromRuleValue(value, templParams_[desc]).toStdString();
}

std::string stringFromRuleValueCode( const char* desc, float value, std::string code )
{
   return meteo::map::stringFromRuleValue(value, templParams_[desc], QString::fromStdString(code)).toStdString();
}

dict stringForCloud(boost::python::object& data, const char* punch)
{
  boost::python::dict stringsForCloud;
  boost::python::extract<boost::python::object> objectExtractor(data);
  boost::python::object o=objectExtractor();
  std::string className = boost::python::extract<std::string>(o.attr("__class__").attr("__name__"));
  if ( 0 != className.compare("MeteoDataProto") ){
    error_log << QObject::tr("Получен объект: %1, ожидался MeteoDataProto").arg(QString::fromStdString(className));
    return stringsForCloud;
  }
  meteo::surf::MeteoDataProto dataProto;
  std::string serialize = boost::python::extract< std::string >( data.attr("SerializeToString")());
  dataProto.ParseFromString(serialize);
  QString punchname = QString::fromStdString(punch);
  bool ok = false;
  meteo::puanson::proto::Puanson punchobj = meteo::map::WeatherLoader::instance()->punchparams( punchname, &ok );
  if ( false == ok ) {
    warning_log << QObject::tr("Шаблон наноски '%1' не найден").arg(punchname);
  }
  QMap< QString, QString > stringMap = meteo::map::stringForCloud( &dataProto, punchobj );
  for ( auto key : stringMap.keys() ) {
    stringsForCloud[key.toStdString()] = stringMap[key].toStdString();
  }
  return stringsForCloud;
}

BOOST_PYTHON_MODULE(libpuansonwrap)
{
  class_<meteo::map::Puanson, boost::noncopyable>( "Puanson" )
    .def( init<const meteo::Property&>(args("property")))
    .def("stringForParam", stdStringForParam, args("name"))
    .def("fontForParam", &meteo::map::Puanson::fontForParam, args("name"))
    .def("colorForParam", &meteo::map::Puanson::colorForParam, args("name"))
    .def("fontForParamHtml", &meteo::map::Puanson::fontForParamHtml, args("name") )
    .def("colorForParamHtml", &meteo::map::Puanson::colorForParamHtml, args("name") )
    .def("setPunch",setPunch, args("path"))
    .def("setPunchById",setPunchById, args("path") )
    .def("setMeteoData",&meteo::map::Puanson::setMeteodata,args("meteoData"))
    .def("setParamValue",setParamValue,args("name", "param"))
  ;
  def("loadPunch", loadPunch);
  def("stringFromRuleValue", stringFromRuleValue);
  def("stringForCloud", stringForCloud);
  def("stringFromRuleValueCode", stringFromRuleValueCode);
}
