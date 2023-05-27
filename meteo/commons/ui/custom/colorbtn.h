#ifndef METEO_COMMONS_UI_CUSTOM_COLORBTN_H
#define METEO_COMMONS_UI_CUSTOM_COLORBTN_H

#include <qtoolbutton.h>
#include <qchar.h>
#include <qfont.h>

class ColorButton: public QToolButton
{
  Q_OBJECT
  public:
    ColorButton( QWidget* parent );
    ~ColorButton();

    QRgb color() const { return rgb_; }
    void setColor( QRgb rgb );

  private:
    QRgb rgb_;

  private slots:
    void slotClicked();

  signals:
    void colorChanged( QRgb rgb );

};

#endif
