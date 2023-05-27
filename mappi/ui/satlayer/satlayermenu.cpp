#include "satlayermenu.h"

#include <qaction.h>
#include <qapplication.h>
#include <qcolordialog.h>
#include <qicon.h>
#include <qlabel.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <qwidgetaction.h>

//#include <cross-commons/map/tmap.h>
#include <cross-commons/debug/tlog.h>

#include <commons/obanal/tfield.h>

//#include <sql/nspgbase/tsqlquery.h>

#include <meteo/commons/ui/custom/alphaedit.h>
#include <meteo/commons/ui/custom/doublespinaction.h>
#include <meteo/commons/ui/custom/gradienteditor.h>
#include <meteo/commons/ui/custom/layerename.h>

#include <mappi/settings/mappisettings.h>
#include <mappi/ui/satelliteimage/satelliteimage.h>

#include "satlayer.h"

namespace meteo {
namespace map {

SatLayerMenu::SatLayerMenu( Layer* l, QObject* p )
  : LayerMenu( l, p ),
    paletteAct_(0)
{
  paletteAct_= new QAction( QIcon(":/meteo/icons/tools/fill.xpm"), QObject::tr("Палитра"), this );
  connect( paletteAct_, &QAction::triggered, this, &SatLayerMenu::slotColorFillDialog );
}

SatLayerMenu::~SatLayerMenu()
{
  delete paletteAct_;
  paletteAct_ = 0;
}

void SatLayerMenu::addActions( QMenu* menu )
{
  SatLayer* l = maplayer_cast<SatLayer*>(layer_);
  if ( 0 == l ) {
    return;
  }

  TColorGradList grad = mappi::inter::Settings::instance()->palette(l->thematicTypeName().toStdString());
  if ( !grad.isEmpty() ) {
    menu->addAction(paletteAct_);
    menu->addSeparator();
  }

  LayerMenu::addActions(menu);
}

void SatLayerMenu::slotColorFillDialog()
{
  SatLayer* l = maplayer_cast<SatLayer*>(layer_);
  if ( 0 == l ) {
    return;
  }

  TColorGradList palette = l->palette();

  GradientEditor* dlg = new GradientEditor(palette);
  if ( dlg->exec() == QDialog::Accepted ) {
    palette = dlg->gradient();
    l->setPalette(palette);

    SatelliteImage* img = l->currentObject();
    if ( nullptr != img ) {
      img->resetCache();
      l->repaint();
    }
  }

  delete dlg;
}

QPixmap SatLayerMenu::createPixmap(Qt::PenStyle style, int width, bool isSelected, QString text /* = QString::null*/)
{
  QFont font;
  QFontMetrics fm(font);
  int textWidth = fm.width("6");
  if (text.isNull()) textWidth = 0;

  QPixmap pixmap(80 + textWidth, 20);
  QColor color = Qt::white;
  if (isSelected) {
    //    color = QColor(175, 215, 240);
    color = QColor(200, 233, 252);
  }
  pixmap.fill(color);
  
  QBrush brush;
  brush.setStyle(Qt::SolidPattern);
  QPen pen(brush, width, style);

  QPainter pntr(&pixmap);
  pntr.setBrush(brush);
  pntr.setPen(pen);
  pntr.drawLine(10, 10, 70, 10);
  if (!text.isNull()) {
    pntr.drawText(QRect(75, 5, textWidth, 10), Qt::AlignCenter, text);
  }

  return pixmap;
}

void SatLayerMenu::changeStylePixmap(QMenu* parent, int val)
{
  if( 0 == parent) {
    return;
  }

  QList<QAction*> allact = parent->actions();
  for (int idx = 0; idx < allact.size(); idx++) {
    QWidgetAction* wa = qobject_cast<QWidgetAction*>(allact.at(idx));
    if( 0 != wa ){
      QLabel* w = qobject_cast<QLabel*>(wa->defaultWidget());
      if (0 != w) {
        if (wa->data().toInt() == val) {
          w->setPixmap(createPixmap(Qt::PenStyle(wa->data().toInt()), 2, true));
        } else {
          w->setPixmap(createPixmap(Qt::PenStyle(wa->data().toInt()), 2, false));
        }
      }
    }
  }
}

void SatLayerMenu::changeWidthPixmap(QMenu* parent, int val)
{ 
  if( 0 == parent) {
    return;
  }
  
  QList<QAction*> allact = parent->actions();
  for (int idx = 0; idx < allact.size(); idx++) {
    QWidgetAction* wa = qobject_cast<QWidgetAction*>(allact.at(idx));
    if( 0 != wa ){
      QLabel* w = qobject_cast<QLabel*>(wa->defaultWidget());
      if (0 != w) {
        if (wa->data().toInt() == val) {
          w->setPixmap(createPixmap(Qt::SolidLine, wa->data().toInt(), true, wa->data().toString()));
        } else {
          w->setPixmap(createPixmap(Qt::SolidLine, wa->data().toInt(), false, wa->data().toString()));
        }
      }
    }
  }
}

} // map
} // meteo
