#include <boost/python.hpp>
#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <cross-commons/app/paths.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <qcoreapplication.h>

using namespace boost::python;
using namespace meteo::internal;

class PyTAdminSettingsWrap {
public:
  PyTAdminSettingsWrap(){
    if ( nullptr == QCoreApplication::instance() ){
      static char** argv = { };
      static int argc = 0;
      new QCoreApplication(argc, argv);
    }

    TAPPLICATION_NAME( "meteo" );

    if ( nullptr == meteo::gSettings() ){
      auto settings = meteo::global::Settings::instance();
      meteo::gSettings(settings);
      settings->load();
    }

    auto asettings = meteo::TUserSettings::instance();
    if (  nullptr != asettings  ){
      if ( false == asettings->load() ) {
        warning_log << QObject::tr("Ошибка: не удалость загрузить настройки администрирования");
      }
    }
  }
};

boost::python::dict roles(PyTAdminSettingsWrap&)
{
  auto settings = meteo::TUserSettings::instance();
  auto values = settings->roles();  
  boost::python::dict returnValue;
  for ( auto key: values.keys() ){
    auto value = values[key].toUtf8();
    std::size_t size = static_cast<std::size_t>( value.size() );    
    returnValue[key] = boost::python::str( value.data(), size );
  }
  return returnValue;
}

boost::python::object user( PyTAdminSettingsWrap&, const boost::python::object& obj )
{
  const char* value = boost::python::extract<const char*>( str(obj).encode("utf-8"));
  auto settings = meteo::TUserSettings::instance();
  bool ok = false;    
  auto userProto = settings->getUser( QString::fromStdString( value ), &ok );  
  if ( false == ok ) {
    return boost::python::object();
  }
  boost::python::object mode = boost::python::import("appconf_pb2");
  boost::python::object proto = mode.attr("User")(); //Вызов конструктора объекта PYTHON
  proto.attr("login") = userProto.username();
  proto.attr("name") = userProto.name();
  proto.attr("password") = userProto.password();
  proto.attr("role") = userProto.role();
  proto.attr("surname") = userProto.surname();
  proto.attr("patron") = userProto.patron();
  proto.attr("rank") = userProto.rank();
  proto.attr("gender") = static_cast<int>(userProto.gender());
  proto.attr("birthdate") = userProto.birthdate();
  return proto;
}


bool setUser( PyTAdminSettingsWrap&, boost::python::object& opt )
{
  boost::python::extract<boost::python::object> objectExtractor(opt);
  boost::python::object o=objectExtractor();
  const std::string& className = boost::python::extract<std::string>(o.attr("__class__").attr("__name__"));
  if ( 0 != className.compare("User") ){
    error_log << QObject::tr("Получен объект: %1, ожидался User").arg(QString::fromStdString(className));
    return false;
  }  
  meteo::app::User userProto;
  userProto.set_username( boost::python::extract<std::string>( boost::python::str(opt.attr("login")) ) );
  userProto.set_name( boost::python::extract<const char*>( str(opt.attr("name")).encode("utf-8")) );
  userProto.set_password( boost::python::extract<const std::string>( boost::python::str(opt.attr("password")) ) );
  userProto.mutable_role()->set_id( boost::python::extract<int>( opt.attr("role")) );
  userProto.set_surname( boost::python::extract<std::string>( boost::python::str(opt.attr("surname")) ) );
  userProto.set_patron( boost::python::extract<std::string>( boost::python::str(opt.attr("patron")) ) );
  userProto.mutable_rank()->set_id( boost::python::extract<int>( opt.attr("rank")) );
  int gender = boost::python::extract<int>( opt.attr("gender"));
  userProto.set_gender( static_cast<meteo::app::User_GenderType>( gender ) );
  userProto.set_birthdate( boost::python::extract<std::string>( boost::python::str(opt.attr("patron")) )  );

  return meteo::TUserSettings::instance()->setUser(userProto);
}

bool removeUser( PyTAdminSettingsWrap&, const boost::python::object& obj )
{
  const char* login = boost::python::extract<const char*>( str(obj).encode("utf-8"));
  return meteo::TUserSettings::instance()->removeUser(QString::fromStdString(login));
}

boost::python::list dbUsers(PyTAdminSettingsWrap&)
{
  auto users = meteo::TUserSettings::instance()->userList();
  boost::python::list outputData;
  for( auto user : users.keys() ){
    auto data = user.toUtf8();
    size_t size = static_cast<size_t>(data.size());
    outputData.append( std::string(data.data(), size ) );
  }
  return outputData;
}

boost::python::dict ranks( PyTAdminSettingsWrap& )
{
  QMap<int, QString> ranks = meteo::TUserSettings::instance()->ranks();
  boost::python::dict returnValue;
  for ( auto key: ranks.keys() ){
    auto value = ranks[key].toUtf8();
    std::size_t size = static_cast<std::size_t>( value.size() );
    returnValue[key] = boost::python::str( value.data(), size );
  }
  return returnValue;
}

boost::python::dict getGenders( PyTAdminSettingsWrap& )
{
  const QMap<int, QString> &genders = meteo::TUserSettings::instance()->genders();
  boost::python::dict returnValues;
  for ( auto key: genders.keys() ){
    auto value = genders[key].toUtf8();
    std::size_t size = static_cast<std::size_t>( value.size() );
    returnValues[key] = boost::python::str( value.data(), size );
  }
  return returnValues;
}

bool checkPassword( PyTAdminSettingsWrap&, boost::python::object& login, boost::python::object& password )
{
  const std::string clogin = boost::python::extract<std::string>( boost::python::str(login) );
  const std::string cpassword = boost::python::extract<std::string>( boost::python::str(password) );
  return meteo::TUserSettings::instance()->authUser( QString::fromStdString(clogin), QString::fromStdString(cpassword) );
}

BOOST_PYTHON_MODULE(libsettingswrap)
{
  class_<PyTAdminSettingsWrap>( "PyTAdminSettingsWrap" )
    .def("roles",roles)
    .def("ranks", ranks)
    .def("user", user )
    .def("setUser", setUser)
    .def("removeUser", removeUser)
    .def("dbUsers", dbUsers)
    .def("genders", getGenders)
    .def("checkPassword", checkPassword);
}
