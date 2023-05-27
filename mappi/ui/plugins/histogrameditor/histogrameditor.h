#ifndef MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMEDITOR_H
#define MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMEDITOR_H

#include <qwidget.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/custom/doubleslider.h>
#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <QImage>
#include <qvector.h>

namespace Ui {
class HistogramEditor;
}

namespace meteo {

class HistogramEditor : public map::MapWidget {
  Q_OBJECT
public:
  explicit HistogramEditor(map::MapWindow* window);
  ~HistogramEditor();

private:
  QString uuidLayer_ = "Ко всем";
  QImage* img_= nullptr;
  QImage* originalImg_ = nullptr;
  QVector<float> histogram_;
  Ui::HistogramEditor* ui_ = nullptr;
  DoubleSlider* slider_ = nullptr;
  bool positionChanged_ = false;
  void loadImageFromLayer();
  void createHistogram( QImage* image);
  void showHistogram();
  void transformHistogram(int min, int max);
  void showPreview();
  void fillLayersBox();
  void ifAllImg();

protected:
  virtual bool eventFilter(QObject* obj, QEvent* event);

public slots:
  void slotLoadImageFromlayer(const QString& string);

private slots:
  void slotEqualization();
  void slotHandleTransform();
  void slotReturnImageOnLayer();
  void slotPositionChanged( int min, int max);
  void slotCancelTransform();
};

} //meteo

#endif // MAPPI_UI_PLUGINS_HISTOGRAMEDITOR_HISTOGRAMEDITOR_H
