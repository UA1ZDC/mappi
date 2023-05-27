#include "coloralertdlg.h"
#include "ui_coloralertdlg.h"

#include <qmenu.h>
#include <qevent.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <commons/textproto/pbcache.h>
#include <commons/textproto/pbtools.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/rpc/rpc.h>

#include <meteo/commons/global/global.h>

#include "conditionitem.h"
#include "delegate.h"

namespace meteo {

namespace tablo {
QString confFile() { return QDir::homePath() + "/.meteo/meteotablo.conf"; }
} // tablo

ColorAlertDlg::ColorAlertDlg(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::ColorAlertDlg)
{
  ui_->setupUi(this);

  ui_->tree->setItemDelegateForColumn(tablo::kColumnValue, new tablo::Delegate(nullptr));
  ui_->tree->setSelectionMode(QAbstractItemView::NoSelection);

  setStatus("");

  ui_->tree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect( ui_->okBtn, SIGNAL(clicked(bool)), SLOT(slotApply()) );
  connect( ui_->tree, SIGNAL(customContextMenuRequested(QPoint)), SLOT(slotContextMenu(QPoint)) );
  connect( ui_->tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(slotItemChanged(QTreeWidgetItem*,int)) );
  connect( ui_->tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotEditItem(QTreeWidgetItem*,int)) );
  connect( ui_->paramCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotParamChanged(int)) );
}

ColorAlertDlg::~ColorAlertDlg()
{
  delete ui_;
}

void ColorAlertDlg::loadSettings()
{
  ui_->tree->clear();

  if ( QFile::exists(tablo::confFile()) && !TProtoText::fromFile(tablo::confFile(), &opt_) ) {
    debug_log << tr("Не удалось загрузить настройки (%1).").arg(tablo::confFile());
    return;
  }

  for ( int i=0,isz=opt_.columns_size(); i<isz; ++i ) {
    QString name = pbtools::toQString(opt_.columns(i).meteo_descr());

    auto addFunc = [this](const QString& name)
    {
      QString text = name;
      meteodescr::Property prop;
      bool ok = TMeteoDescriptor::instance()->property(name, &prop);

      if ( ok ) {
        text += " - " + prop.description.toLower();
      }
      if ( ok && !prop.unitsRu.isEmpty() && "NO" != prop.unitsRu ) {
        text += " (" + prop.unitsRu + ")";
      }

      ui_->paramCombo->addItem(text, name);
    };

    addFunc(name);
    if ( "w" == name ) {
      addFunc("w_tr");
      addFunc("w_w_");
    }
    else if ( "V" == name ) {
      addFunc("VV");
    }
    else if ( "h" == name ) {
      addFunc("hgr");
    }
  }

  slotParamChanged(ui_->paramCombo->currentIndex());
}

void ColorAlertDlg::setStatus(const QString& text)
{
  ui_->statusLabel->setText(text);
}

QString ColorAlertDlg::currentDescr() const
{
  return ui_->paramCombo->itemData(ui_->paramCombo->currentIndex()).toString();
}

void ColorAlertDlg::setCurrent(const QString& descrName)
{
  for ( int i=0,isz=ui_->paramCombo->count(); i<isz; ++i ) {
    if ( descrName == ui_->paramCombo->itemData(i).toString() ) {
      ui_->paramCombo->setCurrentIndex(i);
      return;
    }
  }
}

void ColorAlertDlg::saveSettings() const
{
  if ( !TProtoText::toFile(opt_, tablo::confFile()) ) {
    debug_log << tr("Не удалось сохранить настройки (%1).").arg(tablo::confFile());
  }
}

void ColorAlertDlg::slotContextMenu(const QPoint& pos)
{
  QTreeWidgetItem* item = ui_->tree->itemAt(pos);

  QMenu menu;
  QAction* addAct = menu.addAction(tr("Добавить"));
  QAction* delAct = nullptr;
  if (nullptr != item ) {
    delAct = menu.addAction(QIcon(":/meteo/icons/delete-16.png"), tr("Удалить"));
  }

  QAction* act = menu.exec(QCursor::pos());
  if ( nullptr == act ) {
    return;
  }

  if ( act == delAct ) {
    delete item;
  }
  else if ( act == addAct ) {
    tablo::ConditionItem* i = new tablo::ConditionItem();
    int idx = ui_->tree->indexOfTopLevelItem(item);
    int k = ( -1 == idx ) ? ui_->tree->topLevelItemCount() : idx;
    ui_->tree->insertTopLevelItem(k, i);
  }
}

void ColorAlertDlg::slotItemChanged(QTreeWidgetItem* item, int column)
{
  if ( tablo::kColumnValue == column ) { return; }

  ui_->tree->blockSignals(true);

  Qt::CheckState state = item->checkState(column);
  item->setCheckState(tablo::kColumnGreen, Qt::Unchecked);
  item->setCheckState(tablo::kColumnYellow, Qt::Unchecked);
  item->setCheckState(tablo::kColumnRed, Qt::Unchecked);
  item->setCheckState(column, state);

  ui_->tree->blockSignals(false);
}

void ColorAlertDlg::slotEditItem(QTreeWidgetItem* item, int column)
{
  if ( tablo::kColumnValue != column ) { return; }
  if ( item->data(tablo::kColumnValue, tablo::kRoleValueDescription).isValid() ) { return; }

  Qt::ItemFlags f = item->flags();

  ui_->tree->blockSignals(true);
  item->setFlags(f | Qt::ItemIsEditable);
  ui_->tree->blockSignals(false);

  ui_->tree->editItem(item, column);

  ui_->tree->blockSignals(true);
  item->setFlags(f);
  ui_->tree->blockSignals(false);
}

void ColorAlertDlg::slotParamChanged(int index)
{
  QString name = ui_->paramCombo->itemData(index).toString();
  tablo::ColorAlert opt;
  opt.set_descrname(pbtools::toString(name));
  for ( int i=0,isz=opt_.alert_size(); i<isz; ++i ) {
    if ( pbtools::toQString(opt_.alert(i).descrname()) == name ) {
      opt = opt_.alert(i);
      break;
    }
  }

  ui_->tree->clear();


//  sprinf::MeteoParams request;
//  for ( int i=0; i<=1024; ++i ) {
//    sprinf::MeteoParam* param = request.add_param();
//    param->set_value(i);
//    param->set_descrname(opt.descrname());
//  }

  setStatus(tr("Инициализация..."));
  descr_t descr = TMeteoDescriptor::instance()->descriptor(name);
  global::loadBufrTables( QStringList() << QString::number(descr) );
  auto bufrtable = global::bufrTable( QString::number(descr) );
  QHash<int,QString> hash;
  for ( auto it = bufrtable.begin(), end = bufrtable.end(); it != end; ++it ) {
    auto val = it.key();
    auto description = it.value();
    if ( description.size() == 0 ) { continue; }
    if ( description.toLower().contains(tr("зарезервировано")) ) { continue; }
    if ( description.toLower().contains(tr("зарезервирована")) ) { continue; }
    if ( description.toLower().contains(tr("не используется")) ) { continue; }
    if ( description.toLower().contains(tr("не используются")) ) { continue; }
    hash.insert(int(val), description);
  }

  QHashIterator<int,QString> it(hash);
  while ( it.hasNext() ) {
    it.next();

    int val = it.key();
    QString text = it.value();

    tablo::Condition cond;
    cond.set_operation(tablo::kEqual);
    cond.set_operand_a(val);
    tablo::ConditionItem* item = new tablo::ConditionItem(ui_->tree);
    item->fromProto(cond);
    item->setData(tablo::kColumnValue, Qt::DisplayRole, text);
    item->setData(tablo::kColumnValue, tablo::kRoleValueDescription, text);
    item->setData(tablo::kColumnValue, Qt::ToolTipRole, tr("Значение метеопараметра %1").arg(val));
  }

  for ( int i=opt.condition_size() - 1; i>=0; --i ) {
    tablo::ConditionItem* item = findItem(opt.condition(i));
    if (nullptr == item ) {
      item = new tablo::ConditionItem;
      item->fromProto(opt.condition(i));
      ui_->tree->insertTopLevelItem(0, item);
    }
    else {
      item->setColor(opt.condition(i).color());
    }
  }

  setStatus(tr(""));
}

void ColorAlertDlg::slotApply()
{
  for ( int i=0,isz=ui_->tree->topLevelItemCount(); i<isz; ++i ) {
    QTreeWidgetItem* item = ui_->tree->topLevelItem(i);
    ui_->tree->closePersistentEditor(item, tablo::kColumnValue);
  }

  QString name = currentDescr();
  tablo::ColorAlert* alert = nullptr;
  for ( int i=0,isz=opt_.alert_size(); i<isz; ++i ) {
    if ( pbtools::toQString(opt_.alert(i).descrname()) == name ) {
      alert = opt_.mutable_alert(i);
      break;
    }
  }

  if ( nullptr == alert ) { alert = opt_.add_alert(); }

  alert->CopyFrom(toProto());
  saveSettings();
  accept();
}

tablo::ConditionItem* ColorAlertDlg::findItem(const tablo::Condition& cond) const
{
  for ( int r=0,rsz=ui_->tree->topLevelItemCount(); r<rsz; ++r ) {
    tablo::ConditionItem* i = static_cast<tablo::ConditionItem*>(ui_->tree->topLevelItem(r));

    if ( cond.operation() != i->data(tablo::kColumnValue, tablo::kRoleOperation).toInt() ) {
      continue;
    }
    double a = i->data(tablo::kColumnValue, tablo::kRoleOperandA).toDouble();
    if ( (qFuzzyIsNull(a) && !qFuzzyIsNull(cond.operand_a())) || (!qFuzzyIsNull(a) && qFuzzyIsNull(cond.operand_a())) ) {
      continue;
    }
    if ( !qFuzzyCompare(cond.operand_a(), a) ) {
      continue;
    }
    double b = i->data(tablo::kColumnValue, tablo::kRoleOperandB).toDouble();
    if ( (qFuzzyIsNull(b) && !qFuzzyIsNull(cond.operand_b())) || (!qFuzzyIsNull(b) && qFuzzyIsNull(cond.operand_b())) ) {
      continue;
    }
    if ( !qFuzzyCompare(cond.operand_b(), b) ) {
      continue;
    }

    return i;
  }

  return nullptr;
}

tablo::ColorAlert ColorAlertDlg::toProto() const
{
  tablo::ColorAlert alert;
  alert.set_descrname(pbtools::toString(currentDescr()));

  for ( int i=0,isz=ui_->tree->topLevelItemCount(); i<isz; ++i ) {
    tablo::ConditionItem* item = static_cast<tablo::ConditionItem*>(ui_->tree->topLevelItem(i));

    tablo::Condition cond = item->toProto();

    *alert.add_condition() = cond;
  }

  return alert;
}

}
