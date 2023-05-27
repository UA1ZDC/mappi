#include "../../tlog.h"

#include <cross-commons/app/paths.h>

#include <qthread.h>
#include <qtextcodec.h>
#include <qapplication.h>
#include <qmutex.h>

#include <iostream>


#define THREAD_COUNT  256
#define TEST_TEXT     "ТЕСТ: одна строка - одно предложение"

class TTestThread : public QThread
{
public:
  TTestThread( int id ) : _id(id)  {}
  ~TTestThread(){
    info_log << "thread" << _id << " -- destroy";
  }

  virtual void run(){
    for( int count = 0; count < 100; count++ ) {
      debug_msg << tr(TEST_TEXT);
    }
  }

private:
  int _id;
};


void msgHandler( tlog::Priority priority, const QString &facility, const QString& fileName, int line, const QString &msg )
{
  Q_UNUSED( priority );
  Q_UNUSED( facility );
  Q_UNUSED( fileName );
  Q_UNUSED( line );
  Q_UNUSED( msg );

  std::cout << ASCII_FROM_QSTRING( msg ) << std::endl;
}

int main( int argc, char** argv )
{
  TAPPLICATION_NAME("test");
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app( argc, argv );

  TLog::setMsgHandler( msgHandler );

  TTestThread* threads[THREAD_COUNT];
  for( int i=0; i<THREAD_COUNT; ++i ){
    threads[i] = new TTestThread( i );
    threads[i]->start();
    QObject::connect( threads[i], SIGNAL(finished()),
                      threads[i], SLOT(deleteLater())
                      );
  }

  app.exec();

  return 0;
}
