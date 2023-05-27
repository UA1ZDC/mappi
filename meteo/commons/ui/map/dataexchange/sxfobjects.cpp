#include "sxfobjects.h"
#include "exchange_algorithms.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/sxfcodes.pb.h>

#include <qfont.h>
#include <qdatastream.h>

using namespace meteo;
using namespace dataexchange;
using namespace sxf;

const int GraphicHeader::length = 12;

GraphicHeader::GraphicHeader(int32_t primitive_length, int32_t count):
  graphic_begin(0x7FFF7FFE),
  length_bytes(length + primitive_length),
  primitive_count(count)
{
}

const QByteArray GraphicHeader::serializeToByteArray() const
{
  QByteArray result;
  result += serializeValue(graphic_begin);
  result += serializeValue(length_bytes);
  result += serializeValue(primitive_count);

  return result;
}

uint32_t GraphicHeader::read(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);

  ds >> graphic_begin;
  ds >> length_bytes;
  ds >> primitive_count;

  return length;
}


GraphicPrimitive::GraphicPrimitive() :
  primitive_length(0),
  type(0)
{
}

GraphicPrimitive::~GraphicPrimitive()
{
}

const QByteArray GraphicPrimitive::serializeToByteArray(bool/* full = true*/) const
{
  QByteArray result;
  if (type != 0) {
    result += serializeValue(primitive_length);
    result += serializeValue(type);
  }
  return result;
}

uint64_t GraphicPrimitive::readHeader(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  ds >> primitive_length;
  ds >> type;
  return sizeof(primitive_length) +  sizeof(type);
}

QSharedPointer<GraphicPrimitive> GraphicPrimitive::create(uint16_t type)
{
  QSharedPointer<GraphicPrimitive> obj;
  switch (type) {
  case SOLID_LINE:
    obj = QSharedPointer<GraphicPrimitive>(new LineGraphicObject);
    break;
  case DASH_LINE:
    obj = QSharedPointer<GraphicPrimitive>(new DashLineGraphicObject);
    break;
  case GLASS_LINE:
    obj = QSharedPointer<GraphicPrimitive>(new LineGlassGraphicObject);
    break;
  case SECTIONS_LINE:
  case USER_LINE:
    obj = QSharedPointer<GraphicPrimitive>(new LineDummyObject);
    break;
  case AREA_OBJECT:
    obj = QSharedPointer<GraphicPrimitive>(new AreaGraphicObject);
    break;
  case CIRCLE_OBJECT:
    obj = QSharedPointer<GraphicPrimitive>(new CircleGraphicObject);
    break;
  case TEXT_VECT:
    obj = QSharedPointer<GraphicPrimitive>(new VectorTextGraphicObject);
    break;
  case AREA_SHADE:
    obj = QSharedPointer<GraphicPrimitive>(new AreaShadingObject);
    break;
  case AREA_GLASS:
    obj = QSharedPointer<GraphicPrimitive>(new AreaGlassObject);
    break;
  case TEXT_OBJECT:
    obj = QSharedPointer<GraphicPrimitive>(new TextGraphicObject);
    break;
  case CUSTOMTEXT_OBJECT:
    obj = QSharedPointer<GraphicPrimitive>(new CustomTextGraphicObject);
    break;
  case VECTOR_SIGN:
    obj = QSharedPointer<GraphicPrimitive>(new VectorSign);
    break;

  default: {
    //warning_log << QObject::tr("Примитив %1 не поддерживается").arg(type);
    }
  };
  return obj;
}

QSharedPointer<sxf::GraphicPrimitive> GraphicPrimitive::readPrimitive(const QByteArray& ba, int length, int offset, int primitive)
{
  QSharedPointer<sxf::GraphicPrimitive> obj;

  switch (primitive) {
  case sxf::AREA_OBJECT: {
    sxf::AreaGraphicObject* gobj = new sxf::AreaGraphicObject();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
  }
    break;
  case sxf::SOLID_LINE : {
    sxf::LineGraphicObject* gobj = new sxf::LineGraphicObject();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
  }
    break;
  case sxf::DASH_LINE: {
    sxf::DashLineGraphicObject* gobj = new sxf::DashLineGraphicObject();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
  }
    break;
  case sxf::SECTIONS_LINE: 
  case sxf::USER_LINE: {
    sxf::LineDummyObject* gobj = new sxf::LineDummyObject();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
  }
    break;
  case sxf::CIRCLE_OBJECT : {
    sxf::CircleGraphicObject* gobj = new sxf::CircleGraphicObject();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
  }
    break;
  case sxf::OBJECTS_SET: {
    sxf::SetOfObjects* gobj = new sxf::SetOfObjects();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
    break;
  }    
  case sxf::TEXT_OBJECT: {
    sxf::TextGraphicObject* gobj = new sxf::TextGraphicObject();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
    break;
  }  
  case sxf::CUSTOMTEXT_OBJECT: {
    sxf::CustomTextGraphicObject* gobj = new sxf::CustomTextGraphicObject();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
    break;
  }  
  case sxf::VECTOR_SIGN: {
    sxf::VectorSign* gobj = new sxf::VectorSign();
    gobj->read(ba.mid(offset, length - offset), false);
    obj = QSharedPointer<sxf::GraphicPrimitive>(gobj);
    break;
  }  

  default: {
    // warning_log << QObject::tr("Примитив %1 не поддерживается").arg(primitive);
  }
    }
  

  return obj;
}

LineGraphicObject::LineGraphicObject() :
  GraphicPrimitive(),
  color(0),
  width(100)
{
  primitive_length = 12;
  type = SOLID_LINE;
}

LineGraphicObject::~LineGraphicObject()
{
}

const QByteArray LineGraphicObject::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  if (full) {
    result += GraphicPrimitive::serializeToByteArray();
  }

  result += serializeValue(color);
  result += serializeValue(width);

  return result;
}

uint32_t LineGraphicObject::read(const QByteArray& ba, bool full /* = true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }

  if (ba.size() >= 8 + offset) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> color;
    ds >> width;
    offset += 8;
  }

  return offset;
}

DashLineGraphicObject::DashLineGraphicObject() :
  LineGraphicObject(),
  dash_length(4000),
  space_length(1000)
{
  primitive_length = 20;
  type = DASH_LINE;
}

const QByteArray DashLineGraphicObject::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  result += LineGraphicObject::serializeToByteArray(full);
  
  result += serializeValue(dash_length);
  result += serializeValue(space_length);

  return result;
}

uint32_t DashLineGraphicObject::read(const QByteArray& ba, bool full /* = true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }

  if (ba.size() >= 4*4 + offset) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> color;
    ds >> width;
    ds >> dash_length;
    ds >> space_length;
    offset += 4*4;
  }
  return offset;
}

LineGlassGraphicObject::LineGlassGraphicObject():
  color(0),
  width(100),
  bright(0),
  contrast(0),
  transp(100)
{
  primitive_length = 20 + 4;
  type = GLASS_LINE;
}

uint32_t LineGlassGraphicObject::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }

  if (ba.size() >= 20 + offset) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> color;
    ds >> width;
    ds >> bright;
    ds >> contrast;
    ds >> transp;
    offset += 20;
  }
  
  return offset;
}

LineDummyObject::LineDummyObject():
  length(0)
{
}

uint32_t LineDummyObject::read(const QByteArray& ba, bool full)
{
  Q_UNUSED(ba);
  Q_UNUSED(full);
  return 0;
}

AreaGraphicObject::AreaGraphicObject() :
  GraphicPrimitive(),
  color(0)
{
  primitive_length = 8;
  type = AREA_OBJECT;
}

const QByteArray AreaGraphicObject::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  if (full) {
    result += GraphicPrimitive::serializeToByteArray();
  } 
  //  var(color);
  result += serializeValue(color);

  return result;
}

uint32_t AreaGraphicObject::read(const QByteArray& ba, bool full /* = true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }

  if (ba.size() >= 4 + offset) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> color;
    offset += 4 + offset;
  }

  return offset;
}

AreaShadingObject::AreaShadingObject():
  length(0),
  angle(0),
  step(0),
  line_type(0)
{
  primitive_length = 16 + 4;
  type = AREA_SHADE;  
}

AreaShadingObject::~AreaShadingObject()
{
  line.clear();
}


uint32_t AreaShadingObject::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }

  if (ba.size() >= offset + 16) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);

    ds >> length;
    ds >> angle;
    ds >> step;
    ds >> line_type;
    offset += 16;

    line = create(line_type);
    if (! line.isNull()) {
      offset += line->read(ba.mid(offset, ba.size() - offset));
    }
  }
  
  return offset;
}

AreaGlassObject::AreaGlassObject():
  color(0),
  bright(0),
  contrast(0),
  transp(100)
{
  primitive_length = 16 + 4;
  type = AREA_GLASS;  
}

uint32_t AreaGlassObject::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }

  if (ba.size() < 16 + offset) {
    return offset;
  }

  QDataStream ds(ba.mid(offset, ba.size() - offset));
  ds.setByteOrder(QDataStream::LittleEndian);
  ds >> color;
  ds >> bright;
  ds >> contrast;
  ds >> transp;

  return offset + 16;
}

CircleGraphicObject::CircleGraphicObject():
  GraphicPrimitive(),
  color(0),
  width(100),
  radius(100)
{
  primitive_length = 16;
  type = CIRCLE_OBJECT;
}

CircleGraphicObject::~CircleGraphicObject()
{
}

const QByteArray CircleGraphicObject::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  if (full) {
    result += GraphicPrimitive::serializeToByteArray();
  }

  result += serializeValue(color);
  result += serializeValue(width);
  result += serializeValue(radius);

  return result;
}

uint32_t CircleGraphicObject::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }

  if (ba.size() >= 8 + offset) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> color;
    ds >> width;
    ds >> radius;
    offset += 12;
  }

  return offset;
}

TextGraphicObject::TextGraphicObject() :
  GraphicPrimitive(),
  color(0),
  background_color(-1),
  shadow_color(0xFFFFFFFF), //  shadow_color(0x00FFFFFF),
  height(4000),
  weight(400),//100 - узкий
  align(24),
  reserved1(0),
  reserved2(0),
  wide(0),
  horizontal(1),
  italic(0),
  underline(0),
  strikeline(0),
  fontcode(0),
  code(0),
  scaleFlag(0)
{
  primitive_length = 32 + 4;
  type = TEXT_OBJECT;
}

const QByteArray TextGraphicObject::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  if (full) {
    result += GraphicPrimitive::serializeToByteArray();
  } 

  result += serializeValue(color);
  result += serializeValue(background_color);
  result += serializeValue(shadow_color);
  result += serializeValue(height);
  result += serializeValue(weight);
  result += serializeValue(align);
  result += serializeValue(reserved1);
  result += serializeValue(reserved2);
  result += serializeValue(wide);
  result += serializeValue(horizontal);
  result += serializeValue(italic);
  result += serializeValue(underline);
  result += serializeValue(strikeline);
  result += serializeValue(fontcode);
  result += serializeValue(code);
  result += serializeValue(scaleFlag);

  return result;
}

uint32_t TextGraphicObject::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }
  if (ba.size() >= offset + 32) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> color;
    ds >> background_color;
    ds >> shadow_color;
    ds >> height;		   
    ds >> weight;		   
    ds >> align;		   
    ds >> reserved1;	   
    ds >> reserved2;
    ds >> wide;		   
    ds >> horizontal;	   
    ds >> italic;	   
    ds >> underline;	   
    ds >> strikeline;	   
    ds >> fontcode;	   
    ds >> code;		   
    ds >>scaleFlag;	   
    offset += 32;
  }

  return offset;
}


CustomTextGraphicObject::CustomTextGraphicObject() :
  TextGraphicObject()
{
  code = 0xCC;

  fillWithValue(font_name, sizeof(font_name));

  QByteArray ba(QString("Arial").toStdString().c_str());
  parseFromByteArray(ba, font_name, sizeof(font_name)-1);

  primitive_length = 68;
  type = CUSTOMTEXT_OBJECT;
}

const QByteArray CustomTextGraphicObject::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  result += TextGraphicObject::serializeToByteArray(full);
  result += serializeValue(font_name, sizeof(font_name));

  return result;
}

uint32_t CustomTextGraphicObject::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = TextGraphicObject::read(ba, full);

  if ((uint32_t)ba.size() >= offset + sizeof(font_name)) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
  
    ds.readRawData(font_name, sizeof(font_name));
    offset += sizeof(font_name);
  }

  return offset;
}

VectorTextGraphicObject::VectorTextGraphicObject():
  TextGraphicObject(),
  semcode(0)
{
  fillWithValue(text, sizeof(text));

  primitive_length = 72;
  type = TEXT_VECT;
}

const QByteArray VectorTextGraphicObject::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  result += TextGraphicObject::serializeToByteArray(full);
  result += serializeValue(semcode);
  result += serializeValue(text, sizeof(text));

  return result;
}

uint32_t VectorTextGraphicObject::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = 0;
  offset = TextGraphicObject::read(ba, full);  

  if ((uint32_t)ba.size() >= offset + sizeof(semcode) + sizeof(text)) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> semcode;
    ds.readRawData(text, sizeof(text));
    offset += sizeof(semcode) + sizeof(text);
  }

  return offset;
}

void VectorTextGraphicObject::setText(const QString& atext, const sxf::FontCode& font, const QColor& acolor)
{
  strncpy(text, atext.toLatin1().data(), sizeof(text)-1);
  fontcode = font.namecode(); 
  reserved1 = font.fontcode();  
 
  color = (acolor.alpha() << 24) + ((acolor.blue() & 0xff) << 16) + (acolor.green() << 8) + (acolor.red() & 0xff);
  //  color = (((acolor.rgb() & 0xff) << 16) + (acolor.rgb() & 0xff00) + ((acolor.rgb() >> 16) & 0xff));
}

TrueTypeSign::TrueTypeSign():
  sign_num(0),
  top_offset(0),
  left_offset(0),
  bot_offset(0),
  right_offset(0),
  pnt_vertic(0),
  pnt_hor(0)  
{  
  code = 0;
  primitive_length = 68 + 28;
  type = TRUE_TYPE;
}

const QByteArray TrueTypeSign::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  result = CustomTextGraphicObject::serializeToByteArray(full);
  
  result += serializeValue(sign_num);
  result += serializeValue(top_offset);
  result += serializeValue(left_offset);
  result += serializeValue(bot_offset);
  result += serializeValue(right_offset);
  result += serializeValue(pnt_vertic);
  result += serializeValue(pnt_hor);

  return result;
}

uint32_t TrueTypeSign::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = CustomTextGraphicObject::read(ba, full);

  if (ba.size() >= offset + 28) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);

    ds >> sign_num;
    ds >> top_offset;
    ds >> left_offset;
    ds >> bot_offset;
    ds >> right_offset;
    ds >> pnt_vertic;
    ds >> pnt_hor;

    offset += 28;
  }

  return offset;
}

//! по умолчанию для текста
VectorObject::VectorObject(int8_t chain, GraphicsType param):
  chain_type(chain),
  param_type(param),
  obj(0)
{
  switch (param) {
  case SOLID_LINE:
    obj = QSharedPointer<GraphicPrimitive>(new LineGraphicObject);
    break;
  case DASH_LINE:
    obj = QSharedPointer<GraphicPrimitive>(new DashLineGraphicObject);
    break;
  case AREA_OBJECT:
    obj = QSharedPointer<GraphicPrimitive>(new AreaGraphicObject);
    break;
  case TEXT_VECT:
    obj = QSharedPointer<GraphicPrimitive>(new VectorTextGraphicObject);
    break;
  default:
    {}
  };
}

VectorObject::VectorObject():
  chain_type(0),
  param_type(0),
  length(0)
{
}

VectorObject::~VectorObject()
{
  obj.clear();
}

void VectorObject::calcLength()
{
  length = obj->primitive_length - 4;
  //  var(length);
}

const QByteArray VectorObject::serializeToByteArray() const
{
  if (obj == 0) {
    return QByteArray();
  }

  QByteArray result;
  result += serializeValue(chain_type);
  result += serializeValue(param_type);
  result += serializeValue(length);
  result += obj->serializeToByteArray(false);
  result += serializeValue(xy.count());
  for (int idx = 0; idx < xy.count(); idx++) {
    result += serializeValue(xy.at(idx).first);
    result += serializeValue(xy.at(idx).second);
  }

  //  debug_log << "vecobj" << result.size() << length;

  return result;
}

uint32_t VectorObject::read(const QByteArray& ba)
{
  int offset = 0;
  
  if (ba.size() < 4) {
    return offset;
  }

  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  ds >> chain_type;
  ds >> param_type;
  ds >> length;
  offset += 4;
  if (length == 0) return offset;

  obj = GraphicPrimitive::create(param_type);
  if (obj != 0) {
    obj->read(ba.mid(offset, length), false);
    offset += length;
  }
  ds.skipRawData(length);
  if (ds.atEnd()) {
    return offset;
  }

  uint32_t cnt;
  ds >> cnt;
  offset += 4;
  
  if ((uint32_t)ba.size() < offset + cnt*8) {
    return offset;
  }
  
  for (uint32_t idx = 0; idx < cnt; idx++) {
    QPair<int32_t, int32_t> onexy;
    ds >> onexy.first;
    ds >> onexy.second;
    xy.append(onexy);
  }

  offset += cnt * 8;
  
  calcLength();

  return offset;
}

VectorSign::VectorSign():
  fulllength(0),       
  pnt_vertic(0),       
  pnt_hor(0),          
  pnt_base(0),         
  mark_vertic_start(0),
  mark_vertic_end(0),  
  size_vertic(0),      
  mark_hor_start(0),   
  mark_hor_end(0),     
  size_hor(0),         
  align(0),    
  mirror(0),   
  contract(1), 
  strech(0),   
  centering(0),
  reserved1(0),
  reserved2(0),
  max_dim(0)  
{ 
  type = VECTOR_SIGN;
}

VectorSign::~VectorSign()
{
  for (int idx = 0; idx < obj.count(); idx++) {
    obj[idx].clear();
  }
}

QSharedPointer<VectorObject> VectorSign::addObject(int8_t chain_type, GraphicsType param_type)
{
  QSharedPointer<VectorObject> child = QSharedPointer<VectorObject>(new VectorObject(chain_type, param_type));
  obj.append(child);
  return child;
}

void VectorSign::calcLength()
{
  fulllength = baselength;

  for (int idx = 0; idx < obj.count(); idx++) {
    fulllength += obj.at(idx)->length + 8 + obj.at(idx)->xy.count()*8;
  }

  primitive_length = fulllength + 4;
}

const QByteArray VectorSign::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  if (full) {
    result = GraphicPrimitive::serializeToByteArray();
  }

  result += serializeValue(fulllength);
  result += serializeValue(pnt_vertic);
  result += serializeValue(pnt_hor);
  result += serializeValue(pnt_base);
  result += serializeValue(mark_vertic_start);
  result += serializeValue(mark_vertic_end);
  result += serializeValue(size_vertic);
  result += serializeValue(mark_hor_start);
  result += serializeValue(mark_hor_end);
  result += serializeValue(size_hor);
  result += serializeValue(align);
  result += serializeValue(mirror);
  result += serializeValue(contract);
  result += serializeValue(strech);
  result += serializeValue(centering);
  result += serializeValue(reserved1);
  result += serializeValue(reserved2);
  result += serializeValue(max_dim);
  result += serializeValue(obj.count());
  
  //  var(result.size());
  
  for (int idx = 0; idx < obj.count(); idx++) {
    result += obj.at(idx)->serializeToByteArray();
  }

  //  debug_log << result.size() << fulllength << primitive_length;

  return result;
}

uint32_t VectorSign::read(const QByteArray& ba, bool full /*= true*/)
{
  int offset = 0;
  if (full) {
    offset = GraphicPrimitive::readHeader(ba);
  }
  if (ba.size() < offset + 10*4 + 16) {
    return offset;
  }
  
  QDataStream ds(ba.mid(offset, ba.size() - offset));
  ds.setByteOrder(QDataStream::LittleEndian);

  ds >> fulllength;
  ds >> pnt_vertic;
  ds >> pnt_hor;
  ds >> pnt_base;
  ds >> mark_vertic_start;
  ds >> mark_vertic_end;
  ds >> size_vertic;
  ds >> mark_hor_start;
  ds >> mark_hor_end;
  ds >> size_hor;
  ds >> align;
  ds >> mirror;
  ds >> contract;
  ds >> strech;
  ds >> centering;
  ds >> reserved1;
  ds >> reserved2;
  ds >> max_dim;
  int cnt;
  ds >> cnt;
  
  offset += 10*4 + 16;

  for (int idx = 0; idx < cnt; idx++) {
    QSharedPointer<VectorObject> gobj = QSharedPointer<VectorObject>(new VectorObject);
    offset += gobj->read(ba.mid(offset, ba.size() - offset));
    obj.append(gobj);    
  }

  calcLength();

  return offset;
}


DecoratedLine::DecoratedLine():
  length(0),
  length_start(0),
  length_end(0),
  length_first(0),
  length_last(0),
  length_firstSign(0),
  length_secondSign(0),
  flagUniform(0),
  reserved1(0),
  reserved2(0),
  base_length(4),
  base_num(127),
  base(0),
  start_length(4),
  start_num(127),
  start(0),
  end_length(4),
  end_num(0),
  end(0),
  left_length(4),
  left_num(127),
  left(0),
  right_length(4),
  right_num(127),
  right(0),
  fill1_length(4),
  fill1_num(127),
  fill1(0),
  fill2_length(4),
  fill2_num(127),
  fill2(0)
{
  type = DECORATE_LINE;
}

DecoratedLine::~DecoratedLine()
{
  base.clear();
  start.clear();
  end.clear();
  left.clear();
  right.clear();
  fill1.clear();
  fill2.clear();
}

void DecoratedLine::calcLength()
{
  length = 7*4 + 4 + base_length + start_length + end_length + 
    left_length + right_length + fill1_length + fill2_length;
  
  primitive_length = length + 4;
}

const QByteArray DecoratedLine::serializeToByteArray(bool full /*= true*/) const
{
  QByteArray result;
  if (full) {
    result = GraphicPrimitive::serializeToByteArray();
  }

  result += serializeValue(length);
  result += serializeValue(length_start);
  result += serializeValue(length_end);
  result += serializeValue(length_first);
  result += serializeValue(length_last);
  result += serializeValue(length_firstSign);
  result += serializeValue(length_secondSign);
  result += serializeValue(flagUniform);
  result += serializeValue(reserved1);
  result += serializeValue(reserved2);
  result += serializeValue(base_length);
  result += serializeValue(base_num);
  if (!base.isNull()) {
    result += base->serializeToByteArray(false);
  }
  result += serializeValue(start_length);
  result += serializeValue(start_num);
  if (!start.isNull()) {
    result += start->serializeToByteArray(false);
  }
  result += serializeValue(end_length);
  result += serializeValue(end_num);
  if (!end.isNull()) {
    result += end->serializeToByteArray(false);
  }
  result += serializeValue(left_length);
  result += serializeValue(left_num);
  if (!left.isNull()) {
    result += left->serializeToByteArray(false);
  }
  result += serializeValue(right_length);
  result += serializeValue(right_num);
  if (!right.isNull()) {
    result += right->serializeToByteArray(false);
  }
  result += serializeValue(fill1_length);
  result += serializeValue(fill1_num);
  if (!fill1.isNull()) {
    result += fill1->serializeToByteArray(false);
  }
  result += serializeValue(fill2_length);
  result += serializeValue(fill2_num);
  if (!fill2.isNull()) {
    result += fill2->serializeToByteArray(false); 
  }

  return result;
}

SetOfObjects::~SetOfObjects()
{
  for (int idx = 0; idx < obj.count(); idx++) {
    obj[idx].clear();
  }
}

uint32_t SetOfObjects::read(const QByteArray& ba, bool full)
{
  Q_UNUSED(full);
  header.read(ba);

  if (ba.size() < header.length_bytes) {
    error_log << QObject::tr("Ошибка размера данных");
    return false;
  }
  
  uint offset = header.length;

  for (int idx = 0; idx < header.primitive_count; idx++) {
    QDataStream ds(ba.mid(offset, ba.size() - offset));
    ds.setByteOrder(QDataStream::LittleEndian);
    uint16_t objlength;
    uint16_t type;
    ds >> objlength;
    ds >> type;
    QSharedPointer<GraphicPrimitive> gobj = GraphicPrimitive::create(type);
    if (gobj != 0) {
      gobj->read(ba.mid(offset, objlength));
    }
    obj.append(gobj);

    offset += objlength;
  }
  return offset;
}
