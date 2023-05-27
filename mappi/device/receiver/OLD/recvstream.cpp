#include "recvstream.h"

#include <qmutex.h>
#include <qdir.h>
#include <qimage.h>

#include <commons/textproto/pbtools.h>

#include <meteo/commons/global/log.h>
#include <meteo/commons/global/global.h>
//#include <meteo/commons/rpc/tcontroller.h>

#include <mappi/proto/sessiondataservice.pb.h>

namespace mappi {
namespace receive {

static constexpr int kTimerInterval = 100;

RecvStream::RecvStream()
  : editMutex_(new QMutex())
  , readMutex_(new QMutex())
  , timer_(new QTimer(this))
{
  timer_->setInterval(kTimerInterval);
  connect(timer_, &QTimer::timeout,         this,   &RecvStream::readFileData);
  connect(this,   &RecvStream::startSignal, timer_, static_cast<void(QTimer::*)()>(&QTimer::start));
  connect(this,   &RecvStream::stopSignal,  timer_, &QTimer::stop);
}

RecvStream::~RecvStream()
{
  delete editMutex_;
  delete readMutex_;
}
  
bool RecvStream::init(const QString& site, const meteo::GeoPoint& siteCoord, const QString& resDir)
{
  if(false == editMutex_->tryLock(1000)) return false;

  header_.site = site;
  header_.siteCoord = siteCoord;
  resultDir_ = resDir + "sessions/";

  editMutex_->unlock();

  return true;
}
bool RecvStream::setFileSource(const QString& file)
{
  if(false == editMutex_->tryLock(1000)) return false;

  source_ = kFileSrc;
  currentFile_ = file;
  pos_ = 0;

  editMutex_->unlock();

  return true;
}


bool RecvStream::start(const QString& name, conf::RecvMode mode, int32_t direct, const MnSat::TLEParams& tle)
{
  if(false == editMutex_->tryLock(1000)) return false;

  status_ = conf::kStart;

  header_.satellite = name;
  header_.direction = direct;
  header_.mode = mode;
  header_.tle = tle;
  header_.start = QDateTime::currentDateTimeUtc(); //TODO в момент получения данных?

  result_.close();
  
  QDir dir;
  dir.mkpath(resultDir_ + header_.start.toString("yyyy-MM-dd/"));

  QString satShort = header_.satellite;
  satShort.remove(' ');
  result_.setFileName(resultDir_ + header_.start.toString("yyyy-MM-dd/") + header_.start.toString("yyyyMMddhhmm") + "_" + satShort +
          (header_.direction == 0 ? 'n' : 's') + ".raw"); //TODO

  result_.open(QIODevice::WriteOnly);

  if (!startReceive()) {
    result_.close();
    result_.remove();
    editMutex_->unlock();
    return false;
  }
  return true;
}

void RecvStream::stop(const mappi::conf::Site& site, QString* fileName, uint64_t* dbId)
{
  bool lock = readMutex_->tryLock(60 * 1000);

  status_ = conf::kEnd;
  
  if (source_ == kFileSrc)
  {
    Q_EMIT(stopSignal());
    readFileData();
    sourceFile_.close();
    pos_ = 0;
  }

  if (result_.size() != 0) {
    header_.stop = QDateTime::currentDateTimeUtc();
    
    QByteArray ba; 
    header_ >> ba;
    uint16_t size = ba.size();
    QByteArray bas;
    bas.append(size >> 8);
    bas.append(size & 0xff);
    
    result_.write(ba);
    result_.write(bas);
    result_.close();
    
    *dbId = saveToDb(site);
    *fileName = result_.fileName();
  }
  else {
    *fileName = QString::null;
    *dbId = 0;
    result_.close();
    result_.remove();
  }

  if(lock) {
    readMutex_->unlock();
  }
  editMutex_->unlock();
}

  //для сохранения в заголовок не реального времени, а из расписания (для виртуальных запусков)
  void RecvStream::stop(const mappi::conf::Site& site, const QDateTime& start, const QDateTime& end,
			QString* fileName, uint64_t* dbId)
{
  bool lock = readMutex_->tryLock(60 * 1000);

  status_ = conf::kEnd;
  
  if (source_ == kFileSrc)
  {
    Q_EMIT(stopSignal());
    readFileData();
    sourceFile_.close();
    pos_ = 0;
  }

  if (result_.size() != 0) {
    header_.start = start;
    header_.stop = end;
     
    QByteArray ba; 
    header_ >> ba;
    uint16_t size = ba.size();
    QByteArray bas;
    bas.append(size >> 8);
    bas.append(size & 0xff);
    
    result_.write(ba);
    result_.write(bas);
    result_.close();
    
    *dbId = saveToDb(site);
    *fileName = result_.fileName();
  }
  else {
    *fileName = QString::null;
    *dbId = 0;
    result_.close();
    result_.remove();
  }

  if(lock) {
    readMutex_->unlock();
  }
  editMutex_->unlock();
}

void RecvStream::finish() { Q_EMIT(finished()); }


//! Получение потока данных
bool RecvStream::startReceive()
{
  bool ok = false;

  switch (source_)
  {
  case kFileSrc:
    //открыть и читать из файла -> сохранять в файл
    ok = startFileRecv();
    break;
  case kUdpSrc:
    //открыть сокет и читать из него -> сохранять в файл
    ok = startUdpRecv();
    break;
  default:
    break;
  }

  return ok;
}

//! Получение потока из файла
bool RecvStream::startFileRecv()
{
  sourceFile_.setFileName(currentFile_);
  Q_EMIT(startSignal());
  return true;
}

//! Чтение потока из файла (чтение файла по мере появления в нём данных). //TODO пока никто не передаёт по сети)
void RecvStream::readFileData()
{
  if(false == readMutex_->tryLock()) return;

  if (!sourceFile_.isOpen()) {
    sourceFile_.open(QIODevice::ReadOnly);
  }

  uint64_t maxsize = sourceFile_.size() - pos_;
  if (maxsize > 0 || status_ == conf::kEnd) {
    sourceFile_.seek(pos_);

    QByteArray data = sourceFile_.read(maxsize);

    if (data.size() > 0)
    {
      result_.write(data);
      pos_ += data.size();

      Q_EMIT(receivedData(data, status_));
    }

    if (status_ == conf::kStart) {
      status_ = conf::kContinue;
    }
  }
  readMutex_->unlock();
}

//! Получение потока по сети
bool RecvStream::startUdpRecv()
{
  return false;
}

uint64_t RecvStream::saveToDb(const mappi::conf::Site& site)
{
  uint64_t sessionId = 0;   

  meteo::rpc::Channel* ch = meteo::global::serviceChannel(meteo::settings::proto::kDataControl);
  if (nullptr == ch) {
    error_log << meteo::msglog::kServiceConnectFailedSimple.arg(meteo::settings::proto::kDataControl);
    return 0;
  }

  mappi::proto::SessionData request;
  request.set_sat_name(header_.satellite.toStdString());
  request.set_date_start(header_.start.toString(Qt::ISODate).toStdString());
  request.set_date_end(header_.stop.toString(Qt::ISODate).toStdString());
  request.mutable_site()->operator=(site);
  request.set_server_path(pbtools::toString(result_.fileName()));

  QByteArray tle;
  QDataStream ds(&tle, QIODevice::WriteOnly);
  ds << header_.tle;
  tle = tle.toBase64();
  request.set_tle(tle, tle.size());

  // WARNING старый RPC
  //mappi::proto::Response* response = ch->remoteCall(&mappi::proto::SessionDataService::SaveSession,
  //						    request, 10000, true);

  mappi::proto::Response* response = ch->remoteCall(&mappi::proto::SessionDataService::SaveSession_new,
  						    request, 10000, true);
  if (nullptr == response) {
    delete ch;
    return 0;
  }

  if (response->has_result()) {
    sessionId = response->index();
  }
 
  delete response;
  delete ch;

  return sessionId;
}

} //receive
} // mappi
