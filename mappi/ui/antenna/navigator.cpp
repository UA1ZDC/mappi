#include "navigator.h"
#include <QPaintEvent>


namespace mappi {

namespace antenna {

static const qreal MARGIN_TB = 20;
static const qreal MARGIN_LR = 30;

Navigator::Navigator(QWidget* parent /*=*/, Qt::WindowFlags f /*=*/) :
    QWidget(parent, f),
  scene_(new Scene(coord_, this)),
  view_(new View(coord_, scene_, this)),
  hLayout_(new QHBoxLayout(this))
{
  hLayout_->setContentsMargins(MARGIN_LR, MARGIN_TB, MARGIN_LR, MARGIN_TB);
  hLayout_->addWidget(view_);

  QFont font;
  font.setPixelSize(12);
  font.setBold(true);
  setFont(font);

  QObject::connect(view_, &View::newPosition, this, &Navigator::newPosition);
  QObject::connect(view_, &View::keepMoving, this, &Navigator::keepMoving);
  QObject::connect(view_, &View::stop, this, &Navigator::stop);
}

Navigator::~Navigator()
{
}

void Navigator::setScope(float azimutCorrect, const Profile& profile)
{
  scene_->setZeroAzimut(azimutCorrect);
  coord_.x().setRange(profile.azimut.min, profile.azimut.max);
  coord_.y().setRange(profile.elevat.min, profile.elevat.max);
}

void Navigator::setCurrentPositon(float az, float en)
{
  scene_->setCurrentPosition(az, en);
  view_->update();
}

void Navigator::ready()
{
  view_->allowPosition(true);
  view_->update();
}

void Navigator::waiting()
{
  view_->allowPosition(false);
  view_->update();
}

void Navigator::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event)
  QPainter painter(this);

  QSize size = view_->size();
  coord_.setScope(size.width(), size.height());

  QFontMetrics fm(font());

  float offset = 0;
  int degree = 0;

  CoordGrid azimutGrid(coord_.x());
  azimutGrid.init();
  while (azimutGrid.next(&offset, &degree)) {
    QString text = QString::number(degree);
    QRect rect = fm.boundingRect(text);

    int x = MARGIN_LR + offset - (rect.width() / 2);
    painter.drawText(x, 3, rect.width(), rect.height(), Qt::AlignCenter, text);
    painter.drawText(x, size.height() + MARGIN_TB + 3, rect.width(), rect.height(), Qt::AlignCenter, text);
  }

  offset = 0;
  degree = 0;

  CoordGrid elevatGrid(coord_.y());
  elevatGrid.init(true);
  while (elevatGrid.next(&offset, &degree, true)) {
    QString text = QString::number(degree);
    QRect rect = fm.boundingRect(text);

    int y = MARGIN_TB + offset - (rect.height() / 2);
    painter.drawText(MARGIN_LR - rect.width() - 5, y, rect.width(), rect.height(), Qt::AlignCenter, text);
    painter.drawText(size.width() + MARGIN_LR + 5, y, rect.width(), rect.height(), Qt::AlignCenter, text);
  }
}

}

}
