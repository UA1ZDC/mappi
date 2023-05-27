#include <stdlib.h>

#include <qcoreapplication.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/global/weatherloader.h>
#include <meteo/bank/settings/banksettings.h>
#include <meteo/bank/global/global.h>

void updateLibrary( auto container )
{
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  if ( nullptr == md ) {
    error_log << QObject::tr("Нулевой казатель.");
    return;
  }
  meteo::map::internal::WeatherLoader* wl = meteo::map::WeatherLoader::instance();
  for ( auto p : container ) {
    for ( int i = 0, sz = p.rule_size(); i < sz; ++i ) {
      meteo::puanson::proto::Id* id = p.mutable_rule(i)->mutable_id();
      QString name = QString::fromStdString( id->name() );
      QString parent;
      if ( true == md->isComponent( name, &parent ) ) {
        id->set_parent( parent.toStdString() );
      }
    }
    wl->addPunchToLibrary(p);
  }
}

int main( int argc, char* argv[] )
{
  TAPPLICATION_NAME("meteo");
  QCoreApplication app( argc, argv );
  ::meteo::gGlobalObj(new ::meteo::BankGlobal);

  if ( false == meteo::bank::Settings::instance()->load() ) {
    error_log << QObject::tr("Не удалось загрузить настройки.");
    return EXIT_FAILURE;
  }

  meteo::map::internal::WeatherLoader* wl = meteo::map::WeatherLoader::instance();
  if ( nullptr == wl ) {
    error_log << QObject::tr("Нулевой казатель.");
    return EXIT_FAILURE;
  }

  auto punches = wl->punchlibrary();
  updateLibrary(punches);
  punches = wl->punchlibraryspecial();
  updateLibrary(punches);

  return EXIT_SUCCESS;
}
