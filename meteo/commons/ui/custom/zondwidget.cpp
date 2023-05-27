#include "zondwidget.h"

#include <qmenu.h>
#include <qevent.h>
#include <qheaderview.h>
#include <qstandarditemmodel.h>

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/textproto/pbtools.h>

const QStringList kHeadersList = QStringList() << QObject::tr("Тип")
                                               << QObject::tr("P, гПа")
                                               << QObject::tr("H, м")
                                               << QObject::tr("T, °C")
                                               << QObject::tr("Td, °C")
                                               << QObject::tr("D, °C")
                                               << QObject::tr("dd, °")
                                               << QObject::tr("ff, м/с")
                                                  ;
const QStringList kHeadersTips = QStringList() << QObject::tr("<html><head/><body><p>Тип уровня:<br/>1 - земная или водная поверхность<br/>100 - изобарическая поверхность</p></body></html>")
                                               << QObject::tr("Давление, гПа")
                                               << QObject::tr("Геопотенциальная высота, м")
                                               << QObject::tr("Температура, °C")
                                               << QObject::tr("Точка росы, °C")
                                               << QObject::tr("Дефицит, °C")
                                               << QObject::tr("Направление ветра, °")
                                               << QObject::tr("Скорость ветра, м/с")
                                                  ;

ZondWidget::ZondWidget(QWidget* parent)
  : QTableView(parent)
{
  noData_ = true;

  lvlType_ = QList<int>() << 1 << 100;
  setSortingEnabled(true);
  sortByColumn(kLvlType, Qt::AscendingOrder);

  model_ = new QStandardItemModel(this);

  DataProxyModel* proxy = new DataProxyModel(this);
  proxy->setSourceModel(model_);
  setModel(proxy);

  connect( model_, SIGNAL(itemChanged(QStandardItem*)), SLOT(slotItemChanged(QStandardItem*)) );

  model_->setColumnCount(kHeadersList.size());
  model_->setHorizontalHeaderLabels(kHeadersList);

  setColumnWidth(kLvlType, 52);
  setColumnWidth(kP, 70);
  setColumnWidth(kH, 54);
  setColumnWidth(kT, 60);
  setColumnWidth(kTd, 60);
  setColumnWidth(kD, 62);
  setColumnWidth(kdd, 60);
  setColumnWidth(kff, 62);

  for ( int i=0,isz=kHeadersTips.size(); i<isz; ++i ) {
    model_->setHeaderData(i, Qt::Horizontal, kHeadersTips[i], Qt::ToolTipRole);
  }
}

void ZondWidget::setLabel(const QString& text)
{
  removeRows();

  int row = appendRow();

  setSpan(row,0,1,8);

  QModelIndex i = model_->index(row,0);
  model_->setData(i, text);
  model_->setData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
  model_->item(row, 0)->setEditable(false);

  noData_ = true;
}

int ZondWidget::columnCount() const
{
  return model_->columnCount();
}

int ZondWidget::rowCount() const
{
  return model_->rowCount();
}

void ZondWidget::setData(const meteo::surf::OneZondValueOnStation& data)
{
  zond_ = data;
  zond_.mutable_ur()->Clear();

  removeRows();

  for ( int i=0,isz=data.ur_size(); i<isz; ++i ) {
    const meteo::surf::Uroven& ur = data.ur(i);

    if ( !lvlType_.contains(ur.level_type()) ) { continue; }
    if ( 1 != ur.level_type() && ur.p().quality() > control::DOUBTFUL ) { continue; }
    if ( 1 == ur.level_type() && qFuzzyCompare( 1, 1 + ur.p().value()) ) { continue; }

    int row = appendRow();

    QModelIndex idx = model_->index(row, kLvlType);
    model_->setData(idx, ur.level_type());
    model_->item(row, kLvlType)->setEditable(true);

    idx = model_->index(row, kP);
    model_->setData(idx, ur.p().value());
    model_->setData(idx, ur.p().quality(), kQualRole);
    model_->item(row, kP)->setEditable(true);

    idx = model_->index(row, kH);
    double h = ur.h().value();
    model_->setData(idx, static_cast<int>(h));
    model_->setData(idx, ur.h().quality(), kQualRole);
    model_->item(row, kH)->setEditable(true);

    idx = model_->index(row, kT);
    model_->setData(idx, toFixed(ur.t().value(), 1));
    model_->setData(idx, ur.t().quality(), kQualRole);
    model_->item(row, kT)->setEditable(true);

    idx = model_->index(row, kTd);
    model_->setData(idx, toFixed(ur.td().value(), 1));
    model_->setData(idx, ur.td().quality(), kQualRole);
    model_->item(row, kTd)->setEditable(true);

    idx = model_->index(row, kD);
    model_->setData(idx, toFixed(ur.d().value(), 1));
    model_->setData(idx, ur.d().quality(), kQualRole);
    model_->item(row, kD)->setEditable(true);

    idx = model_->index(row, kdd);
    model_->setData(idx, static_cast<int>(ur.dd().value()));
    model_->setData(idx, ur.dd().quality(), kQualRole);
    model_->item(row, kdd)->setEditable(true);

    idx = model_->index(row, kff);
    model_->setData(idx, toFixed(ur.ff().value(), 1));
    model_->setData(idx, ur.ff().quality(), kQualRole);
    model_->item(row, kff)->setEditable(true);

  }

  noData_ = data.ur_size() == 0;
}

meteo::surf::OneZondValueOnStation ZondWidget::toZond() const
{
  if ( noData_ ) { return meteo::surf::OneZondValueOnStation(); }

  meteo::surf::OneZondValueOnStation z = zond_;

  for ( int r=0,isz=rowCount(); r<isz; ++r ) {
    meteo::surf::Uroven* ur = z.add_ur();

    QModelIndex idx = model_->index(r, kLvlType);
    ur->set_level_type(idx.data(Qt::DisplayRole).toInt());

    idx = model_->index(r, kP);
    ur->mutable_p()->set_value(idx.data(Qt::DisplayRole).toDouble());
    ur->mutable_p()->set_quality(idx.data(kQualRole).toInt());

    idx = model_->index(r, kH);
    ur->mutable_h()->set_quality(idx.data(kQualRole).toInt());
    ur->mutable_h()->set_value(idx.data(Qt::DisplayRole).toDouble());

    idx = model_->index(r, kT);
    ur->mutable_t()->set_value(idx.data(Qt::DisplayRole).toDouble());
    ur->mutable_t()->set_quality(idx.data(kQualRole).toInt());

    idx = model_->index(r, kTd);
    ur->mutable_td()->set_value(idx.data(Qt::DisplayRole).toDouble());
    ur->mutable_td()->set_quality(idx.data(kQualRole).toInt());

    idx = model_->index(r, kD);
    ur->mutable_d()->set_value(idx.data(Qt::DisplayRole).toDouble());
    ur->mutable_d()->set_quality(idx.data(kQualRole).toInt());

    idx = model_->index(r, kdd);
    ur->mutable_dd()->set_value(idx.data(Qt::DisplayRole).toDouble());
    ur->mutable_dd()->set_quality(idx.data(kQualRole).toInt());

    idx = model_->index(r, kff);
    ur->mutable_ff()->set_value(idx.data(Qt::DisplayRole).toDouble());
    ur->mutable_ff()->set_quality(idx.data(kQualRole).toInt());
  }

  return z;
}

void ZondWidget::slotItemChanged(QStandardItem* item)
{
  model_->blockSignals(true);

  double v = item->data(Qt::DisplayRole).toDouble();
  int q = ( !qFuzzyCompare(-9999, v) ) ? 0 : control::DOUBTFUL + 1;
  item->setData(q, kQualRole);

  model_->blockSignals(false);
}

void ZondWidget::contextMenuEvent(QContextMenuEvent* e)
{
  if ( noData_ ) { return; }

  QModelIndex idx = indexAt(e->pos());

  QMenu menu;

  QAction* rmAct = 0;
  if ( idx.isValid() ) {
    rmAct = menu.addAction(QIcon(":/meteo/"), tr("Удалить уровень"));
  }
  QAction* addAct = menu.addAction(QIcon(":/meteo/"), tr("Добавить уровень"));

  QAction* a = menu.exec(mapToGlobal(e->pos()));

  if ( 0 == a ) { return; }

  if ( a == rmAct ) {
    model_->removeRow(idx.row());
  }
  else if ( a == addAct ) {
    int row = appendRow();
    scrollTo(model_->index(row,0));
  }
}

int ZondWidget::appendRow()
{
  int row = rowCount();
  model_->setRowCount(row + 1);

  return row;
}

void ZondWidget::removeRows()
{
  model_->setRowCount(0);
}

double ZondWidget::toFixed(double v, int w) const
{
  if ( qFuzzyCompare(v, -9999) ) { return v; }

  const int k = 10 * w;
  return double(int64_t(v*k)) / k;
}


//
//
// DateFormatProxyModel
//
//

DataProxyModel::DataProxyModel(QObject* parent)
  : QIdentityProxyModel(parent)
{
}

QVariant DataProxyModel::data(const QModelIndex& index, int role) const
{
  if ( role != Qt::DisplayRole ) {
    return QIdentityProxyModel::data(index, role);
  }


  double v = QIdentityProxyModel::data(index, Qt::DisplayRole).toDouble();
  if ( qFuzzyCompare(v, -9999) ) {
    return tr("н/д");
  }

  return QIdentityProxyModel::data(index, role);
}
