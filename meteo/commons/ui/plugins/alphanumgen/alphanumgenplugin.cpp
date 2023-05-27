#include "alphanumgenplugin.h"
#include "kn01.h"
#include "kn03.h"
#include "kn04.h"

#include <qaction.h>

#include <cross-commons/app/paths.h>
#include <meteo/commons/global/log.h>
#include <cross-commons/debug/tlog.h>

#include <meteo/commons/ui/mainwindow/mainwindow.h>

namespace meteo {


static const auto kMenuPath = QList<QPair<QString, QString>>(
{
      QPair<QString, QString>("tasks", QObject::tr("Прикладные задачи")),
      QPair<QString, QString>("tlg", QObject::tr("Формирование телеграмм"))
});

AlphanumgenPlugin::AlphanumgenPlugin()
  : app::MainWindowPlugin("ancplugin")
{
  {
    static const QString& kn01 = QObject::tr("kn01");
    QAction* kn01Action = mainwindow()->findAction(kn01);
    if ( nullptr == kn01Action ){
      warning_log.msgBox() << msglog::kTargetMenyNotFound.arg(kn01);
    }
    else {
      kn01Action->setDisabled(false);
      QObject::connect(kn01Action, &QAction::triggered, this, &AlphanumgenPlugin::createKn01Dialog);
    }
  }

  {
    static const QString& kn03 = QObject::tr("kn03");
    QAction* kn03Action = mainwindow()->findAction(kn03);
    if ( nullptr == kn03Action ){
      warning_log.msgBox() << msglog::kTargetMenyNotFound.arg(kn03);
    }
    else {
      kn03Action->setDisabled(false);
      QObject::connect(kn03Action, &QAction::triggered, this, &AlphanumgenPlugin::createKn03Dialog);
    }
  }
  {
    static const QString& kn04 = QObject::tr("kn04");
    QAction* kn04Action = mainwindow()->findAction(kn04);
    if ( nullptr == kn04Action ){
      warning_log.msgBox() << msglog::kTargetMenyNotFound.arg(kn04);
    }
    else {
      kn04Action->setDisabled(false);
      QObject::connect(kn04Action, &QAction::triggered, this, &AlphanumgenPlugin::createKn04Dialog);
    }
  }
}

AlphanumgenPlugin::~AlphanumgenPlugin()
{
}

void AlphanumgenPlugin::createKn01Dialog()
{
  mainwindow()->toMdi(new Kn01(mainwindow()));
}

void AlphanumgenPlugin::createKn03Dialog()
{
  mainwindow()->toMdi(new Kn03(mainwindow()));
}

void AlphanumgenPlugin::createKn04Dialog()
{
  mainwindow()->toMdi(new Kn04(mainwindow()));
}

} // meteo
