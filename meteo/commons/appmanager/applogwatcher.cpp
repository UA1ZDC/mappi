#include "applogwatcher.h"
#include "tappcontrol.h"

#include <qfile.h>
#include <qfilesystemwatcher.h>
#include <qbytearray.h>
#include <qtimer.h>

#include <cross-commons/debug/tlog.h>
#include <commons/textproto/pbtools.h>
#include <meteo/commons/global/global.h>

namespace meteo
{
namespace app
{

static const int kMaxFileSize = 1024*1024*10; //10Mb

AppLogWatcher::AppLogWatcher( QObject* p, int32_t secs )
  :QObject(p),
  timer_( new QTimer(this) ),
  sendsecs_(secs)
{
  QObject::connect( timer_, SIGNAL( timeout()), this, SLOT( slotTimeout() ) );
  chunk_ = QByteArray( 1024*512, '\0' );
}

AppLogWatcher::~AppLogWatcher()
{
  QObject::disconnect( timer_, SIGNAL( timeout()), this, SLOT( slotTimeout() ) );
  delete timer_;
  QMap< QString, QFile* >::iterator it  = files_.begin();
  QMap< QString, QFile* >::iterator end  = files_.end();
  for ( ; it != end; ++it ) {
    QFile* file = it.value();
    file->close();
    delete file;
  }
  files_.clear();
  subscribes_.clear();
}

bool AppLogWatcher::addSubscribe( const QString& filename, const OutSubscribe& subscribe )
{
  QFile* file = 0;
  if ( false == files_.contains(filename) ) {
    file = new QFile( filename, this );
    if ( false == file->open( QIODevice::WriteOnly | QIODevice::ReadOnly | QIODevice::Append ) ) {
      error_log << meteo::msglog::kFileReadWriteFailed.arg(filename,file->errorString());
      return false;
    }
    files_.insert( filename, file );
  }
  else {
    file = files_[filename];
  }

//  //debug_log << "Client subscribed f =" << file << "r =" << subscribe.first << "d =" << subscribe.second;

  wasdisconnected_ = false;

  if ( false == subscribes_.contains(file) ) { //если подписчиков на файл нет, отправляется последняя порция логов
    subscribes_.insertMulti( file, subscribe );
    slotFileChanged(file); //TODO проверить
  }
  else {                                       //если есть подписчики, то новому абоненту отправляется последнее сообщение другого абонента
    const OutSubscribe& s = subscribes_[file]; //TODO проверить
    subscribes_.insertMulti( file, subscribe );
    subscribe.first->CopyFrom( *(s.first) );
    subscribe.second->Run();
  }

  return true;
}

void AppLogWatcher::rmSubscribe( const OutSubscribe& subscribe )
{
  QFile* file = subscribes_.key( subscribe, 0 );
  if ( 0 == file ) {
    return;
  }

  wasdisconnected_ = true;

  QList<OutSubscribe> list = subscribes_.values(file);
//  debug_log << "size before =" << list.size();
//  debug_log << "Client unsubscribed f =" << file << "r =" << subscribe.first << "d =" << subscribe.second;
  subscribes_.remove(file);
  list.removeAll(subscribe);
//  debug_log << "size after =" << list.size();
  for ( int i = 0, sz = list.size(); i < sz; ++i ) {
    subscribes_.insertMulti( file, list[i] );
  }
}

void AppLogWatcher::setOutFiles( const QStringList& outfilelist )
{
  for ( int i = 0, sz = outfilelist.size(); i < sz; ++i ) {
    QString filename = outfilelist[i];
    QFile* file = 0;
    if ( false == files_.contains(filename) ) {
      file = new QFile( filename, this );
      if ( false == file->open( QIODevice::WriteOnly | QIODevice::ReadOnly | QIODevice::Append ) ) {
        error_log << meteo::msglog::kFileReadWriteFailed.arg(filename,file->errorString());
        return;
      }
      files_.insert( filename, file );
    }
  }
  timer_->start(1000*sendsecs_);
}

void AppLogWatcher::slotTimeout()
{
  QMap< QString, QFile* >::iterator it = files_.begin();
  QMap< QString, QFile* >::iterator end = files_.end();
  for ( ; it != end; ++it ) {
    QFile* file = it.value();
    if ( file->pos() != file->size() - 1 ) {
      slotFileChanged(file);
      if ( kMaxFileSize < file->size() ) {
        if ( kManagerLogFile == file->fileName().split("/").last() ) {
          Control::instance()->restoreOutput();
          file->close();
          file->open(QIODevice::WriteOnly | QIODevice::ReadOnly | QIODevice::Append);
          file->resize(0);
          file->seek(0);
          file->close();
          Control::instance()->outputToFile();
          file->open(QIODevice::WriteOnly | QIODevice::ReadOnly | QIODevice::Append);
        }
        else {
          file->resize(0);
          file->seek(0);
        }
      }
    }
  }
}

void AppLogWatcher::slotFileChanged( QFile* file )
{
  QList<OutSubscribe> subscrlist = subscribes_.values( file );
  if ( 0 == subscrlist.size() ) {
    return;
  }
  wasdisconnected_ = false;

  int64_t chunksize = chunk_.size();
  chunk_.fill( '\0', chunksize );
  if ( file->size() - file->pos() > chunksize ) {
    file->seek( file->size() - chunksize );
  }
  int64_t readsize = file->read( chunk_.data(), chunksize );

  if ( 0 == readsize ) { return; }

  std::string line = pbtools::toString(QString::fromLocal8Bit( chunk_.data(), readsize ));
  for ( int i = 0, sz = subscrlist.size(); i < sz; ++i ) {
    if ( wasdisconnected_ ) { return; }

    const OutSubscribe& subscr = subscrlist[i];
    subscr.first->clear_logstring();
    subscr.first->add_logstring(line);
  }

  for ( int i = 0, sz = subscrlist.size(); i < sz; ++i ) {
    if ( wasdisconnected_ ) { return; }

    const OutSubscribe& subscr = subscrlist[i];
    if ( 0 != subscr.first->logstring_size() ) {
      subscr.second->Run();
    }
  }
}

}
}
