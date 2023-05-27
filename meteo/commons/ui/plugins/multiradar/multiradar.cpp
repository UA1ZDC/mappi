#include "multiradar.h"
#include "ui_multiradar.h"
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>

#include <qfile.h>
#include <qhash.h>
#include <qtemporaryfile.h>
#include <qmovie.h>

#include <Magick++.h>

#include <cross-commons/app/paths.h>
#include <sql/nspgbase/tsqlquery.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/layermrl.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/widgets/layerswidget.h>
#include <meteo/commons/global/weatherloader.h>

namespace meteo {
namespace map {

MultiRadar::MultiRadar( MapWindow* view ) :
  MapWidget(view),
  ui_(new Ui::MultiRadar),
  movie_(new QMovie),
  item_(0),
  animation_(false)
{
  ui_->setupUi(this);
  QObject::connect( ui_->spinpast, SIGNAL( valueChanged( int ) ), this, SLOT( slotPastSpinChanged( int ) ) );
  QObject::connect( ui_->spinfuture, SIGNAL( valueChanged( int ) ), this, SLOT( slotFutureSpinChanged( int ) ) );
  QObject::connect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotAccept() ) );
  QObject::connect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( slotReject() ) );
}

MultiRadar::~MultiRadar()
{
  delete item_; item_ = 0;
  delete movie_; movie_ = 0;
  delete ui_; ui_ = 0;
}

void MultiRadar::setCurrentLayer( Layer* layer )
{
  currentlayer_ = layer;
  current_ = layer->info();
  QString rdrname = QString::fromStdString( current_.template_name() );
  if ( false == WeatherLoader::instance()->radarlibrary().contains(rdrname) ) {
    error_log.msgBox() << QObject::tr("Не найден шаблон наноски радаров %1").arg(rdrname);
    return;
  }

  currentcolor_ = WeatherLoader::instance()->radarlibrary()[rdrname];

  curdt_ = TSqlQuery::datetimeFromString( current_.datetime() );
  if ( false == curdt_.isValid() ) {
    ui_->lblterm->setText( QObject::tr("Срок не выбран") );
    ui_->okbtn->setDisabled(true);
    ui_->spinpast->setDisabled(true);
    ui_->spinfuture->setDisabled(true);
  }
  else {
    ui_->lblterm->setText( curdt_.toString("dd-MM-yyyy hh:mm") );
    ui_->lblpast->setText( QObject::tr("-%1 часов").arg( ui_->spinpast->value() ) );
    ui_->lblfuture->setText( QObject::tr("+%1 часов").arg( ui_->spinpast->value() ) );
  }
}

void MultiRadar::stopAnimation()
{
  animation_ = false;
  movie_->stop();
  delete item_; item_ = 0;
  mapscene()->turnEvents();
  QWidget::show();
}

QList<QDateTime> MultiRadar::getTerms()
{
  QList<QDateTime> terms;
  qApp->setOverrideCursor( Qt::WaitCursor );
  rpc::TController* ctrl = global::serviceController( meteo::settings::proto::kSrcData );
  qApp->restoreOverrideCursor();
  if ( 0 == ctrl ) {
    error_log.msgBox() << QObject::tr("Не удалось подключиться к сервису данных");
    return terms;
  }
  QDateTime dtbeg( curdt_.addSecs( -ui_->spinpast->value()*3600 ) );
  QDateTime dtend( curdt_.addSecs( ui_->spinfuture->value()*3600 ) );
  surf::DataRequest request;
  request.set_type( surf::kRadarMapType );
  request.set_date_start( dtbeg.toString(Qt::ISODate ).toStdString() );
  request.set_date_end( dtend.toString(Qt::ISODate ).toStdString() );
  request.add_meteo_descr( currentcolor_.descr() );
  request.set_level_h( current_.h1() );
  request.set_level_h2( current_.h2() );
  qApp->setOverrideCursor( Qt::WaitCursor );
  surf::DataDescResponse* response = ctrl->remoteCall(
      &surf::SurfaceService::GetAvailableRadar,
      request,
      50000,
      true
      );
  qApp->restoreOverrideCursor();
  delete ctrl; ctrl = nullptr;
  if ( 0 == response ) {
    error_log.msgBox() << QObject::tr("Запрос количества данных не выполонен");
    return terms;
  }
  if ( false == response->result() ) {
    error_log << QObject::tr("Ошибка в ответе о количестве данных %1")
      .arg( QString::fromStdString( response->comment() ) );
  }
  for ( int i = 0, sz = response->descr_size(); i < sz; ++i ) {
    const surf::DataDesc& desc = response->descr(i);
    if ( 5 > desc.count() ) {
      continue;
    }
    QDateTime dt = TSqlQuery::datetimeFromString( desc.date() );
    if ( false == dt.isValid() ) {
      error_log << QObject::tr("Неверный формат даты %1")
        .arg( QString::fromStdString( desc.date() ) );
      continue;
    }
    terms.append(dt);
  }
  delete response;
  return terms;
}

void MultiRadar::buildImage( Document* doc, QImage* img, const QDateTime& dt )
{
  surf::DataRequest request;
  request.set_date_start( dt.toString(Qt::ISODate).toStdString() );
  request.add_meteo_descr( currentcolor_.descr() );
  request.set_level_h( current_.h1() );
  request.set_level_h2( current_.h2() );
  request.set_type(surf::kRadarMapType);
  qApp->setOverrideCursor( Qt::WaitCursor );
  rpc::TController* ctrl = global::serviceController( meteo::settings::proto::kSrcData );
  if ( 0 == ctrl ) {
    qApp->restoreOverrideCursor();
    error_log.msgBox() << QObject::tr("Не удалось подключиться к сервису данных");
    return;
  }
  surf::ManyMrlValueReply* response = ctrl->remoteCall(
      &surf::SurfaceService::GetMrlDataOnAllStation,
      request,
      50000,
      true
      );
  delete ctrl; ctrl = nullptr;
  if ( 0 != response ) {
    LayerMrl* l = new LayerMrl( doc, currentcolor_ );
    proto::WeatherLayer i;
    i.CopyFrom(current_);
    i.set_datetime( dt.toString(Qt::ISODate).toStdString() );
    l->setInfo(i);
    l->setProtoRadar(*response);
    QPainter pntr(img);
    doc->drawDocument( &pntr, doc->documentRect(), img->rect() );
    delete l;
  }
  qApp->restoreOverrideCursor();
  delete response;
}

void MultiRadar::slotPastSpinChanged( int value )
{
  ui_->lblpast->setText( QObject::tr("-%1 часов").arg(value) );
}

void MultiRadar::slotFutureSpinChanged( int value )
{
  ui_->lblfuture->setText( QObject::tr("+%1 часов").arg(value) );
}

void MultiRadar::slotAccept()
{
  QList<QDateTime> terms = getTerms();
  if ( 0 == terms.size() ) {
    error_log.msgBox() << QObject::tr("Нет данных за выбранный срок");
    return;
  }
  qApp->setOverrideCursor( Qt::WaitCursor );
  Magick::Blob gif;
  currentlayer_->setVisisble(false);
  Document* doc = mapdocument()->stub( mapdocument()->center(), mapdocument()->documentsize() );
  try {
  std::list<Magick::Image> images;
  for ( int i = 0, sz = terms.size(); i < sz; ++i ) {
    QImage img( doc->documentsize(), QImage::Format_RGB32 );
    img.fill( doc->backgroundColor() );
    buildImage( doc, &img, terms[i] );
    QByteArray arr;
    QBuffer buf(&arr);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf,"JPEG");
    Magick::Blob blob( arr.data(), arr.size() );
    Magick::Image frame;
    frame.adjoin(true);
    frame.read(blob);
    frame.magick("GIF");
    frame.animationDelay(30);
    images.push_back(frame);
  }
  delete doc;
  Magick::writeImages( images.begin(), images.end(), &gif );
  }
  catch (Magick::Error& my_error) {
    error_log.msgBox() << QObject::tr("Ошибка создания анимированного изображения = %1").arg( my_error.what() );
    QApplication::restoreOverrideCursor();
    currentlayer_->setVisisble(true);
    return;
  }
  catch ( ... ) {
    error_log.msgBox() << QObject::tr("Ошибка создания анимированного изображения");
    currentlayer_->setVisisble(true);
    QApplication::restoreOverrideCursor();
    return;
  }
  currentlayer_->setVisisble(true);
  arr_ = QByteArray(static_cast<const char*>(gif.data()), gif.length() );
  qApp->restoreOverrideCursor();
  buildProxyMovie();
}

void MultiRadar::buildProxyMovie()
{
  buf_.close();
  buf_.setBuffer(&arr_);
  buf_.open( QIODevice::ReadOnly );
  movie_->setDevice(&buf_);
  QLabel* lbl = new QLabel;
  lbl->setMovie(movie_);
  item_ = mapscene()->addWidget(lbl);
  item_->setPos( mapdocument()->documentTopLeft() );
  mapscene()->muteEvents();
  movie_->start();
  QWidget::close();
  animation_ = true;
}

void MultiRadar::slotReject()
{
  QWidget::close();
}

}
}
