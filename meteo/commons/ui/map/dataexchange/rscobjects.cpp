#include "rscobjects.h"
#include "sxfobjects.h"
#include "sxfstructures.h"
#include "metasxf.h"
#include "exchange_algorithms.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/ui/map/document.h>
#include <meteo/commons/ui/map/geotext.h>
#include <meteo/commons/ui/map/geopolygon.h>
#include <meteo/commons/ui/map/puanson.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/ui/map/geogroup.h>
#include <meteo/commons/ui/map/wind.h>
#include <meteo/commons/global/weatherloader.h>


#include <qstring.h>
#include <string.h>
#include <qbytearray.h>

using namespace meteo;

const uint32_t rsc::ObjectView::header_length = 8;
const int rsc::Header::header_size = 328;
const uint32_t rsc::Semantics::size = 84;

rsc::Header::Header()
{
  clear();
  isValid = false;
}

rsc::Header::Header(const QByteArray& ba)
{
  isValid = false;
  clear();
  if ( read(ba)) {
    isValid = true;
  }
}

void rsc::Header::clear()
{
  id = length = vers = 0;
  coding = num_state = num_mdif = 0;
  language = max_id = 0;
  dt[0] = 0;
  some1[0] = 0;
  obj_offset = obj_len = obj_cnt = 0;
  sem_offset = sem_len = sem_cnt = 0;
  clsem_offset = clsem_len = clsem_cnt = 0;
  default_offset = default_len = default_cnt = 0;
  probsem_offset = probsem_len = probsem_cnt = 0;
  layers_offset = layers_len = layers_cnt = 0;
  lim_offset = lim_len = lim_cnt = 0;
  param_offset = param_len = param_cnt = 0;
  print_offset = print_len = print_cnt = 0;
  pal_offset = pal_len = pal_cnt = 0;
  font_offset = font_len = font_cnt = 0;

  lib_offset = lib_len = lib_cnt = 0;
  imsem_offset = imsem_len = imsem_cnt = 0;
  tbl_offset = tbl_len = tbl_cnt = 0;

  some2[0] = 0;
}

bool rsc::Header::read(const QByteArray& ba)
{
  if (ba.size() < header_size) return false;

  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);

  ds >> id;        
  ds >> length;    
  ds >> vers;	     
  ds >> coding;    
  ds >> num_state; 
  ds >> num_mdif;  
  ds >> language;  
  ds >> max_id;
  ds.readRawData((char*)dt, sizeof(dt));
  ds.readRawData((char*)some1, sizeof(some1));     
  ds >> obj_offset;     
  ds >> obj_len;	  
  ds >> obj_cnt;	  
  ds >> sem_offset;	  
  ds >> sem_len;	  
  ds >> sem_cnt;	  
  ds >> clsem_offset;	  
  ds >> clsem_len;	  
  ds >> clsem_cnt;	  
  ds >> default_offset; 
  ds >> default_len;	  
  ds >> default_cnt;	  
  ds >> probsem_offset; 
  ds >> probsem_len;	  
  ds >> probsem_cnt;	  
  ds >> layers_offset;  
  ds >> layers_len;	
  ds >> layers_cnt;	
  ds >> lim_offset;	
  ds >> lim_len;	
  ds >> lim_cnt;	
  ds >> param_offset; 
  ds >> param_len;	
  ds >> param_cnt;	
  ds >> print_offset; 
  ds >> print_len;	
  ds >> print_cnt;	
  ds >> pal_offset; 
  ds >> pal_len;	
  ds >> pal_cnt;	
  ds >> font_offset; 
  ds >> font_len;	
  ds >> font_cnt;	

  ds >> lib_offset; 
  ds >> lib_len;	
  ds >> lib_cnt;	
  ds >> imsem_offset; 
  ds >> imsem_len;	
  ds >> imsem_cnt;	
  ds >> tbl_offset; 
  ds >> tbl_len;	
  ds >> tbl_cnt;	


  ds.readRawData((char*)some2, sizeof(some2));	
  
  QByteArray dateData(dt, sizeof(dt));
  QDate date(dateData.mid(0, 4).toInt(), dateData.mid(4, 2).toInt(), dateData.mid(6, 2).toInt());
 
  if (id != 0x00435352 || length != (uint)ba.size() || !date.isValid()) {
    return false;
  }

  return true;
}

const QByteArray rsc::Header::serializeToByteArray() const
{
  QByteArray result;

  result += dataexchange::serializeValue(id);
  result += dataexchange::serializeValue(length);
  result += dataexchange::serializeValue(vers);
  result += dataexchange::serializeValue(coding);
  result += dataexchange::serializeValue(num_state);
  result += dataexchange::serializeValue(num_mdif);
  result += dataexchange::serializeValue(language);
  result += dataexchange::serializeValue(max_id);
  result += dataexchange::serializeValue(dt, sizeof(dt));
  result += dataexchange::serializeValue(some1, sizeof(some1));
  result += dataexchange::serializeValue(obj_offset);
  result += dataexchange::serializeValue(obj_len); 
  result += dataexchange::serializeValue(obj_cnt);
  result += dataexchange::serializeValue(sem_offset);
  result += dataexchange::serializeValue(sem_len);
  result += dataexchange::serializeValue(sem_cnt);
  result += dataexchange::serializeValue(clsem_offset); 
  result += dataexchange::serializeValue(clsem_len);
  result += dataexchange::serializeValue(clsem_cnt);
  result += dataexchange::serializeValue(default_offset);
  result += dataexchange::serializeValue(default_len);
  result += dataexchange::serializeValue(default_cnt); 
  result += dataexchange::serializeValue(probsem_offset);
  result += dataexchange::serializeValue(probsem_len);
  result += dataexchange::serializeValue(probsem_cnt);
  result += dataexchange::serializeValue(layers_offset);
  result += dataexchange::serializeValue(layers_len); 
  result += dataexchange::serializeValue(layers_cnt);
  result += dataexchange::serializeValue(lim_offset);
  result += dataexchange::serializeValue(lim_len);
  result += dataexchange::serializeValue(lim_cnt);
  result += dataexchange::serializeValue(param_offset); 
  result += dataexchange::serializeValue(param_len);
  result += dataexchange::serializeValue(param_cnt);
  result += dataexchange::serializeValue(print_offset);
  result += dataexchange::serializeValue(print_len);
  result += dataexchange::serializeValue(print_cnt); 
  result += dataexchange::serializeValue(pal_offset);
  result += dataexchange::serializeValue(pal_len);
  result += dataexchange::serializeValue(pal_cnt);
  result += dataexchange::serializeValue(font_offset);
  result += dataexchange::serializeValue(font_len); 
  result += dataexchange::serializeValue(font_cnt);
  result += dataexchange::serializeValue(lib_offset);
  result += dataexchange::serializeValue(lib_len);
  result += dataexchange::serializeValue(lib_cnt);
  result += dataexchange::serializeValue(imsem_offset);
  result += dataexchange::serializeValue(imsem_len);
  result += dataexchange::serializeValue(imsem_cnt);
  result += dataexchange::serializeValue(tbl_offset);
  result += dataexchange::serializeValue(tbl_len);
  result += dataexchange::serializeValue(tbl_cnt);
  result += dataexchange::serializeValue(some2, sizeof(some2));

  return result;
}


rsc::Object::Object(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  ds >> length;
  ds >> code;
  ds >> incode;
  ds >> ident;
  ds.readRawData(short_name, sizeof(short_name));
  ds.readRawData(name, sizeof(name));
  ds >> localization;
  ds >> layer;
  ds >> scale_flag;
  ds >> vis_lo;
  ds >> vis_hi;
  ds >> ext_loc;
  ds >> direct;
  ds >> sem_flag;
  ds >> snumber;
  ds >> text_cnt;
  ds >> contract_flag;
  ds >> max_scale;
  ds >> min_scale;
  ds >> visible_flag;
  ds >> reserved;
  //  debug_log << code << incode << ident << length << short_name;
}

const QByteArray rsc::Object::serializeToByteArray() const
{
  QByteArray result;
  
  result += dataexchange::serializeValue(length);
  result += dataexchange::serializeValue(code);
  result += dataexchange::serializeValue(incode);
  result += dataexchange::serializeValue(ident);
  result += dataexchange::serializeValue(short_name, sizeof(short_name));
  result += dataexchange::serializeValue(name, sizeof(name));
  result += dataexchange::serializeValue(localization);
  result += dataexchange::serializeValue(layer);
  result += dataexchange::serializeValue(scale_flag);
  result += dataexchange::serializeValue(vis_lo);
  result += dataexchange::serializeValue(vis_hi);
  result += dataexchange::serializeValue(ext_loc);
  result += dataexchange::serializeValue(direct);
  result += dataexchange::serializeValue(sem_flag);
  result += dataexchange::serializeValue(snumber);
  result += dataexchange::serializeValue(text_cnt);
  result += dataexchange::serializeValue(contract_flag);
  result += dataexchange::serializeValue(max_scale);
  result += dataexchange::serializeValue(min_scale);
  result += dataexchange::serializeValue(visible_flag);
  result += dataexchange::serializeValue(reserved);

  return result;
}

rsc::ObjectView::ObjectView(const QByteArray& ba)
{
  readHeader(ba);
  readPrimitive(ba);
}

rsc::ObjectView::~ObjectView()
{    
  obj.clear();
}

bool rsc::ObjectView::readHeader(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  ds >> length;
  ds >> incode;
  ds >> primitive;

  //var(primitive);
  return true;
}

bool rsc::ObjectView::readPrimitive(const QByteArray& ba)
{
  obj = sxf::GraphicPrimitive::readPrimitive(ba, length, 8, primitive);

  return true;
}

const QByteArray rsc::ObjectView::serializeToByteArray() const
{
  QByteArray result;

  result += dataexchange::serializeValue(length);
  result += dataexchange::serializeValue(incode);
  result += dataexchange::serializeValue(primitive);

  // switch (primitive) {
  // case sxf::SOLID_LINE: {
  //   sxf::LineGraphicObject* o = static_cast<sxf::LineGraphicObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  // }
  //   break;
  // case sxf::DASH_LINE: {
  //   sxf::DashLineGraphicObject* o = static_cast<sxf::DashLineGraphicObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  // }
  //   break;
  // case sxf::GLASS_LINE:{
  //   sxf::LineGlassGraphicObject* o = static_cast<sxf::LineGlassGraphicObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::SECTIONS_LINE:
  // case sxf::USER_LINE:{
  //   sxf::LineDummyObject* o = static_cast<sxf::LineDummyObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::AREA_OBJECT:{
  //   sxf::AreaGraphicObject* o = static_cast<sxf::AreaGraphicObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::CIRCLE_OBJECT:{
  //   sxf::CircleGraphicObject* o = static_cast<sxf::CircleGraphicObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::TEXT_VECT:{
  //   sxf::VectorTextGraphicObject* o = static_cast<sxf::VectorTextGraphicObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::AREA_SHADE:{
  //   sxf::AreaShadingObject* o = static_cast<sxf::AreaShadingObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::AREA_GLASS:{
  //   sxf::AreaGlassObject* o = static_cast<sxf::AreaGlassObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::TEXT_OBJECT:{
  //   sxf::TextGraphicObject* o = static_cast<sxf::TextGraphicObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::CUSTOMTEXT_OBJECT:{
  //   sxf::CustomTextGraphicObject* o = static_cast<sxf::CustomTextGraphicObject*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;
  // case sxf::VECTOR_SIGN:{
  //   sxf::VectorSign* o = static_cast<sxf::VectorSign*>(obj.data());
  //   result += o->serializeToByteArray(false);
  //   }
  //   break;

  // default: {
  //   warning_log << QObject::tr("Примитив %1 не поддерживается").arg(primitive);
  //   }
  // }

  return result;
}

rsc::Layer::Layer(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  ds >> length;
  ds.readRawData(name, sizeof(name));
  ds.readRawData(short_name, sizeof(short_name));
  ds >> number;
  ds >> order;
  ds >> sem_cnt;
  for (uint i=0; i < sem_cnt; i++) {
    if (ds.atEnd()) {
      break;
    }
    uint8_t val;
    ds >> val;
    sem << val;;
  }
}

rsc::Limits::Limits(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);

  ds >> length;	
  ds >> code;		
  ds >> localization;	
  ds >> reserved1;	
  ds >> reserved2;	
  ds >> reserved3;	
  ds >> semcode1;	
  ds >> cnt1;		
  ds >> default1;	
  ds >> semcode2;	
  ds >> cnt2;		
  ds >> default2;	
  double v;
  for (uint i = 0; i < cnt1; i++) {
    if (ds.atEnd()) {
      break;
    }
    ds >> v;
    sem1 << v;	
  }
  for (uint i = 0; i < cnt2; i++) {
    ds >> v;
    sem2 << v;
  }

  uint mcnt = (cnt2 == 0) ? cnt1 : cnt1*cnt2;
  for (uint i = 0; i < mcnt; i++) {
    if (ds.atEnd()) {
      break;
    }
    uint8_t vv;
    ds >> vv;
    snumber << vv;
  }
}

rsc::Font::Font(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  file = ba.mid(0, 32);
  name = ba.mid(32, 32);

  ds.skipRawData(64);

  ds >> code;
  ds >> sym;
  ds >> code_page;
  
}

rsc::Tables::Tables(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  ds >> color_offset;
  ds >> color_length;
  ds >> color_cnt;
  ds.readRawData(reserved, sizeof(reserved));
  ds >> unk_offset;
  ds >> unk_length;
  ds >> unk_cnt;
  ds.readRawData(reserved1, sizeof(reserved1));
}


const QByteArray rsc::Tables::serializeToByteArray() const
{
  QByteArray result;

  result += dataexchange::serializeValue(color_offset);
  result += dataexchange::serializeValue(color_length);
  result += dataexchange::serializeValue(color_cnt);
  result += dataexchange::serializeValue(reserved, sizeof(reserved));
  result += dataexchange::serializeValue(unk_offset);
  result += dataexchange::serializeValue(unk_length);
  result += dataexchange::serializeValue(unk_cnt);
  result += dataexchange::serializeValue(reserved1, sizeof(reserved1));

  return result;
}

rsc::Semantics::Semantics(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);

  ds.readRawData(some, sizeof(some));
  ds >> sem_offset;
  ds >> sem_cnt;
  ds >> defsem_offset;
  ds >> defsem_cnt;
}

const QByteArray rsc::Semantics::serializeToByteArray() const
{
  QByteArray result;
  
  result += dataexchange::serializeValue(some, sizeof(some));
  result += dataexchange::serializeValue(sem_offset);
  result += dataexchange::serializeValue(sem_cnt);
  result += dataexchange::serializeValue(defsem_offset);
  result += dataexchange::serializeValue(defsem_cnt);
 
  return result;
}


rsc::Document::Document(const QByteArray& ba):
  header_(ba),
  isValid_(false)
{
  QPixmap p(1000,1000);
  pixScale_ = p.widthMM();
  if (pixScale_ <= 0) {
    pixScale_ = 200;
  }

  if (!header_.isValid) {
    return;
  }

  viewba_.clear();
  readPalette(ba);
  readLayers(ba);
  readObjects(ba);
  //readAndChangeViews(ba);
  readViews(ba);
  readLimits(ba);
  readFonts(ba);
  isValid_ = true;
}

rsc::Document::~Document()
{
  isValid_ = false;
}

//! одна стандартная палитра
void rsc::Document::readPalette(const QByteArray& ba)
{
  QDataStream ds(ba.mid(header_.pal_offset, ba.length() - header_.pal_offset));
  for (uint i=0; i< 256; i++) {
    ds >> pal_.rgb[i];
    pal_.rgb[i] >>= 8;    
  }
  ds.readRawData(pal_.name, sizeof(pal_.name));
}

//! чтение данных слоёв
void rsc::Document::readLayers(const QByteArray& ba)
{
  uint offset = header_.layers_offset;
  for (uint i = 0; i < header_.layers_cnt; i++) {
    if (offset >= (uint64_t)ba.size()) break;

    rsc::Layer lay(ba.mid(offset, ba.length()-offset));
    layers_.insert(lay.number, lay);
    offset += lay.length;
  }
}

//! Чтение данных объектов
void rsc::Document::readObjects(const QByteArray& ba)
{
  uint offset = header_.obj_offset;
  for (uint i = 0; i < header_.obj_cnt; i++) {
    if (offset >= (uint64_t)ba.size()) break;
    rsc::Object obj(ba.mid(offset, ba.length()-offset));
    objs_.insertMulti(sxf::ObjectKey(obj.code, obj.localization), obj);

    offset += obj.length;
  }
}

//-------------------------
//! Изменение классификатора. Для корректного отображения пуансона
void rsc::Document::changeMeteoSign(uint incode, sxf::VectorSign* sign)
{
  int code = -1;
  QList<sxf::ObjectKey> keys = objs_.keys();
  for (int idx = 0; idx < keys.size(); idx++) {
    if (objs_.value(keys.at(idx)).incode == incode) {
      code = objs_.value(keys.at(idx)).code;
      break;
    }
  }

  if (-1 == code) {
    error_log << "Strange error";
    return;
  }

  sxf::ClassifType type = sxf::ClassifType(sxf::MetaSxf::instance()->meteoType(code));
  var(type);

  int dx = 0;
  int dy = 0;

  switch (type) {
  case sxf::kNMeteo:
    break;
  case sxf::kaMeteo:
    dx = 12000;
    break;
  case sxf::kCMeteo:
    if (code >= 43 && code <= 51) {
      dy = -5000;
    } else if (code >= 52 && code <= 60) {
      dy = -9000;
    } else {
      dy = 4000;
    }
    break;
  case sxf::kwMeteo:
    dx = -7500;
    break;
  case sxf::kW1Meteo:
    dx = 10000;
    dy = 6000;
    break;
  case sxf::kW2Meteo:
    dx = 21000;
    dy = 6000;
    break;
  case sxf::kSign:
    if ((code >= 1175 && code <= 1183) || code == 1015 || code == 1016) {
      return;
    }
    if (code >= 1184 && code <= 1186) {
      dx = -3000;
      dy =  3000;
    }
    if (code == 1187) {
      dy = 2000;
    }
    if (code == 1189) {
      dy = -2000;
    }

    if (code >= 1009 && code <= 1011) {
      dx = -3000;
      dy = -5500;
    }
    break;
  default: {
    return;
  }
  }

  var(code);
  var(sign->pnt_vertic);
  var(sign->pnt_hor);
  var(sign->pnt_base);
  var(sign->mark_vertic_start);
  var(sign->mark_vertic_end); 
  var(sign->size_vertic);  
  var(sign->mark_hor_start);  
  var(sign->mark_hor_end);
  var(sign->size_hor);  
  var(sign->max_dim);  

  var(sign->align);	  
  var(sign->mirror);	  
  var(sign->contract); 
  var(sign->strech);	  
  var(sign->centering);


  if (dy < 0) {
    sign->mark_vertic_start += abs(dy);
    sign->mark_vertic_end += abs(dy);
  } else {
    sign->mark_vertic_start += 4000;
    sign->mark_vertic_end += 4000;    
  }
  if (dx < 0) {
    sign->mark_hor_start += abs(dx);
    sign->mark_hor_end += abs(dx);
  } else {
    sign->mark_hor_start += 4000;
    sign->mark_hor_end += 4000;
  }
  if (type == sxf::kNMeteo || type == sxf::kSign) {
    sign->size_vertic = 4000;
    sign->size_hor = 4000;   
  } else if (type == sxf::kCMeteo) {
    sign->size_vertic = 3000;
    sign->size_hor = 3000;   
  } else if (type == sxf::kW2Meteo) {
    sign->size_vertic = 3500;
    sign->size_hor = 3500;   
    sign->max_dim = 30000;
  } else {
    sign->size_vertic = 3000;
    sign->size_hor = 3000; 
  }

  
  if (dy < 0) {
    if (code < 1033 || code > 1041) {
      sign->pnt_vertic += abs(dy);
    }
  }
  if (dx < 0) {
    sign->pnt_hor += abs(dx);
  }

  //sign->pnt_base = 1000;
  for (int idx = 0; idx < sign->obj.size(); idx++) {
    for (int pnt = 0; pnt < sign->obj.at(idx)->xy.size(); pnt++) {
      debug_log << idx << pnt;
      var(sign->obj.at(idx)->xy.at(pnt).first);
      var(sign->obj.at(idx)->xy.at(pnt).second);
      if (sign->obj.at(idx)->chain_type == 0 || sign->obj.at(idx)->chain_type == 1) {
	sign->obj[idx]->xy[pnt].first += dx;
	sign->obj[idx]->xy[pnt].second += dy;
      } else if (type != sxf::kwMeteo && pnt != 1 && !(code >= 1009 && code <= 1011)) { //почему-то дуги в kwMeteo двигать не надо
	sign->obj[idx]->xy[pnt].first += dx;
	if (dy > 0) {
	  sign->obj[idx]->xy[pnt].second += dy;
	}
      }
    }
  }

  
}

//! Чтение и изменение классификатора. Для корректного отображения пуансона
void rsc::Document::readAndChangeViews(const QByteArray& ba)
{
  uint offset = header_.param_offset;
  for (uint i = 0; i < header_.param_cnt; i++) {
    if (offset >= (uint64_t)ba.size()) break;
    rsc::ObjectView view(ba.mid(offset, ba.length()-offset));

    int oldsize = viewba_.size();
    if (view.primitive == sxf::VECTOR_SIGN) {
      sxf::VectorSign* sign = static_cast<sxf::VectorSign*>(view.obj.data());
      changeMeteoSign(view.incode, sign);
   
      QByteArray sba =  view.serializeToByteArray() + sign->serializeToByteArray(false);
      // if (sba.size() !=  (int)view.length) {
      // 	// var(sba.toHex());
      // 	// var(ba.mid(offset, view.length).toHex());
      // 	debug_log << "diff" << view.length - sba.size() << ba.mid(offset + view.length, 4).toHex() << view.incode << view.length << sign->fulllength;
	
      // }
      viewba_.append(sba);
      if (sba.size() !=  (int)view.length) {
	viewba_.append(ba.mid(offset + sba.size(), view.length - sba.size()));
      }
      //var(sba.size());
    } else {
      viewba_.append(ba.mid(offset, view.length));      
    }
    
    debug_log << "offset = " << offset - header_.param_offset << "l=" << view.length << viewba_.size() << viewba_.size() - oldsize << view.incode;

    views_.insert(view.incode, view);
    offset += view.length;
  }
  var(views_.size());
}
//--------------------------------


//! Чтение данных объектов
void rsc::Document::readViews(const QByteArray& ba)
{
  uint offset = header_.param_offset;
  for (uint i = 0; i < header_.param_cnt; i++) {
    if (offset >= (uint64_t)ba.size()) break;
    rsc::ObjectView view(ba.mid(offset, ba.length()-offset));
    views_.insert(view.incode, view);
    offset += view.length;
  }
}

//! Чтение порогов
void rsc::Document::readLimits(const QByteArray& ba)
{
  uint offset = header_.lim_offset;
  for (uint i = 0; i < header_.lim_cnt; i++) {
    if (offset >= (uint64_t)ba.size()) break;
    rsc::Limits limit(ba.mid(offset, ba.length()-offset));
    limits_.insert(QPair<uint32_t, uint8_t>(limit.code, limit.localization),  limit);
    offset += limit.length;
  }
}



//! Чтение шрифтов
void rsc::Document::readFonts(const QByteArray& ba)
{
  uint offset = header_.font_offset;
  for (uint i = 0; i < header_.font_cnt; i++) {
    if (offset >= (uint64_t)ba.size()) break;
    rsc::Font font(ba.mid(offset, ba.length()-offset));
    fonts_.insert(font.code,  font);
    offset += 72;
  }
}

//! Поиск использующихся слоев
bool rsc::Document::fillObjectsLayers(const QMap<sxf::ObjectKey, sxf::ObjectDescription>& objCodes,
				      QMap<sxf::ObjectKey, uint8_t>* objLayers,
				      QMap<uint8_t, sxf::LayersDescription>* legend)
{
  if (0 == legend || 0 == objLayers) return false;

  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");

  QMapIterator<sxf::ObjectKey, sxf::ObjectDescription> objit(objCodes);
  while (objit.hasNext()) {
    objit.next();
    QMultiMap<sxf::ObjectKey, Object>::const_iterator it = objs_.find(objit.key());
    while (objs_.end() != it && it.key() == objit.key()) {
      int layer = it.value().layer;
      const Layer& lay = layers_.find(layer).value();
      objLayers->insert(objit.key(), lay.number);
      if (! legend->contains(layer)) {
	sxf::LayersDescription ld(codec->toUnicode(lay.name, strlen(lay.name)));
	sxf::ObjectDescription od(objit.value().count);
	od.name = codec->toUnicode(it.value().name, strlen(it.value().name));
	ld.obj.insertMulti(objit.key(), od);
	legend->insert(layer, ld);
      } else {
	sxf::ObjectDescription od(objit.value().count);
	od.name = codec->toUnicode(it.value().name, strlen(it.value().name));
	legend->operator[](layer).obj.insertMulti(objit.key(), od);
      }
      ++it;
    }
  }

  return true;
}


//! Выбор объекта классификатора в соответствеии с кодом, локализацией, значениями семантики
const rsc::Object& rsc::Document::object(uint32_t code, uint8_t loc, const QList<sxf::Semantic>& sem) const
{
  QPair<uint32_t, uint8_t> key(code,loc);
  if (objs_.count(key) <= 1 || limits_.count(key) == 0) {
    return objs_.find(key).value();
  }
  
  const Limits& lim = limits_.find(key).value();
  int snumber = findSerialNumber(lim, sem);

  QMultiMap<QPair<uint32_t, uint8_t>, Object >::const_iterator it = objs_.find(key);
  while (it != objs_.end() && it.key() == key) {
    const Object& cur = it.value();
    if (snumber == cur.snumber) {
      break;
    }
    ++it;
  }

  return it.value();
}

//! Поиск номера серии для объектов, зависящих от значения семантики
int rsc::Document::findSerialNumber(const Limits& lim, const QList<sxf::Semantic>& sem) const
{
  int osem1 = findSemanticValue(sem, lim.semcode1, lim.default1);
  int semidx1 = 0;
  for (uint i = 0; i < lim.cnt1; i++) {
    if (osem1 <= lim.sem1.at(i)) {
      semidx1 = i;
      break;
    }
  }
  semidx1 += 1;

  int semidx2 = 0;
  if (lim.semcode2 != 0) {
    int osem2 = findSemanticValue(sem, lim.semcode2, lim.default2);
    for (uint i = 0; i < lim.cnt2; i++) {
      if (osem2 <= lim.sem2.at(i)) {
	semidx2 = i;
	break;
      }
    }
  }
  semidx2 += 1;
  
  semidx1 = (semidx1 * semidx2 ) - 1;
  if (semidx1 >= lim.snumber.count()) {
    return -1;
  }

  return lim.snumber.at(semidx1);
}

//! Поиск в списке семантик с кодом semCode
int rsc::Document::findSemanticValue(const QList<sxf::Semantic>& sem, int semCode, int defaultValue) const
{
  int res = defaultValue;

  for (int i = 0; i < sem.count(); i++) {
    if (sem.at(i).code == semCode) {
      bool ok;
      res = sem.at(i).value.toDouble(&ok);
      if (!ok) {
	error_log << QObject::tr("Ошибка семантики объекта. Код семантики = '%1'. Значение = '%2'").arg(semCode).arg(sem.at(i).value);
	res = defaultValue;
      }
      break;
    }
  }

  return res;
}

//! Отрисовка объекта, описанного в классификаторе, на карте
/*! 
  \param sxf Параметры объекта из sxf
  \param rsc Соответствующие sxf параметры объекта их классификатора
  \param lay Слой, на котором отрисовывать
  \param origScale Исходный масштаб карты
  \return количество отрисованных объектов (с учетом подобъектов)
*/
uint64_t rsc::Document::drawObject(const sxf::Object& sxf, const rsc::Object& rsc, map::Layer* lay, float origScale) const
{
  if (!isValid_) return false;

  const ObjectView& view = views_.find(rsc.incode).value();

  if (view.obj.isNull() || 0 == view.length || 0 == lay) {
    //error_log << QObject::tr("Не прочитаны свойства объекта %1").arg(view.primitive);
    return false;
  }
  
  uint64_t cnt = 0;
  const QList<sxf::FullMetric> submet = sxf.submetric();
  for (int sub = -1; sub < submet.count(); sub++) {
     map::Object* submapobj = drawObject(sxf, view.obj.data(), view.primitive, lay, sub);
     if (0 != submapobj) {
      int lo, hi;
      sxf::sxfToMapGeneralization(rsc.vis_lo, 15 - rsc.vis_hi, &lo, &hi);
      //      debug_log << rsc.vis_lo << 15 - rsc.vis_hi << lo << hi;
      meteo::Generalization g;
      if (lo != -1) {
        g.setLowLimit(lo);
      }
      if (hi != -1) {
        g.setHighLimit(hi);
      }
      submapobj->setGeneral(g); 
      submapobj->setOriginScale(origScale);
      ++cnt;
    }
  }

  return cnt;
}

//! Отрисовка графического примитива на карте
/*! 
  \param sxf Параметры объекта из sxf
  \param lay Слой, на котором отрисовывать
  \param origScale Исходный масштаб карты
  \return количество отрисованных объектов (с учетом подобъектов)
*/
uint64_t rsc::Document::drawObject(const sxf::Object& sxf, map::Layer* lay, float origScale) const
{
  if (sxf.graphicObject().isNull() || 0 == lay) {
    //error_log << QObject::tr("Не прочитаны свойства объекта");
    return false;
  }
  
  uint64_t cnt = 0;
  const QList<sxf::FullMetric> submet = sxf.submetric();
  for (int sub = -1; sub < submet.count(); sub++) {
    map::Object* submapobj = drawObject(sxf, sxf.graphicObject().data(), sxf.graphicObject()->type, lay, sub);
     if (0 != submapobj) {
      int lo, hi;
      sxf::sxfToMapGeneralization(sxf.header().generalization_level & 0xf, 15 - (sxf.header().generalization_level >> 4), &lo, &hi);
      meteo::Generalization g;
      if (lo != -1) {
        g.setLowLimit(lo);
      }
      if (hi != -1) {
        g.setHighLimit(hi);
      }
      // debug_log << "scale" << lo << hi << sxf.header().generalization_level << (sxf.header().generalization_level & 0xf)
      // 		<< (15 - (sxf.header().generalization_level >> 4));
      submapobj->setGeneral(g);
      submapobj->setOriginScale(origScale);
      ++cnt;
    }
  }

  return cnt;
}


map::Object*  rsc::Document::drawObject(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, uint16_t primitive,
					map::Layer* lay, int sub) const 
{
  if (obj == 0) {
    return 0;
  }
  map::Object* mapobj = 0;

  std::string ornament;
  if (sxf.header().localization == sxf::kLinear) {
    ornament = sxf::MetaSxf::instance()->ornamentName(sxf.header().classification_code);
  } 

  if (ornament.length() != 0) {
    mapobj = drawOrnamentLine(QString::fromStdString(ornament), lay);
  } else {
    //var(primitive);

    switch (primitive) {
    case sxf::AREA_OBJECT: 
      mapobj = drawArea(sxf, obj, lay, sub);
      break;
    case sxf::SOLID_LINE: 
      mapobj = drawSolidLine(sxf, obj, lay);
      break;
    case sxf::DASH_LINE: 
      mapobj = drawDashLine(sxf, obj, lay);
      break;
    case sxf::USER_LINE:
    case sxf::SECTIONS_LINE:
      mapobj = drawDummyLine(sxf, obj, lay);
      break;
    case sxf::OBJECTS_SET:
      mapobj = drawObjectsSet(sxf, obj, lay);
      break;
    case sxf::AREA_SHADE:
      mapobj = drawAreaShade(sxf, obj, lay);
      break;
    case sxf::TEXT_OBJECT:
      mapobj = drawText(sxf, obj, lay, sub);
      break;
    case sxf::AREA_GLASS:
      mapobj = drawAreaGlass(sxf, obj, lay);
      break;
    case sxf::GLASS_LINE:
      mapobj = drawGlassLine(sxf, obj, lay);
      break;
    case sxf::CUSTOMTEXT_OBJECT:
      mapobj = drawCustomText(sxf, obj, lay, sub);
      break;
    case sxf::VECTOR_SIGN:
      mapobj = drawVectorSign(sxf, obj, lay);
      break;
    default: 
      warning_log << QObject::tr("Примитив %1 не поддерживается").arg(primitive);
    }
  }

  if (0 != mapobj) {
    GeoVector gv;
    if (sub == -1) {
      const QList<sxf::Metric>& metr = sxf.metric();
      for (int idx = 0; idx < metr.count(); idx++) {
	//debug_log << idx << MnMath::rad2deg(metr.at(idx).x) <<  MnMath::rad2deg(metr.at(idx).y);
	gv.append(GeoPoint(metr.at(idx).x, metr.at(idx).y, metr.at(idx).h));
	if (primitive == sxf::TEXT_OBJECT || mapobj->type() == kWind) {
	  break;
	}
      }
    } else if (sub < sxf.submetric().count()) {
      const QList<sxf::Metric>& metr = sxf.submetric().at(sub).coords;
      for (int idx = 0; idx < metr.count(); idx++) {
	gv.append(GeoPoint(metr.at(idx).x, metr.at(idx).y, metr.at(idx).h));
	if (primitive == sxf::TEXT_OBJECT || mapobj->type() == kWind) {
	  break;
	}
      }
    }

    if (sxf.isSpline()) {
      mapobj->setSplineFactor(10);
    }
    mapobj->setSkelet(gv);

   } else {
    //error_log << QObject::tr("Объект карты не сформирован") << primitive;
  }
  
  //TODO признак сжатия и масштабирования

  return mapobj;  
}

//! Линия с орнаментом
map::GeoPolygon* rsc::Document::drawOrnamentLine(const QString& ornament, map::Layer* lay) const
{
  if (lay == 0) return 0;
  
  map::GeoPolygon* gp = new map::GeoPolygon(lay);
  gp->setOrnament(ornament);

  QPen pen;
  QMap<QString, meteo::map::proto::Ornament> ornaments = meteo::map::WeatherLoader::instance()->ornamentlibrary();
  foreach (const QString& name, ornaments.keys()) {
    if (name == ornament) {
      if ( name.contains("cold") || name.contains("inactive") ) {
	pen.setColor(QColor(0,50,220,255));
      }
      if ( name.contains("warm")      ) { pen.setColor(QColor(200,0,0,255)); }
      if ( name.contains("occlusion") ) { pen.setColor(QColor(100,50,30,255)); }
      if ( name.contains("coldsec")  )  { pen.setStyle(Qt::DashLine); }
      
      break;
    }
  }
  gp->setPen(pen);

  return gp;
}

//! Площадной объект, примтив 135
map::GeoPolygon* rsc::Document::drawArea(const sxf::Object& /*sxf*/, sxf::GraphicPrimitive* obj, map::Layer* lay, int sub) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::AreaGraphicObject* gobj = static_cast<sxf::AreaGraphicObject*>(obj);
  if (gobj == 0) {
    error_log << "error";
    return 0;
  }
  
  map::GeoPolygon* gp = new map::GeoPolygon(lay);
  
  gp->setClosed(true);
  QBrush brush = gp->qbrush();
  QPen pen = gp->qpen();
  brush.setStyle(Qt::SolidPattern);

  if (sub == -1 || lay->document() == 0) {
    if ((gobj->color & 0xff000000) == 0xf0000000) {
      brush.setColor(pal_.rgb[gobj->color & 0xff]);
      pen.setColor(pal_.rgb[gobj->color & 0xff]);
    } else {
      QColor c(gobj->color & 0xff, (gobj->color >> 8) & 0xff, (gobj->color >> 16) & 0xff, (gobj->color >> 24) & 0xff);
      brush.setColor(c);
      pen.setColor(c);
    }
  } else {
    brush.setColor(lay->document()->backgroundColor());
    pen.setColor(lay->document()->backgroundColor());
  }
  gp->setBrush(brush);
  gp->setPen(pen);

  // gp->property().pen.setColor(0xff00ff);

  return gp;
}

//! Заштрихованная площадь, примтив 153
map::GeoPolygon* rsc::Document::drawAreaShade(const sxf::Object& /*sxf*/, sxf::GraphicPrimitive* obj, map::Layer* lay) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::AreaShadingObject* gobj = static_cast<sxf::AreaShadingObject*>(obj);
  if (gobj == 0 || gobj->line.isNull()) {
    error_log << "error";
    return 0;
  }
  
  map::GeoPolygon* gp = new map::GeoPolygon(lay);
  
  gp->setClosed(true);
  QBrush brush = gp->qbrush();
  if (gobj->angle < 22.5) {
    brush.setStyle(Qt::HorPattern);
  } else if (gobj->angle < 67.5) {
    brush.setStyle(Qt::BDiagPattern);
  } else if (gobj->angle < 115.5) {
    brush.setStyle(Qt::VerPattern);
  } else if (gobj->angle < 157.5) {
    brush.setStyle(Qt::FDiagPattern);
  } else {
    brush.setStyle(Qt::HorPattern);
  }

  uint32_t color = static_cast<sxf::LineGraphicObject*>(gobj->line.data())->color;

  if ((color & 0xff000000) == 0xf0000000) {
    brush.setColor(pal_.rgb[color & 0xff]);
  } else {
    //QColor c(color & 0xff, (color >> 8) & 0xff, color >> 16);
    QColor c(color & 0xff, (color >> 8) & 0xff, (color >> 16) & 0xff, (color >> 24) & 0xff);
    brush.setColor(c);
  }
  gp->setBrush(brush);
  QPen pen = gp->qpen();
  pen.setStyle(Qt::NoPen);
  gp->setPen(pen);

  return gp;
}

//! Площадной, цветное стекло, примтив 167
map::GeoPolygon* rsc::Document::drawAreaGlass(const sxf::Object& /*sxf*/, sxf::GraphicPrimitive* obj, map::Layer* lay) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::AreaGlassObject* gobj = static_cast<sxf::AreaGlassObject*>(obj);
  if (gobj == 0) {
    error_log << "error";
    return 0;
  }
  
  map::GeoPolygon* gp = new map::GeoPolygon(lay);
  
  gp->setClosed(true);
  QBrush brush = gp->qbrush();
  QPen pen = gp->qpen();
  brush.setStyle(Qt::SolidPattern);
  QColor color;
  if ((gobj->color & 0xff000000) == 0xf0000000) {
    color = pal_.rgb[gobj->color & 0xff];
  } else {
    color = QColor(gobj->color & 0xff, (gobj->color >> 8) & 0xff, gobj->color >> 16);
  }

  color.setAlpha(gobj->transp*2.55);
  brush.setColor(color);
  pen.setColor(color);
  gp->setBrush(brush);
  gp->setPen(pen);

  return gp;
}

//! Сплошная линия, примитив 128
map::GeoPolygon* rsc::Document::drawSolidLine(const sxf::Object& /*sxf*/, sxf::GraphicPrimitive* obj, map::Layer* lay) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::LineGraphicObject* gobj = static_cast<sxf::LineGraphicObject*>(obj);
  if (gobj == 0) {
    error_log << "error";
    return 0;
  }
  
  map::GeoPolygon* gp = new map::GeoPolygon(lay);
 
  QBrush brush = gp->qbrush();
  QPen pen = gp->qpen();

  brush.setStyle(Qt::NoBrush);
  pen.setWidth(gobj->width/ pixScale_);
  if ((gobj->color & 0xff000000) == 0xf0000000) {
    pen.setColor(pal_.rgb[gobj->color & 0xff]);
  } else {
    //QColor c(gobj->color & 0xff, (gobj->color >> 8) & 0xff, gobj->color >> 16);
    QColor c(gobj->color & 0xff, (gobj->color >> 8) & 0xff, (gobj->color >> 16) & 0xff, (gobj->color >> 24) & 0xff);
    pen.setColor(c);
  }
  gp->setBrush(brush);
  gp->setPen(pen);

  return gp;
}


//! Пунктирная линия, примитив 129
map::GeoPolygon* rsc::Document::drawDashLine(const sxf::Object& /*sxf*/, sxf::GraphicPrimitive* obj, map::Layer* lay) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::DashLineGraphicObject* gobj = static_cast<sxf::DashLineGraphicObject*>(obj);
  if (gobj == 0) {
    error_log << "error";
    return 0;
  }
  
  map::GeoPolygon* gp = new map::GeoPolygon(lay);

  QBrush brush = gp->qbrush();
  QPen pen = gp->qpen();
  
  brush.setStyle(Qt::NoBrush);
  pen.setStyle(Qt::DashLine);
  pen.setWidth(gobj->width / pixScale_);
  if ((gobj->color & 0xff000000) == 0xf0000000) {
    pen.setColor(pal_.rgb[gobj->color & 0xff]);
  } else {
    QColor c(gobj->color & 0xff, (gobj->color >> 8) & 0xff, (gobj->color >> 16) & 0xff, (gobj->color >> 24) & 0xff);
    //    QColor c(gobj->color & 0xff, (gobj->color >> 8) & 0xff, gobj->color >> 16);
    pen.setColor(c);
  }
  gp->setBrush(brush);
  gp->setPen(pen);

  return gp;
}

//! Линия, цветное стекло, примитив 168
map::GeoPolygon* rsc::Document::drawGlassLine(const sxf::Object& /*sxf*/, sxf::GraphicPrimitive* obj, map::Layer* lay) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::LineGlassGraphicObject* gobj = static_cast<sxf::LineGlassGraphicObject*>(obj);
  if (gobj == 0) {
    error_log << "error";
    return 0;
  }
  
  map::GeoPolygon* gp = new map::GeoPolygon(lay);

  QBrush brush = gp->qbrush();
  QPen pen = gp->qpen();
  
  brush.setStyle(Qt::NoBrush);
  pen.setWidth((gobj->width <= 4 ? gobj->width : 5) / pixScale_);
  QColor color;
  if ((gobj->color & 0xff000000) == 0xf0000000) {
    color = pal_.rgb[gobj->color & 0xff];
  } else {
    color = QColor(gobj->color & 0xff, (gobj->color >> 8) & 0xff, (gobj->color >> 16) & 0xff, (gobj->color >> 24) & 0xff);
    //    color = QColor(gobj->color & 0xff, (gobj->color >> 8) & 0xff, gobj->color >> 16);
  }

  color.setAlpha(gobj->transp*2.55);
  pen.setColor(color);
  gp->setBrush(brush);
  gp->setPen(pen);

  return gp;
}


//! Наборная линия, примитив 158 (рисуется пунктир)
map::GeoPolygon* rsc::Document::drawDummyLine(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, map::Layer* lay) const
{
  Q_UNUSED(obj);

  if (lay == 0) return 0;

  map::GeoPolygon* gp = new map::GeoPolygon(lay);

  QBrush brush = gp->qbrush();
  QPen pen = gp->qpen();
  
  brush.setStyle(Qt::NoBrush);
  
  switch (sxf.header().classification_code) {
  case 31431110:
    pen.setStyle(Qt::DashLine);
    pen.setColor(QColor(123, 179, 224));
    break;
  case 79811000:
    pen.setStyle(Qt::DashDotLine);
    pen.setColor(Qt::gray);
    break;
  case 81140000:
    pen.setStyle(Qt::DashDotLine);
    pen.setColor(Qt::black);
    break;
  };

  gp->setBrush(brush);
  gp->setPen(pen);

  return gp;
}

//! Набор примитивов, примитив 147
map::Object* rsc::Document::drawObjectsSet(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, map::Layer* lay) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::SetOfObjects* gobj = static_cast<sxf::SetOfObjects*>(obj);
  if (gobj == 0 || gobj->obj.count() == 0) {
    error_log << "error";
    return 0;
  }
  
  map::GeoGroup* parent = new map::GeoGroup(lay);
  map::Object* gp = 0;

  for (int i = 0; i < gobj->obj.count(); i++) {
    if (!gobj->obj.at(i).isNull()) {
      gp = drawObject(sxf, gobj->obj.at(i).data(), gobj->obj.at(i)->type, lay, -1);
      if (0 != gp) {
	parent->add(gp);
      }
    }
  }

  return parent;
}

//! Тескт из классификатора, 142
map::Object* rsc::Document::drawText(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, map::Layer* lay, int sub) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::TextGraphicObject* gobj = static_cast<sxf::TextGraphicObject*>(obj);
  if (gobj == 0) {
    error_log << "error";
    return 0;
  }
  
  map::GeoText* gp = new map::GeoText(lay);
  if (0 != gp) {
    fillTextProp(gp, gobj);

    if (sxf.isScale()) {
      gp->setTextScale(kScaleUp);
      if (!sxf.isContract()) {
	gp->setTextScale(kScaleBoth);
      }
    } else if (!sxf.isContract()) {
      gp->setTextScale(kScaleDown);
    }    
    
    if (sub == -1) {
      gp->setText(sxf.label());
    } else if (sub < sxf.submetric().count()) {
      if (sxf.submetric().at(sub).label.hasLabel_) {
	gp->setText(sxf.submetric().at(sub).label.text);
      }
    }

  }

  return gp;
}

//! Тескт с шрифтом пользователя, 152
map::Object* rsc::Document::drawCustomText(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, map::Layer* lay, int sub) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::CustomTextGraphicObject* gobj = static_cast<sxf::CustomTextGraphicObject*>(obj);
  if (gobj == 0) {
    //error_log << "error";
    return 0;
  }

  map::GeoText* gp = new map::GeoText(lay);
  if (0 != gp) {
    fillTextProp(gp, gobj, QString(gobj->font_name));
    if (sub == -1) {
      gp->setText(sxf.label());
    } else if (sub < sxf.submetric().count()) {
      if (sxf.submetric().at(sub).label.hasLabel_) {
	gp->setText(sxf.submetric().at(sub).label.text);
      }
    }

    //scaleflag
  }

  return gp;
}


//! Векторный знак (только пуансон)
map::Object* rsc::Document::drawVectorSign(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, map::Layer* lay) const
{
  if (obj == 0 || lay == 0) return 0;

  sxf::VectorSign* vsign = static_cast<sxf::VectorSign*>(obj);
  if (vsign == 0) {
    //error_log << "error";
    return 0;
  }

  //ветер игнорируем, строим по семантике
  if (sxf.header().classification_code != 0) {
    return 0;
  }
  
  //пуансон, ветер по семантике
  QString puanCode;
  int dd = -1;
  int ff = -1;
  TMeteoData md;

  for (int idx = 0; idx < sxf.semantic().length(); idx++) {
    //debug_log << "sem" << idx << sxf.semantic().at(idx).code << sxf.semantic().at(idx).type << sxf.semantic().at(idx).value;
    if (sxf.semantic().at(idx).code == meteo::sxf::kNameSemantic) {
      if (sxf.semantic().at(idx).value.contains("Puanson")) {
	puanCode = sxf.semantic().at(idx).value.section(' ', 1);
      }
    } else if (sxf.semantic().at(idx).code == meteo::sxf::kddSemantic) {
      dd = sxf.semantic().at(idx).value.toInt();
    } else if (sxf.semantic().at(idx).code == meteo::sxf::kffSemantic) {
      ff = sxf.semantic().at(idx).value.toInt();
    } else if (sxf.semantic().at(idx).code == meteo::sxf::kMeteoSemantic) {
      QStringList vals = sxf.semantic().at(idx).value.split(';');
      if (vals.size() == 4) {
	md.add(vals.at(0), TMeteoParam(vals.at(2), vals.at(1).toFloat(), control::QualityControl(vals.at(3).toInt())));
      }
    }
  }

  //var(puanCode);

  if (puanCode.isEmpty()) {
    return 0;
  }
  else {
    //    debug_log << "puanson text";
    map::Puanson* parent = new map::Puanson(lay);
    const QMap< QString, meteo::puanson::proto::Puanson >& plib = meteo::map::WeatherLoader::instance()->punchlibrary();
    if (plib.contains(puanCode)) {
      parent->setPunch(meteo::map::WeatherLoader::instance()->punchlibrary().value(puanCode));
    } else {
      parent->setPunch(meteo::map::WeatherLoader::instance()->punchlibraryspecial().value(puanCode));
    }
    parent->setMeteodata(md);

    if (dd != -1 && ff != -1)  {
      //debug_log << "wind" << dd << ff;
      parent->setDdff(dd, ff);
    }

    return parent;
  }
  
  return 0;
}

//! Заполнение параметров текста
void rsc::Document::fillTextProp(map::GeoText* gp, const sxf::TextGraphicObject* gobj, const QString& fontname /*= QString()*/) const
{
  if (0 == gp || 0 == gobj) {
    return;
  }

  QPen pen = gp->qpen();
  if ((gobj->color & 0xff000000) == 0xf0000000) {
    pen.setColor(pal_.rgb[gobj->color & 0xff]);
  } else {
    QColor c(gobj->color & 0xff, (gobj->color >> 8) & 0xff, (gobj->color >> 16) & 0xff, (gobj->color >> 24) & 0xff);
    //    QColor c(gobj->color & 0xff, (gobj->color >> 8) & 0xff, gobj->color >> 16);
    pen.setColor(c);
  }
  gp->setPen(pen);
  gp->setOutlineColor(gobj->background_color);
  //    var(gp->property().outlinecolor);
  //background_color
  //shadow_color
  
  QFont font = gp->qfont();
  font.setPixelSize(gobj->height / pixScale_);
  int pntSize = QFontInfo(font).pointSize();
  font.setPointSize(pntSize);
  
  //weight
  //align
  //wide
  //horisontal
  font.setItalic(gobj->italic);
  font.setUnderline(gobj->underline);
  font.setStrikeOut(gobj->strikeline);
  QString family;
  if (!fontname.isEmpty()) {
    family = fontname;
  } else if (gobj->fontcode > 0) {
    family = fonts_.value(gobj->fontcode).name;
  } else {
    family = fonts_.value(1).name;
  }

  if (!family.isEmpty()) {
    font.setFamily(fonts_.value(gobj->fontcode).name);
  }
  //  debug_log << "font" << family << fontname << gobj->fontcode << fonts_.value(gobj->fontcode).name;

  gp->setFont(font);
}

double rsc::Document::calcAzimuthFromNorth(const GeoPoint& geo1, const GeoPoint& geo2) const
{
  // debug_log << MnMath::rad2deg(geo1.lat()) << MnMath::rad2deg(geo1.lon()) << "\n" 
  // 	    << MnMath::rad2deg(geo2.lat()) << MnMath::rad2deg(geo2.lon()) << "\n";

  //третья точка треугольника (geo2.lat(), geo1.lon())

  double a = fabs(geo2.lat() - geo1.lat());
  double b;
  if (geo2.lon() < 0 && geo1.lon() > 0) {
    b = 2*M_PI + geo2.lon() - geo1.lon();
  } else if (geo2.lon() > 0 && geo1.lon() < 0) {
    b = 2*M_PI + geo1.lon() - geo2.lon();
  } else {
    b = fabs(geo2.lon() - geo1.lon());
  }

  double c = acos(cos(a)*cos(b)); //теорема косинусов, угол C = 90
  double B = 0;
  if (c != 0) {
    B = asin(sin(b) / sin(c)); //теорема синусов
  }

  //  debug_log << B << MnMath::rad2deg(B);

  if (geo2.lat() < geo1.lat()) {
    if (geo2.lon() < geo1.lon() || (geo2.lon() > 0 && geo1.lon() < 0)) {
      B += M_PI;
    } else {
      B = M_PI - B;
    }
  } else if (geo2.lon() < geo1.lon() || (geo2.lon() > 0 && geo1.lon() < 0)) {
    B = 2*M_PI - B;
  }
  
  //debug_log << a << b << c << B << MnMath::rad2deg(B);

  return B;
}
