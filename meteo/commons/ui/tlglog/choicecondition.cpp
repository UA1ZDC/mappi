#include "choicecondition.h"
#include "ui_choicecondition.h"
#include <cross-commons/debug/tlog.h>


choicecondition::choicecondition(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::choicecondition),
  enterName_(0),
  settings_(0)
{
  ui_->setupUi(this);
  ui_->buttonBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("Сохранить"));
  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setText(QObject::tr("Отмена"));

  ui_->condlist->setAlternatingRowColors(true);
  ui_->grouplist->setAlternatingRowColors(true);

  ui_->btnadd->setIcon(QIcon(":/meteo/icons/add.png"));
  ui_->btndel->setIcon(QIcon(":/meteo/icons/delete.png"));
  ui_->btnChoice->setIcon(QIcon(":/meteo/icons/ok.png"));
  ui_->bOpenConditions->setIcon(QIcon(":/meteo/icons/eye.png"));
  ui_->btnisEditCond->setIcon(QIcon(":/meteo/icons/edit.png"));
  ui_->bSavecond->setIcon(QIcon(":/meteo/icons/save.png"));
  ui_->btnisEditCond->setCheckable(true);
  ui_->btnisEditCond->setChecked(false);

  ui_->btnadd->setToolTip("Сохранение новой группы фильтров");
  ui_->btnChoice->setToolTip("Установить текущую группу фильтров");
  ui_->btndel->setToolTip("Удалить выбранную группу фильтров");
  ui_->bOpenConditions->setToolTip("Подробнее");
  ui_->btnisEditCond->setToolTip("Редактирование");
  ui_->bSavecond->setToolTip("Сохранить внесенные изменения");

  connect(ui_->buttonBox->button(QDialogButtonBox::Ok),SIGNAL(clicked(bool)),SLOT(accept()) );
  connect(ui_->buttonBox->button(QDialogButtonBox::Cancel),SIGNAL(clicked(bool)),SLOT(reject()) );

  connect(ui_->btnadd,SIGNAL(clicked(bool)),this,SLOT(slotAddGroup()) );
  connect(ui_->btnChoice,SIGNAL(clicked(bool)),this,SLOT(slotChoiceGroup()) );
  connect(ui_->btndel,SIGNAL(clicked(bool)),this,SLOT(slotRemoveGroup()) );

  connect(ui_->grouplist,SIGNAL(itemDoubleClicked(QListWidgetItem*)),SLOT(slotChoiceGroup()) );
  connect(ui_->grouplist,SIGNAL(itemClicked(QListWidgetItem*)),SLOT(slotGroupSelect()) );
  connect(ui_->grouplist,SIGNAL(itemSelectionChanged()),SLOT(slotGroupSelect()) );

  connect(ui_->btnisEditCond,SIGNAL(clicked(bool)),SLOT(slotCondEditable(bool)) );
  connect(ui_->condlist->model(),SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)),SLOT(slotCondEdit() ) );
  connect(ui_->bSavecond,SIGNAL(clicked(bool)),SLOT(slotSaveCond()) );
  ui_->bSavecond->setDisabled(true);
  ui_->condlist->setSelectionMode(QListWidget::ContiguousSelection);
  connect(ui_->bOpenConditions,SIGNAL(clicked(bool)),SLOT(slotHideCondPanel(bool)));
//  ui_->cond_wgt->hide();
//  adjustSize();
  settings_ = new QSettings(QDir::homePath() + "/.meteo/" + "TlgMonitorForm" + ".ini", QSettings::IniFormat);
}

choicecondition::~choicecondition()
{
  delete ui_;
  delete settings_; settings_ = 0;
}

void choicecondition::setGroupCondition(QMap<QString, QMap<int, QString> > group_cond_list,
                                        QString current_group, QMap<int, QString> conditions,
                                        QMap<QString, QMap<int, QString> > group_cond_human_list)
{
  ui_->grouplist->clear();
  current_group_.clear();
  group_cond_list_.clear();
  group_cond_human_list_.clear();
  conditions_.clear();
  checkpanelconditions_.clear();
  ui_->btnisEditCond->setChecked(false);
  ui_->bSavecond->setDisabled(true);
  current_group_ = current_group;
  group_cond_list_ = group_cond_list;
  group_cond_human_list_ = group_cond_human_list;
  conditions_ = conditions;
  checkpanelconditions_ = group_cond_human_list_[current_group_];
  foreach (QString str, group_cond_list_.keys()) {
    if( "0" != str ){
      ui_->grouplist->addItem(str);
    }
    else{
      if(!group_cond_list_[str].isEmpty()){
        ui_->btnadd->setDisabled(false);
      }
      else{
        ui_->btnadd->setDisabled(true);
      }
    }
  }
  slotRestoreGeometry();
}

void choicecondition::accept()
{
  ui_->condlist->clear();
  ui_->grouplist->clear();
  ui_->bSavecond->setDisabled(true);
  ui_->btnisEditCond->setChecked(false);
  slotSaveGeometry();
  QDialog::accept();
}

bool choicecondition::ChoiceGroup()
{
  if(ui_->grouplist->currentItem() == 0){
    return false;
  }
  return true;
}

void choicecondition::slotRemoveGroup(){
  if(ChoiceGroup()==false){
    return;
  }
  if(current_group_ == ui_->grouplist->currentItem()->text()){
    current_group_.clear();
  }
  group_cond_list_.remove(ui_->grouplist->currentItem()->text());
  group_cond_human_list_.remove(ui_->grouplist->currentItem()->text());
  delete ui_->grouplist->currentItem();
}

void choicecondition::slotAddGroup()
{
  if(!group_cond_list_["0"].isEmpty()){
    if(enterName_ == 0){
      enterName_ = new EnterGroupName;
    }
    QString str;
    if( QDialog::Accepted == enterName_->exec() ){
      str = enterName_->groupName();
    ui_->grouplist->addItem(str);
    group_cond_list_.insert(str,group_cond_list_["0"]);
    group_cond_list_.remove("0");
    group_cond_human_list_.insert(str,group_cond_human_list_["0"]);
    group_cond_human_list_.remove("0");
    current_group_ = str;
    ui_->grouplist->clear();
    ui_->grouplist->addItems(group_cond_list_.keys());
    ui_->btnadd->setDisabled(true);
    }
  }
}

void choicecondition::slotChoiceGroup()
{
 if( false == ChoiceGroup() ){
   return;
 }
  current_group_ = ui_->grouplist->currentItem()->text();
}

void choicecondition::slotHideCondPanel(bool on)
{
  if( on == true ){
//    ui_->widget->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding);
    ui_->widget->setSizePolicy(QSizePolicy::Policy::Fixed,QSizePolicy::Policy::Expanding);
    ui_->cond_wgt->show();
    adjustSize();
//    ui_->cond_wgt->adjustSize();
//    ui_->horizontalWidget->adjustSize();
//    ui_->horizontalWidget_4->adjustSize();
//    ui_->condlist->adjustSize();

    if( 0 != ui_->bOpenConditions){
    }
  }
  else {
//    ui_->horizontalWidget->setSizePolicy(QSizePolicy::Policy::Minimum,QSizePolicy::Policy::Fixed);
//    ui_->widget->setSizePolicy(QSizePolicy::Policy::Fixed,QSizePolicy::Policy::Fixed);
    ui_->widget->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding);
    ui_->cond_wgt->hide();
    ui_->horizontalWidget->hide();
    ui_->horizontalWidget_4->adjustSize();
    adjustSize();
    ui_->horizontalWidget->show();
    if( 0 != ui_->bOpenConditions ){
    }
  }
  adjustSize();
}

void choicecondition::slotGroupSelect()
{
  if(group_cond_human_list_.isEmpty() && group_cond_list_.isEmpty()){
    ui_->condlist->clear();
    return;
  }
  if(ui_->grouplist->currentItem() != 0){
  ui_->condlist->clear();
  }
  foreach (int column, group_cond_human_list_[ui_->grouplist->currentItem()->text()].keys()) {
    ui_->condlist->insertItem(0,group_cond_human_list_[ui_->grouplist->currentItem()->text()].value(column));
    ui_->condlist->item(0)->setData(Qt::UserRole+10,column);
    ui_->condlist->item(0)->setFlags( Qt::NoItemFlags | Qt::ItemIsEnabled );
  }
  ui_->bSavecond->setDisabled(true);
  slotCondEditable(ui_->btnisEditCond->isChecked());
}

void choicecondition::slotCondEdit()
{
  ui_->bSavecond->setEnabled(true);
}

void choicecondition::slotCondEditable(bool on)
{
  if( 0 == ui_->condlist->count()){
    ui_->bSavecond->setDisabled(true);
    return;
  }
  ui_->condlist->clear();
  if(ChoiceGroup()==false){
    return;
  }
  foreach (int column, group_cond_human_list_[ui_->grouplist->currentItem()->text()].keys()) {
    ui_->condlist->insertItem(0,group_cond_human_list_[ui_->grouplist->currentItem()->text()].value(column));
    if( false == on ){
      ui_->condlist->item(0)->setFlags( Qt::NoItemFlags | Qt::ItemIsEnabled );
    }
    else{
      ui_->condlist->item(0)->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled );
    }
  }
  ui_->bSavecond->setDisabled(true);
}

void choicecondition::slotSaveCond()
{
  if(ui_->bSavecond->isEnabled()){
  }
  for(int i=0;i<ui_->condlist->count();i++){
    QString itemtext = ui_->condlist->item(i)->text();
    int column;
    QString groupname = ui_->grouplist->currentItem()->text();

    QString regexptext;
    QString regex1;
    QString regex2;
    if(!itemtext.isNull() || !itemtext.isEmpty()){
      QStringList strlst = itemtext.split(" ");
      if(strlst[0] == "T1")            {column = 0;}
      else if(strlst[0] == "T2")       {column = 1;}
      else if(strlst[0] == "A1")       {column = 2;}
      else if(strlst[0] == "A2")       {column = 3;}
      else if(strlst[0] == "Сообщение"){column = 4;}
      else if(strlst[0] == "CCCC")     {column = 5;}
      else if(strlst[0] == "YYGGgg")   {column = 6;}
      else if(strlst[0] == "BBB")      {column = 7;}
      else if(strlst[0] == "Дата")     {column = 8;}

      if( strlst[2] == "Пустой"){
        regex1+=("^$");
      }
      if(strlst[2] == "Не_пустой"){
        regex1+=("(?!^$)");
      }
      if( strlst[2] != "Пустой" && strlst[2] != "Не_пустой"){
        if(strlst[1] == "="){
          regex1+=(strlst[2]);
        }
        else if(strlst[1] == "!="){
          regex1+="[^"+strlst[2]+"]";
        }
      }
      if(strlst[3] == "Регистр_Выкл"){
        regex1+=",0";
      }
      else{
        regex1+=",1";
      }
      if(strlst[4] == "Транслит_Выкл"){
        regex1+=",0";
      }
      else{
        regex1+=",1";
      }
      if( strlst.count()>6 ){
        if( strlst[8] == "Пустой"){
          regex2+=("^$");
        }
        if(strlst[8] == "Не_пустой"){
          regex2+=("(?!^$)");
        }
        if( strlst[8] != "Пустой" && strlst[8] != "Не_пустой"){
          if(strlst[1] == "="){
            regex2+=(strlst[2]);
          }
          else if(strlst[7] == "!="){
            regex2+="[^"+strlst[8]+"]";
          }
        }
        if(strlst[9] == "Регистр_Выкл"){
          regex2+=",0";
        }
        else{
          regex2+=",1";
        }
        if(strlst[10] == "Транслит_Выкл"){
          regex2+=",0";
        }
        else{
          regex2+=",1";
        }
        if(strlst[5] == "AND"){
          regex1+=",AND,";
        }
        else{
          regex1+=",OR,";
        }
        regexptext+=regex1+regex2;
      }
      regexptext+=regex1;
    }
    group_cond_human_list_[groupname].remove(column);
    group_cond_human_list_[groupname].insert(column,itemtext);
    group_cond_list_[groupname].remove(column);
    group_cond_list_[groupname].insert(column,regexptext);
  }
  ui_->bSavecond->setDisabled(true);
}

void choicecondition::slotSaveGeometry()
{
  settings_->setValue("groupcondwindowgeometry",saveGeometry());
  settings_->setValue("bOpenConditions",ui_->bOpenConditions->isChecked());
  settings_->setValue("btnisEditCond",ui_->btnisEditCond->isChecked());
}

void choicecondition::slotRestoreGeometry()
{
  slotHideCondPanel(settings_->value("bOpenConditions").toBool());
  restoreGeometry(settings_->value("groupcondwindowgeometry").toByteArray());
  ui_->bOpenConditions->setChecked(settings_->value("bOpenConditions").toBool());
  ui_->btnisEditCond->setChecked(settings_->value("btnisEditCond").toBool());
}
