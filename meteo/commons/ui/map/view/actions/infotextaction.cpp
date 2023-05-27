#include "infotextaction.h"
#include "../mapview.h"

#include <QGraphicsTextItem>

namespace meteo {
namespace map {

  
  InfoTextAction::InfoTextAction(const QString& text, QPoint& pnt, MapScene* scene):
    Action(scene, "infotextaction"),
    pnt_(pnt)
  {
    text_ = scene->addText(text);
    text_->setPlainText(text);
    QRectF r = text_->boundingRect();
    text_->setPos(pnt - QPoint(r.width() / 2, r.height() / 2));
    item_ = text_;
  }

  InfoTextAction::~InfoTextAction()
  {
    item_ = 0;
    if (0 != text_) {
      delete text_;
      text_ = 0;
    }
  }

  void InfoTextAction::setTextSize(int size)
  {
    if (0 == text_) return;
    QFont font = text_->font();
    font.setPointSize(size);
    text_->setFont(font);
    QRectF r = text_->boundingRect();
    text_->setPos(pnt_ - QPoint(r.width() / 2, r.height() / 2));
  }

  void InfoTextAction::setTextColor(const QColor& color)
  {
    if (0 == text_) return;
    text_->setDefaultTextColor(color);
  }
  

}
}
