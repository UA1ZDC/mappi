#include "fieldsynopaction.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/layeriso.h>
#include <commons/obanal/tfield.h>
#include <meteo/commons/ui/map/incut.h>
#include <meteo/commons/ui/map/view/mapincut.h>




namespace {
//const QString widgetName() { return QString("fieldsynopwidget"); }
}

namespace meteo {
namespace map {


FieldSynopAction::FieldSynopAction(MapScene *scene)
  :Action(scene, "fieldsynopaction")
//    fieldSynWidget_(0),
//    fieldSynProxy_(0),
//    drag_(false),
//    press_(false),
//    ok_(false),
//    menu_(0)
{

  /*
  fieldSynWidget_ = new FieldSynopWidget();
  connect(WidgetHandler::instance(), SIGNAL(open(QString, QString)), SLOT(slotWidgetOpen(QString, QString)));

  if( 0 != fieldSynWidget_ ){
    fieldSynProxy_ = new FieldSynoProxy();
    fieldSynProxy_->setZValue(15001);
    fieldSynWidget_->setTypeWidget(FieldSynopWidget::DoubleButton);
    fieldSynWidget_->setObjectName(widgetName());
    scene_->addItem(fieldSynProxy_);
    fieldSynProxy_->setWidget(fieldSynWidget_);
    fieldSynWidget_->close();
    QVariant v = WidgetHandler::instance()->value( widgetName() + ".value");
    if( true == v.isNull() ){
      setDefaultPosition();
    }
    else{
      fixPos_ = v.toPoint();
    }
  }
  scene_->document()->turnEvents();
  if (scene_->document()->eventHandler() != 0) {
    scene_->document()->eventHandler()->installEventFilter(this);
  }
*/

}

FieldSynopAction::~FieldSynopAction()
{
}
/*


void FieldSynopAction::mouseMoveEvent(QMouseEvent *e)
{
  initFieldAction();
  if(0 == mapview() || 0 == fieldSynWidget_ || 0 == fieldSynProxy_  || 0 == scene_){
    return;
  }
  if ( false == hasView() ) {
    return;
  }
  QPoint point = mapview()->mapToScene( e->pos() ).toPoint();
  drag_ = false;

  GeoPoint gp = scene_->screen2coord(point);
  QString resultstr = fieldsResult(gp);

  if( true == resultstr.isEmpty() ){
    resultstr = QObject::tr("Нет полей для отображения");
  }

  fieldSynWidget_->setText(resultstr);
  fieldSynProxy_->update();
  QPoint pnt = fieldSynProxy_->pos().toPoint() - scene_->document()->documentTopLeft();
  fieldSynWidget_->setViewportPos(pnt);
  if ( fieldSynWidget_->width() > fieldSynWidget_->minimumWidth() ) {
    fieldSynWidget_->setMinimumWidth( fieldSynWidget_->width() );
  }

}

void FieldSynopAction::mouseReleaseEvent(QMouseEvent *e)
{
//  static bool slock = false;
//  if( fieldSynProxy_->isUnderMouse() ){
//    press_ = false;
//    fixPos_ = e->pos()-fieldSynProxy_->clickPos();
//  }
//  if( 0 == fieldSynWidget_ || 0 == fieldSynProxy_ ){
//    Action::mouseReleaseEvent(e);
//    return;
//  }
//  adjustProxy();
//  if( Qt::SHIFT == qApp->keyboardModifiers() ){
//    if( 0 != fieldSynWidget_ ){
//      fieldSynWidget_->setLock(slock);
//      slock = !slock;
//      //setDefaultPosition();
//    }
//  }else if( true == drag_ ){
//    if( true == fields_->lock() ){
//      FieldsProxyWidget* proxy = new FieldsProxyWidget;
//      proxy->setZValue(10000);
//      FieldsWidget* fw = new FieldsWidget;
//      fw->setTypeWidget(FieldsWidget::CloseButton);
//      proxylist_.append(proxy);
//      fw->adjustSize();
//      fw->setBtnVisible(false);
//      fw->setText(fields_->text());
//      proxy->setGeoPoint(fieldProxy_->gp());
//      scene_->addItem(proxy);
//      proxy->setWidget(fw);
//      proxy->setPos(fields_->pos()- QPoint(13,10));
//    }
//  }
//  drag_ = false;
  Action::mouseReleaseEvent(e);

}
*/

/*
void FieldSynopAction::wheelEvent(QWheelEvent *event)
{
  Action::wheelEvent(event);
}

void FieldSynopAction::initFieldAction()
{
  if( ok_ == true ){
    return;
  }
  if( 0 != scene_ ){
    QString uuid = scene_->mapview()->property("uuid").toString();
    if( 0 == TFormAdaptor::instance()->handler(uuid) ){
      return;
    }
    QMainWindow* mw = qobject_cast<QMainWindow*>(TFormAdaptor::instance()->handler(uuid)->mainwindow());
    if( 0 != mw ){
      if( 0 == menu_ ){
        menu_ = new QMenu(QObject::tr("Синоп"), mw);
        mw->menuBar()->addMenu(menu_);
        menu_->setObjectName("view_syn");
      }
      if( 0 != menu_ ){
        QAction* act = menu_->addAction( QObject::tr("Синоптическая ситуация") );
        connect(act, SIGNAL(triggered()), SLOT(slotFields()));
        ok_ = true;
      }
    }
  }

}

QString FieldSynopAction::fieldsResult(const GeoPoint &gp) const
{
  QString resultstr;
  QStringList vallist;
  QList<Layer*> layers = scene_->document()->layers();
  for ( int i = 0, sz = layers.size(); i < sz; ++i ) {
    Layer* l = layers[i];
    if ( true == l->hasField() ) {
      //bool ok = false;
//      double val = l->fieldValue( gp, &ok );
      QString sit;
      QString fenom;
      int num_sit;

      debug_log << gp.lat();

      obanal::TField* f = l->field();

//      QString val = obanal::TField::oprSynSit5(gp,*a,*b);

      f->oprSynSit5(gp,&sit,&fenom,&num_sit);

      var( sit );
      var( fenom );
      var( num_sit );

//      if ( true == ok ) {
//        vallist.append( l->shortName() + "  " + val);
//      }
    }
  }

  int sz = vallist.size();
  for ( int i = 0; i < sz; ++i ) {
    resultstr += QString("%1").arg(vallist[i]);
    if( i < sz - 1 ){
      resultstr += "<br>";
    }
  }
  return resultstr;


}

MapView *FieldSynopAction::mapview()
{
  if( false == scene_->hasMapview() ){
    return 0;
  }
  else{
    return scene_->mapview();
  }

}

bool FieldSynopAction::proxyIsVisible()
{
  if( 0 == fieldSynWidget_ || 0 == fieldSynProxy_ ){
    return false;
  }
  QPoint pnt = fieldSynProxy_->pos().toPoint() + fieldSynProxy_->rect().center().toPoint();
  if( scene_->document()->documentRect().contains(pnt) ){
    return true;
  }
  else{
    return false;
  }

}

void FieldSynopAction::setDefaultPosition()
{
  QPoint pnt;
  MapIncut* incut = scene_->incut(Incut::BOTTOM_RIGHT);
  if( 0 != incut ){
    pnt = QPoint(30, incut->boundingRect().height());
  }
  else{
    pnt = QPoint(0,0);
  }
  QPoint viewportBottomRight = scene_->document()->documentBottomRight() - scene_->document()->documentTopLeft();
  fixPos_ = viewportBottomRight - QPoint(fieldSynProxy_->boundingRect().width(), fieldSynProxy_->boundingRect().height()) - pnt;

}

void FieldSynopAction::slotFields()
{
  if( 0 != fieldSynWidget_ ){
    fieldSynWidget_->show();
  }

}

void FieldSynopAction::slotWidgetOpen(const QString &name, const QString &uuid)
{
  Q_UNUSED(uuid)
  if( widgetName() == name ){
    if( false == proxyIsVisible() ){
      setDefaultPosition();
    }
  }

}
*/



}
}
