#ifndef METEO_COMMONS_UI_MAINWINDOW_MDISUBWINDOW_H
#define METEO_COMMONS_UI_MAINWINDOW_MDISUBWINDOW_H

#include <qmdisubwindow.h>

namespace meteo {
namespace app {

class MdiSubWindow : public QMdiSubWindow{
  Q_OBJECT
  public:
    MdiSubWindow( QWidget* parent, QWidget* widget );
    ~MdiSubWindow();
    void setVisibleChildWindow(bool visible);

  protected:
    void closeEvent( QCloseEvent* closeEvent );
    void paintEvent( QPaintEvent* e );
    void showEvent( QShowEvent* e );
    void focusInEvent( QFocusEvent* e );
    void focusOutEvent( QFocusEvent* e );

  private:
    bool eventFilter( QObject* o, QEvent* e );

   private slots:
    void slotSetTitle(const QString & t);

  signals:
    void mdiSubWindowClosed( MdiSubWindow* window );

};

}
}

#endif
