#include "docslistplugin.h"

#include <qaction.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/app/paths.h>

#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/mainwindow/mainwindowplugin.h>
#include <meteo/commons/ui/mainwindow/mainwindow.h>

#include "docslistwidget.h"

namespace meteo {

/*!
 * \brief kMenuPath - Путь к пункту меню { id: "service", title: "Сервис"} ->
 * Для добавления вложенности нужно добавить пару в список
*/
static const auto kMenuPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("document", QObject::tr("Документы"))
});

QWidget* DocsListPlugin::createWidget(meteo::app::MainWindow *mw, const QString &option) const
{
  Q_UNUSED(option);  
  return new DocsListWidget(mw);
}

DocsListPlugin::DocsListPlugin():
  app::MainWindowPlugin("docslistplugin"),
  widget_(nullptr)
{
  QAction* action = mainwindow()->addActionToMenu({"docs", QObject::tr("Просмотр документов")}, kMenuPath);
  QObject::connect(action, &QAction::triggered, this, &DocsListPlugin::slotOpenWindow);
}

void DocsListPlugin::slotOpenWindow()
{
  QAction* a = qobject_cast<QAction*>(sender());

  if ( nullptr == a ) { return; }

  if ( nullptr == widget_ ) {
    widget_ = new DocsListWidget( mainwindow() );
    mainwindow()->toMdi(widget_);
    widget_->slotFillDocsList();
    connect( widget_, SIGNAL(destroyed(QObject*)), SLOT(slotWidgetDeleted()) );
  }

  widget_->resetDocFilter();
  if ( args_.contains(a) ) {
    widget_->setDocFilter(args_.value(a));
  }
  widget_->show();
  widget_->setFocus();
}

void DocsListPlugin::slotWidgetDeleted()
{
  widget_ = nullptr;
}

} // meteo
