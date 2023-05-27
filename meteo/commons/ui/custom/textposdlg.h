#ifndef METEO_COMMONS_UI_CUSTOM_TEXTPOSDLG_H
#define METEO_COMMONS_UI_CUSTOM_TEXTPOSDLG_H

#include <qdialog.h>
#include <qmap.h>

#include <meteo/commons/proto/meteo.pb.h>

QList <meteo::Position> allAlignsList();

namespace Ui {
class TextPoswidget;
}

class TextPosdlg : public QDialog
{
  Q_OBJECT

public:
  explicit TextPosdlg(QWidget* parent = 0);
  virtual ~TextPosdlg();

  void setCurrent(meteo::Position pos);
  meteo::Position currentPos() const;

  void setAllowedAligns(const QMap<meteo::Position, bool>& aligns);
  void setAllowedAlign(meteo::Position pos, bool allow);
  QMap <meteo::Position, bool> allowedAligns();

private slots:
  void slotCurrentChanged();

private:
  Ui::TextPoswidget* ui_;
  QMap <meteo::Position, bool> allowedAligns_;

  meteo::Position pos_;
  QMap<meteo::Position,QPushButton*> posbtn_;

};

#endif
