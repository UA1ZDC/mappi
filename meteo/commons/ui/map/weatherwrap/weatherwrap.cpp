#include "weatherwrap.h"

#include <qbuffer.h>
#include <qpainter.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/map/weather.h>
#include <meteo/commons/ui/map/vprofiledoc.h>
#include <meteo/commons/ui/map/legend.h>

#include <qapplication.h>
#include <qcoreapplication.h>
#include <qtimer.h>


namespace meteo {
namespace map {

CreateMap::CreateMap(proto::Map& map, meteo::map::VProfileDoc* doc ) :
  QObject(),
  map_(map),
  doc_(doc)
{
}

void CreateMap::slotCreateMap()
{
  meteo::map::Weather w;
  w.createMap(doc_, map_);
  emit finished();
}

VProfiler::VProfiler()
{
  doc_ = new VProfileDoc();
  auto prop = doc_->property();
  prop.set_projection(ProjectionType::kGeneral);
  doc_->setProperty(prop);
  map_.set_source(proto::DataSource::kField);
  map_.set_data_type(surf::DataType::kAeroFix);
  map_.set_format(proto::DocumentFormat::kPng);
  map_.mutable_document()->mutable_doc_center()->set_lat_deg(22500);
  map_.mutable_document()->mutable_doc_center()->set_lon_deg(35000);
  map_.mutable_document()->mutable_mapsize()->set_height(500);
  map_.mutable_document()->mutable_mapsize()->set_width(1500);
  map_.mutable_document()->mutable_docsize()->set_height(440);
  map_.mutable_document()->mutable_docsize()->set_width(1320);
  map_.mutable_document()->set_doctype(proto::DocumentType::kVerticalCut);
//  map_.set_hour(6);
  map_.set_center(4);
}

 boost::python::object VProfiler::createVProfile()
{
  int argc = 1;
  char* test = new char[5];
  memcpy(test, "test", 5);
  char** test2 = new char*[1];
  test2[0] = test;
  QCoreApplication* app = new QCoreApplication( argc, test2, false );
  map_.set_offsetutc(av12proto_.shifttime());
  CreateMap cm(map_, doc_);
  QTimer::singleShot(200, &cm, SLOT( slotCreateMap() ) );
  QObject::connect(&cm, SIGNAL(finished()), app, SLOT( quit() ));
  app->exec();
  doc_->legend()->setVisible(false);
  QByteArray arr;
  QImage pix(doc_->documentsize(), QImage::Format_ARGB32);
  pix.fill(doc_->backgroundColor());
  QPainter pntrUtf8(&pix);
  doc_->drawDocument(&pntrUtf8);
  QBuffer buf(&arr);
  buf.open(QIODevice::WriteOnly);
  pix.save(&buf, QString("PNG").toUtf8().data(), 100);

  boost::python::object data(boost::python::handle<>(PyBytes_FromStringAndSize(
    reinterpret_cast<const char*>(arr.data()) , 
    static_cast<Py_ssize_t>(arr.size()) )));
  return data;
}

void VProfiler::addStation(const std::string station)
{
  surf::Point* p = map_.add_point();
  p->set_index(station);
}

void VProfiler::setDatetime(const std::string dateTime)
{
  map_.set_datetime(dateTime);
}

void VProfiler::setEchelon(int echelon)
{
  map_.set_echelon(echelon);
}

void VProfiler::setSerialProto(const std::string serialProto) {
  // std::string serialize = boost::python::extract< std::string >( serialProto );
  // av12proto_.ParseFromString(serialize);
  av12proto_.ParseFromString(serialProto);
}

void VProfiler::initFromProto()
{
  if (0 < av12proto_.stations_size()) {
    for(int i=0;i<3;++i) {
      for(int j=0, sz=av12proto_.stations_size();j<sz;++j) {
        if(i == av12proto_.stations(j).av12type()) {
          surf::Point* p = map_.add_point();
          p->CopyFrom(av12proto_.stations(j));
          if ( av12proto_.typetime() == meteo::map::proto::kNoUTC ) {
            QDateTime dateTime = QDateTime::fromString(QString::fromStdString( p->date_time() ), "yyyy-MM-ddThh:mm:ss");
            dateTime = dateTime.addSecs(-av12proto_.shifttime());
            p->set_date_time(dateTime.toString(Qt::ISODate).toStdString());
          }
        }
      }
    }
  }
  if(true == av12proto_.has_echelon()) {
    map_.set_echelon(av12proto_.echelon());
  }
  if ( true == av12proto_.has_date() ){
    if ( true == av12proto_.has_timeend() ) {
      QDateTime dateTime;
      dateTime.setDate(QDate::fromString(QString::fromStdString(av12proto_.date()), "yyyy-MM-dd"));
      dateTime.setTime(QTime::fromString(QString::fromStdString(av12proto_.timeend()), "hh:mm:ss"));
      if ( av12proto_.typetime() == meteo::map::proto::kNoUTC ) {
        dateTime = dateTime.addSecs(-av12proto_.shifttime());
      }
      int hour = QTime::fromString(QString::fromStdString(av12proto_.timestart()), "hh:mm:ss").msecsTo(QTime::fromString(QString::fromStdString(av12proto_.timeend()), "hh:mm:ss"))/1000/60/60;
      map_.set_datetime(dateTime.toString(Qt::ISODate).toStdString());
      map_.set_hour(hour);
//      map_.set_hour(12);
    }
  }
  if ( map_.point_size() > 2){
    map_.set_profile(proto::ProfileType::kBulletinSpace);
  }
  else if ( (map_.point_size() <= 2)
         && (map_.point(0).index() != map_.point(1).index())) {
    map_.set_profile(proto::ProfileType::kBulletinSpace);
  }
  else {
    map_.set_profile(proto::ProfileType::kBulletinTime);
  }
}

void VProfiler::initAv12d()
{
  map_.mutable_document()->mutable_doc_center()->set_lat_deg(22500);
  map_.mutable_document()->mutable_doc_center()->set_lon_deg(35000);
  map_.mutable_document()->mutable_mapsize()->set_height(500);
  map_.mutable_document()->mutable_mapsize()->set_width(500);
  map_.mutable_document()->mutable_docsize()->set_height(440);
  map_.mutable_document()->mutable_docsize()->set_width(1320);
}

BOOST_PYTHON_MODULE(libweatherwrap)
{
  boost::python::class_<VProfiler>( "VProfiler" )
      .def( "createVProfile", &VProfiler::createVProfile )
      .def( "addStation", &VProfiler::addStation, boost::python::args("station") )
      .def( "setDatetime", &VProfiler::setDatetime, boost::python::args("setDatetime") )
      .def( "setEchelon", &VProfiler::setEchelon, boost::python::args("setEchelon") )
      .def( "setSerialProto", &VProfiler::setSerialProto, boost::python::args("serialProto") )
      .def( "initFromProto", &VProfiler::initFromProto)
      .def( "initAv12d", &VProfiler::initAv12d)
  ;

}

}
}
