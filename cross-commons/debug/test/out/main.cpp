#include <iostream>

#include <qtextcodec.h>
#include <qapplication.h>

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tdebug.h>


void msgHandler( tlog::Priority priority, const QString &facility, const QString& fileName, int line, const QString &msg )
{
  Q_UNUSED( priority );
  Q_UNUSED( facility );
  Q_UNUSED( fileName );
  Q_UNUSED( line );
  Q_UNUSED( msg );

  std::cout << "[cout]: " << ASCII_FROM_QSTRING( msg ) << std::endl;
}

int main( int argc, char** argv )
{
  TAPPLICATION_NAME("test");
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app( argc, argv );

  // TLog
  TLog( tlog::DBG, LOG_MODULE_NAME ) << "DEBUG OUT TEST: " << " - test message - " << "ok!";
  TLog( tlog::DBG, LOG_MODULE_NAME ) << "DEBUG OUT TEST: " << QObject::tr( " - кириллица  в trUtf8 - " ) << "ok!";

  TLog::setMsgHandler( msgHandler );
  TLog() << "Simple test message. " << QObject::tr( "Сообщение кириллицей" );
  TLog::setMsgHandler( 0 );

  // qDebug
  qDebug() << QObject::tr( "qDebug() -- тест" );

  none_log      << "message without priority - " << QObject::tr( "сообщение без приоритета" );
  debug_log     << "debug messsage - " << QObject::tr( "отладочное сообщение" );
  info_log      << "info messsage - " << QObject::tr( "информационно сообщение" );
  warning_log   << "warning messsage - " << QObject::tr( "предупреждение" );
  error_log     << "error messsage - " << QObject::tr( "сообщение об ошибке" );
  critical_log  << "critical messsage - " << QObject::tr( "критическая ошибка" );

  TLog::setMsgHandler( TLog::clearStdOut );

  none_log      << "message without priority - " << QObject::tr( "сообщение без приоритета" );
  debug_log     << "debug messsage - " << QObject::tr( "отладочное сообщение" );
  info_log      << "info messsage - " << QObject::tr( "информационно сообщение" );
  warning_log   << "warning messsage - " << QObject::tr( "предупреждение" );
  error_log     << "error messsage - " << QObject::tr( "сообщение об ошибке" );
  critical_log  << "critical messsage - " << QObject::tr( "критическая ошибка" );

  none_msg      << "message without priority - " << QObject::tr( "сообщение без приоритета" );
  debug_msg     << "debug messsage - " << QObject::tr( "отладочное сообщение" );
  info_msg      << "info messsage - " << QObject::tr( "информационно сообщение" );
  warning_msg   << "warning messsage - " << QObject::tr( "предупреждение" );
  error_msg     << "error messsage - " << QObject::tr( "сообщение об ошибке" );
  critical_msg  << "critical messsage - " << QObject::tr( "критическая ошибка" );

  return 0;
}
