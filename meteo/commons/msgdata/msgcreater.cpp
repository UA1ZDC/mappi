#include "msgcreater.h"

#include <qtimer.h>

namespace meteo {

MsgCreater::MsgCreater(QObject* parent)
  : QObject(parent)
{
}

void MsgCreater::slotInit()
{
  if ( nullptr != timer_ ) {
    timer_->stop();
    timer_->deleteLater();
    disconnect(timer_, 0, this, 0);
  }

  timer_ = new QTimer(this);
  timer_->setInterval(500);
  connect( timer_, &QTimer::timeout, this, &MsgCreater::slotRun );
}

void MsgCreater::slotRun()
{
  if ( true == run_ ) { return; }
  run_ = true;

  while ( queue_->size(kCreate) > 0 )
  {
    MsgQueueItem item = queue_->head(kCreate);

//    const tlg::MessageNew& msg = item.msg();
//    MsgInfo msginfo(msg);

    if ( false == item.nextStage() ) {
      emit msgProcessed(item.id());
    }
  }

  run_ = false;
}

} // meteo
