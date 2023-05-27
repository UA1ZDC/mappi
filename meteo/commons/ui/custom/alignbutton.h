#ifndef METEO_COMMONS_UI_CUSTOM_ALIGNBUTTON_H
#define METEO_COMMONS_UI_CUSTOM_ALIGNBUTTON_H

#include <qtoolbutton.h>
#include <qmap.h>
#include <meteo/commons/global/common.h>

class AlignButton : public QToolButton
{
  Q_OBJECT
public:
  static QIcon icoForAlign( int align );

  AlignButton( QWidget* parent );
  ~AlignButton();

  int align() const { return align_; }
  void setAlign( int align );

  void setAllowedAligns(const QMap<meteo::Position, bool>& aligns);
  void setAllowedAlign(meteo::Position pos, bool allow);
  QMap <meteo::Position, bool> allowedAligns();
  void setPoint(QPoint point);
  void clearPoint() { setPoint_ = false; }

private:
  bool setPoint_ = false;
  QPoint point_;
  int align_;
  QMap <meteo::Position, bool> allowedAligns_;

  void setupIcon();

private slots:
  void slotClicked();

signals:
  void posChanged( int pos );

};

#endif
