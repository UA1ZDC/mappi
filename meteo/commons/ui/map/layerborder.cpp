#include "layerborder.h"
#include "layermenu.h"
#include <meteo/commons/global/global.h>


namespace meteo {
namespace map {

namespace {
  Layer* createLayer( Document* d )
  {
    return new LayerBorder(d);
  }
  static const bool res = singleton::PtkppFormat::instance()->registerLayerHandler( LayerBorder::Type, createLayer );
}

LayerBorder::LayerBorder(Document* map) : Layer (map)
{

}

LayerMenu* LayerBorder::layerMenu()
{
  if ( nullptr == menu_ ) {
    menu_ = new BorderMenu(this);
  }
  return menu_;
}

void LayerBorder::changeColor(QColor color)
{
  meteo::Property prop;
  for ( auto object : objects() ) {
    if ( object->type() == PrimitiveTypes::kGroup) {
      for ( auto child : object->childs() ) {
        if (child->type() == PrimitiveTypes::kPolygon) {
          prop = child->property();
          prop.mutable_pen()->set_color(color.rgba());
          prop.mutable_brush()->set_color(color.rgba());
          child->setProperty(prop);
        }
      }
    }
    else if (object->type() == PrimitiveTypes::kPolygon) {
      prop = object->property();
      prop.mutable_pen()->set_color(color.rgba());
      prop.mutable_brush()->set_color(color.rgba());
      object->setProperty(prop);
    }
  }
  if ( prop.pen().has_color() ) {
    proto::Document doc = global::lastBlankParams(meteo::map::proto::DocumentType::kGeoMap);
    doc.mutable_border_property()->CopyFrom(prop);
    global::saveLastBlankParams(doc);
  }
  repaint();
}

void LayerBorder::changeWidth(int value)
{
  meteo::Property prop;
  for ( auto object : objects() ) {
    if ( object->type() == PrimitiveTypes::kGroup) {
      for ( auto child : object->childs() ) {
        if (child->type() == PrimitiveTypes::kPolygon) {
          prop = child->property();
          prop.mutable_pen()->set_width(value);
          child->setProperty(prop);
        }
      }
    }
    else if (object->type() == PrimitiveTypes::kPolygon) {
      prop = object->property();
      prop.mutable_pen()->set_width(value);
      object->setProperty(prop);
    }
  }
  if ( prop.pen().has_width() ) {
    proto::Document doc = global::lastBlankParams(meteo::map::proto::DocumentType::kGeoMap);
    doc.mutable_border_property()->CopyFrom(prop);
    global::saveLastBlankParams(doc);
  }
  repaint();
}

void LayerBorder::changeStyle(Qt::PenStyle style)
{
  meteo::Property prop;
  for ( auto object : objects() ) {
    if ( object->type() == PrimitiveTypes::kGroup) {
      for ( auto child : object->childs() ) {
        if (child->type() == PrimitiveTypes::kPolygon) {
          prop = child->property();
          QPen pen = pen2qpen(prop.pen());
          pen.setStyle(style);
          prop.mutable_pen()->CopyFrom(qpen2pen(pen));
          child->setProperty(prop);
        }
      }
    }
    else if (object->type() == PrimitiveTypes::kPolygon) {
      prop = object->property();
      QPen pen = pen2qpen(prop.pen());
      pen.setStyle(style);
      prop.mutable_pen()->CopyFrom(qpen2pen(pen));
      object->setProperty(prop);
    }
  }
  if ( prop.pen().has_style() ) {
    proto::Document doc = global::lastBlankParams(meteo::map::proto::DocumentType::kGeoMap);
    doc.mutable_border_property()->CopyFrom(prop);
    global::saveLastBlankParams(doc);
  }
  repaint();
}

void LayerBorder::loadParams()
{
  proto::Document doc = global::lastBlankParams(meteo::map::proto::DocumentType::kGeoMap);
  if ( doc.has_border_property() ) {
    auto property = doc.border_property();
    for ( auto object : objects() ) {
      if ( object->type() == PrimitiveTypes::kGroup) {
        for ( auto child : object->childs() ) {
          if (child->type() == PrimitiveTypes::kPolygon) {
            child->setProperty(property);
          }
        }
      }
      else if (object->type() == PrimitiveTypes::kPolygon) {
        object->setProperty(property);
      }
    }
  }
  repaint();
}

}
}


