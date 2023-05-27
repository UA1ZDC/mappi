#include "tgribparser.h"

#include <math.h>

#include <qdom.h>
#include <qfile.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <cross-commons/debug/tlog.h>

#include "tgribformat.h"

using namespace google::protobuf;
using namespace grib;

//---
// void printData(const uchar* data, uint64_t /*size*/)
// {
//   printf("Print data\n");
//   int startIdx = 0;

//   for (unsigned i=0; i< 10; i++) {
//     printf("%2d: %2x %3d\n", i+startIdx, data[i], data[i]);
//   }
// }
//---

TGribParser::TGribParser():_data(0), _size(0), _msg(0), _templ(0xff),_version(-1)
{
}

TGribParser::~TGribParser()
{
}

//! создание по шаблону templ и заполнение данными из строки src
/*! 
  \param templ   шаблон
  \param version версия GRIB
  \param src     сериализованная строка с данными
*/
bool TGribParser::createFromString(uint16_t templ, int version, const std::string& src)
{
  google::protobuf::Message* msg = createDefinition(templ, version);
  if (0 == msg) return false;
  msg->ParseFromString(src);
  return true;
}

//! Учтановка параметров парсера
/*! 
  \param data данные
  \param size размер
  \param file файл с правилами для парсинга в соответствии с номером шаблона
  \param version версия GRIB
  \param templ номер шаблона
*/
void TGribParser::setup(const uchar* data, uint64_t size, const QString& file, short version, uint16_t templ)
{
  _data = data;
  _size = size;
  _version = version;
  _templ = templ;
  _file = file;
}

//! Парсинг
/*! 
  \return false в случае ошибки, иначе - true
*/
bool TGribParser::parse()
{
  if (!_data || _size == 0) return true;

  //  printData(_data, _size);

  bool ok = parseTemplate();

  // if (ok) {
  //   printvar(_templ);
  //   printvar(QString::fromStdString(_msg->DebugString()));
  // }

  return ok;
}

//! Парсинг
/*! 
  \return false в случае ошибки, иначе - true
*/
bool TGribParser::parseTemplate()
{
  QDomNode node = setContent(_file, _templ);
  if (node.isNull()) {
    error_log<<QObject::tr("В %1 нет описания для template=%2").arg(_file).arg(_templ);
    return false;
  }

  _msg = createDefinition(_templ, _version);
  if (_msg == 0) {
    error_log<<QObject::tr("Нет объекта для хранения описания данных для template=")<<_templ;
    return false;
  }

  return parse(node);
}

//!  Парсинг
/*! 
  \param node набор правил
  \return false в случае ошибки, иначе - true
*/
bool TGribParser::parse(QDomNode& node)
{
  int startIdx = node.toElement().attribute("startIdx").toInt();

  parseMessage(node, _msg, -startIdx);

  return true;
}

//! Для полей, которым нужна "ручная" обработка. Реализация в конкретных классах, здесь реализация, чтоб не пропустить ошибку
void TGribParser::parseCustom(google::protobuf::Message* /*msg*/, const uint8_t* /*data*/, uint32_t /*dataSize*/, QDomElement& node, int32_t /*startIdx*/)
{
  warning_log<<"Not realised parsing for"<<node.attribute("name");
}

//! Парсинг
/*! 
  \param node набор правил
  \param sub  структура для заполнения
  \param startIdx начальное смещение в массиве исходных данных
*/
void TGribParser::parseMessage(QDomNode& node, google::protobuf::Message* sub, int startIdx)
{
  const Reflection * refl = sub->GetReflection();
  
  for (QDomNode nList = node.firstChild(); !nList.isNull(); nList = nList.nextSibling()) {
    if (nList.nodeName() != "val") continue;
   
    QDomElement el = nList.toElement();
    uint32_t idxoffset = 0;
    if (el.hasAttribute("offset_pbnum") && el.hasAttribute("offset_size")) {
      idxoffset = el.attribute("offset_size").toUInt() * refl->GetUInt32(*sub, sub->GetDescriptor()->FindFieldByNumber(el.attribute("offset_pbnum").toUInt()));
    }

    if (el.attribute("type") == "custom") {
      parseCustom(sub, _data, _size, el, startIdx + idxoffset);
    } else if (el.attribute("type") == "scaled") {
      int number = el.attribute("pbnum").toUInt();
      float val = calcValueFactor(_data, el, startIdx + idxoffset);
      refl->SetFloat(sub, sub->GetDescriptor()->FindFieldByNumber(number), val);
    } else {
      int idx = el.attribute("idx").toUInt() + startIdx + idxoffset;
      int size = el.attribute("size").toUInt();
      int number = el.attribute("pbnum").toUInt();

      const FieldDescriptor * field = sub->GetDescriptor()->FindFieldByNumber(number);
      
      if (el.attribute("field") == "repeated") {
	fillRepeated(sub, el, refl, startIdx);
      } else {
	switch (field->cpp_type()) {
	case FieldDescriptor::CPPTYPE_UINT32: {
	  uint32_t val=0;
	  grib::char2dec(_data + idx, size, &val);
	  refl->SetUInt32(sub, field, val);
	} 
	  break;
	case FieldDescriptor::CPPTYPE_INT32: {
	  int32_t val=0;
	  bool allOnes = false;
	  grib::char2dec(_data + idx, size, &val, true, &allOnes);
	  if (allOnes) {
	    val = std::numeric_limits<int32_t>::max();
	  }
	  refl->SetInt32(sub, field, val);
	} 
	  break;
	case FieldDescriptor::CPPTYPE_BOOL: {
	  refl->SetBool(sub, field, _data + idx);
	} 
	  break;
	case FieldDescriptor::CPPTYPE_FLOAT: {
	  float val = grib::ieee2double(_data + idx, _version);
	  refl->SetFloat(sub, field, val);
	}
	  break;
	case FieldDescriptor::CPPTYPE_MESSAGE: {
	  Message *msg = refl->MutableMessage(sub, field);
	  parseMessage(el, msg, startIdx + idxoffset);
	}
	  break;
	default: {
	  error_log<<"Parsing ptotobuf type="<<field->cpp_type()<<"for attribute ="<<el.attribute("name")<<"not realised";
	}
	}
      }
    }
  }
}


//! Заполнение параметра, являющегося массивом данных
/*! 
  \param msg сруктура, параметр которой надо заполнить
  \param el  правило
  \param refl отображение структуры
  \param startIdx начальное смещение в исходном массиве
*/
void TGribParser::fillRepeated(google::protobuf::Message* msg, QDomElement& el, const google::protobuf::Reflection * refl, int startIdx)
{
  int idx = el.attribute("idx").toUInt() + startIdx;
  uint32_t cnt = 0;
  int size = el.attribute("size").toUInt();
  int number = el.attribute("pbnum").toUInt();

  if (el.hasAttribute("cnt_pbnum")) {
    cnt = refl->GetUInt32(*msg, msg->GetDescriptor()->FindFieldByNumber(el.attribute("cnt_pbnum").toUInt()));
    if (el.hasAttribute("offset_pbnum") && el.hasAttribute("offset_size")) {
      uint32_t idxoffset = refl->GetUInt32(*msg, msg->GetDescriptor()->FindFieldByNumber(el.attribute("offset_pbnum").toUInt()));
      idx += idxoffset * el.attribute("offset_size").toUInt();
    }
  } else {
    cnt = (_size - idx)/size;
  }

  const FieldDescriptor * field = msg->GetDescriptor()->FindFieldByNumber(number);
  for (uint32_t i =0; i< cnt; i++) {
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_UINT32) {//TODO другие типы
      uint32_t val;
      grib::char2dec(_data + idx, size, &val);
      refl->AddUInt32(msg, field, val);
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_INT32) {
      int32_t val;
      grib::char2dec(_data + idx, size, &val, true);
      refl->AddInt32(msg, field, val);
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      Message *sub = refl->AddMessage(msg, field);
      parseMessage(el, sub, startIdx);
    } else {
      error_log<<"Parsing ptotobuf type="<<field->cpp_type()<<"for attribute ="<<el.attribute("name")<<"not realised";
      break;
    }
    idx += size;
  }
}

//! Чтение из xml правил
/*! 
  \param fileName файл с правилами
  \param templ номер шаблона
  \return правила
*/
QDomNode TGribParser::setContent(const QString& fileName, uint16_t templ)
{
  QDomNode node;

  QFile file(fileName);
  if ( ! file.open( QIODevice::ReadOnly ) ) {
    error_log << QObject::tr("Ошибка открытия файла %1 для чтения").arg(fileName);
    return node;
  }

  QDomDocument* doc = new QDomDocument;

  if (doc->setContent(&file)) {
    node = findXmlTemplate(doc->documentElement(), QString::number(templ));
  }

  delete doc;
  file.close();

  return node;
}

//! Поиск правила по шаблону
/*! 
  \param el элемент со всей структурой xml
  \param attrVal номер шаблона
  \return структура с правилами
*/
QDomNode TGribParser::findXmlTemplate(const QDomElement& el, const QString& attrVal)
{
  QDomNode node = el.firstChild();

  while ( !node.isNull() ) {
    QString val = node.toElement().attribute( "template" );
    if (val == attrVal) break;
    
    node = node.nextSibling();
  }
  
  return node;
}

//! Вычисление значения масштабированной величины
/*! 
  \param data исходный массив с данными
  \param child правило для получения значения
  \param startIdx начальное смещение
  \return вычисленное значение
*/
float TGribParser::calcValueFactor(const uchar* data, const QDomElement& child, int32_t startIdx)
{
  int idx = child.attribute("factor_idx").toInt() + startIdx;
  if (idx < 0) return 0;

  uchar f = data[idx];

  uint32_t val = 0;
  int size = child.attribute("value_size").toInt();

  idx = child.attribute("value_idx").toInt() + startIdx;
  if (idx < 0) return 0;

  grib::char2dec(data + idx, size, &val);

  return val*pow(10, -f);
}
