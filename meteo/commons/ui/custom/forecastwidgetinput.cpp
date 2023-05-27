#include "forecastwidgetinput.h"


ForecastWidgetInput::ForecastWidgetInput( QWidget* p )
  : QWidget(p)
{
  

  // наименование лэйбл
  label_= new QLabel(this);

  // лайаут
  layout_= new QHBoxLayout(this);

  // поле ввода
  edit_ = new QLineEdit(this);
  edit_->setInputMask("###");

  layout_->addWidget(label_);
  layout_->addWidget(edit_);
  layout_->setSpacing(0);
  layout_->addSpacerItem( new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Minimum) );

  connect( edit_, SIGNAL(textChanged(QString)), SLOT(inputChanged(QString)) );
  this->setLayout(layout_);

}

ForecastWidgetInput::~ForecastWidgetInput()
{
  delete label_;
  delete layout_;
  delete edit_;
}



/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetInput::setName(QString name)
{
  name_ = name;
  this->label_->setText(name);
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetInput::setGradation(int grad)
{
  gradation_ = grad;
}



/**
 *
 *    S L O T S
 *
 * 
 */

void ForecastWidgetInput::inputChanged(QString txt){
  txt = txt.trimmed();
  if ( txt=="" ){
    this->text_ = QString("");
    emit inputChange();
  }else{

    // 
    // если около 0
    // 
    if ( txt=="00" ){
      this->text_ = QString("около 0°С");
      emit inputChange();
      return;
    }    

    int beg;
    beg = txt.toInt();

    // 
    // если около 0
    // 
    if ( txt=="-0" ){
      this->text_ = QString("%1...%2°С").arg(beg).arg( beg-this->gradation_ );
      emit inputChange();
      return;
    }    

    // 
    // если меньше нуля
    // 
    if (beg<0){
      this->text_ = QString("%1...%2°С").arg(beg).arg( beg-this->gradation_ );
      emit inputChange();
      return;
    }
    
    // если все остальное
    this->text_ = QString("%1...%2°С").arg(beg).arg( beg+this->gradation_ );
    emit inputChange();
  }

  
}

