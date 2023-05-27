#ifndef METEO_COMMONS_UI_CUSTOM_CLICKLABEL_H
#define METEO_COMMONS_UI_CUSTOM_CLICKLABEL_H

#include <qlabel.h>

class ClickLabel : public QLabel
{
  Q_OBJECT
  public:
    ClickLabel( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ClickLabel( const QString& text, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~ClickLabel();

  protected:
    void mouseDoubleClickEvent( QMouseEvent* ev );

  signals:
    void doubleclick();
};

#endif
