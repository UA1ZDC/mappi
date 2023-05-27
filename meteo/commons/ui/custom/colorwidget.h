#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <qcolordialog.h>
#include <qradiobutton.h>

class IsoColorDialog : public QColorDialog
{
  Q_OBJECT
public:
  enum ColorIsolineOption {
    SingleIsoline = 1,
    MultiIsoline  = 2
  };
  enum IsoMode {
    UseOption = 1,
    NoOption  = 2
  };

  IsoColorDialog( double value = 0, IsoMode mode = NoOption, QWidget* parent = 0);
  ColorIsolineOption colorIsolineOption() const;
private:
  IsoMode mode_;
  QRadioButton* r1_;
  QRadioButton* r2_;
};

#endif // COLORWIDGET_H
