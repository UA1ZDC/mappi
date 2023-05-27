#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_PRINTPREVIEW_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_PRINTPREVIEW_H

#include <qmainwindow.h>
#include <qprinter.h>

namespace Ui {
  class PrintPreview;
}

namespace meteo {
namespace map {


class PrintPreview : public QMainWindow {
  Q_OBJECT
  public:
  PrintPreview(QPrinter* printer, QWidget *parent = 0, Qt::WindowFlags flags = 0);
  ~PrintPreview();

  void updatePreview();

signals:
  void paintRequested(QPrinter *printer);
  void closed();

public slots:
  void setAllPages();
	       
private slots:
  void firstPage();
  void lastPage();
  void nextPage();
  void prevPage();
  void zoomIn();
  void zoomOut();
  void zoomFactorChanged();
  void previewChanged();
  void setSinglePage();
  void pageNumChanged();

private:

  void setupActions();
  void updateNavigateActions();
  void keyPressEvent( QKeyEvent * e);
  void closeEvent(QCloseEvent* e);
  void setupActionIcon(QAction *action, const QLatin1String &name);

private:

  Ui::PrintPreview* ui_; //NOTE: реализация в коде, а не ui
};

}
}



#endif
