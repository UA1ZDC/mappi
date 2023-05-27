#include "astrowrap.h"
#include <meteo/commons/astrodata/observerpoint.h>
#include <cross-commons/debug/tlog.h>


AstroWrap::AstroWrap()
{
  calendar_ = new astro::AstroDataCalendar();
}

AstroWrap::~AstroWrap()
{
  delete calendar_;
  calendar_ = nullptr;
}

void AstroWrap::setStation(float lat, float lon, float height)
{
  astro::ObserverPoint& point = calendar_->observerPoint();
  //error_log << lon << lat << height;
  point.setLatitude(lat);
  point.setLongitude(lon);
  point.setHeight(height);
}

void AstroWrap::setDate( std::string begin, std::string end )
{
  QDateTime dtBegin = QDateTime::fromString(QString::fromStdString(begin), Qt::ISODate);
  QDateTime dtEnd = QDateTime::fromString(QString::fromStdString(end), Qt::ISODate);
  astro::ObserverPoint& p = calendar_->observerPoint();
  p.setFirstDateTime(QDateTime(dtBegin.date().addDays(-1), QTime()));
  p.setSecondDateTime(QDateTime(dtEnd.date(), QTime()));
}

void AstroWrap::calcPoint()
{
  calendar_->clear();
  calendar_->makeUp();
}

std::string AstroWrap::getSunRise()
{
  auto time = calendar_->day(0).getSunRise();
  return time.toString("hh:mm").toStdString();
}

std::string AstroWrap::getSunSet()
{
  auto time = calendar_->day(0).getSunSet();
  return time.toString("hh:mm").toStdString();
}

std::string AstroWrap::getLunarRise()
{
  auto time = calendar_->day(0).getLunarRise();
  return time.toString("hh:mm").toStdString();
}

std::string AstroWrap::getLunarSet()
{
  auto time = calendar_->day(0).getLunarSet();
  return time.toString("hh:mm").toStdString();
}

std::string AstroWrap::getTwilightNauticalBegin()
{
  auto time = calendar_->day(0).getTwilightNauticalBegin();
  return time.toString("hh:mm").toStdString();
}

std::string AstroWrap::getTwilightNauticalEnd()
{
  auto time = calendar_->day(0).getTwilightNauticalEnd();
  return time.toString("hh:mm").toStdString();
}

std::string AstroWrap::getLunarQuartet()
{
  astro::AstroData::lunar_quarter_t quart = calendar_->day(0).getLunarQuarter();
  std::string quartStr;
  switch (quart) {
  case astro::AstroData::lunar_quarter_t::LUNAR_NEW :
    quartStr = QObject::tr("Новолуние").toStdString();
    break;
  case astro::AstroData::lunar_quarter_t::LUNAR_Q1 :
    quartStr = QObject::tr("Первая четверть").toStdString();
    break;
  case astro::AstroData::lunar_quarter_t::LUNAR_FULL :
    quartStr = QObject::tr("Полнолуние").toStdString();
    break;
  case astro::AstroData::lunar_quarter_t::LUNAR_Q3 :
    quartStr = QObject::tr("Последняя четверть").toStdString();
    break;
  case astro::AstroData::lunar_quarter_t::LUNAR_YOUNG :
    quartStr = QObject::tr("Молодая луна").toStdString();
    break;
  case astro::AstroData::lunar_quarter_t::LUNAR_WAXING :
    quartStr = QObject::tr("Прибывающая луна").toStdString();
    break;
  case astro::AstroData::lunar_quarter_t::LUNAR_WANING :
    quartStr = QObject::tr("Убывающая луна").toStdString();
    break;
  case astro::AstroData::lunar_quarter_t::LUNAR_OLD :
    quartStr = QObject::tr("Старая луна").toStdString();
    break;
  case astro::AstroData::lunar_quarter_t::LUNAR_UNKNOWN :
    quartStr = QObject::tr("Неизвестно").toStdString();
    break;
  }
  return  quartStr;
}

BOOST_PYTHON_MODULE(libastrowrap)
{
  boost::python::class_<AstroWrap>( "AstroWrap" )
      .def( "setStation", &AstroWrap::setStation, boost::python::args("lat, lon, height")  )
      .def( "setDate", &AstroWrap::setDate, boost::python::args("begin", "end") )
      .def( "calcPoint", &AstroWrap::calcPoint)
      .def( "getSunRise", &AstroWrap::getSunRise )
      .def( "getSunSet", &AstroWrap::getSunSet )
      .def( "getLunarRise", &AstroWrap::getLunarRise )
      .def( "getLunarSet", &AstroWrap::getLunarSet )
      .def( "getTwilightNauticalBegin", &AstroWrap::getTwilightNauticalBegin )
      .def( "getTwilightNauticalEnd", &AstroWrap::getTwilightNauticalEnd )
      .def( "getLunarQuartet", &AstroWrap::getLunarQuartet )
  ;

}
