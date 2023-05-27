#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_MAPRUBBER_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_MAPRUBBER_H

#include <meteo/commons/ui/map/view/actions/selectaction.h>

#include <qwidget.h>

namespace Ui {
  class MapRubber;
}

namespace meteo {
namespace map {
  class Document;

  //! Виджет для создания рамки выделения
  class MapRubber : public QWidget, public SelectActionIface
  {
    Q_OBJECT
    public:
    explicit MapRubber(MapView* view_, QWidget* parent = 0, const QString& acceptText = QString());
    ~MapRubber();

    void setPoints(const GeoPoint& nw, const GeoPoint& ne, const GeoPoint& sw, const GeoPoint& se, const QRect& rect);
    void getPoints(GeoPoint* nw, GeoPoint* ne, GeoPoint* sw, GeoPoint* se, QRect* rect);
    QRect band() { return band_; }
    
    bool isRubberPressed() const;
    void rubberFinished();
    void resetRubber(MapScene* scene); 
    void removeRubber();
    void fillCoordsFromDocument(map::Document* doc = 0);
    void acceptRubber();

  signals:
    void accept();

  protected:
    QString acceptText() { return acceptText_; }

  private slots:
    void slotRubberClicked(bool on);
    void slotMajorCoordsEdited();
    void slotMinorCoordsEdited();

  private:
    Document* currentDocument() const;

  private:
    Ui::MapRubber* ui_;
    MapView* view_;
    QRect band_;

    QString acceptText_;
  };
}
}

#endif
