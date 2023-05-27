#ifndef METEO_COMMONS_UI_MAP_VIEW_ACTIONS_H
#define METEO_COMMONS_UI_MAP_VIEW_ACTIONS_H


#include <meteo/commons/ui/map/view/actions/action.h>

#include <qpoint.h>

namespace meteo {
  namespace map {
    
    //! Текст на карте, поверх рамки
    class InfoTextAction : public Action {
      Q_OBJECT
      public:
      InfoTextAction(const QString& text, QPoint& pnt, MapScene* scene);
      ~InfoTextAction();

      void setTextSize(int size);
      void setTextColor(const QColor& color);

    private:
      QPoint pnt_;
      QGraphicsTextItem* text_;
           
    };

  }
}

#endif
