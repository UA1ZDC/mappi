#include "incut.h"

#include <qevent.h>
#include <qsettings.h>
#include <qpainter.h>
#include <qdatetime.h>
#include <quuid.h>
#include <qtextdocument.h>

#include <cross-commons/debug/tlog.h>
#include <cross-commons/funcs/mn_funcs.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/common.h>

#include "document.h"
#include "ramka.h"

namespace meteo {
namespace map {

Incut::Incut( Document* doc )
  : ispixmap_(false),
  uuid_( QUuid::createUuid().toString() ),
  document_(doc),
  label_( new internal::IncutLabel() ),
  position_(kTopLeft),
  visible_(true)
{
  label_->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

  document_->addIncut(this);

  initLabels();
  loadLastParams();
  repaint();
}

Incut::~Incut()
{
//  saveLastParams();
  document_->removeIncut(this);
  delete label_; label_ = nullptr;
  repaint();
}

void Incut::setPositionOnDocument( Position pos )
{
  position_ = pos;
//  saveLastParams();
  repaint();
}

void Incut::setPositionOnDocument( const QPoint& pos )
{
  floatposition_ = pos;
  position_ = kFloat;
//  saveLastParams();
  repaint();
}

void Incut::initLabels()
{
  Podpis map;
  map.key = internal::kMAP_KEY;
  map.align = Qt::AlignHCenter;
  map.visible = true;
  textlabels_.insert( internal::kMAP_KEY, map );
  Podpis wmo;
  wmo.key = internal::kWMO_KEY;
  wmo.align = Qt::AlignHCenter;
  wmo.visible = true;
  textlabels_.insert( internal::kWMO_KEY, wmo );
  Podpis a;
  a.key = internal::kAUTHOR_KEY;
  a.align = Qt::AlignHCenter;
  a.visible = true;
  textlabels_.insert( internal::kAUTHOR_KEY, a );
  Podpis date;
  date.key = internal::kDATE_KEY;
  date.align = Qt::AlignHCenter;
  date.visible = true;
  textlabels_.insert( internal::kDATE_KEY, date );
  Podpis term;
  term.key = internal::kTERM_KEY;
  term.align = Qt::AlignHCenter;
  term.visible = true;
  textlabels_.insert( internal::kTERM_KEY, term );
  Podpis scale;
  scale.key = internal::kSCALE_KEY;
  scale.align = Qt::AlignHCenter;
  scale.visible = true;
  textlabels_.insert( internal::kSCALE_KEY, scale );
  Podpis proj;
  proj.key = internal::kPROJ_KEY;
  proj.align = Qt::AlignHCenter;
  proj.visible = false;
  textlabels_.insert( internal::kPROJ_KEY, proj );
  Podpis baselayer;
  baselayer.key = internal::kBASELAYER_KEY;
  baselayer.align = Qt::AlignHCenter;
  baselayer.visible = false;
  textlabels_.insert( internal::kBASELAYER_KEY, baselayer );
  Podpis layer;
  baselayer.key = internal::kLAYER_KEY;
  baselayer.align = Qt::AlignHCenter;
  baselayer.visible = false;
  textlabels_.insert( internal::kLAYER_KEY, layer );
  order_.append(internal::kMAP_KEY);
  order_.append(internal::kWMO_KEY);
  order_.append(internal::kDATE_KEY);
  order_.append(internal::kTERM_KEY);
  order_.append(internal::kSCALE_KEY);
  order_.append(internal::kPROJ_KEY);
  order_.append(internal::kAUTHOR_KEY);
  order_.append(internal::kBASELAYER_KEY);
  order_.append(internal::kLAYER_KEY);
}

QString Incut::mapName() const
{
  if ( true == textlabels_[internal::kMAP_KEY].value.isEmpty() ) {
    return QString::fromStdString( document_->info().title() );
  }
  return textlabels_[internal::kMAP_KEY].value;
}

QString Incut::WMOHeader() const
{
  if ( true == textlabels_[internal::kWMO_KEY].value.isEmpty() ) {
    return document_->wmoHeader();
  }
  return textlabels_[internal::kWMO_KEY].value;
}

QString Incut::author() const
{
  if ( true == textlabels_[internal::kAUTHOR_KEY].value.isEmpty() ) {
    return QObject::tr("Составил: %1").arg(document_->author() );
  }
  return textlabels_[internal::kAUTHOR_KEY].value;
}

QString Incut::date() const
{
  if ( true == textlabels_[internal::kDATE_KEY].value.isEmpty() ) {
    return document_->dateString();
  }
  return textlabels_[internal::kDATE_KEY].value;
}

QString Incut::term() const
{
  if ( true == textlabels_[internal::kTERM_KEY].value.isEmpty() ) {
    return document_->termString();
  }
  return textlabels_[internal::kTERM_KEY].value;
}

QString Incut::scale() const
{
  return QObject::tr("Масштаб 1:%L1")
    .arg( MnCommon::splitDigits( document_->santimeteresScaleProjCenter(), 3, ' ' ) );
}

QString Incut::projection() const
{
  QString proj;
  switch ( document_->property().projection() ) {
    case kGeneral:
      proj = QObject::tr("Генеральная проекция");
      break;
    case kMercat:
      proj = QObject::tr("Проекция меркатора");
      break;
    case kStereo:
      proj = QObject::tr("Стереографическая проекция");
      break;
    case kConical:
      proj = QObject::tr("Коническая проекция");
      break;
    case kSatellite:
      proj = QObject::tr("Спутниковая проекция");
      break;
  }
  return proj;
}

QString Incut::baseLayer() const
{
  QString str;
  for ( int i = 0, sz = document_->layers().size(); i < sz; ++i ) {
    Layer* l = document_->layers()[i];
    if ( false == l->isBase() ) {
      continue;
    }
    str += l->name() + '\n';
  }
  if ( 0 != str.size() ) {
    str = str.left( str.size() - 1 );
    str.prepend( QObject::tr("Базовые слои:\n") );
  }
  return str;
}

QString Incut::layer() const
{
  QString str;
  for ( int i = 0, sz = document_->layers().size(); i < sz; ++i ) {
    Layer* l = document_->layers()[i];
    if ( true == l->isBase() ) {
      continue;
    }
    str += l->name() + '\n';
  }
  if ( 0 != str.size() ) {
    str = str.left( str.size() - 1 );
    str.prepend( QObject::tr("Слои:\n") );
  }
  return str;
}

void Incut::copyFrom( Incut* i )
{
  if ( nullptr == i) {
    return;
  }
  ispixmap_ = i->ispixmap_;
  position_ = i->position_;
  floatposition_ = i->floatposition_;
  textlabels_ = i->textlabels_;
  order_ = i->order_;
  if ( true == ispixmap_ ) {
    label_->setImage( i->label_->image() );
  }
}

void Incut::repaint() const
{
  if ( nullptr == document_ || nullptr == document_->eventHandler() ) {
    return;
  }
  DocumentEvent* e = new DocumentEvent( DocumentEvent::IncutChanged );
  document_->eventHandler()->postEvent(e);
}

void Incut::setTextAlignment( Qt::Alignment align )
{
  label_->setAlignment(align);
//  saveLastParams();
}

void Incut::setVisible( bool fl )
{
  visible_ = fl;
  repaint();
//  saveLastParams();
}

void Incut::setProto( const proto::IncutParams& params )
{
  for ( int i = 0, sz = params.label_size(); i < sz; ++i ) {
    const proto::IncutLabel& lbl = params.label(i);
    QString k = QString::fromStdString( lbl.key() );
    QString v = QString::fromStdString( lbl.value() );
    bool vis = lbl.visible();
    if ( false == order_.contains(k) ) {
      order_.append(k);
    }
    Podpis& p = textlabels_[k];
    p.key = k;
    p.value = v;
    p.visible = vis;
  }
  ispixmap_ = false;
  position_ = params.pos();
  setVisible( params.visible() );
  
  document_->property_.mutable_incut()->CopyFrom(params);
}


void Incut::resetFromClearProto( const proto::IncutParams& params )
{
  proto::IncutParams new_params;
  new_params.CopyFrom(params);
  for ( int i = 0, sz = new_params.label_size(); i < sz; ++i ) {
    proto::IncutLabel* lbl = new_params.mutable_label(i);
    QString k = QString::fromStdString( lbl->key() );
    QString v = QString::fromStdString( lbl->value() );
    bool vis = lbl->visible();
    if ( false == order_.contains(k) ) {
      order_.append(k);
    }
    Podpis& p = textlabels_[k];
    p.key = k;
    lbl->set_value(p.value.toStdString());
    p.visible = vis;
  }
  ispixmap_ = false;
  position_ = new_params.pos();
  setVisible( new_params.visible() );
  document_->property_.mutable_incut()->CopyFrom(new_params);

}


proto::IncutParams Incut::proto() const
{
  proto::IncutParams params;
  params.set_pos( position() );
  params.set_align( qtalignmentflag2position( label_->alignment() ) );
  params.set_visible( visible_ );
  for ( int i = 0, sz = order_.size(); i < sz; ++i ) {
    const QString& k = order_[i];
    if ( false == textlabels_.contains(k) ) {
      continue;
    }
    const Podpis& p = textlabels_[k];
    proto::IncutLabel* lbl = params.add_label();
    lbl->set_key( p.key.toStdString() );
    lbl->set_value( p.value.toStdString() );
    lbl->set_visible( p.visible );
  }

  return params;
}

proto::IncutParams Incut::getClearProto() const
{
  proto::IncutParams params;
  params.set_pos( position() );
  params.set_align( qtalignmentflag2position( label_->alignment() ) );
  params.set_visible( visible_ );
  for ( int i = 0, sz = order_.size(); i < sz; ++i ) {
    const QString& k = order_[i];
    if ( false == textlabels_.contains(k) ) {
      continue;
    }
    const Podpis& p = textlabels_[k];
    proto::IncutLabel* lbl = params.add_label();
    lbl->set_key( p.key.toStdString() );
    lbl->clear_value();
    lbl->set_visible( p.visible );
  }
  return params;
}

void Incut::loadLastParams()
{
  proto::Document doc = global::lastBlankParams( document_->property().doctype() );
  proto::IncutParams params;
  params.CopyFrom( doc.incut() );

  setProto(params);
  //resetFromClearProto(params);
  repaint();
//  saveLastParams();
}

void Incut::saveLastParams()
{
  //proto::IncutParams params = proto();
  proto::IncutParams params = getClearProto();
  proto::Document doc = global::lastBlankParams( document_->property().doctype() );
  doc.mutable_incut()->CopyFrom(params);
  global::saveLastBlankParams(doc);
}

QRectF Incut::boundingRect() const
{
  if ( false == visible_ ) {
    return QRectF();
  }
  if ( false == ispixmap_ ) {
    QString text = incutText();
    label_->setText(text);
  }

  QPoint pnt;
  int otstup = document_->ramkaWidth() + 5;
  switch ( position_ ) {
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



void Incut::drawOnDocument( QPainter* p )
{
  if ( false == visible_ ) {
    return;
  }
  if ( false == ispixmap_ ) {
    label_->setText(incutText());
  }
  QPoint pnt = boundingRect().topLeft().toPoint();
  label_->draw( pnt, p );
}

QString Incut::incutText() const {

  bool has_autor = false;
  QString text;
  for ( int i = 0, sz = order_.size(); i < sz; ++i ) {
    const QString& k = order_[i];
    if ( false == textlabels_.contains(k) ) {
      error_log << QObject::tr("Не найдена подпись врезки %1").arg(k);
      continue;
    }
    const Podpis& p = textlabels_[k];
    if ( false == p.visible ) {
      continue;
    }
    QString align("center");
    if ( Qt::AlignLeft == p.align ) {
      align = QString("left");
    }
    else if ( Qt::AlignRight == p.align ) {
      align = QString("right");
    }
    QString val;


    if ( internal::kMAP_KEY == k ) {
      val = mapName();
    }
    else if ( internal::kWMO_KEY == k ) {
      val = WMOHeader();
    }
    else if ( internal::kDATE_KEY == k ) {
      val = date();
    }
    else if ( internal::kTERM_KEY == k ) {
      val = term();
    }
    else if ( internal::kSCALE_KEY == k ) {
      val = scale();
    }
    else if ( internal::kPROJ_KEY == k ) {
      val = projection();
    }
    else if ( internal::kBASELAYER_KEY == k ) {
      val = baseLayer();
    }
    else if ( internal::kLAYER_KEY== k ) {
      val = layer();
    }
    else if ( internal::kAUTHOR_KEY == k ) {
        has_autor = true;
    }
    else {
      val = p.value;
    }
    if ( true == val.isEmpty() ) {
      continue;
    }
    QStringList vallist = val.split('\n');
    for ( int j = 0, jsz = vallist.size(); j < jsz; ++j ) {
      text += QString("<div align='center'>%1</div>").arg(vallist[j]);
    }
  }

  if(true == has_autor&& false == author().isEmpty() ){
      text += QString("<div align='center'>%1</div>").arg(author());
    }
  return text;
}

void Incut::setImage( const QImage& img )
{
  ispixmap_ = true;
  if ( nullptr != label_ ) {
    label_->setImage(img);
  }
  repaint();
}

void Incut::setLabel( const QString& key, const QString& value )
{
  Podpis& p = textlabels_[key];
  p.key = key;
  p.value = value;
  p.visible = true;
  ispixmap_ = false;
  if ( false == order_.contains(key) ) {
    order_.append(key);
  }
  repaint();
//  saveLastParams();
}

bool Incut::labelVisible( const QString& key ) const
{
  if ( false == textlabels_.contains(key) ) {
    return false;
  }
  return textlabels_[key].visible;
}

bool Incut::mapVisible() const
{
  return labelVisible(internal::kMAP_KEY);
}

bool Incut::wmoVisible() const
{
  return labelVisible(internal::kWMO_KEY);
}

bool Incut::authorVisible() const
{
  return labelVisible(internal::kAUTHOR_KEY);
}

bool Incut::dateVisible() const
{
  return labelVisible(internal::kDATE_KEY);
}

bool Incut::termVisible() const
{
  return labelVisible(internal::kTERM_KEY);
}

bool Incut::scaleVisible() const
{
  return labelVisible(internal::kSCALE_KEY);
}

bool Incut::projectionVisible() const
{
  return labelVisible(internal::kPROJ_KEY);
}

bool Incut::baseLayerVisible() const
{
  return labelVisible(internal::kBASELAYER_KEY);
}

bool Incut::layerVisible() const
{
  return labelVisible(internal::kLAYER_KEY);
}

void Incut::setMapName( bool fl )
{
  Podpis& p = textlabels_[internal::kMAP_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setMapName( const QString& name )
{
  Podpis& p = textlabels_[internal::kMAP_KEY];
  p.visible = true;
  p.value = name;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setWMOHeader( bool fl )
{
  Podpis& p = textlabels_[internal::kWMO_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setWMOHeader( const QString& val )
{
  Podpis& p = textlabels_[internal::kWMO_KEY];
  p.visible = true;
  p.value = val;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setAuthor( bool fl )
{
  Podpis& p = textlabels_[internal::kAUTHOR_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setAuthor( const QString& val )
{
  Podpis& p = textlabels_[internal::kAUTHOR_KEY];
  p.visible = true;
  p.value = val;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setDate( bool fl )
{
  Podpis& p = textlabels_[internal::kDATE_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setDate( const QString& val )
{
  Podpis& p = textlabels_[internal::kDATE_KEY];
  p.visible = true;
  p.value = val;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setTerm( bool fl )
{
  Podpis& p = textlabels_[internal::kTERM_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setTerm( const QString& val )
{
  Podpis& p = textlabels_[internal::kTERM_KEY];
  p.visible = true;
  p.value = val;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setScale( bool fl )
{
  Podpis& p = textlabels_[internal::kSCALE_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setProjection( bool fl )
{
  Podpis& p = textlabels_[internal::kPROJ_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setBaseLayerVisible( bool fl )
{
  Podpis& p = textlabels_[internal::kBASELAYER_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setLayerVisible( bool fl )
{
  Podpis& p = textlabels_[internal::kLAYER_KEY];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

void Incut::setAllLabelVisible(bool v){

  QMap< QString, Podpis >::iterator it = textlabels_.begin();
  QMap< QString, Podpis >::iterator eit = textlabels_.end();
  for ( ; it != eit; ++it ) {
    it.value().visible = v;
    }
  repaint();
}

void Incut::setLabel( const QString& key, bool fl )
{
  Podpis& p = textlabels_[key];
  p.visible = fl;
  ispixmap_ = false;
  repaint();
//  saveLastParams();
}

namespace internal {

IncutLabel::IncutLabel()
{
  if ( false == global::createMapSettingsDir() ) {
    error_log << QObject::tr("Не удалось создать директорию настроек. Свойства картографической врезки невозможно загрузить");
  }
}

IncutLabel::~IncutLabel()
{
}

void IncutLabel::setImage( const QImage& img )
{
  image_ = img.copy();
}

void IncutLabel::draw( const QPoint& topleft, QPainter* pntr )
{
  pntr->drawImage( topleft, image_ );
  pntr->save();
  pntr->setPen( QPen( kMAP_RAMKAPEN_DEFAULT ) );
  pntr->setBrush( QBrush( QColor( 0, 0, 0, 0 ) ) );
  pntr->drawRect( QRect( topleft, image_.size() ) );
  pntr->restore();
}

void IncutLabel::setText( const QString& text )
{
  QTextDocument td;
  td.setHtml(text);
  td.adjustSize();
  QImage img( td.size().toSize(), QImage::Format_ARGB32 );
  img.fill( Qt::white );
  QPainter pntr(&img);
  td.drawContents(&pntr);
  setImage(img);
}


}
}
}
