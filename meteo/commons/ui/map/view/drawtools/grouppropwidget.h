#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPPROPWIDGET_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPPROPWIDGET_H

#include "propwidget.h"

namespace Ui {
class GroupPropWidget;
}

namespace meteo {
namespace map {

class GroupPropWidget : public PropWidget
{
  Q_OBJECT
public:
  explicit GroupPropWidget(QWidget *parent = 0);
  virtual ~GroupPropWidget();

  virtual bool hasValue() const;
  virtual void setValue(double value, bool enable = true);
  virtual double value() const;
  virtual QString unit() const;

  virtual meteo::Property toProperty() const { return meteo::Property(); }
  virtual void setProperty(const meteo::Property& prop) { (void)prop; }

private:
  Ui::GroupPropWidget* ui_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_GROUPPROPWIDGET_H
