#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_ARROWDLG_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_ARROWDLG_H

#include <qlist.h>
#include <qpair.h>
#include <qwidget.h>

#include <meteo/commons/ui/map/object.h>

class QToolButton;

namespace meteo {
namespace map {

//!
class ArrowDlg : public QWidget
{
  Q_OBJECT
public:
  //!
  explicit ArrowDlg(QWidget* parent = 0);

  //!
  ArrowType currentArrowType() const { return type_; }

  static QString icoForType(ArrowType type);

public slots:
  void slotItemSelected();

signals:
  void arrowTypeChanged();

private:
  QList<QToolButton*> buttons_;

  ArrowType type_;
};

}
}

#endif // METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_ARROWDLG_H
