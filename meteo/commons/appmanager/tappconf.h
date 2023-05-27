#ifndef METEO_COMMONS_APPMANAGER_TAPPCONF_H
#define METEO_COMMONS_APPMANAGER_TAPPCONF_H

namespace meteo {
namespace app {

class AppConfig;

class Conf
{
private:
  Conf() = delete;
  ~Conf() = delete;
public:
  static bool load( AppConfig* conf );
  static void killProcess();
};

} // app
} // meteo

#endif
