#ifndef METEO_COMMONS_UI_CUSTOM_ALPHAEDIT_H
#define METEO_COMMONS_UI_CUSTOM_ALPHAEDIT_H

#include <qwidget.h>

class QAction;

namespace Ui {
  class AlphaEdit;
}

class AlphaEdit : public QWidget
{
  Q_OBJECT
  public:
    AlphaEdit( QWidget* parent, QAction* a );
    ~AlphaEdit();

    int percent() const ;
    void setPercent( int prcnt );

  protected:
    void keyPressEvent( QKeyEvent* e );

  private:
    Ui::AlphaEdit* ui_;
    QAction* action_;
};

#endif
