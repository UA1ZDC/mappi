#ifndef METEO_COMMONS_UI_PLUGINS_PLOTTINGPLUGIN_ISOSETTINGSWIDGET_H
#define METEO_COMMONS_UI_PLUGINS_PLOTTINGPLUGIN_ISOSETTINGSWIDGET_H

#include <commons/funcs/tcolorgrad.h>

#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/ui/map/layeriso.h>

#include <QtWidgets>

namespace meteo {
namespace app {
class MainWindow;
}
namespace map{

class IsoSettingsWidget : public QWidget
{
  Q_OBJECT
  public:
    IsoSettingsWidget( meteo::app::MainWindow* window );
    void setPixmap(const QPixmap& pix);
    void setWidth(int width);
    void setStep(float step);
    void setMin(float min);
    void setMax(float max);
    void setColorMin(const QColor& color);
    void setColorMax(const QColor& color);
    void setColorGradient(const QColor& color);
    void setStyle(Qt::PenStyle style);
    void setSettings(int level, int typeLevel, const proto::FieldColor& color );
    void setDefault();
    QColor colorMin() const;
    QColor colorMax() const;
    Qt::PenStyle lineStyle() const;
    int lineWidth() const;
    float step() const;
    float min() const;
    float max() const;

    void update( LayerIso* l );

  private:
    meteo::app::MainWindow* mainwindow_;
    QLabel* label_;
    QToolButton* btn_;

    QMenu* menu_;
    QMenu* stylemenu_;
    QMenu* widthmenu_;
    void paint();
    void clearPixmap();
    // QPixmap createPixmap(Qt::PenStyle style, int width, bool isSelected, QString text = QString());
    // void changeStylePixmap(QWidgetAction* act);
    // void changeWidthPixmap(QWidgetAction* act);

    int width_;
    Qt::PenStyle style_;
    QColor colorMin_;
    QColor colorMax_;
    QAction* actionMin_;
    QAction* actionMax_;
    QDoubleSpinBox* stepEdit_;
    QDoubleSpinBox* minEdit_;
    QDoubleSpinBox* maxEdit_;

  private slots:
    void slotExecMenu();
    void slotStyleChanged();
    void slotMinChanged();
    void slotMaxChanged();
    void slotColorMinChanged();
    void slotColorMaxChanged();
    void slotWidthChanged();
    void slotStepChanged();

  signals:
    void colorMinChanged(const QColor& color);
    void colorMaxChanged(const QColor& color);
    void styleChange(Qt::PenStyle);
    void widthChange(int width);
    void stepChange(float step);
    void minChange(float min);
    void maxChange(float max);
    void settingsChange();
};

}
}

#endif // ISOSETTINGSWIDGET_H
