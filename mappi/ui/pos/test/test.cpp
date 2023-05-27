#include <stdlib.h>

#include <qbytearray.h>
#include <qdatastream.h>
#include <qapplication.h>
#include <qimage.h>
#include <qpainter.h>
#include <qfileinfo.h>
#include <qdebug.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/app/options.h>
#include <cross-commons/debug/tlog.h>
#include <sql/dbi/dbiquery.h>
#include <sat-commons/satellite/satellite.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/ui/map/loader.h>
#include <meteo/commons/ui/map/loader/old.geo/oldgeo.h>
#include <meteo/commons/ui/map/loader/ptkpp.geo/ptkppgeo.h>
#include <mappi/global/global.h>
#include <mappi/proto/sessiondataservice.pb.h>
#include <mappi/proto/fileservice.pb.h>
#include <mappi/settings/mappisettings.h>
#include <mappi/ui/pos/pos.h>
#include <mappi/ui/pos/posdocument.h>

#include "thread.h"

static commons::ArgParser* options = commons::ArgParser::instance();

const commons::Arg kDbid   = commons::Arg::make( "-i", "--id",      true );
const commons::Arg kCh     = commons::Arg::make( "-c", "--channel", true );
const commons::Arg kType   = commons::Arg::make( "-t", "--type",    true );
const commons::Arg kLoader = commons::Arg::make( "-l", "--loader",    true );
const commons::Arg kHelp   = commons::Arg::make( "-h", "--help",    false );

std::string paramhelp( const commons::Arg& arg, const QString& descr )
{
  QString str = QObject::tr("    %1")
                .arg( arg.help() ).leftJustified(40);
  return (str+descr).toStdString();
}

void usage()
{
  std::cout << QObject::tr("Отрисовка береговой черты на снимке").toStdString() << '\n';
  std::cout
      << QObject::tr("Использование: ").toStdString()
      << qApp->applicationName().toStdString() << '\n'
      << QObject::tr("Параметры:").toStdString()
      << '\n';

  std::cout << paramhelp( kDbid,        QObject::tr("id сессиии в базе данных") ) << '\n';
  std::cout << paramhelp( kCh,          QObject::tr("Номер канала") ) << '\n';
  std::cout << paramhelp( kType,        QObject::tr("Тип преобразования (f2x - наноска береговой черты на картинку, x2f - наноска снимка на карту в стереографиеской проекции)") ) << '\n';
  std::cout << paramhelp( kLoader,      QObject::tr("Географическая основа (geo.old или ptkpp)") ) << '\n';
  std::cout << paramhelp( kHelp,        QObject::tr("Эта справка") ) << '\n';
}

bool getImage( int sessionid, int numch, QImage* img, QString* filename )
{
  meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
  if ( 0 == ch ) {
    error_log << QObject::tr("Ошибка подключения к сервису данных");
    return false;
  }
  mappi::proto::ThematicData request;
  request.set_session_id(sessionid);
  request.set_channel_number(numch);
  request.set_type(mappi::proto::kGrayScale);

  mappi::proto::ThematicList* response = ch->remoteCall(
      &mappi::proto::SessionDataService::GetAvailableThematic,
      request,
      10000,
      true
      );
  delete ch;
  if ( 0 == response ) {
    error_log << QObject::tr("Ошибка получения ответа от сервиса данных");
    return false;
  }
  if ( false == response->result() ) {
    error_log << QObject::tr("Запрос изображения выполнен с ошибкой = %1")
      .arg( QString::fromStdString( response->comment() ) );
    delete response;
    return false;
  }
  if ( 0 == response->themes_size() ) {
    error_log << QObject::tr("Пустой ответ от сервиса данных");
    delete response;
    return false;
  }
  std::string path = response->themes(0).path();

  QFileInfo fi( QString::fromStdString(path) );
  *filename = fi.fileName();

  delete response;

  Thread thread(path);
  thread.start();
  bool res = thread.wait();
  if ( false == res ) {
    error_log << QObject::tr("Файл %1 не получен")
      .arg( QString::fromStdString(path) );
    return false;
  }

  *img = QImage::fromData( thread.data() );

  return true;
}

struct Params {
  QDateTime start;
  QDateTime end;
  MnSat::TLEParams tle;
};

bool getSession(  int sessionid, Params* params )
{
  ::meteo::rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kDataControl );
  if ( 0 == ch ) {
    error_log << QObject::tr("Ошибка подключения к сервису данных");
    return false;
  }
  mappi::proto::SessionData request;
  request.set_id(sessionid);

  mappi::proto::SessionList* response = ch->remoteCall(
      &mappi::proto::SessionDataService::GetSession,
      request,
      10000,
      true
      );
  delete ch;
  if ( 0 == response ) {
    error_log << QObject::tr("Ошибка получения ответа от сервиса данных");
    return false;
  }
  if ( false == response->result() ) {
    error_log << QObject::tr("Запрос параметров сессии выполнен с ошибкой = %1")
      .arg( QString::fromStdString( response->comment() ) );
    delete response;
    return false;
  }
  if ( 0 == response->sessions_size() ) {
    error_log << QObject::tr("Параметры сессии %1 не найдены")
      .arg( sessionid );
    delete response;
    return false;
  }
  params->start = meteo::DbiQuery::datetimeFromString( response->sessions(0).date_start() );
  params->end = meteo::DbiQuery::datetimeFromString( response->sessions(0).date_end() );
  QByteArray arr = QByteArray::fromBase64( QByteArray( response->sessions(0).tle().data(), response->sessions(0).tle().size() ) );


  delete response;

  QDataStream stream(arr);
  stream >> params->tle;

  return true;
}

bool buildImageF2X( const Params& params, QImage* img )
{

  Satellite s;
  s.setTLEParams(params.tle);


  commons::ArgParser* options = commons::ArgParser::instance();
  QString loader("geo.old");
  if ( true == options->installed(kLoader) ) {
    loader = options->at(kLoader).value();
  }

  meteo::map::proto::Document proto;
  proto.set_opengeopolygon(true);
  proto.set_projection( meteo::kSatellite );
  proto.mutable_docsize()->CopyFrom( meteo::qsize2size( QSize( img->width(), img->height() ) ) );
  proto.set_scale(0);
  proto.mutable_mapsize()->CopyFrom( proto.docsize() );
  proto.set_geoloader( loader.toStdString() );
//  proto.set_geoloader("ptkpp");

  meteo::map::Loader::instance();

  meteo::map::PosDocument doc(proto);
  *img = img->convertToFormat( QImage::Format_ARGB32 );
  doc.init( params.start, params.end, &s, 55.37*meteo::DEG2RAD, 2048, 6 );
  QPainter pntr(img);
  doc.setScreenCenter( QPoint(img->width()/2, img->height()/2) );
  doc.drawDocument(&pntr, QRect( QPoint(0,0), img->size() ), doc.documentRect() );
  return true;
}

bool buildImageX2F( const Params& params, QImage* img )
{
  Satellite s;
  s.setTLEParams(params.tle);
  meteo::POSproj pos;
  pos.setSatellite( params.start, params.end, &s, 55.37*meteo::DEG2RAD, 2048, 6 );
  meteo::GeoPoint gp = pos.getMapCenter();

  meteo::map::proto::Document proto;
  proto.set_projection( meteo::kStereo );
  proto.mutable_docsize()->CopyFrom( meteo::qsize2size( QSize( 2048, 1536 ) ) );
  proto.set_scale(16);
  proto.mutable_mapsize()->CopyFrom( proto.docsize() );
  proto.mutable_doc_center()->CopyFrom( meteo::geopoint2pbgeopoint(gp) );
  proto.mutable_map_center()->CopyFrom(proto.doc_center());
  proto.set_geoloader("geo.old");

  meteo::map::Loader::instance();

  meteo::map::Document doc(proto);

  doc.init();
  QImage mapimg( 2048, 2048, QImage::Format_ARGB32 );
  mapimg.fill( doc.backgroundColor() );
  QPainter pntr(&mapimg);
  doc.setScreenCenter( QPoint(mapimg.width()/2, mapimg.height()/2) );
  doc.drawDocument(&pntr, QRect( QPoint(0,0), img->size() ), doc.documentRect() );
  QImage copy( mapimg.width(), mapimg.height(), QImage::Format_ARGB32 );
  copy.fill( QColor(0,0,0,0) );
  unsigned char* pixels = img->bits();
  unsigned char* tpixels = copy.bits();
  unsigned char trpr = 255;
  pntr.end();
  pntr.begin(&copy);
  doc.drawDocument(&pntr, QRect( QPoint(0,0), copy.size() ), doc.documentRect() );
  for ( int i = 0; i < img->height(); ++i ) {
    for ( int j = 0; j < img->width(); ++j ) {
      meteo::GeoPoint gp;
      pos.X2F_one( QPointF(j - img->width()*0.5, i - img->height()*0.5), &gp );
      QPoint pnt = doc.coord2screen(gp);
      unsigned char val = pixels[j+i*img->width()];
      ::memcpy(tpixels+(pnt.x()+pnt.y()*mapimg.width())*4, &val, 1);
      ::memcpy(tpixels+(pnt.x()+pnt.y()*mapimg.width())*4+1, &val, 1);
      ::memcpy(tpixels+(pnt.x()+pnt.y()*mapimg.width())*4+2, &val, 1);
      ::memcpy(tpixels+(pnt.x()+pnt.y()*mapimg.width())*4+3, &trpr, 1);
    }
  }
  auto layers = doc.layers();
  for ( auto l : layers ) {
    if ( QObject::tr("Береговая черта") == l->name() ) {
      for ( auto o : l->objects() ) {
        o->setClosed(false);
      }
//      pntr.translate( -doc.documentRect().topLeft().x(), -doc.documentRect().topLeft().y() );
//      l->render(&pntr, doc.documentRect(), &doc );
    }
    else {
      l->setVisisble(false);
    }
  }
  doc.drawDocument(&pntr, QRect( QPoint(0,0), copy.size() ), doc.documentRect() );
  pntr.end();
  *img = copy;
  return true;
}

int main( int argc, char* argv[] )
{
  bool x2f = false;
  TAPPLICATION_NAME("meteo");
  
  // ::meteo::gGlobalObj(new ::mappi::MappiGlobal);
  // if ( false == meteo::mappi::TMeteoSettings::instance()->load() ) {
  //   error_log << QObject::tr("Не удалось загрузить настройки.");
  //   return EXIT_FAILURE;
  // }

      
  meteo::gSettings(mappi::inter::Settings::instance());
  
  if ( false == meteo::gSettings()->load() ) {
    error_log << meteo::msglog::kSettingsLoadFailed;
    return EXIT_FAILURE;
  }
    
  QApplication app( argc, argv, false );
  
  commons::ArgParser* options = commons::ArgParser::instance();
  if ( false == options->parse( argc, argv ) ) {
    usage();
    return EXIT_FAILURE;
  }
  if ( true == options->installed(kHelp) ) {
    usage();
    return EXIT_SUCCESS;
  }
  if ( false == options->installed(kDbid) ) {
    error_log << QObject::tr("Не указан id сессии в базе данных");
    usage();
    return EXIT_FAILURE;
  }
  if ( false == options->installed(kCh) ) {
    error_log << QObject::tr("Не указан номер канала");
    usage();
    return EXIT_FAILURE;
  }
  if ( true == options->installed(kType) && "x2f" == options->at(kType).value() ) {
    x2f = true;
  }
  QImage img;
  QString filename;
  bool res = getImage( options->at(kDbid).value().toInt(), options->at(kCh).value().toInt(), &img, &filename );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось получить изображение");
    return EXIT_FAILURE;
  }
  Params params;
  res = getSession( options->at(kDbid).value().toInt(), &params );
  if ( false == res ) {
    error_log << QObject::tr("Не удалось получить параметры сессии");
    return EXIT_FAILURE;
  }

  debug_log << QObject::tr("Расчетное количество строк = %1")
    .arg( ( ( params.start.msecsTo( params.end ) )*0.001 )*6.0 );
  debug_log << QObject::tr("Истинное количество строк = %2")
    .arg(img.height());

  if ( false == x2f ) {
    res = buildImageF2X( params, &img );
  }
  else {
    res = buildImageX2F( params, &img );
  }
  img.save(filename);

  return EXIT_SUCCESS;
}
