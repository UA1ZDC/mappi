#include <meteo/commons/ui/custom/longintspinbox.h>
#include <qlineedit.h>
#include <cstdio>

LongIntSpinBox::LongIntSpinBox(QWidget *parent) : QAbstractSpinBox(parent)
{
  this->setValue(0);
}

QValidator::State LongIntSpinBox::validate(QString &input, int &) const
{
  bool ok = false;
  long int value = input.toLong(&ok);
  if ( ( true == ok) && ( this->min_ <= value) && ( value <= this->max_ ) ){
    return QValidator::State::Acceptable;
  }
  else {
    return QValidator::State::Invalid;
  }
}

long int	LongIntSpinBox::maximum() const
{
  return this->min_;
}

long int	LongIntSpinBox::minimum() const
{
  return this->max_;
}

void LongIntSpinBox::setMaximum(long int max)
{
  this->setRange( this->min_, max);
}

void LongIntSpinBox::setMinimum(long int min)
{
  this->setRange( min, this->max_ );
}

void LongIntSpinBox::setRange( long int minimum, long int maximum)
{
  this->min_ = minimum;
  this->max_ = maximum;
  this->setValue(this->value()); /* Обработка если новое значение min/max приводит к выходу за пределы допустимых значений */
}

void LongIntSpinBox::setSingleStep(long int val)
{
  this->singleStep_ = val;
}

void	LongIntSpinBox::stepBy(int steps)
{
  long int currentValue = this->value();
  this->setValue( currentValue + steps * singleStep_ );
}

long int LongIntSpinBox::value() const
{
  return this->text().toLong();
}

void LongIntSpinBox::setValue(long int val)
{
  if ( val >= this->max_) {
    this->lineEdit()->setText(QString::number(this->max_));
  }
  else if ( val <= this->min_ ){
    this->lineEdit()->setText(QString::number(this->min_));
  }
  else {
    this->lineEdit()->setText(QString::number(val));
  }
}

QAbstractSpinBox::StepEnabled LongIntSpinBox::stepEnabled() const
{
  long int currentValue = this->value();
  if ( currentValue <= this->min_ ){
    return QAbstractSpinBox::StepUpEnabled;
  }
  else if ( currentValue >= this->max_ ){
    return QAbstractSpinBox::StepDownEnabled;
  } else {
    return QAbstractSpinBox::StepUpEnabled  | StepDownEnabled;
  }
}
