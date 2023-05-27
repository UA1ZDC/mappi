#include "legend.h"

#include <qevent.h>
#include <qsettings.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <quuid.h>

#include <commons/meteo_data/tmeteodescr.h>
#include <commons/obanal/tfield.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/map_isoline.pb.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/global/common.h>
#include <meteo/commons/punchrules/punchrules.h>

#include "layer.h"
#include "layeriso.h"
#include "layermrl.h"
#include "document.h"
#include "ramka.h"
#include "incut.h"
#include <meteo/commons/global/gradientparams.h>
#include <meteo/commons/global/radarparams.h>
#include <meteo/commons/global/weatherloader.h>
#include "puanson.h"
#include "geogradient.h"

namespace meteo {
namespace map {

Legend::Legend( Document* doc )
  : uuid_( QUuid::createUuid().toString() ),
  document_( doc ),
  label_( new internal::IncutLabel() ),
  position_(kBottomCenter),
  visible_(true),
  custom_(false),
  orient_( kHorizontal )
{
  label_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
  loadLastParams();
//  repaint();
}

Legend::~Legend()
{
//  saveLastParams();
//  document_->removeLegend(this);
  delete label_; label_ = 0;
  repaint();
}

void Legend::repaint() const
{
  if ( 0 == document_ || 0 == document_->eventHandler() ) {
    return;
  }
  for ( auto l : document_->layers() ) {
    if ( true == l->hasLegendImage() ) {
      l->setLegendImage( QImage() );
    }
  }
  DocumentEvent* e = new DocumentEvent( DocumentEvent::LegendChanged );
  document_->eventHandler()->postEvent(e);
}

void Legend::setProto( const proto::LegendParams& params )
{
  position_ = params.pos();
  orient_ = params.orient();
  document_->property_.mutable_legend()->CopyFrom(params);
  repaint();
//  saveLastParams();
}

proto::LegendParams Legend::proto() const
{
  proto::LegendParams params;
  params.set_pos( position() );
  params.set_orient( orient() );
  return params;
}

Layer* Legend::legendLayer( Document* doc )
{
  Layer* l = doc->activeLayer();
  if ( 0 != l && true == l->visible() ) {
    switch ( l->type() ) {
      case kLayerIso: {
          LayerIso* iso = maplayer_cast<LayerIso*>(l);
          if ( 0 == iso || false == iso->hasGradient() || false == iso->gradientVisible() ) {
            l = 0;
          }
        }
        break;
      case kLayerMrl:
        if ( 0 == maplayer_cast<LayerMrl*>(l) ) {
          l = 0;
        }
        break;
      default:
        l = 0;
        break;
    }
  }
  else {
    l = 0;
  }

  if ( 0 != l ) {
    return l;
  }

  QListIterator<Layer*> it( doc->layers() );
  it.toBack();
  while ( true == it.hasPrevious() ) {
    l = it.previous();
    if ( false == l->visible() ) {
      continue;
    }
    switch ( l->type() ) {
      case kLayerIso: {
          LayerIso* iso = maplayer_cast<LayerIso*>(l);
          if ( 0 == iso || false == iso->hasGradient() || false == iso->gradientVisible() ) {
            l = 0;
          }
        }
        break;
      case kLayerMrl:
        if ( 0 == maplayer_cast<LayerMrl*>(l) ) {
          l = 0;
        }
        break;
      default:
        l = 0;
        break;
    }
    if ( 0 != l ) {
      break;
    }
  }
  return l;
}

void Legend::setVisible( bool fl )
{
  visible_ = fl;
  repaint();
}

void Legend::loadLastParams()
{
  proto::Document doc = global::lastBlankParams( document_->property().doctype() );
  proto::LegendParams params;
  params.CopyFrom( doc.legend() );
  setProto(params);
}

void Legend::saveLastParams()
{
  proto::LegendParams params = proto();
  proto::Document doc = global::lastBlankParams( document_->property().doctype() );
  doc.mutable_legend()->CopyFrom(params);
  doc.mutable_legend()->CopyFrom(params);
  global::saveLastBlankParams(doc);
}

QRectF Legend::boundingRect() const
{
  if ( false == visible_ ) {
    return QRectF();
  }
  QPoint pnt;
  int otstup = document_->ramkaWidth() + 5;
  switch ( position_ ) {
    case kNoPosition:
      return QRectF();
      break;
    case kTopLeft:
      pnt = document_->documentTopLeft() + QPoint(otstup,otstup);
      break;
    case kTopRight:
      pnt = document_->documentTopRight() - QPoint( label_->width(), 0 ) + QPoint(-otstup,otstup);
      break;
    case kBottomLeft:
      pnt = document_->documentBottomLeft() - QPoint( 0, label_->height() ) + QPoint(otstup,-otstup);
      break;
    case kBottomRight:
      pnt = document_->documentBottomRight() - QPoint( label_->width(), label_->height() ) + QPoint(-otstup,-otstup);
      break;
    case kBottomCenter:
      pnt = document_->documentBottom() - QPoint( label_->width()/2, label_->height() ) + QPoint(0,-otstup);
      break;
    case kTopCenter:
      pnt = document_->documentTop() - QPoint( label_->width()/2, 0 ) + QPoint(0,otstup);
      break;
    case kLeftCenter:
      pnt = document_->documentLeft() - QPoint( 0, label_->height()/2 ) + QPoint(otstup,0);
      break;
    case kRightCenter:
      pnt = document_->documentRight() - QPoint( label_->width(), label_->height()/2 ) + QPoint(-otstup,0);
      break;
    case kFloat:
      return QRectF( floatposition_, label_->size() );
      break;
    default:
      pnt = document_->documentTop() - QPoint( label_->width()/2, 0 ) + QPoint(0,otstup);
      break;
  }
  return QRectF( pnt, label_->size() );
}

void Legend::drawOnDocument( QPainter* p )
{
  if ( kNoPosition == position_ ) {
    return;
  }
  if ( false == visible_ ) {
    return;
  }
  if ( false == custom_ ) {
    if ( false == buildPixmap() ) {
      return;
    }
  }
  QPoint pnt = boundingRect().topLeft().toPoint();
  label_->draw( pnt, p );
}

void Legend::setImage( const QImage& pixmap )
{
  if ( 0 != label_ ) {
    label_->setImage(pixmap);
  }
  custom_ = true;
  repaint();
}

bool Legend::buildPixmap()
{
  if ( true == custom_ ) {
    return false;
  }

  Layer* l = legendLayer(document_);
  if ( 0 != l && true == l->visible() ) {
    if ( true == l->hasLegendImage() ) {
      return buildByReadyImage(l);
    }
    switch ( l->type() ) {
      case kLayerIso:
        if ( true == buildByField(l) ) {
          l->setLegendImage( label_->image() );
          return true;
        }
        break;
      case kLayerMrl:
        if ( true == buildByMrl(l) ) {
          l->setLegendImage( label_->image() );
          return true;
        }
        break;
    }
  }
  return false;
}

bool Legend::buildByReadyImage( Layer* l )
{
  label_->setImage( l->legendImage() );
  return true;
}

bool Legend::buildByField( Layer* layer )
{
  LayerIso* l = maplayer_cast<LayerIso*>(layer);
  if ( 0 == l ) {
    error_log << QObject::tr("Тип слоя - kLayerIso. Но сам слой не LayerIso. Легенда не будет построена");
    return false;
  }
  const proto::FieldColor& color = l->fieldColor();
  if ( false == l->hasGradient() ) {
    return false;
  }
  if ( false == l->gradientVisible() ) {
    return false;
  }
  TColorGradList gradient = GradientParams::gradParams( l->level(), l->typeLevel(), color );
  if ( 0 == gradient.size() ) {
    return false;
  }
  QString header;
  if ( kVertical == orient_ && 0 != vlabel_.size() ) {
    header += vlabel_;
  }
  else if ( kHorizontal == orient_ && 0 != hlabel_.size() ) {
    header += hlabel_;
  }
  else {
    if ( true == TMeteoDescriptor::instance()->isExist( l->descriptor() ) ) {
      const meteodescr::Property& prop = TMeteoDescriptor::instance()->property( l->descriptor() );
      if ( kVertical == orient_ ) {
        header += TMeteoDescriptor::instance()->name( l->descriptor() );
        header += QString(", %1").arg( prop.unitsRu );
      }
      else {
        header += QString::fromStdString( color.name() );
        header += QString(", %1").arg( prop.unitsRu );
      }
    }
    else {
      header = QObject::tr("Неизв.");
    }
  }
  QStringList labels;
  QList<QColor> colors;
  auto minval = l->field()->min_field_value();
  auto maxval = l->field()->max_field_value();
  for ( int i = 0, sz = gradient.size(); i < sz; ++i ) {
    const TColorGrad& gr = gradient[i];
    if ( ( minval > gr.begval() && minval > gr.endval() )
      || ( maxval < gr.begval() && maxval < gr.endval() ) ) {
      continue;
    }
    labels.append( QString::number( gr.begval() ) );
    colors.append( gr.begcolor() );
  }
  labels.append( QString::number( gradient.last().endval() ) );
  int maxlblwidth = 0;
  int maxlblheight = 0;
  QImage pix( QSize(1,1), QImage::Format_ARGB32 );
  pix.fill(Qt::white);
  QPainter pntr(&pix);
  QRect header_rect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, header );
  for ( int i = 0, sz = labels.size(); i < sz; ++i ) {
    QRect r = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, labels[i] );
    if ( r.width() > maxlblwidth ) {
      maxlblwidth = r.width();
    }
    if ( r.height() > maxlblheight ) {
      maxlblheight = r.height();
    }
  }
  pntr.end();
  maxlblwidth += 4;
  if ( kVertical == orient_ ) {
    if ( header_rect.width() < maxlblwidth + maxlblheight*2+6 ) {
      header_rect.setWidth( maxlblwidth+ maxlblheight*2+6 );
    }
    QImage result( QSize( header_rect.width(), header_rect.height() + maxlblheight*labels.size() ), QImage::Format_ARGB32 );
    result.fill(Qt::white);
    pntr.begin(&result);
    header_rect.moveTopLeft( QPoint(0,0) );
    pntr.drawText( header_rect, Qt::AlignHCenter | Qt::AlignVCenter, header );
    int height = header_rect.height();
    for ( int i = colors.size() - 1; i > -1; --i ) {
      QRect rect = QRect( QPoint( 0, height ), QSize( maxlblwidth, maxlblheight ) );
      pntr.drawText( rect, Qt::AlignRight| Qt::AlignVCenter, labels[i+1] );
      pntr.save();
      QColor clr = colors[i];
      QBrush br( Qt::SolidPattern );
      br.setColor(clr);
      pntr.setBrush(br);
      rect = QRect( QPoint( maxlblwidth+3, height+maxlblheight/2 ), QSize( maxlblheight*2, maxlblheight ) );
      pntr.drawRect(rect);
      pntr.restore();
      height += maxlblheight;
    }
    QRect rect = QRect( QPoint( 0, height ), QSize( maxlblwidth, maxlblheight ) );
    pntr.drawText( rect, Qt::AlignRight| Qt::AlignVCenter, labels.first() );
    pntr.end();
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
    pntr.begin(&result);
    header_rect.moveTopLeft( QPoint(0,0) );
    pntr.drawText( header_rect, Qt::AlignHCenter | Qt::AlignVCenter, header );
    int width = 0;
    for ( int i = 0; i < colors.size(); ++i ) {
      pntr.save();
      QColor clr = colors[i];
      QBrush br( Qt::SolidPattern );
      br.setColor(clr);
      pntr.setBrush(br);
      QRect rect = QRect( QPoint( maxlblwidth/2 + width, header_rect.height()+2 ), QSize( maxlblwidth, maxlblheight ) );
      pntr.drawRect(rect);
      pntr.restore();
      rect = QRect( QPoint( width, header_rect.height() + maxlblheight + 4 ), QSize( maxlblwidth, maxlblheight ) );
      pntr.drawText( rect, Qt::AlignHCenter | Qt::AlignVCenter, labels[i] );
      width += maxlblwidth;
    }
    QRect rect = QRect( QPoint( width, header_rect.height() + maxlblheight + 4 ), QSize( maxlblwidth, maxlblheight ) );
    pntr.drawText( rect, Qt::AlignHCenter | Qt::AlignVCenter, labels.last() );
    pntr.end();
    label_->setImage(result);
    return true;
  }
  return false;
}

QSize Legend::calcPunchSize( const puanson::proto::CellRule& rule ) const
{
  QSize sz(0,0);
  QImage pix(QSize(1,1), QImage::Format_ARGB32);
  pix.fill( QColor(0,0,0,0) );
  QPainter pntr(&pix);
  const puanson::proto::CellRule::SymbolRule& symbol = rule.symbol();
  QPoint topleft(0,0);
  int maxval_w = 0;
  int maxznk_w = 0;
  int maxval_h = 0;
  int maxznk_h = 0;
  for ( int i = 0, sz = symbol.symbol_size(); i < sz; ++i ) {
    const puanson::proto::CellRule::Symbol& smb = symbol.symbol(i);
    QString str = stringFromRuleValue( smb.minval(), symbol );
    QFont fnt = fontFromRuleValue( smb.minval(), symbol );
    QColor qclr =  colorFromRuleValue( smb.minval(), symbol );
    QString valstr;
    if ( false == smb.has_label() ) { 
      valstr = QObject::tr(" - %1").arg( smb.minval() );
      if ( false == MnMath::isEqual( smb.minval(), smb.maxval() ) ) {
          valstr += QObject::tr(":%1").arg( smb.maxval() );
      }
    }
    else {
      valstr = QObject::tr(" - %1").arg( QString::fromStdString( smb.label() ) );
    }

    QRect valrect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, valstr );
    pntr.save();
    pntr.setFont(fnt);
    pntr.setPen(qclr);
    QRect znakrect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, str );
    pntr.restore();
    if ( maxval_w < valrect.width() ) {
      maxval_w = valrect.width();
    }
    if ( maxval_h < valrect.height() ) {
      maxval_h = valrect.height();
    }
    if ( maxznk_w < znakrect.width() ) {
      maxznk_w = znakrect.width();
    }
    if ( maxznk_h < znakrect.height() ) {
      maxznk_h = znakrect.height();
    }
  }
  int maxh = maxznk_h;
  if ( maxval_h > maxh ) {
    maxh = maxval_h;
  }
  if ( kHorizontal == orient() ) {
    sz = QSize( (maxznk_w+maxval_w)*(symbol.symbol_size()/4+1), maxh*4 );
  }
  else {
    sz = QSize( maxznk_w + maxval_w, maxh*symbol.symbol_size() );
  }
  return sz;
}

bool Legend::buildPunchLegend( const puanson::proto::CellRule& rule, const proto::RadarColor& clr )
{
  TColorGradList gradient = RadarParams::gradParams(clr);
  QSize pixsz = calcPunchSize(rule);
  QImage pix(pixsz, QImage::Format_ARGB32 );
  pix.fill( Qt::white );
  QPainter pntr(&pix);
  const puanson::proto::CellRule::SymbolRule& symbol = rule.symbol();
  QPoint topleft(0,0);
  int maxval_w = 0;
  int maxznk_w = 0;
  int maxval_h = 0;
  int maxznk_h = 0;
  for ( int i = 0, sz = symbol.symbol_size(); i < sz; ++i ) {
    const puanson::proto::CellRule::Symbol& smb = symbol.symbol(i);
    QString str = stringFromRuleValue( smb.minval(), symbol );
    QFont fnt = fontFromRuleValue( smb.minval(), symbol );
    QColor qclr =  colorFromRuleValue( smb.minval(), symbol );
    QString valstr;
    bool hasMeta = false;
    QString patternName;
    meteo::puanson::proto::Puanson puansonProto;
    Puanson puansonGr;
    if ( false == smb.has_label() ) { 
      valstr = QObject::tr(" - %1").arg( smb.minval() );
      if ( false == MnMath::isEqual( smb.minval(), smb.maxval() ) ) {
        valstr += QObject::tr(":%1").arg( smb.maxval() );
      }
    }
    else {
      valstr = QObject::tr(" - %1").arg( QString::fromStdString( smb.label() ) );
      QRegExp exp("*@*@*");
      exp.setPatternSyntax(QRegExp::Wildcard);
      if ( exp.exactMatch(valstr) ) {
        hasMeta = true;
        exp.setPattern("@*@");
        exp.indexIn(valstr);
        QStringList list = exp.capturedTexts();
        patternName = list.first();
        patternName.remove('@');
        valstr.replace(exp,"@");
        meteo::map::internal::WeatherLoader* wl = meteo::map::WeatherLoader::instance();
        if ( nullptr != wl ) {
          wl->loadPunchLibrary();
          if (wl->punchlibrary().contains(patternName)) {
            puansonProto = wl->punchlibrary().value(patternName);
            puansonGr.setPunch(puansonProto);
          }
          else {
            error_log << QObject::tr("Шаблон %1 не обнаружен").arg(patternName);
            hasMeta = false;
          }
        }
        else {
          error_log << QObject::tr("Не обнаружен загрузчик пуансонов");
        }
      }
    }

    QRect valrect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, valstr );
    if (hasMeta) {
      QSize size = valrect.size();
      QSize metaSize = puansonGr.boundingRect().size();
      size.setWidth(size.width() + metaSize.width() );
      int height = ( size.height() > metaSize.height() ) ? size.height() : metaSize.height();
      size.setHeight(height);
      valrect.setSize(size);
    }
    pntr.save();
    pntr.setFont(fnt);
    pntr.setPen(qclr);
    QRect znakrect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, str );
    pntr.restore();
    if ( maxval_w < valrect.width() ) {
      maxval_w = valrect.width();
    }
    if ( maxval_h < valrect.height() ) {
      maxval_h = valrect.height();
    }
    if ( maxznk_w < znakrect.width() ) {
      maxznk_w = znakrect.width();
    }
    if ( maxznk_h < znakrect.height() ) {
      maxznk_h = znakrect.height();
    }
  }
  int maxh = maxznk_h;
  if ( maxval_h > maxh ) {
    maxh = maxval_h;
  }
  QRect znakrect = QRect( 0, 0, maxznk_w, maxh );
  QRect valrect = QRect( 0, 0, maxval_w, maxh );
  for ( int i = 0, sz = symbol.symbol_size(); i < sz; ++i ) {
    QString patternName;
    bool hasMeta = false;
    meteo::puanson::proto::Puanson puansonProto;
    Puanson puansonGr;
    const puanson::proto::CellRule::Symbol& smb = symbol.symbol(i);
    QString str = stringFromRuleValue( smb.minval(), symbol );
    QFont fnt = fontFromRuleValue( smb.minval(), symbol );
    QColor qclr =  colorFromRuleValue( smb.minval(), symbol );
    QString valstr;
    if ( false == smb.has_label() ) { 
      valstr = QObject::tr(" - %1").arg( smb.minval() );
      if ( false == MnMath::isEqual( smb.minval(), smb.maxval() ) ) {
          valstr += QObject::tr(":%1").arg( smb.maxval() );
      }
    }
    else {
      valstr = QObject::tr(" - %1").arg( QString::fromStdString( smb.label() ) );
      QRegExp exp("*@*@*");
      exp.setPatternSyntax(QRegExp::Wildcard);
      if ( exp.exactMatch(valstr) ) {
        hasMeta = true;
        exp.setPattern("@*@");
        exp.indexIn(valstr);
        QStringList list = exp.capturedTexts();
        patternName = list.first();
        patternName.remove('@');
        valstr.replace(exp,"");
        meteo::map::internal::WeatherLoader* wl = meteo::map::WeatherLoader::instance();
        if ( nullptr != wl ) {
          wl->loadPunchLibrary();
          if (wl->punchlibrary().contains(patternName)) {
            puansonProto = wl->punchlibrary().value(patternName);
            puansonGr.setPunch(puansonProto);
            puansonGr.loadTestMeteoData();
          }
          else {
            error_log << QObject::tr("Шаблон %1 не обнаружен").arg(patternName);
            hasMeta = false;
          }
        }
        else {
          error_log << QObject::tr("Не обнаружен загрузчик пуансонов");
        }
      }
    }
    int hgt = topleft.y() + maxh;
    if ( hgt > pixsz.height() ) {
      topleft = QPoint( topleft.x() + maxval_w + maxznk_w, 0 );
    }
    pntr.save();
    pntr.setFont(fnt);
    pntr.setPen(qclr);
    znakrect.moveTo( topleft + QPoint( 0, 0 ) );
    pntr.save();
    pntr.setBrush( QBrush( gradient.color( smb.minval() ) ) );
    pntr.setPen( Qt::NoPen );
    pntr.drawRect( znakrect );
    pntr.restore();
    pntr.drawText( znakrect, Qt::AlignHCenter | Qt::AlignBottom, str );
    pntr.restore();
    valrect.moveTo( topleft + QPoint( maxznk_w, 0 ) );
    pntr.drawText( valrect, Qt::AlignLeft | Qt::AlignBottom, valstr );
    if ( hasMeta ) {
      QRect strRect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, valstr );
      puansonGr.setScreenPos(QPoint( 0, 0 ) );
      QRect rectRend = puansonGr.boundingRect();
      pntr.save();
      QPoint diff = QPoint(0,0)- rectRend.topLeft();
      int transX = diff.x() + topleft.x()+strRect.width()+QPoint( maxznk_w, 0 ).x();
      int transY = diff.y() + topleft.y() + 3;
      pntr.translate( transX, transY );
      puansonGr.render(&pntr, rectRend, QTransform());
      pntr.restore();
    }
    pntr.save();
    pntr.setPen(Qt::gray);
    pntr.restore();
    topleft += QPoint( 0, maxh );
  }
  label_->setImage(pix);
  return true;
}

bool Legend::buildByMrl( Layer* layer )
{
  LayerMrl* l = maplayer_cast<LayerMrl*>(layer);
  if ( 0 == l ) {
    error_log << QObject::tr("Тип слоя - kLayerMrl. Но сам слой не LayerMrl. Легенда не будет построена");
    return false;
  }
  const proto::RadarColor& color = l->radarColor();
  bool haspunch = false;
  puanson::proto::Puanson punch;
  puanson::proto::CellRule rule;
  if ( true == color.has_punch_id() ) {
    QString id = QString::fromStdString( color.punch_id() );
    if ( true == WeatherLoader::instance()->punchlibraryspecial().contains(id) ) {
      punch.CopyFrom(WeatherLoader::instance()->punchlibraryspecial()[id]);
      if ( 0 != punch.rule_size() ) {
        rule.CopyFrom( punch.rule(0) );
        haspunch = true;
      }
    }
  }
  if ( true == haspunch ) {
    return buildPunchLegend( rule, color );
  }
  TColorGradList gradient = RadarParams::gradParams( color );
  if ( 0 == gradient.size() ) {
    return false;
  }
  QString header;
  if ( kVertical == orient_ && 0 != vlabel_.size() ) {
    header += vlabel_;
  }
  else if ( kHorizontal == orient_ && 0 != hlabel_.size() ) {
    header += hlabel_;
  }
  else {
    if ( true == TMeteoDescriptor::instance()->isExist( l->descriptor() ) ) {
      const meteodescr::Property& prop = TMeteoDescriptor::instance()->property( l->descriptor() );
      if ( kVertical == orient_ ) {
        header += TMeteoDescriptor::instance()->name( l->descriptor() );
        header += QString(", %1").arg( prop.unitsRu );
      }
      else {
        header += QString::fromStdString( color.name() );
        header += QString(", %1").arg( prop.unitsRu );
      }
    }
    else {
      header = QObject::tr("Неизв.");
    }
  }
  QStringList labels;
  QList<QColor> colors;
  for ( int i = 0, sz = gradient.size(); i < sz; ++i ) {
    const TColorGrad& gr = gradient[i];
    labels.append( QString::number( gr.begval() ) );
    colors.append( gr.begcolor() );
  }
  labels.append( QString::number( gradient.last().endval() ) );
  for ( int i = 0, sz = labels.size(); i < sz; ++i ) {
  }
  int maxlblwidth = 0;
  int maxlblheight = 0;
  QImage pix( QSize(1,1), QImage::Format_ARGB32 );
  pix.fill(Qt::white);
  QPainter pntr(&pix);
  QRect header_rect = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, header );
  for ( int i = 0, sz = labels.size(); i < sz; ++i ) {
    QRect r = pntr.boundingRect( QRect(), Qt::AlignHCenter | Qt::AlignVCenter, labels[i] );
    if ( r.width() > maxlblwidth ) {
      maxlblwidth = r.width();
    }
    if ( r.height() > maxlblheight ) {
      maxlblheight = r.height();
    }
  }
  pntr.end();
  maxlblwidth += 4;
  if ( kVertical == orient_ ) {
    if ( header_rect.width() < maxlblwidth + maxlblheight*2+6 ) {
      header_rect.setWidth( maxlblwidth+ maxlblheight*2+6 );
    }
    QImage result( QSize(header_rect.width(), header_rect.height() + maxlblheight*labels.size() ), QImage::Format_ARGB32 );
    result.fill(Qt::white);
    pntr.begin(&result);
    header_rect.moveTopLeft( QPoint(0,0) );
    pntr.drawText( header_rect, Qt::AlignHCenter | Qt::AlignVCenter, header );
    int height = header_rect.height();
    for ( int i = colors.size() - 1; i > -1; --i ) {
      QString valbl =  labels[i+1];
      QRect rect = QRect( QPoint( 0, height ), QSize( maxlblwidth, maxlblheight ) );
        pntr.drawText( rect, Qt::AlignRight| Qt::AlignVCenter, valbl );
      pntr.save();
      QColor clr = colors[i];
      QBrush br( Qt::SolidPattern );
      br.setColor(clr);
      pntr.setBrush(br);
      rect = QRect( QPoint( maxlblwidth+3, height+maxlblheight/2 ), QSize( maxlblheight*2, maxlblheight ) );
      pntr.drawRect(rect);
      pntr.restore();
      height += maxlblheight;
    }
    QRect rect = QRect( QPoint( 0, height ), QSize( maxlblwidth, maxlblheight ) );
    QString valbl = labels.first();
      pntr.drawText( rect, Qt::AlignRight| Qt::AlignVCenter, valbl );
    pntr.end();
    label_->setImage(result);
    return true;
  }
  else {
    maxlblwidth += 6;
    if ( header_rect.width() < maxlblwidth*labels.size() + 6 ) {
      header_rect.setWidth( maxlblwidth*labels.size() + 6 );
    }
    QImage result( QSize(header_rect.width(), header_rect.height() + maxlblheight*2 + 4), QImage::Format_ARGB32 );
    result.fill(Qt::white);
    pntr.begin(&result);
    header_rect.moveTopLeft( QPoint(0,0) );
    pntr.drawText( header_rect, Qt::AlignHCenter | Qt::AlignVCenter, header );
    int width = 0;
    for ( int i = 0; i < colors.size(); ++i ) {
      pntr.save();
      QColor clr = colors[i];
      QBrush br( Qt::SolidPattern );
      br.setColor(clr);
      pntr.setBrush(br);
      QRect rect = QRect( QPoint( maxlblwidth/2 + width, header_rect.height()+2 ), QSize( maxlblwidth, maxlblheight ) );
      pntr.drawRect(rect);
      pntr.restore();
      rect = QRect( QPoint( width, header_rect.height() + maxlblheight + 4 ), QSize( maxlblwidth, maxlblheight ) );
      pntr.drawText( rect, Qt::AlignHCenter | Qt::AlignVCenter, labels[i] );
      width += maxlblwidth;
    }
    QRect rect = QRect( QPoint( width, header_rect.height() + maxlblheight + 4 ), QSize( maxlblwidth, maxlblheight ) );
    pntr.drawText( rect, Qt::AlignHCenter | Qt::AlignVCenter, labels.last() );
    pntr.end();
    label_->setImage(result);
    return true;
  }
  return false;
  Q_UNUSED(layer);
  return false;
}

}
}
