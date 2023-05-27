#include "tlog.h"

#include <iostream>

#include <syslog.h>

#include <qdir.h>
#include <qfile.h>
#include <qpoint.h>
#include <qmutex.h>
#include <qthread.h>
#include <qmessagebox.h>
#include <qcoreapplication.h>
#include <qtextcodec.h>

#include <cross-commons/app/paths.h>

static QMutex debugMutex( QMutex::Recursive );

int syslogPriority(tlog::Priority p)
{
  switch ( p )
  {
    case tlog::kNone:     return LOG_DEBUG;
    case tlog::kDebug:    return LOG_DEBUG;
    case tlog::kInfo:     return LOG_INFO;
    case tlog::kWarning:  return LOG_WARNING;
    case tlog::kError:    return LOG_ERR;
    case tlog::kCritical: return LOG_CRIT;
  }
  return LOG_DEBUG;
}

fMsgHandler TLog::_funcOut = 0;
QString TLog::logFileName_ = QString();

fMsgHandler TLog::setMsgHandler(fMsgHandler f)
{
  fMsgHandler old = _funcOut;
  _funcOut = f;
  return old;
}

void TLog::stdOut(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString& msg)
{
  QString out;
  if ( tlog::kNone == priority ) {
    out = msg;
  }
  else {
    out = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + tlog::kPriorityStr[priority];
    if( !facility.isEmpty() ) { out += '[' + facility + "] ";  }
    if( !fileName.isEmpty() ) { out += fileName +':' + QString::number(line) + ' ';  }
    out += msg;
  }

//  if( priority <= tlog::kWarning ){
//    std::cout << ASCII_FROM_QSTRING( out ) << std::endl;
//    std::cout.flush();
//  }
//  else {
    std::cerr << ASCII_FROM_QSTRING( out ) << std::endl;
    std::cerr.flush();
//  }
}

void TLog::clearStdOut(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString& msg)
{
  QString out = tlog::kPriorityStr[priority];
  if( tlog::kDebug == priority ){
    if( !facility.isEmpty() ) { out += '[' + facility + "] ";  }
    if( !fileName.isEmpty() ) { out += fileName +':' + QString::number(line) + ' ';  }
  }
  out += msg;

//  if( priority <= tlog::kWarning ){
//    std::cout << ASCII_FROM_QSTRING( out ) << std::endl;
//    std::cout.flush();
//  }
//  else {
    std::cerr << ASCII_FROM_QSTRING( out ) << std::endl;
    std::cerr.flush();
//  }
}

void TLog::syslogOut(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString& msg)
{

  QString out = "[" + facility + "] " + msg;
#ifdef WIN32
  ::openlog( const_cast<char*>(MnCommon::applicationName()), LOG_NDELAY, LOG_USER);
  ::syslog( syslogPriority(priority), const_cast<char*>("%s"), ASCII_FROM_QSTRING( out ));
#else
  ::openlog( MnCommon::applicationName(), LOG_NDELAY, LOG_USER );
  ::syslog( syslogPriority(priority), "%s", ASCII_FROM_QSTRING( out ));
#endif
  ::closelog();

  if ( tlog::kDebug == priority ) {
    TLog::stdOut( priority, facility, fileName, line, msg );
  }
}

void TLog::fileOut(tlog::Priority priority, const QString& facility, const QString& fileName, int line, const QString& msg)
{
  QMutexLocker lock(&debugMutex);

  QFile file(logFileName_);
  if ( logFileName_.isEmpty() || !file.open(QFile::WriteOnly | QFile::Append) ) {
    return;
  }

  QString out;
  if ( tlog::kNone == priority ) {
    out = msg;
  }
  else {
    out = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + tlog::kPriorityStr[priority];
    if( !facility.isEmpty() ) { out += '[' + facility + "] ";  }
    if( !fileName.isEmpty() ) { out += fileName +':' + QString::number(line) + ' ';  }
    out += msg;
  }
  out += "\n";

  file.write(ASCII_FROM_QSTRING(out));
}

void TLog::setLogFileName(const QString& fileName)
{
  debugMutex.lock();

  if ( !QFileInfo(fileName).isAbsolute() ) {
    TLog::logFileName_ = "/var/log/" + fileName;
  }
  else {
    TLog::logFileName_ = fileName;
  }

  debugMutex.unlock();
}

TLog::~TLog()
{
  if ( !--stream_->ref ) {
    if ( stream_->message_output ) {

      debugMutex.lock();
      TLog::_funcOut( priority_, facility_, fileName_, line_, stream_->buffer );
      debugMutex.unlock();

      if ( showMsgBox_ ) {
        const bool isGuiThread = QThread::currentThread() == QCoreApplication::instance()->thread();
        if ( isGuiThread ) {
          QMessageBox msgBox;
          switch ( priority_ ) {
            case tlog::kNone:
            case tlog::kDebug:
            case tlog::kInfo:
              msgBox.setWindowTitle(QObject::tr("Cообщение"));
              msgBox.setText(stream_->buffer);
              msgBox.setStandardButtons(QMessageBox::Ok);
              msgBox.setDefaultButton(QMessageBox::Ok);
              msgBox.setButtonText(0x00000400,QObject::tr("Закрыть"));
              msgBox.setIcon(QMessageBox::Information);
              msgBox.exec();
              break;
            case tlog::kWarning:
              msgBox.setWindowTitle(QObject::tr("Предупреждение"));
              msgBox.setText(stream_->buffer);
              msgBox.setStandardButtons(QMessageBox::Ok);
              msgBox.setDefaultButton(QMessageBox::Ok);
              msgBox.setButtonText(0x00000400,QObject::tr("Закрыть"));
              msgBox.setIcon(QMessageBox::Warning);
              msgBox.exec();
              break;
            case tlog::kError:
            case tlog::kCritical:
              msgBox.setWindowTitle(QObject::tr("Ошибка"));
              msgBox.setText(stream_->buffer);
              msgBox.setStandardButtons(QMessageBox::Ok);
              msgBox.setDefaultButton(QMessageBox::Ok);
              msgBox.setButtonText(0x00000400,QObject::tr("Закрыть"));
              msgBox.setIcon(QMessageBox::Critical);
              msgBox.exec();
              break;
          }
        }
      }
    }

    delete stream_;
    stream_ = 0;
  }
}

TLog& TLog::operator << (const QString& s)
{
  if( tlog::kDebug == priority_ ){
    stream_->ts << '\"' << s  << '\"';
  }
  else {
    stream_->ts << s;
  }
  return maybeSpace();
}

TLog& TLog::operator << (const QStringList& l)
{
  QStringList::const_iterator it  = l.constBegin();
  QStringList::const_iterator end = l.constEnd();
  stream_->ts << '{' << '\n';
  while( it != end ){
    stream_->ts << '"' << *it << '"' << '\n';
    ++it;
  }
  stream_->ts << '}' << '\n';

  return maybeSpace();
}

TLog& TLog::operator << (const QList<QPoint>& l)
{
  QValueList<QPoint>::const_iterator it  = l.constBegin();
  QValueList<QPoint>::const_iterator end = l.constEnd();
  stream_->ts << '{';
  while( it != end ){
    stream_->ts << '(' << (*it).x() << ';' << (*it).y() << ')';
    ++it;

    if( it != end ) { stream_->ts << ','; }
  }
  stream_->ts << '}' << '\n';

  return maybeSpace();
}
