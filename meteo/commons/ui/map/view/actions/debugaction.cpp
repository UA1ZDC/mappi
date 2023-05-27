#include "debugaction.h"

#include <qevent.h>
#include <qtoolbutton.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsproxywidget.h>
#include <qgraphicswidget.h>
#include <qobject.h>
#include <qdebug.h>
#include <qicon.h>
#include <qdialog.h>

#include <cross-commons/debug/tlog.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/object.h>

#include "../mapview.h"
#include "../mapscene.h"
#include "../menu.h"
#include "../widgetitem.h"

#include "ui_debug.h"
#include "../../layeriso.h"

namespace meteo
{
namespace map
{

DebugAction::DebugAction(MapScene *scene)
    : Action(scene, "debugaction"),
      btndebug_(new ActionButton),
      widgetdebug_(new WidgetItem(btndebug_)),
      ui_(0),
      searchtype_(kNoSearch)
{
    wgt_ = btndebug_;
    btndebug_->setToolTip(QObject::tr("Отладка"));
    btndebug_->setIcon(QIcon(":/meteo/icons/map/bug-off.png"));
    scene_->addItem(widgetdebug_);
    item_ = widgetdebug_;
    item_->setZValue(10001);
    showact_ = new QAction(QObject::tr("Отладка"), this);
    QObject::connect(showact_, SIGNAL(triggered()), this, SLOT(slotShowToggle()));
    QObject::connect(showact_, SIGNAL(triggered()), btndebug_, SLOT(toggle()));
}

DebugAction::~DebugAction()
{
    delete showact_;
    showact_ = 0;
    for (int i = 0, sz = drawed_.size(); i < sz; ++i) {
        delete drawed_[i];
    }
    drawed_.clear();
}

void DebugAction::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    if (false == hasView()) {
        return;
    }
    if (true == widgetdebug_->isUnderMouse()) {
        if (false == btndebug_->isChecked()) {
            btndebug_->setChecked(true);
            slotShowToggle();
        } else {
            btndebug_->setChecked(false);
            slotShowToggle();
        }
        widgetdebug_->update();
    }
}

void DebugAction::mouseMoveEvent(QMouseEvent *e)
{
    if (false == hasView()) {
        return;
    }
    if (0 == ui_) {
        return;
    }
    for (int i = 0, sz = drawed_.size(); i < sz; ++i) {
        delete drawed_[i];
    }
    drawed_.clear();
    QPoint point = view()->mapToScene(e->pos()).toPoint();
    ui_->info->setText(QObject::tr("Экранная координата %1:%2").arg(point.x()).arg(point.y()));
    QPoint cartpoint = scene_->document()->transform().inverted().map(point);
    QPoint abscoord = scene_->document()->absolutetransform().map(point);
    ui_->info->append(QObject::tr("Картезианская координата %1:%2")
        .arg( cartpoint.x() )
        .arg( cartpoint.y() )
        );
    ui_->info->append(QObject::tr("Абсолютная координата №1 %1:%2").arg( abscoord.x() ).arg( abscoord.y() ) );
    GeoPoint gp = scene_->document()->screen2coord(point);
    QString str;
    ui_->info->append(QObject::tr("Географическая координата %1").arg(str << gp));
    QPointF scr;
    QPoint scr2;
    scene_->document()->projection()->F2X_one( gp, &scr );
    scene_->document()->projection()->F2X_one( gp, &scr2 );
    ui_->info->append( QObject::tr("Геогр в экран =  %1 %2").arg( scr.x(), 0, 'f', 2 ).arg( scr.y(), 0, 'f',2 ) );
    ui_->info->append( QObject::tr("Геогр в экран =  %1 %2").arg( scr2.x() ).arg( scr2.y() ) );
    ui_->info->append( QObject::tr("toPint = %1 %2").arg( scr.toPoint().x()).arg(scr.toPoint().y() ) );
    Layer *l = scene_->document()->activeLayer();
    QList<Object *> objects;
    QList<QLine> lines;
    Object *o = 0;
    if (0 != l) {
        switch (searchtype_) {
        case kNoSearch:
            break;
        case kSearchUnder:
            objects = scene_->document()->objectsUnderScreenPoint(point, l);
            break;
        case kSearchNear: {
            QPoint cross;
            o = scene_->document()->objectNearScreenPoint(point, l, &cross);
            if (0 != o) {
                objects.append(o);
                lines.append(QLine(point, cross));
            }
        }
        break;
        }
    }
    
    LayerIso *liso = dynamic_cast<LayerIso *>(l);
    if(0 != l && 0 != o){
      debug_log<<"*********";
      liso->containGeo(o);
      
   /*   ui_->info->append(
        QObject::tr("Объект %1 - %2")
        .arg(scene_->document()->center().strLat())
        .arg(scene_->document()->center().strLon()));
     */ 
      
    }
    
    
    
    
    
    ui_->info->append(
        QObject::tr("Центр документа: %1 - %2")
        .arg(scene_->document()->center().strLat())
        .arg(scene_->document()->center().strLon()))
    ;
    ui_->info->append(
        QObject::tr("Мсштаб: %1")
        .arg(scene_->document()->scale()));
    ui_->info->append(
        QObject::tr("Вычисленный угол между 'вверх' и направлением на север: %1")
        .arg(scene_->document()->deviationFromNorth(gp)*RAD2DEG)
    );
    for (int i = 0, sz = objects.size(); i < sz; ++i) {
        ui_->info->append(QString("Тип объекта: %1").arg(objects[i]->type()));
        ui_->info->append(QString("Uuid объекта: %1").arg(objects[i]->uuid()));
        str = "";
        str += QString::number(0.000001 * int64_t(objects[i]->boundingRect(QTransform()).at(0).width()) *
                               int64_t (objects[i]->boundingRect(QTransform()).at(0).height()));
        ui_->info->append(QString("Ширина объекта: %1").arg(objects[i]->boundingRect(QTransform()).at(0).width()));
        ui_->info->append(QString("высота объекта: %1").arg(objects[i]->boundingRect(QTransform()).at(0).height()));
        ui_->info->append(QString("Площадь объекта: %1").arg(str));
        ui_->info->append(QString("Объект замкнут: %1").arg(objects[i]->isClosed()));
        ui_->info->append(QString("на обрезе: %1").arg(objects[i]->isObrez()));
        if (objects[i]->cartesianSkelet().size() >= 1) {
            int x1 = objects[i]->cartesianSkelet().first().first().x();
            int y1 = objects[i]->cartesianSkelet().first().first().y();
            int x2 = objects[i]->cartesianSkelet().first().last().x();
            int y2 = objects[i]->cartesianSkelet().first().last().y();
            ui_->info->append(QString("x1 объекта1: %1").arg(x1));
            ui_->info->append(QString("y1 объекта1: %1").arg(y1));
            ui_->info->append(QString("x2 объекта1: %1").arg(x2));
            ui_->info->append(QString("y2 объекта1: %1").arg(y2));
        }
        if (objects[i]->cartesianSkelet().size() >= 2) {
            int x1 = objects[i]->cartesianSkelet().at(1).first().x();
            int y1 = objects[i]->cartesianSkelet().at(1).first().y();
            int x2 = objects[i]->cartesianSkelet().at(1).last().x();
            int y2 = objects[i]->cartesianSkelet().at(1).last().y();
            ui_->info->append(QString("x1 объекта2: %1").arg(x1));
            ui_->info->append(QString("y1 объекта2: %1").arg(y1));
            ui_->info->append(QString("x2 объекта2: %1").arg(x2));
            ui_->info->append(QString("y2 объекта2: %1").arg(y2));
        }
//    ui_->info->append( QString("Скелет объекта: %1").arg(str) );
//    ui_->info->append( QString::fromStdString( objects[i]->property().Utf8DebugString() ) );
//    ui_->info->append( QObject::tr("Генерализация: ") + QString::fromStdString( objects[i]->general().proto().Utf8DebugString() ) );
        ui_->info->append(QString("Масштаб документа: %1").arg(scene_->document()->scale()));
        ui_->info->append(QString("Видимость объекта: %1").arg(objects[i]->visible(scene_->document()->scale())));





        //   ui_->info->append( QString("QRect объекта: %1").arg(str) );
        QVector<QPolygon> screen = scene_->document()->objectScreenSkelet(objects[i]->uuid());
        for (int j = 0, jsz = screen.size(); j < jsz; ++j) {
            const QPolygon &poly = screen[j];
            if (0 == poly.size()) {
                continue;
            }
            if (poly.first() == poly.last()) {
                QGraphicsPolygonItem *item = new QGraphicsPolygonItem;
                item->setPolygon(QPolygonF(poly));
                item->setFillRule(Qt::OddEvenFill);
                item->setPen(QPen(Qt::red));
                drawed_.append(item);
                scene_->addItem(item);
            } else {
                QPainterPath path;
                path.addPolygon(poly);
                QGraphicsPathItem *item = new QGraphicsPathItem(path);
                item->setPen(QPen(Qt::red));
                drawed_.append(item);
                scene_->addItem(item);
            }
        }
        if (i < lines.size()) {
            const QLine &l = lines[i];
            QGraphicsLineItem *item = new QGraphicsLineItem(l);
            item->setPen(QPen(Qt::red));
            drawed_.append(item);
            scene_->addItem(item);
        }
    }
}

void DebugAction::addActionsToMenu(Menu *menu) const
{
    menu->addDocumentAction(showact_);
}

void DebugAction::slotActionChanged(QAbstractButton *btn)
{
    if (btn  == ui_->nearbtn && true == btn->isChecked()) {
        searchtype_ = kSearchNear;
    } else if (btn  == ui_->underbtn && true == btn->isChecked()) {
        searchtype_ = kSearchUnder;
    } else {
        searchtype_ = kNoSearch;
    }
}

void DebugAction::slotShowToggle()
{
    if (0 == ui_) {
        QDialog *dlg = new QDialog(view(), Qt::WindowStaysOnTopHint);
        dlg->move(QPoint(0, 0));
        ui_ = new Ui::Debug;
        ui_->setupUi(dlg);
        QObject::connect(ui_->buttonGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(slotActionChanged(QAbstractButton *)));
        dlg->show();
    } else {
        if (0 != ui_->info->parent()) {
            delete ui_->info->parent();
        }
        delete ui_;
        ui_ = 0;
        for (int i = 0, sz = drawed_.size(); i < sz; ++i) {
            delete drawed_[i];
        }
    }
    drawed_.clear();
}

}
}
