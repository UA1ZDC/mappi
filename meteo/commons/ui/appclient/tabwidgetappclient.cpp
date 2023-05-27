#include "tabwidgetappclient.h"

#include <qinputdialog.h>
#include <qevent.h>

namespace meteo {

TabBarAppClient::TabBarAppClient(QWidget* parent, QSettings* settings)
  : QTabBar(parent),
    settings_(settings)
{
}

void TabBarAppClient::mouseDoubleClickEvent(QMouseEvent* e)
{
  if (e->button () != Qt::LeftButton) {
    QTabBar::mouseDoubleClickEvent (e);
    return;
  }
  int idx = currentIndex ();
  bool ok = true;

  QInputDialog dlg;
  dlg.setInputMode(QInputDialog::TextInput);
  dlg.setOkButtonText(QObject::tr("Принять"));
  dlg.setCancelButtonText(QObject::tr("Отмена"));

  QString newName = dlg.getText(this, QObject::tr("Переименование"),
                                QObject::tr("Введите новое название:"),
                                QLineEdit::Normal,
                                tabText (idx),
                                &ok);

  if( true != newName.isEmpty() ){
    if (ok) {
      setTabText (idx, newName);
    }
    if( 0 != settings_ ){
      settings_->setValue(host_+".arm", newName);
    }
  }
}

void TabBarAppClient::setHost(const QString& host)
{
  host_ = host;
}

TabWidgetAppClient::TabWidgetAppClient(QWidget* parent, QSettings* settings) :
  QTabWidget(parent)
{
  bar_ = new TabBarAppClient(this, settings);
  setTabBar(bar_);
}

void TabWidgetAppClient::setHost(const QString& host)
{
  bar_->setHost(host);
}

} // meteo
