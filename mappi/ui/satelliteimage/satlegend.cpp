#include "satlegend.h"

#include <qpainter.h>

#include <meteo/commons/ui/map/incut.h>
#include <meteo/commons/ui/map/document.h>

#include <mappi/ui/satlayer/satlayer.h>

namespace meteo {
namespace map {

SatLegend::SatLegend(Document* doc)
  : Legend(doc)
{
}

bool SatLegend::buildPixmap()
{
  SatLayer* l = maplayer_cast<SatLayer*>(document()->activeLayer());
  if ( 0 == l || !l->visible() ) {
    return false;
  }

  const TColorGradList& palette = l->palette();

  if ( palette.isEmpty() ) { return false; }

  QString header = "Таблица метеорологических параметров: ";
  if ( meteo::kVertical == orient_ && 0 != vlabel_.size() ) {
    header += vlabel_;
  }
  else if ( meteo::kHorizontal == orient_ && 0 != hlabel_.size() ) {
    header += hlabel_;
  }
  else {
//    if ( true == TMeteoDescriptor::instance()->isExist( l->descriptor() ) ) {
//      const meteodescr::Property& prop = TMeteoDescriptor::instance()->property( l->descriptor() );
//      if ( proto::kVertical == orient_ ) {
//        header += TMeteoDescriptor::instance()->name( l->descriptor() );
//        header += QString(", %1").arg( prop.unitsRu );
//      }
//      else {
//        header += QString::fromStdString( color.name() );
//        header += QString(", %1").arg( prop.unitsRu );
//      }
//    }
//    else {
//      header = QObject::tr("Неизв.");
//    }
  }

  QStringList labels;
  QList<QColor> colors;
  for ( int i = 0, sz = palette.size(); i < sz; ++i ) {
    const TColorGrad& gr = palette[i];
    if(gr.title().isEmpty())
    {
    labels.append( QString::number( gr.begval() ) );
    } else
    {
      labels.append(gr.title());
    }
    colors.append( gr.begcolor() );
  }
  bool has_title = true;
  if(palette.last().title().isEmpty()){
  labels.append( QString::number( palette.last().endval() ) );
    has_title = false;
  } else {
    labels.append(  palette.last().title() );
  }

  int maxlblwidth = 0;
  int maxlblheight = 0;
  QImage pix( QSize(1,1), QImage::Format_ARGB32 );
  pix.fill(Qt::white);
  QPainter painter(&pix);
  QRect header_rect = painter.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, header );
  Qt::AlignmentFlag alflag = Qt::AlignHCenter;
  if(has_title) {
    alflag = Qt::AlignRight;
  }


  for ( int i = 0, sz = labels.size(); i < sz; ++i ) {
    QRect r = painter.boundingRect( QRect(), alflag | Qt::AlignVCenter, labels[i] );


    if ( r.width() > maxlblwidth ) {
      maxlblwidth = r.width();
    }
    if ( r.height() > maxlblheight ) {
      maxlblheight = r.height();
    }
  }
  painter.end();
  maxlblwidth += 4;
  if ( meteo::kVertical == orient_ ) {
    if ( header_rect.width() < maxlblwidth + maxlblheight*2+6 ) {
      header_rect.setWidth( maxlblwidth+ maxlblheight*2+6 );
    }
    QImage result( QSize( header_rect.width(), header_rect.height() + maxlblheight*labels.size() ), QImage::Format_ARGB32 );
    result.fill(Qt::white);
    painter.begin(&result);
    header_rect.moveTopLeft( QPoint(0,0) );
    painter.drawText( header_rect, Qt::AlignHCenter | Qt::AlignVCenter, header );
    int height = header_rect.height();
    for ( int i = colors.size() - 1; i > -1; --i ) {
      QRect rect = QRect( QPoint( 0, height ), QSize( maxlblwidth, maxlblheight ) );
      painter.drawText( rect, Qt::AlignRight| Qt::AlignVCenter, labels[i+1] );
      painter.save();
      QColor clr = colors[i];
      QBrush br( Qt::SolidPattern );
      br.setColor(clr);
      painter.setBrush(br);
      rect = QRect( QPoint( maxlblwidth+3, height+maxlblheight/2 ), QSize( maxlblheight*2, maxlblheight ) );
      painter.drawRect(rect);
      painter.restore();
      height += maxlblheight;
    }
    if(!has_title) {
    QRect rect = QRect( QPoint( 0, height ), QSize( maxlblwidth, maxlblheight ) );
    painter.drawText( rect, Qt::AlignRight| Qt::AlignVCenter, labels.first() );
    }
    painter.end();
    label_->setImage(result);
    return true;
  }
  else {
    maxlblwidth += 6;
    if ( header_rect.width() < maxlblwidth*labels.size() + 6 ) {
      header_rect.setWidth( maxlblwidth*labels.size() + 6 );
    }
    QImage result( QSize( header_rect.width(), header_rect.height() + maxlblheight*2 + 4 ), QImage::Format_ARGB32 );
    result.fill(Qt::white);
    painter.begin(&result);
    header_rect.moveTopLeft( QPoint(0,0) );
    painter.drawText( header_rect, Qt::AlignHCenter | Qt::AlignVCenter, header );
    int width = 0;
    for ( int i = 0; i < colors.size(); ++i ) {
      painter.save();
      QColor clr = colors[i];
      QBrush br( Qt::SolidPattern );
      br.setColor(clr);
      painter.setBrush(br);
      QRect rect = QRect( QPoint( maxlblwidth/2 + width, header_rect.height()+2 ), QSize( maxlblwidth, maxlblheight ) );
      painter.drawRect(rect);
      painter.restore();
      rect = QRect( QPoint( width, header_rect.height() + maxlblheight + 4 ), QSize( maxlblwidth, maxlblheight ) );
      painter.drawText( rect, alflag | Qt::AlignVCenter, labels[i] );
      width += maxlblwidth;
    }
    if(!has_title) {
    QRect rect = QRect( QPoint( width, header_rect.height() + maxlblheight + 4 ), QSize( maxlblwidth, maxlblheight ) );
      painter.drawText( rect, alflag | Qt::AlignVCenter, labels.last() );
    }
    painter.end();
    label_->setImage(result);
    return true;
  }

  return false;
}


} // map
} // meteo
