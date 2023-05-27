#include "regexpconditiondialog.h"
#include "ui_regexpconditiondialog.h"
#include <qpushbutton.h>
#include <cross-commons/debug/tlog.h>

namespace {
enum {
  Null       = 0,
  Equall     = 1,
  NotEquall  = 2,
  Empty      = 3,
  NotEmpty   = 4,
  More       = 5,
  Less       = 6,
  MoreEquall = 7,
  LessEquall = 8
};

const QMap<int, QString>& basicConditions()
{
  static QMap<int, QString> bc;
  if (bc.isEmpty() == true) {
    bc.insert(Null, QString());
    bc.insert(Equall, QString::fromUtf8("равняется"));
    bc.insert(NotEquall, QString::fromUtf8("не равняется"));
    bc.insert(Empty ,QString::fromUtf8("пустое"));
    bc.insert(NotEmpty, QString::fromUtf8("не пустое"));
  }
  return bc;
}

const QMap<int, QString>& numberConditions()
{
  static QMap<int, QString> nc;
  if (nc.isEmpty() == true) {
    nc.insert(More, QString::fromUtf8("Больше чем"));
    nc.insert(Less, QString::fromUtf8("Меньше чем"));
    nc.insert(MoreEquall, QString::fromUtf8("Больше или равно чем"));
    nc.insert(LessEquall, QString::fromUtf8("Меньше или равно чем"));
  }
  return nc;
}

const QStringList& pgNumberTypes()
{
  static QStringList nt;
  if (nt.isEmpty() == true) {
    nt.append("bigint");
    nt.append("integer");
  }
  return nt;
}

}

RegExpConditionDialog::RegExpConditionDialog(QWidget *parent) :
  QDialog(parent),
  ui_(0)
{
  ui_ = new Ui::SqlCondition2;
  ui_->setupUi( this );
  ui_->registr1->setIcon(QIcon(":/meteo/icons/registr.png"));
  ui_->registr2->setIcon(QIcon(":/meteo/icons/registr.png"));

  ui_->translite1->setIcon(QIcon(":/meteo/icons/translit.png"));
  ui_->translite2->setIcon(QIcon(":/meteo/icons/translit.png"));

  QString tooltipregistr ("При нажатой кнопке, фильтр не различает заглавные и строчные буквы");
  QString tooltiptranslit("При нажатой кнопке, фильтр переводит буквы кириллицы в латинские");

  ui_->registr1->setToolTip(tooltipregistr);
  ui_->registr2->setToolTip(tooltipregistr);
  ui_->translite1->setToolTip(tooltiptranslit);
  ui_->translite2->setToolTip(tooltiptranslit);

  ui_->buttonBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("Принять"));
  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setText(QObject::tr("Отмена"));

  ui_->registr1->setCheckable(true);
  ui_->registr2->setCheckable(true);
  ui_->translite1->setCheckable(true);
  ui_->translite2->setCheckable(true);
  connect(ui_->conditionCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotCurrentIndexChanged(int)));
  connect(ui_->conditionEdit,SIGNAL(textChanged(QString)),SLOT(slotConditionEditChanged(QString)));
  connect(ui_->conditionCombo2, SIGNAL(currentIndexChanged(int)), SLOT(slotCurrentIndexChanged2(int)));
}

void RegExpConditionDialog::setCondition(const QString& column, const QString& condit)
{
  condition_ = column;
  ui_->conditionCombo->clear();
  ui_->conditionCombo2->clear();
  ui_->column_label->setText(condition_);
  ui_->conditionEdit->clear();
  ui_->conditionEdit2->clear();
  sql_.clear();
  checkpanelcond_.clear();
  foreach( int key, ::basicConditions().keys() ){
    ui_->conditionCombo->insertItem(key, ::basicConditions()[key]);
    ui_->conditionCombo2->insertItem(key, ::basicConditions()[key]);
  }

  if( ::pgNumberTypes().contains(datatype_) ){
    foreach( int key, ::numberConditions().keys() ){
      ui_->conditionCombo->insertItem(key, ::numberConditions()[key]);
      ui_->conditionCombo2->insertItem(key, ::numberConditions()[key]);
    }
  }
  ui_->conditionCombo->setCurrentIndex(Equall);
  ui_->conditionCombo2->setCurrentIndex(Null);

  if(!condit.isNull() || !condit.isEmpty()){
    QStringList strlst = condit.split(" ");
    debug_log << strlst;
    if ( strlst.count() > 1 ) {
      if(strlst[1] == "="){
        ui_->conditionCombo->setCurrentIndex(Equall);
      }
      if(strlst[1] == "!="){
        ui_->conditionCombo->setCurrentIndex(NotEquall);
      }
      if ( strlst.count() > 2 ) {
        if( strlst[2] == "Пустой"){
          ui_->conditionCombo->setCurrentIndex(Empty);
        }
        if(strlst[2] == "Не_пустой"){
          ui_->conditionCombo->setCurrentIndex(NotEmpty);
        }
        if( strlst[2] != "Пустой" && strlst[2] != "Не_пустой"){
          ui_->conditionEdit->setText(strlst[2]);
        }
        if ( strlst.count() > 3 ) {
          if(strlst[3] == "Регистр_Выкл"){
            ui_->registr1->setChecked(false);
          }
          else{
            ui_->registr1->setChecked(true);
          }
          if (strlst.count() > 4) {
            if( strlst[4] == "Транслит_Выкл"){
              ui_->translite1->setChecked(false);
            }
            else{
              ui_->translite1->setChecked(true);
            }
            if ( strlst.count() > 7 ) {
              if(strlst[7] == "="){
                ui_->conditionCombo2->setCurrentIndex(Equall);
              }
              if(strlst[7] == "!="){
                ui_->conditionCombo2->setCurrentIndex(NotEquall);
              }
              if ( strlst.count() > 8) {
                if( strlst[8] == "Пустой"){
                  ui_->conditionCombo2->setCurrentIndex(Empty);
                }
                if(strlst[8] == "Не_пустой"){
                  ui_->conditionCombo2->setCurrentIndex(NotEmpty);
                }
                if( strlst[8] != "Пустой" && strlst[8] != "Не_пустой"){
                  ui_->conditionEdit2->setText(strlst[8]);
                }
                if(strlst[5] == "AND"){
                  ui_->radioAnd->setChecked(true);
                }
                else{
                  ui_->radioOr->setChecked(true);
                }
                if ( strlst.count() > 9 ) {
                  if(strlst[9] == "Регистр_Выкл"){
                    ui_->registr2->setChecked(false);
                  }
                  else{
                    ui_->registr2->setChecked(true);
                  }
                  if ( strlst.count() > 10 ) {
                    if( strlst[10] == "Транслит_Выкл"){
                      ui_->translite2->setChecked(false);
                    }
                    else{
                      ui_->translite2->setChecked(true);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

void RegExpConditionDialog::accept()
{  
  QString value1 = ui_->conditionEdit->text();
  QString value2 =  ui_->conditionEdit2->text();
  QString regex1;
  QString regex2;
  QString cond1;
  QString cond2;
  QString eq1;
  QString eq2;
  QString registr1;
  QString registr2;
  QString translite1;
  QString translite2;

  if(ui_->registr1->isChecked()){
    registr1 +="Регистр_Вкл";
  }
  else{
    registr1 +="Регистр_Выкл";
  }

  if(ui_->registr2->isChecked()){
    registr2 +="Регистр_Вкл";
  }
  else{
    registr2 +="Регистр_Выкл";
  }

  if(ui_->translite1->isChecked()){
    translite1 +="Транслит_Вкл";
  }
  else{
    translite1 +="Транслит_Выкл";
  }

  if(ui_->translite2->isChecked()){
    translite2 +="Транслит_Вкл";
  }
  else{
    translite2 +="Транслит_Выкл";
  }

  if( Empty == ui_->conditionCombo->currentIndex() ){
    regex1+=("^$");
    value1 = "Пустой";
  }
  else if( NotEmpty == ui_->conditionCombo->currentIndex()  ){
    regex1+=("(?!^$)");
    value1 = "Не_пустой";
  }
  else if( Equall == ui_->conditionCombo->currentIndex()  ){
    if(ui_->translite1->isChecked()){
     value1 = meteo::global::kTranslitFunc(value1);
    }
    regex1+=(value1);
    eq1 = "=";
  }
  else if( NotEquall == ui_->conditionCombo->currentIndex()  ){
    if(ui_->translite1->isChecked()){
     value1 = meteo::global::kTranslitFunc(value1);
    }
    QString str;
    str += "[^"+value1+"]";
    regex1+=(str);
    eq1 = "!=";
  }


  if( Null != ui_->conditionCombo2->currentIndex() ){
    if( Empty == ui_->conditionCombo2->currentIndex() ){
      regex2+=("^$");
      value2 = "Пустой";
    }
    else if( NotEmpty == ui_->conditionCombo2->currentIndex()  ){
      regex2+=("(?!^$)");
      value2 = "Не_пустой";
    }
    else if( Equall == ui_->conditionCombo2->currentIndex()  ){
      if(ui_->translite2->isChecked()){
       value2 = meteo::global::kTranslitFunc(value2);
      }
      regex2+=(value2);
      eq2 = "=";
    }
    else if( NotEquall == ui_->conditionCombo2->currentIndex()  ){
      if(ui_->translite2->isChecked()){
       value2 = meteo::global::kTranslitFunc(value2);
      }
      QString str;
      str += "[^"+value2+"]";
      regex2+=(str);
      eq2 = "!=";
    }
  }
  cond1 = QString("%1 %2 %3 %4 %5").arg(condition_).arg(eq1).arg(value1).
      arg(registr1).arg(translite1);
  cond2 = QString("%1 %2 %3 %4 %5").arg(condition_).arg(eq2).arg(value2).
      arg(registr2).arg(translite2);

  if( false == regex1.isEmpty()){
    sql_ += regex1;
    if(ui_->registr1->isChecked()){
      sql_ += ",1";
    }
    else{
      sql_ += ",0";
    }
    if(ui_->translite1->isChecked()){
      sql_ += ",1";
    }
    else{
      sql_ += ",0";
    }
    checkpanelcond_ +=cond1;
  }
  if( false == regex2.isEmpty() ){
    ui_->radioAnd->isChecked() ? sql_ += ",AND," + regex2 : sql_ += ",OR," + regex2;
    ui_->radioAnd->isChecked() ? checkpanelcond_ += " AND " + cond2 : checkpanelcond_ += " OR " + cond2;
    if(ui_->registr2->isChecked()){
      sql_ += ",1";
    }
    else{
      sql_ += ",0";
    }
    if(ui_->translite2->isChecked()){
      sql_ += ",1";
    }
    else{
      sql_ += ",0";
    }
    if( regex1.isEmpty() ){
    }
  }
  if(sql_.isEmpty()){
    sql_.clear();
  }

  QDialog::accept();
}

void RegExpConditionDialog::slotCurrentIndexChanged(int index)
{
  switch( index ){
  case Null :
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    ui_->conditionEdit->clear();
    ui_->conditionEdit->setDisabled(true);
    break;
  case Empty :
  case NotEmpty :
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
    ui_->conditionEdit->clear();
    ui_->conditionEdit->setDisabled(true);
    break;
  default:
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
    ui_->conditionEdit->clear();
    ui_->conditionEdit->setDisabled(false);
  }
}

void RegExpConditionDialog::slotCurrentIndexChanged2(int index)
{
  switch( index ){
    case Null :
    case Empty :
    case NotEmpty :
      ui_->conditionEdit2->clear();
      ui_->conditionEdit2->setDisabled(true);
      break;
    default:
      ui_->conditionEdit2->clear();
      ui_->conditionEdit2->setDisabled(false);
  }
}

void RegExpConditionDialog::slotConditionEditChanged(QString str)
{
  if(str.isNull()){
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
  }
  else{
    ui_->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
  }
}
