#ifndef METEO_COMMONS_UI_CUSTOM_LINGINTSPINBOX_H
#define METEO_COMMONS_UI_CUSTOM_LINGINTSPINBOX_H

#include <qabstractspinbox.h>

class LongIntSpinBox : public QAbstractSpinBox
{
  Q_OBJECT
public:
  explicit LongIntSpinBox(QWidget *parent = nullptr);


  long int	maximum() const;
  long int	minimum() const;
  void	setMaximum(long int max);
  void	setMinimum(long int min);

  void	setRange(long int minimum, long int maximum);

  void	setSingleStep(long int val);

  virtual void	stepBy(int steps) override;
  virtual StepEnabled stepEnabled() const override;


  long int value() const;
public slots:
  void	setValue(long int val);

protected:
  virtual QValidator::State validate(QString &input, int &pos) const override;

private:
  long int min_ = INT64_MIN;
  long int max_ = INT64_MAX;
  long int singleStep_ = 1;
};


#endif
