#ifndef FIELDSWIDGET_H
#define FIELDSWIDGET_H

#include <meteo/commons/ui/map/view/widgets/coordwidget.h>

#include <commons/geobasis/geopoint.h>
#include <QtWidgets>

namespace meteo {
namespace map {
  struct valLabel;
    

  class FieldsWidget : public CoordWidget
{
  Q_OBJECT
public:

  enum columnsType{
    kDate = 0,
    kCenter = 1,
    kName  = 2,
    kLevel = 3,
    kValue = 4,
    kUnit = 5,
    kHour = 6,
    kColumnCount = 7
  };
  

  FieldsWidget(QWidget *parent = 0);
  ~FieldsWidget();
  
  void initWidget();
  
  void setText(const QList<valLabel>& text);
  const QList<valLabel> vallist() const;
 

private:
  QList<valLabel> vlist_;
  QTableWidget *table_;
  
};

}
}

#endif // FIELDSWIDGET_H
