#include "forecastwidgetbutton.h"


ForecastWidgetButton::ForecastWidgetButton( QWidget* p )
  : QPushButton(p),r_(0),g_(145),b_(0)
{
  connect( this, SIGNAL(clicked(bool)), SLOT(buttonClicked()) );

  // инициализируем переменные по умолчанию
  duration_ = 2000;
  // colour_ = QString("#1cb900");
  // устанавливаем курсор в виде пальца
  this->setCursor( QCursor(Qt::PointingHandCursor) );
}

ForecastWidgetButton::~ForecastWidgetButton()
{
}


/**
 * задаем цвет анимации
 * @param color [description]
 */
void ForecastWidgetButton::setColor(int r, int g, int b)
{
  this->r_ = r;
  this->g_ = g;
  this->b_ = b;
  return;
}


/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetButton::setName(QString name)
{
  name_ = name;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetButton::setType(QString type)
{
  type_ = type;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetButton::setCategory(QString category)
{
  category_ = category;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetButton::setBcomma(bool bcomma)
{
  bcomma_ = bcomma;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetButton::setComma(bool comma)
{
  comma_ = comma;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetButton::setCatcomma(bool catcomma)
{
  catcomma_ = catcomma;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetButton::setNotfirst(bool notfirst)
{
  notfirst_ = notfirst;
}



/**
 *
 *    S L O T S
 *
 * 
 */

void ForecastWidgetButton::buttonClicked(){

  QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(this);
  this->setGraphicsEffect(effect);
  QPropertyAnimation *animation = new QPropertyAnimation(effect, "color");
  animation->setStartValue( QColor(r_,g_,b_) );
  animation->setEndValue( QColor( Qt::transparent ) ); //QWidget::foregroundRole()
  animation->setDuration( duration_ );
  animation->start();

  emit clickedWithParam(  name_, type_, category_, bcomma_, comma_, catcomma_, notfirst_ );
}

