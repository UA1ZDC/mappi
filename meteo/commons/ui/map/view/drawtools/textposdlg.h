#ifndef METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_TEXTPOSDLG_H
#define METEO_COMMONS_UI_MAP_VIEW_DRAWTOOLS_TEXTPOSDLG_H

#include <qdialog.h>
#include <meteo/commons/ui/map/object.h>

namespace Ui {
class TextPoswidget;
}

namespace meteo {
namespace map {
class TextPosDlg : public QWidget
{
  Q_OBJECT

public:
  explicit TextPosDlg(QWidget* parent = 0);
  virtual ~TextPosDlg();

  void setCurrent(Position pos);
  Position currentPos() const;
  void setAllowedAligns(const QMap<Position, bool>& aligns);
  void setAllowedAlign(Position pos, bool allow);
  QMap <Position, bool> allowedAligns();

private slots:
  void slotCurrentChanged();

private:
  Ui::TextPoswidget* ui_;

  Position pos_;

  QMap <Position, bool> allowedAligns_;
  QMap<Position,QPushButton*> posbtn_;

signals:
  void posChanged(int);
};

}
}

#endif
