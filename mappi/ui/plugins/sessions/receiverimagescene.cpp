#include "receiverimagescene.h"

#include <qapplication.h>
#include <qrgb.h>
#include <qdir.h>
#include <qgraphicsview.h>
#include <qthread.h>
#include <qscrollbar.h>

#include <mappi/ui/pos/posdocument.h>
#include <mappi/proto/satellite.pb.h>
#include <mappi/proto/reception.pb.h>
#include <mappi/proto/sessiondataservice.pb.h>
#include <mappi/schedule/schedulehelper.hpp>
#include <mappi/services/fileservice/fileserviceclient/filegetter.h>

const static QString tmpPath  = QDir::tempPath() + "/.mappi/satellite";
const static QString tmpImage = tmpPath + QObject::tr("/img_%1_%2.jpg");

namespace mappi {

ReceiverImageScene::ReceiverImageScene(QGraphicsView* parent)
  : QGraphicsScene(parent)
  , noDataItem_(nullptr)
  , document_(nullptr)
    //  , replyReceiver_(new mappi::conf::DataReply())
  , useMap_(true)
  , showLayer_(true)
  , sessionActive_(false)
  , readySatChannels_(false)
{
  //useMap_ = false;
  QDir dir;
  dir.mkdir(tmpPath.left(tmpPath.lastIndexOf("/")));
  dir.mkdir(tmpPath);
  QGraphicsScene::setBackgroundBrush(QBrush(Qt::gray));
}

ReceiverImageScene::~ReceiverImageScene()
{
  clearPictures();
  // delete replyReceiver_;
  delete document_;
}

void ReceiverImageScene::clearPictures()
{
  for(auto& pic : pictures_) delete pic;
  clear();
  pictures_.clear();
}

bool ReceiverImageScene::initDocument(const QSize& size, float scan_angle, int samples, float velocity,
                                      const MnSat::STLEParams &stle)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  bool ok = true;
  if(useMap_) {
    delete document_; document_ = nullptr;
    //Получаем TLE от сервиса расписаний
    SatelliteBase satelliteBase;
    {
      ok &= false == stle.satName.isEmpty();
      if(ok) {
        ok &= satelliteBase.readTLE(stle);
      }
    }
    //create Document
    if(ok) {
      meteo::map::proto::Document doc;
      doc.set_scale(0.0);
      doc.set_projection(meteo::kSatellite);
      doc.set_geoloader("geo.old");
      doc.set_cache(false);
      doc.set_opengeopolygon(true);

      document_ = new meteo::map::PosDocument(doc);
      document_->setMinScale(-1.0f);

      //Переворачиваем карту
      double factor = (conf::kAscending == session_->data().direction ? -1.0 : 1.0);
      document_->setYfactor(factor);
      document_->setXfactor(factor);

      //Устанавливаем размер документа
      document_->resizeDocument(size);
      document_->resizeMap(size);
    }
    //init Projection
    if(ok)
    {
      ok &= document_->init(session_->data().aos, session_->data().los, &satelliteBase, meteo::DEG2RAD * scan_angle, samples, velocity);
      for(auto& layer : document_->layers()) {
        if(layer->name() == QObject::tr("Береговая черта")) {
          for ( auto o : layer->objects() ) {
            o->setClosed(false);
            o->setPenWidth(0);
          }
        }
      }
    }
    //Скрываем лишние слои
    if(ok)
    {
      for(auto& layer : document_->layers()) {
        if(layer->name() == QObject::tr("Границы")) {
          layer->setVisisble(false);
        }
      }
    }
    else
    {
      error_log << QObject::tr("Не удалось инициализировать проекцию");
      delete document_;
      document_ = nullptr;
    }
    slotToggleMap(showLayer_);
  }
  QApplication::restoreOverrideCursor();
  return ok;
}

bool ReceiverImageScene::showLayers() const { return showLayer_; }

void ReceiverImageScene::slotToggleMap(bool show)
{
  showLayer_ = show;
  //Скрываем используемые слои
  if(document_ != nullptr) {
    for(auto& layer : document_->layers())
    {
      if(layer->name() == QObject::tr("Реки")
         || layer->name() == QObject::tr("Озера")
         || layer->name() == QObject::tr("Пункт")
         || layer->name() == QObject::tr("Береговая черта")
         ) {
       layer->setVisisble(show);
      }
    }
  }
  invalidate();
}

	void ReceiverImageScene::setSession(std::shared_ptr<schedule::Session> session, const conf::Instrument& instrument,
							const QDateTime& dt, const MnSat::STLEParams &stle)
{
  //trc;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  //Удаляем информацию о предыдущем сеансе
  clearPictures();

  //Устанавливаем новый сеанс
  session_ = session;

  samples_    = instrument.has_samples()    ? instrument.samples()    : 2048;
  scan_angle_ = instrument.has_scan_angle() ? instrument.scan_angle() : 55.37;
  velocity_   = instrument.has_velocity()   ? instrument.velocity()   : 6;

  //Проверка на то, что сеанс принимается
  bool sessionActive = !session->isIgnored() && session->stage(dt) == schedule::Session::PROCEED;

  //Строим список каналов для инструмента текущего сеанса
  channelAliases_.clear();
  for(const auto& ch : instrument.channel())
    if(ch.has_alias() && ch.has_number())
      channelAliases_.insert(ch.number(), QString::fromStdString(ch.alias()));

  //var(channelAliases_);
  noDataText_.clear();
  QList<QString> channels;
  //Текущий сеанс исполняется
  debug_log << "active" << sessionActive << sessionActive_ << session->data().satellite;
  if(sessionActive)
  {
    if(sessionActive_)
    {      
      if(false == channelAliases_.empty())
      {
        for(auto it = channelAliases_.cbegin(); it != channelAliases_.cend(); ++it) {
          QString name = tmpImage.arg(session->data().satellite).arg(it.key());
	  //var(name);
          QPixmap pix(name);
          if(pix.isNull()) {
            continue;
          }
	  var(it.value());
          satChannels_.append(it.value());
          channels.append(it.value());

          pictures_.insert(it.value(), addPixmap(pix));
          if(currentChannel_ != it.value()) {
            pictures_.value(it.value())->hide();
          }

          pictures_.value(it.value())->setPos(width() / 2.0 - pix.width() / 2.0, height() - pix.height());
        }
      }
    }
    else {
      for(const auto& ch : qAsConst(channelAliases_)) {
        channels.append(ch);
      }
    }
    readySatChannels_ = !channels.empty();

    if(useMap_) {
      if(instrument.has_rt_thin())  samples_ /= instrument.rt_thin();
      size_ = { samples_, static_cast<int>(session->duration() * velocity_) };
      initDocument(size_, scan_angle_, samples_, velocity_, stle);
    }
    else {
      size_ = qobject_cast<QGraphicsView*>(parent())->size();
    }
    sessionActive_ = sessionActive;
    invalidate();
  }
  //Текущий сеанс закончен
  else if(session->stage(dt) == schedule::Session::COMPLETED && !session->isIgnored())
  {  
    QMap<QString, QString> paths;
    proto::ThematicData req;
    req.set_name("grayscale");
    QString tmp = session_->data().aos.addSecs(-5 * 60).toString(Qt::ISODate).replace('T', ' ');
    req.set_date_start(tmp.toStdString());
    tmp = session_->data().los.addSecs(5 * 60).toString(Qt::ISODate).replace('T', ' ');
    req.set_date_end(tmp.toStdString());
    req.set_satellite_name(session->data().satellite.toStdString());
    ::meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
    if (nullptr != ch)
    {
      proto::ThematicList* resp =
          ch->remoteCall(&mappi::proto::SessionDataService::GetAvailableThematic, req, 1000, true);
      if(nullptr != resp)
      {
        if(resp->has_result() && resp->result())
        {
          int id = resp->themes_size() > 0 ? resp->themes(0).session_id() : 0;
          for(auto& them : resp->themes())
          {
            if(them.has_path() && id == them.session_id())
            {
              tmp = QString::fromStdString(them.path());
              tmp = tmp.right(tmp.size() - tmp.lastIndexOf('.') + 1);
              tmp = tmp.left(tmp.lastIndexOf('.'));
              if(channelAliases_.cend() != channelAliases_.constFind(tmp.toInt())) {
                QString channel = channelAliases_.constFind(tmp.toInt()).value();
                channels.append(channel);
                QString path = QString::fromStdString(them.path());
                paths.insert(channel, path);
              }
            }
          }
          delete resp;

        }
        delete ch;
      }
    }
    if(paths.empty()) {
      noDataText_ = QObject::tr("Нет данных");
      for(const auto& ch : qAsConst(channelAliases_)) {
        channels.append(ch);
      }
    }
    else {
      noDataText_ = QObject::tr("Данные запрашиваются у сервера");
      loadImages(paths);
    }
    if(useMap_) {
      size_ = { samples_, static_cast<int>(session->duration() * velocity_) };
      initDocument(size_, scan_angle_, samples_, velocity_, stle);
    }
    else {
      size_ = qobject_cast<QGraphicsView*>(parent())->size();
    }
  }
  //Текущий сеанс ожидается
  else if(!session->isIgnored()) {
   
    for(const auto& ch : qAsConst(channelAliases_)) {
      channels.append(ch);
    }
    if(useMap_) {
      if(instrument.has_rt_thin())  samples_ /= instrument.rt_thin();
      size_ = { samples_, static_cast<int>(session->duration() * velocity_) };
      initDocument(size_, scan_angle_, samples_, velocity_, stle);
    }
    else {
      size_ = qobject_cast<QGraphicsView*>(parent())->size();
    }
    noDataText_ = QObject::tr("Нет данных");
  }
  //Текущий сеанс пропущен
  else {
    for(const auto& ch : qAsConst(channelAliases_)) {
      channels.append(ch);
    }
    noDataText_ = QObject::tr("Данных нет");
    size_ = qobject_cast<QGraphicsView*>(parent())->size();
  }

  //var(channels);
  Q_EMIT(readySatChannels(channels));
  if(nullptr == noDataItem_) {
    noDataItem_ = addText(noDataText_);
    QObject::connect(noDataItem_, &QGraphicsTextItem::destroyed, [&](){ noDataItem_ = nullptr; });
  }
  Q_EMIT(sigResize(size_));

  QApplication::restoreOverrideCursor();
}

void ReceiverImageScene::resize(const QSize& size)
{
  //scale_ и mapScale ограничены (1.0) и (-1.0), поточу что document_ не поддерживает setScale( <= -1.0)
  //size.width() - 4 для того чтобы рамка документа не вылазила за край виджета
  scale_ = static_cast<double>(size.width() - 4) / static_cast<double>(size_.width());
  if (nullptr != document_) {
    if(scale_ >= 1) scale_ = 0.9999f;
  }
  int width  = size_.width()  * scale_;
  int height = size_.height() * scale_;

  setSceneRect(QRect(0.0, 0.0, width, height));
  if(nullptr != noDataItem_) {
    noDataItem_->setPos(width / 2 - noDataText_.size() / 2 * 6, height / 2);
  }

  for(auto& pix : pictures_) {
    pix->setScale(scale_);
    if(conf::kAscending == session_->data().direction) {
      pix->setPos(0.0f, (size_.height()  - pix->pixmap().height()) * scale_);
    }
    else {
      pix->setPos(0.0f, 0.0f);
    }
  }

  if(nullptr != document_) {
    double mapScale = static_cast<double>(size.width()) / document_->mapsize().width();
    mapScale = std::log(1.0 / mapScale) / std::log(2);
   // mapScale = log2(1.0 / mapScale);
    if(mapScale < 0.0) mapScale = 0.0f; 
    var(mapScale); // В основном корректно работает для mapScale > -1.0
    document_->resizeDocument({ width + 1, height });  // width + 1, потому что видно картинку за рамкой
    document_->setScale(mapScale);
  }

  invalidate();
}

void ReceiverImageScene::loadImages(QMap<QString, QString> paths)
{
  if(paths.empty()) return;

  for(auto path = paths.cbegin(); paths.cend() != path; ++path)
  {
    QString key = path.key(); //Переменная связывается с lambda-функцией стр: 368

    //Создаём объект получатель файла
    FileGetter* fileGetter = new FileGetter();
    fileGetter->setFilename(path.value());
    //Связываем с ним функцию ReceiverImageScene::loadImage(QString, QString)
    QObject::connect(fileGetter, &FileGetter::complete, this,
            [&, key](QString filename) { loadImage(key, filename); },
            Qt::QueuedConnection);
    //Убъект удалится после окончания работы
    QObject::connect(fileGetter, &FileGetter::finished, fileGetter, &FileGetter::deleteLater);

    //Создаём поток для получения файла
    QThread* thread = new QThread();
    //Помещаем в него объект получатель
    fileGetter->moveToThread(thread);
    //Связываем начало работы потока с функцией FileGetter::run()
    QObject::connect(thread,     &QThread::started,   fileGetter, &FileGetter::run);
    //Поток остонавливается после удаления получателя
    QObject::connect(fileGetter, &QThread::destroyed, thread,     &QThread::quit);
    //Поток удаляется после остоновки
    QObject::connect(thread,     &QThread::finished,  thread,     &QThread::deleteLater);
    //Запускаем поток
    thread->start();
  }
}

void ReceiverImageScene::slotSwitchChannel(const QString& channel)
{
  if(channel.isEmpty()) return;

  currentChannel_ = channel;

  if(pictures_.empty()) return;

  auto pic = pictures_.constFind(channel);

  for(auto it = pictures_.cbegin(); it != pictures_.cend(); ++it) {
    if(it != pic && it.value()->isVisible()) {
      it.value()->hide();
    }
  }
  if(pictures_.end() != pic) {
    pic.value()->show();
  }
  else {
    if(nullptr == noDataItem_) {
      noDataItem_ = addText("Нет данных");
      QObject::connect(noDataItem_, &QGraphicsTextItem::destroyed, [&](){ noDataItem_ = nullptr; });
    }
    noDataItem_->setPos(width() / 2, height() / 2);
  }
}

void ReceiverImageScene::subscribe(meteo::rpc::Channel* channel)
{
  trc;
  mappi::conf::DataRequest request;
  request.set_subscr(mappi::conf::SubscrType::kDataSubscr);
  channel->subscribe(&mappi::conf::ReceptionService::GetDataMulti, request,
		     this, &ReceiverImageScene::callbackStreamReceiv);
}

void ReceiverImageScene::callbackStreamReceiv(conf::DataReply *reply)
{
  if(nullptr == reply || false == reply->has_data()) return;

  if(true == reply->has_status()) {
    switch (reply->status()) {
    case mappi::conf::StatusSession::kStart:
      debug_log<< "start";
      satChannels_.clear();
      readySatChannels_ = false;
      rows_.clear();
      data_.clear();
      if(sessionActive_) clearPictures();
      if(conf::kAscending == session_->data().direction) {
        qobject_cast<QGraphicsView*>(parent())->verticalScrollBar()->setValue(size_.height());
      }
      break;
    case mappi::conf::StatusSession::kEnd: {
      debug_log << "end";
      sessionActive_ = false;
      printImage(QByteArray(reply->data().data(), reply->data().size()));
      break;
    }
    case mappi::conf::StatusSession::kContinue:
      //debug_log << "continue";
      sessionActive_ = true;
      printImage(QByteArray(reply->data().data(), reply->data().size()));
      break;
    default: return;
    }
  }
  else {
    printImage(QByteArray(reply->data().data(), reply->data().size()));
  }
}

void ReceiverImageScene::drawBackground(QPainter *painter, const QRectF &rect)
{
  QGraphicsScene::drawBackground(painter, rect);
}

void ReceiverImageScene::drawForeground(QPainter *painter, const QRectF &rect)
{
  QGraphicsScene::drawForeground(painter, rect);
  if(nullptr != document_ && useMap_) {
    document_->drawDocument(painter);
  }
}

void ReceiverImageScene::loadImage(const QString& alias, const QString& filename)
{
  QPixmap pic(filename);
  if(pic.isNull()) return;

  //Переворачиваем картинку если направления с с юга на север
  if(conf::kAscending == session_->data().direction) {
    pic = pic.transformed(QTransform().scale(-1.0, -1.0));
  }

  auto picture = pictures_.insert(alias, addPixmap(pic));

  if(currentChannel_ == alias) {
    noDataText_.clear();
    if(nullptr != noDataItem_) {
      noDataItem_->setPlainText(noDataText_);
      noDataItem_->setVisible(false);
    }
    size_ = pic.size();
    Q_EMIT(sigResize(size_));
    picture.value()->show();
  }
  else {
    picture.value()->hide();
  }
}

void ReceiverImageScene::removeTemp()
{
  QDir dir(tmpPath);
  for(const auto &name : dir.entryList()) {
    dir.remove(name);
  }
}

void ReceiverImageScene::printImage(const QByteArray& data)
{
  static QVector<QRgb> palette;
  if(palette.empty()) {
    palette.reserve(256);
    for (int i = 0; i < 256; ++i) {
      palette.append(QRgb((i << 16) + (i << 8) + i));
    }
  }

  int thinning;
  int chcnt;
  int number;
  int rows, cols;
  QByteArray chData;
  QDataStream ds(data);
  ds >> thinning
     >> chcnt;

  QImage* image;
  int magicNumber = -1; // номер одного из полученных каналов
  for(int i = 0; i < chcnt; ++i)
  {
    ds >> number
       >> chData
       >> rows
       >> cols;
    if(magicNumber == -1) {
      if(number == channelAliases_.key(currentChannel_, -1)) {
        magicNumber = number;
      }
    }

    //debug_log << "ready" << readySatChannels_ << session_->getNameSat() << sessionActive_;
    if(false == readySatChannels_ &&
       true == channelAliases_.contains(number) &&
       false == satChannels_.contains(channelAliases_[number]))
    {
      readySatChannels_ = false;
      satChannels_.append(channelAliases_[number]);
    }

    rows_[number] += rows;
    data_[number].append(chData);

    QString name = tmpImage.arg(session_->data().satellite).arg(number);
    image = new QImage(
          reinterpret_cast<const uchar*>(data_.value(number).data()),
          cols,
          rows_.value(number),
          cols,
          QImage::Format_Indexed8);
    if(image->isNull()) {
      //debug_log << "null image";
      return;
    }
    image->setColorCount(palette.size());
    image->setColorTable(palette);
    image->save(name);
    delete image;

    if(sessionActive_) {
      QPixmap pix(name);
      //Переворачиваем картинку если направление с юга на север
      if(conf::kAscending == session_->data().direction) {
        pix = pix.transformed(QTransform().scale(-1.0f, -1.0f));
      }
      if(channelAliases_.contains(number)) {
        QString channelAlias = channelAliases_[number];
        if(false == pictures_.contains(channelAlias)) {
          pictures_.insert(channelAlias, addPixmap(pix));
          if(currentChannel_ != channelAlias) {
            pictures_.value(channelAlias)->hide();
          }
        }
        else {
          pictures_.value(channelAlias)->setPixmap(pix);
        }
        if(conf::kAscending == session_->data().direction) {
          pictures_.value(channelAlias)->setPos(0.0, (size_.height()  - pix.height())* scale_);
        }
        else {
          pictures_.value(channelAlias)->setPos(0.0, 0.0);
        }
      }
      for(auto& picture : pictures_) {
        picture->setScale(scale_);
      }
      invalidate();

    }
  }

  if(sessionActive_) {
    if(false == readySatChannels_) {
      //var(satChannels_);
      Q_EMIT(readySatChannels(satChannels_));
            
      readySatChannels_ = true;
    }
    if(noDataItem_ != nullptr) {
      noDataItem_->setVisible(false);
    }
    noDataText_.clear();
  }
}

}
