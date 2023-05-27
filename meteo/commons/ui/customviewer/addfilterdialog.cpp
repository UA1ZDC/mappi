#include "addfilterdialog.h"
#include "ui_addfilterdialog.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>

TAddFilterDialog::TAddFilterDialog(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::AddFilterDialog)
{
  ui_->setupUi( this );

  ui_->buttonBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("Принять"));
  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setText(QObject::tr("Отмена"));
  ui_->buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(32);
  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(32);

  connect(ui_->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(accept()));
  connect(ui_->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(reject()));
  connect(ui_->groupCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotGroupChange()));
  connect(ui_->nameEdit, SIGNAL(textChanged(QString)), SLOT(slotAnyChange()));
  connect(ui_->t1, SIGNAL(textChanged(QString)), SLOT(slotAnyChange()));
  connect(ui_->t2, SIGNAL(textChanged(QString)), SLOT(slotAnyChange()));
  connect(ui_->a1, SIGNAL(textChanged(QString)), SLOT(slotAnyChange()));
  connect(ui_->a2, SIGNAL(textChanged(QString)), SLOT(slotAnyChange()));
  setWindowTitle(QObject::tr("Добавление фильтра"));
  createList();
  slotAnyChange();

}

QString TAddFilterDialog::id() const
{
 return id_;
}

QString TAddFilterDialog::name() const
{
  return ui_->nameEdit->text();
}

QString TAddFilterDialog::t1() const
{
  return ui_->t1->text();
}

QString TAddFilterDialog::t2() const
{
  return ui_->t2->text();
}

QString TAddFilterDialog::a1() const
{
  return ui_->a1->text();
}

QString TAddFilterDialog::a2() const
{
  return ui_->a2->text();
}

QString TAddFilterDialog::groupID() const
{
  QString name = ui_->groupCombo->currentText();
  return name_id_.key(name);
}

void TAddFilterDialog::fillForm(const ItemInfo& info)
{
  id_ = info.id;
  ui_->groupCombo->setCurrentIndex(ui_->groupCombo->findText(findGroupName(info.groupID)));
  ui_->nameEdit->setText(info.name);
  ui_->t1->setText(info.t1);
  ui_->t2->setText(info.t2);
  ui_->a1->setText(info.a1);
  ui_->a2->setText(info.a2);
  setWindowTitle(QObject::tr("Редактирование фильтра"));
}

QString TAddFilterDialog::findGroupName(const QString& group_id)
{

  foreach( const ItemInfo& info, infomap_ ){
    if( info.id == group_id ){
      return info.name;
    }
  }
  return QString();
}

void TAddFilterDialog::createList()
{/*
  NS_PGBase* db = meteo::global::dbTelegram();
  if (0 == db || false == db->Connected() ) {
    return;
  }

  TSqlQuery query(db);
  query.setQuery("SELECT * FROM get_gmi_condition_group();");

  if ( false == query.exec() ) {
    error_log << meteo::msglog::kDbRequestFailedArg.arg(query.query());
    return;
  }
  int result_size = query.size();
  if(0 == result_size){
    return;
  }
  infomap_.clear();
  name_id_.clear();
  for ( int i = 0; i < result_size; ++i ) {
    ItemInfo info;
    info.id = query.value( i, "_id" );
    info.name = query.value( i, "name" );
    info.t1 = query.value( i, "t1" );
    info.t2 = query.value( i, "t2" );
    info.a1 = query.value( i, "a1" );
    info.a2 = query.value( i, "a2" );
    infomap_.insert(query.value( i, "_id" ), info);
    name_id_.insert(query.value( i, "_id" ), query.value( i, "name" ));
    ui_->groupCombo->addItem(query.value( i, "name" ));
  }
*/
}

bool TAddFilterDialog::checkFill()
{
  if( ui_->nameEdit->text().isEmpty() ){
    return false;
  }

  bool ok = true;

  if( false == ui_->t1->text().isEmpty() && ui_->t1->isEnabled() ){
    ok = false;
  }
  if( false == ui_->t2->text().isEmpty() && ui_->t2->isEnabled() ){
    ok = false;
  }
  if( false == ui_->a1->text().isEmpty() && ui_->a1->isEnabled() ){
    ok = false;
  }
  if( false == ui_->a2->text().isEmpty() && ui_->a2->isEnabled() ){
    ok = false;
  }

  if( true == ok ){
    return false;
  }
  return true;
}

void TAddFilterDialog::slotAnyChange()
{
  if( true == checkFill() ){
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  }
  else{
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
}

void TAddFilterDialog::slotGroupChange()
{
  ui_->t1->clear();
  ui_->t2->clear();
  ui_->a1->clear();
  ui_->a2->clear();

  if( infomap_.contains(groupID()) ){
    ItemInfo info = infomap_.value(groupID());

    if( false == info.t1.isEmpty() ){
      ui_->t1->setText(info.t1);
      ui_->t1->setDisabled(true);
    }
    else{
       ui_->t1->setDisabled(false);
    }

    if( false == info.t2.isEmpty() ){
      ui_->t2->setText(info.t2);
      ui_->t2->setDisabled(true);
    }
    else{
       ui_->t2->setDisabled(false);
    }

    if( false == info.a1.isEmpty() ){
      ui_->a1->setText(info.a1);
      ui_->a1->setDisabled(true);
    }
    else{
       ui_->a1->setDisabled(false);
    }

    if( false == info.a2.isEmpty() ){
      ui_->a2->setText(info.a2);
      ui_->a2->setDisabled(true);
    }
    else{
       ui_->a2->setDisabled(false);
    }
  }
}
