#ifndef METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_PRELOADER_H
#define METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_PRELOADER_H

#include <qwidget.h>
#include <qprocess.h>

namespace Ui{
  class Preloader;
}

namespace meteo {
namespace app {

class Preloader : public QWidget
{
  Q_OBJECT
  public:
    Preloader(QWidget* parent=0);
    ~Preloader();
    void setTextLog(const QString& text);
    void setAppFinish(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    void resizeEvent(QResizeEvent* e);

  private:
   Ui::Preloader* ui_;
};

}
}

#endif // METEO_COMMONS_UI_MAINWINDOW_EMBEDWINDOW_PRELOADER_H
