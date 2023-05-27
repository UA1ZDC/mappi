#include "uhdwindow.h"

#include <qevent.h>
#include <qmainwindow.h>
#include <qmessagebox.h>

#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/proto/map_document.pb.h>
#include <meteo/commons/proto/meteomenu.pb.h>



UhdWindow::UhdWindow(meteo::app::MainWindow* window, const meteo::map::proto::Document& blank) :
  meteo::map::MapWindow(window,blank)
{
  if ( 0 != mapview() ) {
    mapview()->setFocusPolicy( Qt::NoFocus );
  }
}


void UhdWindow::closeEvent(QCloseEvent *ev)
{
  if (false == flExit_) {
    if( 0 == QMessageBox::question( this, tr("Выход"), trUtf8("Закрыть окно: \"Тест аппаратуры приёма\"?"), trUtf8("Да"), trUtf8("Нет"))){
      ev->accept();
    }
    else{
      ev->ignore();
    }
  }
  else {
    ev->accept();
  }
}

void UhdWindow::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape)
  {
    if( 0 == QMessageBox::question( this, tr("Выход"), trUtf8("Закрыть окно: \"Тест аппаратуры приёма\"?"), trUtf8("Да"), trUtf8("Нет"))){
      flExit_=true;
      this->close();
    }
    else {
      flExit_=false;
    }
  }
}

void UhdWindow::setFlExit(bool flag)
{
  flExit_=flag;
}

