#include <qapplication.h>
#include <qcolor.h>
#include <qdir.h>
#include <qimage.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qstringlist.h>

#include <cross-commons/app/targ.h>
#include <cross-commons/app/helpformatter.h>
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>

#include <commons/funcs/tcolorgrad.h>

#include <mappi/landmask/landmask.h>
#include <commons/landmask/elevation.h>

#define HELP(opt, text)  (QStringList() << opt << QObject::tr(text))

const QStringList kHelpOpt = QStringList() << "h" << "help";
const QStringList kLandOpt = QStringList() << "l" << "land";
const QStringList kElevOpt = QStringList() << "e" << "evelation";
const QStringList kGlobeOpt= QStringList() << "g" << "globe";
const QStringList kDayOpt  = QStringList() << "d" << "day";
const QStringList kSstOpt  = QStringList() << "s" << "sst";
const QStringList kAlbOpt  = QStringList() << "a" << "albedo";
const QStringList kVapOpt  = QStringList() << "v" << "vapor";
const QStringList kCoefOpt = QStringList() << "k" << "";

const HelpFormatter kHelp = HelpFormatter()
  << HELP(kHelpOpt, "Эта справка.")
  << HELP(kLandOpt, "Маска суша-вода.")
  << HELP(kElevOpt, "Перепад высот.")
  << HELP(kGlobeOpt, "Перепад высот (GLOBE).")
  << HELP(kDayOpt, "Маска день-ночь.")
  << HELP(kSstOpt, "Температура поверхности моря.")
  << HELP(kAlbOpt, "Альбедо.")
  << HELP(kVapOpt, "Плотность водяного пара.")
  << HELP(kCoefOpt, "коэффициент")
     ;

void createWindow(const QImage& img, const QString& title);

int main(int argc, char** argv)
{
  TAPPLICATION_NAME("meteo");

  QApplication* app = new QApplication(argc,argv);

  TArg args(argc,argv);

  if ( args.contains(kHelpOpt) ) {
    kHelp.print();
    return EXIT_SUCCESS;
  }

  bool landOn = args.contains(kLandOpt);
  bool elevOn = args.contains(kElevOpt);
  bool globeOn= args.contains(kGlobeOpt);
  bool dayOn  = args.contains(kDayOpt);
  bool sstOn  = args.contains(kSstOpt);
  bool albOn  = args.contains(kAlbOpt);
  bool vapOn  = args.contains(kVapOpt);
  bool allOn  = !landOn && !elevOn && !globeOn && !dayOn && !sstOn && !albOn && !vapOn;

  ::mappi::LandMask mask;

  const double kBegLon = 0.0;
  const double kEndLon = 360.0;
  const double kBegLat = 0.0;
  const double kEndLat = 180.0;

//  const double kBegLon = 188.0;
//  const double kEndLon = 190.0;
//  const double kBegLat = 125.0;
//  const double kEndLat = 128.0;

  const int kLon = 180;
  const int kLat = 90;

  const int k = args.value(kCoefOpt, "4").toInt();
  const double step = 1./k;

  const int kImgWidth  = (kEndLon - kBegLon) * k;
  const int kImgHeight = (kEndLat - kBegLat) * k;

  QImage landImg(QSize(kImgWidth,kImgHeight), QImage::Format_ARGB32);
  landImg.fill(Qt::white);

  TColorGradList elevGrad;
  elevGrad.append( TColorGrad(0,1,QColor(120,120,255)) );
  elevGrad.append( TColorGrad(1,100,QColor(0,127,0)) );
  elevGrad.append( TColorGrad(100,250,QColor(30,150,0)) );
  elevGrad.append( TColorGrad(250,500,QColor(70,170,0)) );
  elevGrad.append( TColorGrad(500,1000,QColor(120,200,0)) );
  elevGrad.append( TColorGrad(1000,2000,QColor(200,200,0)) );
  elevGrad.append( TColorGrad(2000,4000,QColor(200,150,0)) );
  elevGrad.append( TColorGrad(4000,5000,QColor(200,50,0)) );
  elevGrad.append( TColorGrad(5000,7000,QColor(255,255,255)) );

  info_log << QObject::tr("Количество координат: %1").arg(kImgHeight*kImgWidth);

  if ( allOn || landOn || dayOn || sstOn )
  {
    bool r = mask.initLandSea();

    info_log << (r ? "[ ok ]" : "[fail]") << "landsea init";

    QRgb* pixel = reinterpret_cast<QRgb*>(landImg.bits());
    for ( int x = 0; x < kImgWidth; ++x ) {
      double lon = (kBegLon + x * step);
      for ( int y = 0; y < kImgHeight; ++y ) {
        double lat = (kBegLat + y * step);
        int v = mask.land(::meteo::GeoPoint((lat-kLat)*::meteo::DEG2RAD, (lon-kLon)*::meteo::DEG2RAD));
        int i = y * kImgWidth + x;
        pixel[i] = v == 1 ? QColor(200,200,10).rgba() : QColor(10,10,200).rgba();
      }
    }

    info_log << "[done] landsea calc";

    if ( allOn || landOn ) {
      createWindow(landImg.mirrored(), QObject::tr("Маска 'суша-вода'"));
    }
  }

  if ( allOn || elevOn )
  {
    bool r = ERR_NOERR == ::meteo::map::LandMask::instance()->init_elevation();
    info_log << (r ? "[ ok ]" : "[fail]") << "elevation init";

    QImage elevImg(QSize(kImgWidth,kImgHeight), QImage::Format_ARGB32);
    elevImg.fill(Qt::white);

    QRgb* pixel = reinterpret_cast<QRgb*>(elevImg.bits());
    for ( int x = 0; x < kImgWidth; ++x ) {
      double lon = (kBegLon + x * step);
      for ( int y = 0; y < kImgHeight; ++y ) {
        double lat = (kBegLat + y * step);
        int v = ::meteo::map::LandMask::instance()->elevation((lat-kLat), (lon-kLon));
        int i = y * kImgWidth + x;
        pixel[i] = elevGrad.color(v, QColor(125,125,255)).rgb();
      }
    }

    info_log << "[done] elevation calc";

    elevImg = elevImg.mirrored();
    createWindow(elevImg, QObject::tr("Маска 'перепад высот'"));
  }


  if ( allOn || globeOn )
  {
    bool r = mask.initElevation();
    info_log << (r ? "[ ok ]" : "[fail]") << "elevation GLOBE init";

    QImage img(QSize(kImgWidth,kImgHeight), QImage::Format_ARGB32);
    img.fill(Qt::white);

    QRgb* pixel = reinterpret_cast<QRgb*>(img.bits());
    for ( int y = 0; y < kImgHeight; ++y ) {
      double lat = (kBegLat + y * step);
      for ( int x = 0; x < kImgWidth; ++x ) {
        double lon = (kBegLon + x * step);
        int v = mask.elevation(::meteo::GeoPoint((lat-kLat)*::meteo::DEG2RAD, (lon-kLon)*::meteo::DEG2RAD));
        int i = y * kImgWidth + x;
        pixel[i] = elevGrad.color(v, QColor(125,125,255)).rgb();
      }
    }

    info_log << "[done] elevation GLOBE calc";

    img = img.mirrored();
    createWindow(img, QObject::tr("Перепад высот (GLOBE)"));
  }

  if ( allOn || dayOn )
  {
    bool r = mask.initDayNight(QDateTime::currentDateTimeUtc());
    info_log << (r ? "[ ok ]" : "[fail]") << "day-night init";

    QImage dayImg = landImg;

    QRgb* pixel = reinterpret_cast<QRgb*>(dayImg.bits());
    for ( int x = 0; x < kImgWidth; x += k*4 ) {
      double lon = (kBegLon + x * step);
      for ( int y = 0; y < kImgHeight; y += k*4 ) {
        double lat = (kBegLat + y * step);
        int v = mask.day(::meteo::GeoPoint((lat-kLat)*::meteo::DEG2RAD, (lon-kLon)*::meteo::DEG2RAD, 100));
        int i = y * kImgWidth + x;
        pixel[i] = 1 == v ? QColor(Qt::white).rgba() : QColor(Qt::black).rgba();
      }
    }

    info_log << "[done] day-night calc";

    dayImg = dayImg.mirrored();
    createWindow(dayImg, QObject::tr("Маска 'день-ночь'"));
  }

  if ( allOn || sstOn )
  {
    bool r = mask.initSst(QDate::currentDate().month());
    info_log << (r ? "[ ok ]" : "[fail]") << "sst init";

    QImage sstImg = landImg;

    QRgb* pixel = reinterpret_cast<QRgb*>(sstImg.bits());
    for ( double lon = kBegLon; lon < kEndLon; lon += step ) {
      for ( double lat = kBegLat; lat < kEndLat; lat += step ) {
        int v = mask.sst(::meteo::GeoPoint((lat-kLat)*::meteo::DEG2RAD, (lon-kLon)*::meteo::DEG2RAD));
        int y = (lat-kBegLat)*k;
        int x = (lon-kBegLon)*k;
        int i = y * kImgWidth + x;

        QColor c = QColor(125,125,255);
        if ( -9999 != v ) {
          v = (v + 350) / 3500.0 * 255;
          c = QColor(0,0,v);
        }
        pixel[i] = c.rgba();
      }
    }
    info_log << "[done] sst calc";

    sstImg = sstImg.mirrored();
    createWindow(sstImg, QObject::tr("Температура поверхности моря"));
  }

  if ( allOn || albOn )
  {
    bool r = mask.initAlbedo(QDate::currentDate().month());
    info_log << (r ? "[ ok ]" : "[fail]") << "albedo init";

    QImage albImg(QSize(kImgWidth,kImgHeight), QImage::Format_ARGB32);
    albImg.fill(Qt::white);

    QRgb* pixel = reinterpret_cast<QRgb*>(albImg.bits());
    for ( double lon = kBegLon; lon < kEndLon; lon += step ) {
      for ( double lat = kBegLat; lat < kEndLat; lat += step ) {
        int v = mask.albedo(::meteo::GeoPoint((lat-kLat)*::meteo::DEG2RAD, (lon-kLon)*::meteo::DEG2RAD));
        QColor c = QColor(120,120,255);
        if ( -10000 == v ) {
          c = QColor(150,150,255);
        }
        else if ( -9999 != v ) {
          v = (v + 10000) / 100. / 200.0 * 255;
          c = QColor(v,v,v);
        }
        int y = (lat-kBegLat)*k;
        int x = (lon-kBegLon)*k;
        int i = y * kImgWidth + x;
        pixel[i] = c.rgba();
      }
    }

    info_log << "[done] albedo calc";

    albImg = albImg.mirrored();
    createWindow(albImg, QObject::tr("Маска 'альбедо'"));
  }

  if ( allOn || vapOn )
  {
    bool r = mask.initHumidity(QDate::currentDate().month());
    info_log << (r ? "[ ok ]" : "[fail]") << "water vapor init";

    QImage wvImg(QSize(kImgWidth,kImgHeight), QImage::Format_ARGB32);
    wvImg.fill(Qt::white);

    QRgb* pixel = reinterpret_cast<QRgb*>(wvImg.bits());
    for ( double lon = kBegLon; lon < kEndLon; lon += step ) {
      for ( double lat = kBegLat; lat < kEndLat; lat += step ) {
        int v = mask.waterVapor(::meteo::GeoPoint((lat-kLat)*::meteo::DEG2RAD, (lon-kLon)*::meteo::DEG2RAD, 100));
        QColor c = QColor(120,120,255);
        if ( -9999 != v ) {
          v = v / 600.0 * 255;
          c = QColor(v,v,0);
        }
        int y = (lat-kBegLat)*k;
        int x = (lon-kBegLon)*k;
        int i = y * kImgWidth + x;
        pixel[i] = c.rgba();
      }
    }

    info_log << "[done] water vapor calc";

    wvImg = wvImg.mirrored();
    createWindow(wvImg, QObject::tr("Плотность водяного пара"));
  }

  app->exec();

  delete app;

  return EXIT_SUCCESS;
}

void createWindow(const QImage& img, const QString& title)
{
  QScrollArea* sa = new QScrollArea;

  QLabel* lbl = new QLabel(sa);
  lbl->setPixmap(QPixmap::fromImage(img));
  sa->setWindowTitle(title);
  sa->setWidget(lbl);
  sa->show();
}
