#include "informwidget.h"

#include "ui_docinfo.h"

#include <meteo/commons/ui/map/document.h>

#include <meteo/commons/global/global.h>

namespace meteo {
namespace map {

InformWidget::InformWidget( MapWindow* p )
  : MapWidget(p),
  ui_( new Ui::DocInfo )
{
  ui_->setupUi(this);
  QObject::connect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotCloseClicked() ) );
}

InformWidget::~InformWidget()
{
  delete ui_; ui_ = nullptr;
}

void InformWidget::showEvent( QShowEvent* e )
{
  QString mapname;
  proto::Map map = mapdocument()->info();
  QString punkt ="";
  for(int i=0;i<map.station_size();++i ){
    punkt+=QString::fromStdString(map.station(i).name().rus())+" ";
  }

  //QString punkt = QString::fromStdString( meteo::global::punkt().name() );
  if ( true == punkt.isEmpty() ) {
    punkt = QObject::tr("Наименование пункта не указано");
  }
  else {
    punkt = QObject::tr("Пункт: ") + punkt;
  }
  QString user = meteo::global::currentUserName();
  if ( true == user.isEmpty() ) {
    user = QObject::tr("Пользователь не указан");
  }
  user = QObject::tr("Составил: ") + meteo::global::currentUserRank() + " " + user;
  QString term = QObject::tr("Дата формироания карты не указана");
  QString rank_cmdr = meteo::global::rank( meteo::global::punkt().rank_cmdr() );
  QString fio_cmdr = QString::fromStdString( meteo::global::punkt().fio_cmdr() );
  if ( true == rank_cmdr.isEmpty() ) {
    rank_cmdr = QObject::tr("Звание командира не указано");
  }
  if ( true == fio_cmdr.isEmpty() ) {
    fio_cmdr = QObject::tr("ФИО командира не указано");
  }

  QString layers = QObject::tr("Слои: \n");

  QString komandir = QObject::tr("Командир: ") + rank_cmdr + " " + fio_cmdr;
  if ( nullptr != mapdocument() ) {
    term = mapdocument()->dateString();
    QString srok = mapdocument()->termString();
    if ( false == srok.isEmpty() ) {
      term += "\n" + srok;
    }
    mapname = QString::fromStdString( map.title() );
    if ( true == mapname.isEmpty() ) {
      mapname = QObject::tr("Пустой документ");
    }
    else {
      mapname = "Документ: " + mapname;
    }
    for ( int i = 0, sz = mapdocument()->layers().size(); i < sz; ++i ) {
      Layer* layer = mapdocument()->layers()[i];
      layers += "  " + layer->name().simplified();
      if ( 0 != layer->info().data_size() ) {
        layers += QObject::tr(" кол-во %1").arg(layer->info().data_size() );
      }
      layers += '\n';
    }
  }
  term = QObject::tr("Дата составления: ") + term;
  QString text = mapname + '\n' + punkt + '\n' + term + '\n' + user + '\n' + komandir + '\n' + layers;
  ui_->info->setText(text);
  MapWidget::showEvent(e);
}

void InformWidget::slotCloseClicked()
{
  MapWidget::close();
}

}
}
