#ifndef METEO_COMMONS_UI_CUSTOM_COLORDLG_H
#define METEO_COMMONS_UI_CUSTOM_COLORDLG_H

#include <qdialog.h>
#include <qlist.h>
#include <qcolor.h>

class QToolButton;

class ColorDlg : public QDialog
{
  Q_OBJECT
public:
  explicit ColorDlg(QWidget* parent = 0);

  QRgb currentColor() const { return rgb_; }

public slots:
  void slotSetColor( QRgb rgb );

private slots:
  void slotChangeColor();
  void slotCustomColor();

private:
  QList<QToolButton*> buttons_;

  QRgb rgb_;
};

#endif
