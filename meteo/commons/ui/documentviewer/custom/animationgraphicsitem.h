#ifndef METEO_COMMONS_DOCUMENTVIEWER_CUSTOM_ANIMATIONGRAPHICSITEM_H
#define METEO_COMMONS_DOCUMENTVIEWER_CUSTOM_ANIMATIONGRAPHICSITEM_H

#include <meteo/commons/ui/documentviewer/custom/imagedisplaygraphicsitem.h>
#include <meteo/commons/ui/documentviewer/custom/animationcontroller.h>
#include <qtimer.h>
#include <qobject.h>

namespace meteo {

class AnimationGraphicsItem : public QObject, public ImageDisplayGraphicsItem
{
  Q_OBJECT
public:
  AnimationGraphicsItem( meteo::map::MapScene* scene, const QList<QImage>& images, int timeout );
  AnimationGraphicsItem( meteo::map::MapScene* scene, const AnimationController &controller);

  virtual ~AnimationGraphicsItem() override;

private:
  void initConnect();

private slots:
  void slotSlideChanged( int index );


private:
  AnimationController controller_;
};

}
#endif
