#include "colorwidget.h"

#include <cross-commons/debug/tlog.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <QtWidgets>

IsoColorDialog::IsoColorDialog(double value, IsoMode mode, QWidget* parent) :
  QColorDialog(parent),
  mode_(mode)
{
  setWindowTitle(QObject::tr("Выбор цвета изолинии"));
  setOption(QColorDialog::ShowAlphaChannel);
  if( UseOption == mode_ ){
    QWidget* w = new QWidget();
    r1_ = new QRadioButton(QObject::tr("Выбранная изолиния"));
    r2_ = new QRadioButton(QObject::tr("Все изолинии со значением %1").arg(value));
    QHBoxLayout* l = new QHBoxLayout(w);
    l->addWidget(r1_);
    l->addWidget(r2_);
    layout()->addWidget(w);
    QVBoxLayout* gl = static_cast<QVBoxLayout*>(layout());
    gl->insertWidget(0, w);
    r1_->setChecked(true);
  }
  QObjectList ob = children();
  //var(ob.count());
  foreach( QObject* ch, ob ){
    //var(ch->metaObject()->className());
    if( ch->metaObject()->className() == QString("QLabel") ){
      QLabel* lbl = static_cast<QLabel*>(ch);
      if ( "&Basic colors" == lbl->text() ){
        lbl->setText(QObject::tr("Базовые цвета"));
      }
      if( "&Custom colors" == lbl->text() )
        lbl->setText(QObject::tr("Пользовательские цвета"));
    }
    if( ch->metaObject()->className() == QString("QPushButton")  ){
      static_cast<QPushButton*>(ch)->setText(QObject::tr("Добавить пользовательский цвет"));
    }
    if( ch->metaObject()->className() == QString("QDialogButtonBox") ){
      QDialogButtonBox* box = static_cast<QDialogButtonBox*>(ch);
      box->button(QDialogButtonBox::Ok)->setText(QObject::tr("Принять"));
      box->button(QDialogButtonBox::Cancel)->setText(QObject::tr("Отмена"));
    }
  }
}

IsoColorDialog::ColorIsolineOption IsoColorDialog::colorIsolineOption() const
{
  if( r1_->isChecked() ){
    return SingleIsoline;
  }else{
    return MultiIsoline;
  }
}
