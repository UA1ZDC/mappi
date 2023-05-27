#include "vkostations.h"
#include "ui_vkostations.h"


#include <meteo/commons/global/global.h>

#include <cross-commons/debug/tlog.h>

#include <qdatetime.h>


using namespace meteo;

enum Columns
  {
   kDtStartCol = 0, //!< начало сохранения в БД
   kDtEndCol   = 1, //!< окончание сохранения в БД
   kClimTypeCol = 2, //!< тип климатических данных (синоп/аэро)
   kTypeCol    = 3,  //!< тип станции
   kIndexCol   = 4,  //!< индекс/икао
   kRuNameCol  = 5,  //!< название рус
   kEnNameCol  = 6,  //!< название англ
   kStationTypeCol = 7, //!< Тип станции
   kCoordCol   = 8,  //!< координаты
   kUtcCol     = 9,  //!< +/- utc
  };

VkoStations::VkoStations(QWidget *parent /*= nullptr*/):
  QDialog(parent),
  ui_(new Ui::VkoStations)
{
  ui_->setupUi(this);

  ui_->stations->setColumnHidden(kTypeCol, true);
  
  connect(ui_->stations, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(selectStation()));
}

VkoStations::~VkoStations()
{
}

QString VkoStations::curIndex()
{
  if (nullptr == ui_->stations->currentItem()) {
    return QString();
  }
  return ui_->stations->currentItem()->text(kIndexCol);
}

QString VkoStations::curRuName()
{
  if (nullptr == ui_->stations->currentItem()) {
    return QString();
  }
  return ui_->stations->currentItem()->text(kRuNameCol);
}

QString VkoStations::curEnName()
{
  if (nullptr == ui_->stations->currentItem()) {
    return QString();
  }
  return ui_->stations->currentItem()->text(kEnNameCol);
}

//TODO from data
QDate VkoStations::curDateStart()
{
  if (nullptr == ui_->stations->currentItem()) {
    return QDate();
  }
  return QDateTime::fromString(ui_->stations->currentItem()->text(kDtStartCol), "dd-MM-yyyy").date();
}

QDate VkoStations::curDateEnd()
{
  if (nullptr == ui_->stations->currentItem()) {
    return QDate();
  }
  return QDateTime::fromString(ui_->stations->currentItem()->text(kDtEndCol), "dd-MM-yyyy").date();
}

int VkoStations::curUTC()
{
  if (nullptr == ui_->stations->currentItem()) {
    return 0;
  }
  return ui_->stations->currentItem()->text(kUtcCol).toInt();
}

QString VkoStations::curCoord()
{
  if (nullptr == ui_->stations->currentItem()) {
    return QString();
  }
  return ui_->stations->currentItem()->text(kCoordCol);
}

meteo::sprinf::MeteostationType VkoStations::climType()
{
  if (nullptr == ui_->stations->currentItem()) {
    return meteo::sprinf::kStationUnk;
  }
  return meteo::sprinf::MeteostationType(ui_->stations->currentItem()->data(kClimTypeCol, Qt::UserRole).toInt());
}

int VkoStations::curStationType()
{
  if (nullptr == ui_->stations->currentItem()) {
    return 0;
  }
  return ui_->stations->currentItem()->data(kUtcCol, Qt::UserRole).toInt();
}

void VkoStations::selectStation()
{
  cur_ = ui_->stations->currentItem();
  accept();
}

void VkoStations::removeSelection()
{
  cur_ = nullptr;
  ui_->stations->setCurrentItem(nullptr);
}

// int VkoStations::exec()
// {
//   fillStations();
//   return QDialog::exec();
// }

void VkoStations::fillStations(const climat::ClimatSaveProp& prop, const sprinf::StationTypes& types)
{
  if (loaded_) return;

  QList<QTreeWidgetItem *> items;
  for (int idx = 0; idx < prop.stations_size(); ++idx) {
    const climat::ClimatStation& sd =  prop.stations(idx);
    QTreeWidgetItem* item = new QTreeWidgetItem;
    QDate dt = QDate::fromString(QString::fromStdString(sd.dtbeg()), Qt::ISODate);
    item->setText(kDtStartCol, dt.toString("dd-MM-yyyy"));
    dt = QDate::fromString(QString::fromStdString(sd.dtend()), Qt::ISODate);
    item->setText(kDtEndCol, dt.toString("dd-MM-yyyy"));
    item->setText(kIndexCol, QString::fromStdString(sd.station().station()));
    item->setText(kTypeCol,  QString::number(sd.station().type()));
    item->setData(kTypeCol,  Qt::UserRole, sd.station().type());
    item->setText(kRuNameCol, QString::fromStdString(sd.station().name().rus()));
    item->setText(kEnNameCol, QString::fromStdString(sd.station().name().international()));
    for (auto st : types.station()) {
      if (st.type() == sd.station().type()) {
	item->setText(kStationTypeCol, QString::fromStdString(st.name()));
	break;
      }
    }
    item->setData(kStationTypeCol, Qt::UserRole, sd.station().type());
    
    GeoPoint pnt = GeoPoint(sd.station().position().lat_radian(), sd.station().position().lon_radian(),
			    sd.station().position().height_meters());
    item->setText(kCoordCol, pnt.toString(true));
    item->setText(kUtcCol, QString::number(sd.utc()));

    if (sd.cltype() == sprinf::kStationSynop) {
      item->setText(kClimTypeCol, QObject::tr("Приземные"));
      for (int i = 0, sz = ui_->stations->columnCount(); i < sz; ++i) {
	item->setBackgroundColor(i, QColor(236, 255, 238));
      }
    } else {
      item->setText(kClimTypeCol, QObject::tr("Аэрологические"));
      for (int i = 0, sz = ui_->stations->columnCount(); i < sz; ++i) {
	item->setBackgroundColor(i, QColor(227, 242, 252));
      }
    }
    item->setData(kClimTypeCol, Qt::UserRole, sd.cltype());
   
    items.append(item);
  }
  ui_->stations->insertTopLevelItems(0, items);

  for (int i = 0, sz = ui_->stations->columnCount(); i < sz; ++i) {
    ui_->stations->resizeColumnToContents(i);
  }
  
  loaded_ = true;
}

void VkoStations::setCurrent(const QString& station, int type, int cltype)
{
  for (int idx = 0; idx < ui_->stations->topLevelItemCount(); idx++) {
    QTreeWidgetItem* cur = ui_->stations->topLevelItem(idx);
    if (nullptr != cur &&
	cur->text(kIndexCol) == station &&
	cur->data(kTypeCol, Qt::UserRole).toInt() == type &&
	cur->data(kClimTypeCol, Qt::UserRole).toInt() == cltype) {
      ui_->stations->setCurrentItem(cur);
      cur_ = cur;
      break;
    }
  }
}

void VkoStations::resetCurrent()
{
  ui_->stations->setCurrentItem(cur_);
}
