#include "forecastwidgetitem.h"


ForecastWidgetItem::ForecastWidgetItem( QWidget* p )
  : QWidget(p)
{

  // кнопка закрытия
  close_= new QToolButton(this);
  close_->setFixedSize(20,20);
  QIcon icon;
  icon.addFile(QLatin1String(":/meteo/icons/tools/delete.png"), QSize(), QIcon::Disabled, QIcon::On);
  close_->setIcon(icon);
  connect( close_, SIGNAL(clicked()), this, SLOT( destroySlot() ) );

  // лайаут
  layout_= new QHBoxLayout(this);
  // лайаут
  label_= new QLabel(this);
  label_->setMaximumHeight(20);
  label_->setStyleSheet("text-align:left;padding:0px;");

  layout_->addWidget(label_);
  layout_->setSpacing(0);
  layout_->addSpacerItem( new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Minimum) );
  layout_->addWidget(close_);

  this->setLayout(layout_);
  this->setStyleSheet("padding:0px;margin:0px;");
  this->setMaximumHeight(27);
  // устанавливаем курсор в виде пальца
  this->setCursor( QCursor(Qt::PointingHandCursor) );

  // испускаем сигнал на то, что список обновлен - создан объект
  emit updated();
}

ForecastWidgetItem::~ForecastWidgetItem()
{
  delete close_;
  delete layout_;
  delete label_;
}


/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetItem::setName(QString name)
{
  name_ = name;
  label_->setText(name);
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetItem::setType(QString type)
{
  type_ = type;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetItem::setCategory(QString category)
{
  category_ = category;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetItem::setBcomma(bool bcomma)
{
  bcomma_ = bcomma;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetItem::setComma(bool comma)
{
  comma_ = comma;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetItem::setCatcomma(bool catcomma)
{
  catcomma_ = catcomma;
}

/**
 * Устанавливаем значение 
 * @param id [description]
 */
void ForecastWidgetItem::setNotfirst(bool notfirst)
{
  notfirst_ = notfirst;
}



/**
 *
 *    S L O T S
 *
 * 
 */

void ForecastWidgetItem::buttonClicked(){

}


void ForecastWidgetItem::destroySlot()
{
  // испускаем сигнал на то, что список обновлен - создан объект
  // emit updated();
  emit deleteItem(this);
  this->deleteLater();
}
