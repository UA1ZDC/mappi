#ifndef CROSS_COMMONS_APP_LOOPWAIT_H
#define CROSS_COMMONS_APP_LOOPWAIT_H

#include <qobject.h>
#include <qeventloop.h>

namespace meteo {


/*! 
 * \brief : Реализация wait без блокировки eventloop
 */
class WaitLoop : public QObject
{
  Q_OBJECT
  public:
    WaitLoop( QObject* parent = nullptr );
    ~WaitLoop();

    void wait( int32_t mescs = 100 );

  public slots:
    void slotQuit();

  private:
    QEventLoop loop_;

  private slots:
    void slotTimeout();
};

}

#endif
