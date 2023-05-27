#include "layerfigure.h"

#include <qdebug.h>
#include <quuid.h>
#include <qpainter.h>
#include <qelapsedtimer.h>
#include <qguiapplication.h>

#include <cross-commons/debug/tlog.h>
#include <sql/psql/psqlquery.h>
#include <commons/geobasis/projection.h>
#include <meteo/commons/global/global.h>

#include "map.h"
#include "document.h"
#include "object.h"
#include "geopolygon.h"
#include "geotext.h"
#include "weatherfigure.h"
#include <meteo/commons/global/weatherloader.h>
#include "layermenu.h"
#include "puanson.h"

namespace meteo {
namespace map {

namespace {
Layer* createLayer( Document* d )
{
  return new LayerFigure(d);
}
static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerFigure::Type, createLayer );
}

LayerFigure::LayerFigure( Document* d, const tablo::Settings& p )
  : Layer(d)
{
  info_.set_type(meteo::kLayerFigure);
  setPattern(p);

}

LayerFigure::LayerFigure( Document* d )
  : Layer(d)
{
  info_.set_type(meteo::kLayerFigure);
}

LayerFigure::~LayerFigure()
{
}

void LayerFigure::render( QPainter* painter, const QRect& target, Document* document )
{
  if ( false == visible() ) {
      return;
    }
  Document* olddoc = document_;
  document_ = document;
  drawedlabels_.clear();
  float k = 0.01f;
  QMapIterator< tablo::Color, QList<WeatherFigure*> > it(figures_);


  while ( true == it.hasNext() ) {
      it.next();
      tablo::Color clr = it.key();
      if ( tablo::kNoColor == clr ) {
          continue;
        }
      QPainterPath pp;
      const QList<WeatherFigure*> flist = it.value();
      int ap = 0;
      if(0<flist.size()){
          ap = flist.at(0)->alphapercent();
          if ( 0 > ap ) {
              ap = 0;
            }
          if ( 100 < ap ) {
              ap = 100;
            }
        }
      for ( int i = 0, sz = flist.size(); i < sz; ++i ) {
          // painter->save();
          WeatherFigure* o = flist[i];
          o->getPainterPath(&pp, target, document->transform() );
          //o->render( painter, target, document );
          // painter->restore();
        }

      QColor qclr;
      switch ( clr ) {
        case tablo::kGreen:
          qclr = Qt::green;
          break;
        case tablo::kYellow:
          qclr = Qt::yellow;
          break;
        default:
        case tablo::kRed:
          qclr = Qt::red;
          break;
        }

      painter->save();
      painter->setOpacity(1.0 - ap*k);
      painter->setPen(Qt::SolidLine);
      QBrush qbr( qclr, Qt::SolidPattern );
      painter->setBrush(qbr);
      painter->drawPath(pp);
      painter->restore();
    }
  foreach(Object* o, objects_){

 // for ( int i = 0, sz = objects_.size(); i < sz; ++i ) {
//      Object* o = objects_[i];
      if ( kFigure == o->type() ) {
          continue;
        }
      painter->save();
      int ap = o->alphapercent();
      if ( 0 > ap ) {
          ap = 0;
        }
      if ( 100 < ap ) {
          ap = 100;
        }
      painter->setOpacity(1.0 - ap*k);
      o->render( painter, target, document );
      painter->restore();
    }

  document_ = olddoc;
}



bool LayerFigure::addFigure(TMeteoData *md )
{
  bool fl = false;
  GeoPoint gp;
  fl = TMeteoDescriptor::instance()->getCoord( *md, &gp );
  if ( false == fl ) {
      return false;
    }
  meteo::map::WeatherFigure* f = new meteo::map::WeatherFigure(this);
  f->setPattern(pattern_);
  f->setMeteodata(*md);
  f->setSkelet(gp);
  return true;
}

void LayerFigure::setData( const surf::DataReply& data )
{
  clearData();
  for ( int i = 0, sz = data.meteodata_size(); i < sz; ++i ) {
      const std::string& str = data.meteodata(i);
      QByteArray arr(str.data(), str.size() );
      TMeteoData md;
      md << arr;
      addFigure(&md);
    }
}

void LayerFigure::setData( const std::unordered_set<meteo::map::Puanson*>& data )
{
  clearData();
  foreach(const meteo::map::Puanson* puans, data){
      if(nullptr == puans) {
          debug_log << QObject::tr("Данные испорчены");
          return;
        }
      TMeteoData md = puans->meteodata();
      addFigure(&md);
    }
}

void LayerFigure::clearData(){
  std::unordered_set<WeatherFigure*> list = objectsByType<WeatherFigure*>();
  for ( auto o: list ) {
      delete o;
    }
  /* for ( int i = 0, sz = clusters_.keys().size(); i < sz; ++i ) {
    delete clusters_[clusters_.keys().at(i)];
  }*/

}


void LayerFigure::setPattern( const tablo::Settings& p )
{
  // if(meteo::global::isEqual(p,pattern_)) return;
  //debug_log<< "p   "<< p.DebugString();
  //debug_log<< "pattern_   "<< pattern_.DebugString();


  pattern_.CopyFrom(p);
  figures_.clear();

  std::unordered_set<GeoPolygon*> plist = Layer::objectsByType<GeoPolygon*>(true);
  for ( auto o: plist ) {
    delete o;
    }

  std::unordered_set<WeatherFigure*> list = objectsByType<WeatherFigure*>();
  for ( auto o: list ) {
      o->setPattern(pattern_);
      WeatherFigure* f = o;
      figures_[f->color( pattern(), f->meteodata() )].append(f);
    }

  repaint();
}


int32_t LayerFigure::dataSize() const
{
  int32_t pos = Layer::dataSize();
  std::string str;
  if ( true == pattern_.IsInitialized() ) {
      pattern_.SerializeToString( &str );
    }
  auto sz = str.size();
  pos += sizeof(sz);
  pos += sz;
  return pos;
}

int32_t LayerFigure::serializeToArray( char* arr ) const
{
  int32_t pos = Layer::serializeToArray(arr);
  std::string str;
  if ( true == pattern_.IsInitialized() ) {
      pattern_.SerializeToString( &str );
    }
  auto sz = str.size();
  ::memcpy( arr + pos, &sz, sizeof(sz) );
  pos += sizeof(sz);
  ::memcpy( arr + pos, str.data(), sz );
  pos += sz;
  return pos;
}

int32_t LayerFigure::parseFromArray( const char* arr )
{
  int32_t pos = Layer::parseFromArray(arr);
  int32_t sz;
  global::fromByteArray( arr + pos, &sz );
  pos += sizeof(sz);
  if ( 0 != sz ) {
      pattern_.ParseFromArray( arr + pos, sz );
    }
  pos += sz;
  return pos;
}
//
//LayerMenu* LayerFigure::layerMenu()
//{
//  if ( 0 == menu_ ) {
//    menu_ = new PunchMenu(this);
//  }
//  return menu_;
//}

}
}
