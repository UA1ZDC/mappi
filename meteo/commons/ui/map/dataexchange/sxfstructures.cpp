#include "sxfstructures.h"
#include "exchange_algorithms.h"
#include "passportbasis.h"
#include "metasxf.h"

#include <cross-commons/debug/tlog.h>
#include <cross-commons/debug/tmap.h>
#include <meteo/commons/ui/map/document.h>


#include <QByteArray>
#include <QDate>

//TODO в sxfcodes.conf
#define LAYNUM 0


namespace {

  //! Соответствие масштаба изображения уровню генерализации
  static const TMap<int, int> generalizLevels = TMap<int, int>()
    // << QPair<int,int>(0, 0)
    << QPair<int,int>(500, 0)
    << QPair<int,int>(1000, 1)
    << QPair<int,int>(2000, 2)
    << QPair<int,int>(5000, 3)
    << QPair<int,int>(10000, 4)
    << QPair<int,int>(25000, 5)
    << QPair<int,int>(50000, 6)
    << QPair<int,int>(100000, 7)
    << QPair<int,int>(200000, 8)
    << QPair<int,int>(500000, 9)
    << QPair<int,int>(1000000, 10)
    << QPair<int,int>(2000000, 11)
    << QPair<int,int>(5000000, 12)
    << QPair<int,int>(10000000, 13)
    << QPair<int,int>(20000000, 14)
    << QPair<int,int>(40000000, 15);

  //TODO в файл
  // static const TMap<int, float> scaleMatching = TMap<int, float>()
  //     << QPair<int, float>(100, 0)
  //     << QPair<int, float>(200, 0)
  //     << QPair<int, float>(500, 1)
  //     << QPair<int, float>(1000, 1.4)
  //     << QPair<int, float>(2000, 2.2)
  //     << QPair<int, float>(2500, 2.6)
  //     << QPair<int, float>(5000, 2.8)
  //     << QPair<int, float>(10000, 3.2)
  //     << QPair<int, float>(25000, 4.4)
  //     << QPair<int, float>(50000, 5.6)
  //     << QPair<int, float>(100000, 6.4)
  //     << QPair<int, float>(200000, 7.6)
  //     << QPair<int, float>(500000, 8.4)
  //     << QPair<int, float>(1000000, 9.6)
  //     << QPair<int, float>(2000000, 11)
  //     << QPair<int, float>(5000000, 12)
  //     << QPair<int, float>(10000000, 13)
  //     << QPair<int, float>(20000000, 14)
  //     << QPair<int, float>(40000000, 17.2)
  //     << QPair<int, float>(80000000, 17.2)
  //     << QPair<int, float>(160000000, 17.2)
  //     << QPair<int, float>(320000000, 17.2);

  int blobSize() { return (2 << 16)-1; }

  int32_t passportInfoFlags()
  {
    int32_t result = 0;
    result |= 0x00000003;  //!< Флаг состояния данных (Состояние 3 - формат обмена данными)
    result |= 0x00000018;  //!< Флаг наличия реальных координат (Вся метрика объектов представлена в реальных координатах)
    result |= 0x00000100;  //!< Флаг кодировки подписи (ANSI Windows)
    result |= 0x00010000;  //!< Флаг точности координат (повышенная точность хранения координат)
    return result;
  }

  int32_t descriptionInfoFlags()
  {
    int32_t result = 0;
    result |= 0x00000003;  //!< Флаг состояния данных (Состояние 3 - формат обмена данными)
    result |= 0x00000018;  //!< Флаг наличия реальных координат (Вся метрика объектов представлена в реальных координатах)
    result |= 0x00000100;  //!< Флаг кодировки подписи (ANSI Windows)
    return result;
  }

  int8_t objectHeaderRecordTags()
  {
    int8_t result = 0;
    result |= 0x01;        //!< 1 - не сжимать графический объект (в документации наобот)
    result |= 0x02;        //!< Наличие семантики
    result |= 0x04;        //!< Флаг указания размера элемента метрики (= 8 байт)
    //    result |= 0x10;        //!< Признак текста в UNICODE (UTF-16)
    return result;
  }

  int8_t objectHeaderMetricTags()
  {
    int8_t result = 0;
    result |= 0x02;        //!< Размерность представления метрики (метрика в имеет трёхмерное представление)
    result |= 0x04;        //!< Тип элемента метрики (представление с плавающей точкой)
    return result;
  }

  int8_t objectGeneralizationLevel()
  {
    int8_t result = 0xf;
    return result;
  }
}

namespace meteo {

using namespace dataexchange;

namespace sxf {

const int Object::Header::length = 32;

const int Passport::length = 400;
const int Document::Description::length = 52;

const int Passport::checksum_offset = 12;
const int Passport::checksum_length = 4;
static const uchar kSxfObjectIdentification[] = {0xff, 0x7f, 0xff, 0x7f};

  //! генерализация из формата sxf в значения карты
  void sxfToMapGeneralization(uint8_t sxfLow, uint8_t sxfHi, int* lowLimit, int* highLimit)
  {
    if (sxfLow >= 15 || sxfLow == 0) {
      *lowLimit = -1;
    } else {
      *lowLimit = sxf::MetaSxf::instance()->mapScale(generalizLevels.key(sxfLow));
      //      *lowLimit = scaleMatching.value(generalizLevels.key(sxfLow));
    }
    
    if (sxfHi >= 15 || sxfHi == 0) {
      *highLimit = -1;
    } else {
      *highLimit = sxf::MetaSxf::instance()->mapScale(generalizLevels.key(sxfHi));
      //*highLimit = scaleMatching.value(generalizLevels.key(sxfHi));
    }
  }

const Metric Object::metricFromGeoPoint(const GeoPoint& gp)
{
  Metric result;
  if (gp.isValid() == true) {
    result.x = gp.lat();
    result.y = gp.lon();
    result.h = gp.alt();
  }
  return result;
}

bool Object::hasMetric() const
{
  return !metric_.isEmpty();
}

void Object::setSpline(bool has_spline)
{
  header_.metric_tags &= 0x3F;   //!< Сбросить признак построения сплайна по метрике
  if (has_spline == true) {
    header_.metric_tags |= 0x80; //!< Установить признак построения сплайна (сплайн проходит через все точки метрики)
  }
}

//! true - установить признак сжатия графики; false - сбросить
void Object::setContract(bool has_contract)
{
  header_.record_tags &= 0xfe;
  if (has_contract) {
    header_.record_tags |= 0x1;
  }
}

bool Object::isContract() const
{
  return 0 != (header_.record_tags & 0x1);
}

void Object::setScale()
{
  header_.metric_tags |= 0x30; 
}

bool Object::isScale() const
{
  return 0 != (header_.metric_tags & 0x30);
}

void Object::addMetric(const Metric& mtr)
{
  metric_.append(mtr);
}

void Object::addMetric(const QList<Metric>& mtr)
{
  metric_.append(mtr);
}

void Object::addSemantic(const Semantic& smt)
{
  semantic_.append(smt);
}

void Object::addSemantic(const QList<Semantic>& smt)
{
  semantic_.append(smt);
}

void Object::addGraphicPrimitive(GraphicsType type, const QColor& color, int width, int dash_len, int space_len)
{
  graphic_.clear();

  int red = color.red(),
    green = color.green(),
    blue = color.blue(),
    alpha = color.alpha();
  QByteArray rgb;
  rgb.reserve(sizeof(int32_t));
  rgb += serializeValue(static_cast<int8_t>(red));
  rgb += serializeValue(static_cast<int8_t>(green));
  rgb += serializeValue(static_cast<int8_t>(blue));
  rgb += serializeValue(static_cast<int8_t>(alpha));
  int32_t c = 0;
  parseFromByteArray(rgb, c);
  switch (type) {
    case SOLID_LINE: {
        LineGraphicObject* gobj = new LineGraphicObject();
        gobj->color = c;
        if (width > 0) {
          gobj->width = static_cast<int32_t>(width);
        }
        graphic_ = QSharedPointer<GraphicPrimitive>(gobj);
      }
      break;
    case DASH_LINE: {
        DashLineGraphicObject* gobj = new DashLineGraphicObject();
        gobj->color = c;
        if (width > 0) {
          gobj->width = static_cast<int32_t>(width);
        }
        if (dash_len > 0) {
          gobj->dash_length = static_cast<int32_t>(dash_len);
        }
        if (space_len > 0) {
          gobj->space_length = static_cast<int32_t>(space_len);
        }
        graphic_ = QSharedPointer<GraphicPrimitive>(gobj);
      }
      break;
    case AREA_OBJECT: {
        AreaGraphicObject* gobj = new AreaGraphicObject();
        gobj->color = c;
        graphic_ = QSharedPointer<GraphicPrimitive>(gobj);
      }
      break;
    default:
      error_log << QString::fromUtf8("Ошибка. Не определён тип графического примитива");
  }

  if (graphic_.isNull() == false) {
    header_.setHasGraphicFlag();
  }

  if (alpha != 255) {
    addSemantic(sxf::Semantic(QString::number(alpha), meteo::sxf::kAlphaSemantic, 2));
  }

}

void Object::addTextPrimitive(const QString& text, const QFont& font, const QColor& color, int pixScale)
{
  graphic_.clear();

  CustomTextGraphicObject* gobj = new CustomTextGraphicObject();
  gobj->color = (color.alpha() << 24) + ((color.blue() & 0xff) << 16) + (color.green() << 8) + (color.red() & 0xff);
  gobj->height = (QFontInfo(font).pixelSize() - 1) * pixScale;
  if (font.weight() == QFont::Bold) {
    gobj->weight = 188; //TODO что-то ещё надо
  }
  gobj->italic = font.italic();
  gobj->underline = font.underline();
  gobj->strikeline = font.strikeOut();
  strncpy(gobj->font_name, font.family().toLocal8Bit().data(), sizeof(gobj->font_name));
  
  graphic_ = QSharedPointer<GraphicPrimitive>(gobj);
  
 if (graphic_.isNull() == false) {
    header_.setHasGraphicFlag();
    addLabel(text);
  }
}


void Object::addTrueTypeText(int signNum, int fontCode, const QFont& font, const QColor& color, int pixScale)
{
  header_.localization = sxf::kPoint;
  graphic_.clear();

  TrueTypeSign* gobj = new TrueTypeSign();
  //  gobj->color = (((color.rgb() & 0xff) << 16) + (color.rgb() & 0xff00) + ((color.rgb() >> 16) & 0xff));
  gobj->color = (color.alpha() << 24) + ((color.blue() & 0xff) << 16) + (color.green() << 8) + (color.red() & 0xff);
  gobj->height = QFontInfo(font).pixelSize() * pixScale;
  if (font.weight() == QFont::Bold) {
    gobj->weight = 188; 
  }

  strncpy(gobj->font_name, font.family().toLocal8Bit().data(), sizeof(gobj->font_name));
  gobj->fontcode = fontCode;

  gobj->sign_num = signNum;
  //var(gobj->sign_num);

  gobj->top_offset = 0;
  gobj->left_offset = 0;
  gobj->bot_offset = gobj->height;
  gobj->right_offset = gobj->height;

  graphic_ = QSharedPointer<GraphicPrimitive>(gobj);
  
 if (graphic_.isNull() == false) {
    header_.setHasGraphicFlag();
  }
}

//! Добавляение текста, шрифт из классификатора с кодом code
void Object::addTextWithClassificator(const QString& text, int32_t code)
{
  addLabel(text);
  setClassificationCode(code, sxf::kLabel);
}

void Object::addWithClassificator(int32_t code, Localization loc)
{
  setClassificationCode(code, loc);
}

sxf::VectorSign* Object::createVectorObject()
{
  VectorSign* gobj = new VectorSign;
  graphic_ = QSharedPointer<GraphicPrimitive>(gobj);
  if (graphic_.isNull() == false) {
    header_.setHasGraphicFlag();
    setClassificationCode(0, sxf::kVector);
  }
  return gobj;
}

void Object::addGraphicPrimitive(const QSharedPointer<GraphicPrimitive>& gobj, Localization loc)
{
  graphic_.clear();
  graphic_ = gobj;
  if (graphic_.isNull() == false) {
    header_.setHasGraphicFlag();
    setClassificationCode(0, loc);
  }
}

  //! преобразрвание генерализации в sxf из значений карты
void Object::setGeneralization(int lowLimit, int highLimit)
{
  int lowLevel = header_.generalization_level & 0xf;
  int highLevel = 15 - (header_.generalization_level >> 4);

  //TODO должен передаваться реальный масштаб
  //а пока так
  Q_UNUSED(lowLimit);

  if (highLimit < 15) {
    highLevel = highLimit;
    lowLevel = lowLimit;
    header_.generalization_level = lowLevel + ((15 - highLevel) << 4);
  } else {
    header_.generalization_level = 0xf;
  }

  // var(lowLevel);
  // var(highLevel);
  return;

  //----
  // if (!generalizLevels.contains(lowLimit)) {
  //   QList<int> keys = generalizLevels.keys();
  //   for (int i = 0; i < keys.count(); i++) {
  //     if (keys.at(i) >= lowLimit) {
  // 	lowLevel = generalizLevels.value(keys.at(i));
  //     }
  //   }
  // }
  // if (!generalizLevels.contains(highLimit)) {
  //   QList<int> keys = generalizLevels.keys();
  //   for (int i = keys.count() - 1; i >= 0; i--) {
  //     if (keys.at(i) < highLimit) {
  // 	highLevel = generalizLevels.value(keys.at(i));
  //     }
  //   }
  // }
  // if (highLevel < lowLevel) {
  //   error_log << QObject::tr("Ошибка задания уровня генерализации");
  //   return;
  // }
  
  // header_.generalization_level = lowLevel + ((15 - highLevel) << 4);
}


void Object::addLabel(const QString& text)
{
  label_.text = text;
  label_.hasLabel_ = true;
}

void Object::addLabel(const QByteArray& data)
{
  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");//TODO кодировка
  addLabel(codec->toUnicode(data.data(), strlen(data.data())));
}

void Object::setClassificationCode(int32_t code, Localization local)
{
  header_.classification_code = code;
  header_.localization &= 0xff00;
  header_.localization |= local;
}

const QByteArray Object::serializeToByteArray(int serialNumber, int groupNumber, Passport* passport, const meteo::ProjectionParams& proj)
{
  QByteArray serializedMetric(serializeMetrics(passport, proj));
  QByteArray serializedSemantic(serializeSemantics());
  if (makeHeader(serialNumber,
                 groupNumber,
                 serializedMetric.size(),
                 serializedSemantic.size()) == false) {
    return QByteArray();
  }

  QByteArray result;
  result += serializeHeader();
  result += serializedMetric;
  result += serializedSemantic;

  return result;
}

bool Object::makeHeader(int serialNumber, int groupNumber, int metricLengthBytes, int semanticLengthBytes)
{
  header_.record_length = static_cast<int32_t>(Header::length
                                               + metricLengthBytes
                                               + semanticLengthBytes);
  header_.metric_length_bytes = static_cast<int32_t>(metricLengthBytes);
  header_.number_in_group = static_cast<int16_t>(serialNumber);
  header_.group_number = static_cast<int16_t>(groupNumber);
  header_.blob_point_count = static_cast<int32_t>(metric_.count());
  header_.metric_point_count = (metric_.count() < ::blobSize()) ? static_cast<int16_t>(metric_.count())
                                                                : static_cast<int16_t>(::blobSize());
  if (label_.hasLabel_ == true) {
    header_.setHasLabelFlag();
  }
  return true;
}

const QByteArray Object::serializeHeader() const
{
  QByteArray result;
  result.reserve(Header::length);

  result += serializeValue(header_.record_begin);
  result += serializeValue(header_.record_length);
  result += serializeValue(header_.metric_length_bytes);
  result += serializeValue(header_.classification_code);
  result += serializeValue(header_.number_in_group);
  result += serializeValue(header_.group_number);
  result += serializeValue(header_.localization);
  result += serializeValue(header_.record_tags);
  result += serializeValue(header_.metric_tags);
  result += serializeValue(header_.generalization_level);
  result += serializeValue(header_.blob_point_count);
  result += serializeValue(header_.subobject_count);
  result += serializeValue(header_.metric_point_count);

  return result;
}

const QByteArray Object::serializeMetrics(Passport* passport, const meteo::ProjectionParams& proj) const
{
  QByteArray result;

  // Сериализация собственных точек метрики:
  QListIterator<Metric> it(metric_);
  while (it.hasNext() == true) {
    const Metric& mtr = it.next();

    if (passport->unit_on_plan == 0) {
      float x, y;
      if (passport->map_projection == 36) {
	meteo::f2xy_merc1sp(proj, mtr.x, mtr.y, &y, &x);
      } else if (passport->map_projection == 23) {
	meteo::f2xy_stereo_polar(proj, mtr.x, mtr.y, &y, &x);
      }
      result += serializeValue(double(x));
      result += serializeValue(double(y));
      result += serializeValue(mtr.h);
    } else {
      result += serializeValue(mtr.x);
      result += serializeValue(mtr.y);
      result += serializeValue(mtr.h);
    }
  }
  // Сериализация подписи:
  if (label_.hasLabel_ == true) {
    result += label_.serializeToByteArray();
  }

  // Сериализация графического описания:
  if (graphic_.isNull() == false) {
    GraphicHeader header(graphic_->primitive_length, 1);
    result += header.serializeToByteArray();
    result += graphic_->serializeToByteArray();
  }

  return result;
}

const QByteArray Object::serializeSemantics() const
{
  QByteArray result;

  QListIterator<Semantic> it(semantic_);
  while (it.hasNext() == true) {
    const Semantic& sm = it.next();
    result += serializeValue(sm.code);
    result += serializeValue(sm.type);
    result += serializeValue(sm.scale_coef);
    //    var(sm.type);
    if (sm.type == 128) {
      result += serializeValue(sm.length);
      result += serializeValue(sm.value.utf16(), sm.length);
    } else if (sm.type == 126) {
      result += QTextCodec::codecForName("Windows-1251")->fromUnicode(sm.value) + '\0';
    } else if (sm.type == 127 || sm.type == 0) {
      result += sm.value.toLatin1() + '\0';
    } else if (sm.type == 8) {
      result += serializeValue(sm.value.toFloat());
    } else {
      result += serializeValue(sm.value.toInt(), sm.type);
    }
    //    var(sm.value);
  }
  //  var(result.size());
  return result;
}

void Object::clear()
{
  metric_.clear();
  submetric_.clear();
  semantic_.clear();
  label_.hasLabel_ = false;
  graphic_.clear();
}

void Object::read(const QByteArray& data, uint64_t poffset, int unit_on_plan, int sxfproj, const meteo::ProjectionParams& proj)
{
  uint64_t offset = poffset;
  clear();

  header_.read(data.mid(poffset, header_.length));
  offset += header_.length;

  bool mlabel = header_.metric_tags & 0x8;
  bool mfloat = header_.metric_tags & 0x4;

  uint64_t graphOffset = 0;
  if (mfloat) {
    graphOffset = readMetric<double>(data.mid(offset, header_.metric_length_bytes),
				     unit_on_plan, sxfproj, proj);
  } else {
    graphOffset = readMetric<int64_t>(data.mid(offset, header_.metric_length_bytes),
				      unit_on_plan, sxfproj, proj);
  }
  
  offset += graphOffset;
  //  offset += header_.metric_length_bytes;

  if (header_.hasGraphicFlag()) {
    readGraphic(data.mid(offset, header_.metric_length_bytes - graphOffset));
  }

  readSemantic(data.mid(poffset + header_.length + header_.metric_length_bytes, 
			header_.record_length - header_.length - header_.metric_length_bytes));

  if (mlabel && label_.text.isEmpty()) {
    for (int i=0; i< semantic_.count(); i++) {
      if (semantic_.at(i).type == 0 || semantic_.at(i).type > 8) {
	label_.text = semantic_.at(i).value;
	break;
      }
    }
  } 
}

//! Заполнение графического объекта
void Object::readGraphic(const QByteArray& data)
{
  if (data.length() < GraphicHeader::length + 4) {
    return;
  }
  GraphicHeader header;
  header.read(data);

  uint16_t length;
  uint16_t primitive;
  QDataStream ds(data.mid(GraphicHeader::length, 4));
  ds.setByteOrder(QDataStream::LittleEndian);
  ds >> length;
  ds >> primitive;

  graphic_ = GraphicPrimitive::readPrimitive(data, data.size(), GraphicHeader::length+4, primitive);
  if (0 != graphic_) {
    graphic_->primitive_length = length;
    graphic_->type = primitive;
  }
}

template<class T> uint64_t Object::readMetric(const QByteArray& data, int unit_on_plan, int sxfproj, const meteo::ProjectionParams& proj)
{
  //чтение метрики
  int cnt = ((uint16_t)header_.metric_point_count == 65535) ? header_.blob_point_count : header_.metric_point_count;
  uint64_t offset = 0;
  int msize = ((header_.record_tags & 0x4) == 0 ) ? 4 : 8; //байт
  bool m3d = header_.metric_tags & 0x2;
  bool mlabel = header_.metric_tags & 0x8;

  for (int idx = 0; idx < cnt; idx++) {
    if (offset >= (uint64_t)data.size()) return 0;
    Metric m;
    offset += m.read<T>(data, offset, msize, m3d);
    m.toGeodetic(unit_on_plan, sxfproj, proj);
 
    metric_.append(m);
  }

  if (mlabel) {
    int len = *(data.data() + offset);
    offset += 1;
    addLabel(data.mid(offset, len));
    offset += len + 1;//замыкающий ноль
  }

  // if (header_.subobject_count != 0/* && mlabel*/) {
  //   var(header_.subobject_count);
  // }
  for (int sub = 0; sub < header_.subobject_count; sub++) {
    if (offset >= (uint64_t)data.size()) return 0;
    uint16_t num;
    uint16_t subcnt;
    QDataStream ds(data.mid(offset, 4));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> num;
    ds >> subcnt;
    offset += 4;

    //var(subcnt);

    FullMetric submetric;
    for (int idx = 0; idx < subcnt; idx++) {
      if (offset >= (uint64_t)data.size()) return 0;
      Metric m;
      offset += m.read<T>(data, offset, msize, m3d);
      m.toGeodetic(unit_on_plan, sxfproj, proj);
	
      submetric.coords.append(m);
    }

    if (mlabel) {
      int len = *(data.data() + offset);
      //var(len);
      offset += 1;
      submetric.addLabel(data.mid(offset, len));
      offset += len + 1;//замыкающий ноль
    }    
    submetric_.append(submetric);    

  }

  return offset;
}


void Object::readSemantic(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  
  //чтение семантики
  uint semLength = ba.size();
  if (semLength > 4) {
    uint curLength = 0;
    while ((curLength + 4) < semLength) {
      Semantic sem;
      ds >> sem.code;
      ds >> sem.type;
      ds >> sem.scale_coef;
      // var(sem.type);
      // var(sem.code);
      // var(sem.scale_coef);

      if (sem.type != 0 && sem.type < 8) {
	sem.length = sem.type;
	if (sem.length != 0) {
	  uint32_t val = 0;
	  ds.readRawData((char*)&val, sem.length);
	  sem.value = QString::number(val * pow(10, sem.scale_coef));
	} 
       	curLength += sem.length + 4;
      } else if (sem.type == 8) {
	float val = 0;
	ds >> val;
	sem.value = QString::number(val);
      } else {
	sem.length = sem.scale_coef;
	if (sem.length != 0) {
	  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");//TODO кодировка
	  sem.value = codec->toUnicode(ba.mid(curLength + 4, sem.length), strlen(ba.mid(curLength + 4, sem.length)));
	}
	curLength += sem.length + 1 + 4;
	ds.skipRawData(sem.length + 1);
      }

      //  debug_log << sem.length << sem.value;
      semantic_.append(sem);
    }
  } else {
    //    debug_log << "sem length = " << semLength;
  }
}

void Object::readSemantic(const QByteArray& ba, QList<sxf::Semantic>* semantic)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  
  //чтение семантики
  uint semLength = ba.size();
  if (semLength > 4) {
    uint curLength = 0;
    while ((curLength + 4) < semLength) {
      Semantic sem;
      ds >> sem.code;
      ds >> sem.type;
      ds >> sem.scale_coef;
       // var(sem.type);
       // var(sem.code);
       // var(sem.scale_coef);
      if (sem.type != 0 && sem.type < 8) {
	sem.length = sem.type;
	if (sem.length != 0) {
	  uint32_t val = 0;
	  ds.readRawData((char*)&val, sem.length);
	  sem.value = QString::number(val * pow(10, sem.scale_coef));
	} 
	curLength += sem.length + 4;
      } else if (sem.type == 8) {
	float val = 0;
	ds >> val;
	sem.value = QString::number(val);
      } else {
	sem.length = sem.scale_coef;
	if (sem.length != 0) {
	  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");//TODO кодировка
	  sem.value = codec->toUnicode(ba.mid(curLength + 4, sem.length), strlen(ba.mid(curLength + 4, sem.length)));
	}
	curLength += sem.length + 1 + 4;
	ds.skipRawData(sem.length + 1);
	//var(sem.value);
      }
      semantic->append(sem);
    }
  } else {
    //    debug_log << "sem length = " << semLength;
  }
}

Document::Document() :
  passport_(0),
  descr_(0),
  offset_(0),
  isValid_(false),
  isSaveDegree_(false)
{
}

Document::~Document()
{
  if (passport_ == 0) {
    delete passport_;
    passport_ = 0;
  }
  if (descr_ == 0) {
    delete descr_;
    descr_ = 0;
  }
}

QDate Document::date()
{
  if (passport_ == 0) return QDate();

  QByteArray dateData(passport_->created_date, sizeof(passport_->created_date));
  QDate date(dateData.mid(0, 4).toInt(), dateData.mid(4, 2).toInt(), dateData.mid(6, 2).toInt());
  return date;
}

QString Document::name()
{
  if (0 == passport_) return QString();

  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
  return codec->toUnicode(passport_->sheet_title, strlen(passport_->sheet_title));
}

float Document::getMapScaleMatch()
{
  // if (0 == passport_ || !scaleMatching.contains(passport_->sheet_scale)) {
  //   return 0;
  // }
  // return scaleMatching.value(passport_->sheet_scale);

  float scale = sxf::MetaSxf::instance()->mapScale(passport_->sheet_scale);

  if (0 == passport_ || scale == -1) {
    return 0;
  }
  return scale;
}

void Document::setGeobasis(const QSharedPointer<PassportBasis>& basis)
{
  basis_ = basis;
}

const QByteArray Document::serializeToByteArray(QString* errorMessage)
{
  if (errorMessage != 0) {
    errorMessage->clear();
  }

  if (makePassport() == false) {
    if (errorMessage != 0) {
      *errorMessage = QString::fromUtf8("Не удалось заполнить паспорт sxf-документа");
    }
    return QByteArray();
  }

  if (makeDescription() == false) {
    if (errorMessage != 0) {
      *errorMessage = QString::fromUtf8("Не удалось заполнить дескриптор данных sxf-документа");
    }
    return QByteArray();
  }

  QByteArray result;
  result += serializePassport();
  result += serializeDescription();

  for (int i = 0, sz = objects_.size(); i < sz; ++i) {
    result += objects_[i].serializeToByteArray(i+1, 0, passport_, proj_);
  }

  if (makeCheckSum(&result) == false) {
    if (errorMessage != 0) {
      *errorMessage = QString::fromUtf8("Не удалось заполнить контрольную сумму sxf-документа");
    }
    return QByteArray();
  }
  return result;
}

bool Document::makePassport()
{
  if (passport_ != 0) {
    delete passport_;
    passport_ = 0;
  }
  passport_ = new Passport();

  if (basis_.isNull() == true) {
    return false;
  }

  passport_->sheet_scale = static_cast<int32_t>(basis_->scale);
  passport_->map_projection = static_cast<int8_t>(basis_->projection);

  QByteArray ba(QDate::currentDate().toString("yyyyMMdd").toStdString().c_str());
  parseFromByteArray(ba, passport_->created_date, sizeof(passport_->created_date)-1);

  passport_->south_west_lat = basis_->southWest.lat();
  passport_->south_west_lon = basis_->southWest.lon();
  passport_->north_west_lat = basis_->northWest.lat();
  passport_->north_west_lon = basis_->northWest.lon();
  passport_->north_east_lat = basis_->northEast.lat();
  passport_->north_east_lon = basis_->northEast.lon();
  passport_->south_east_lat = basis_->southEast.lat();
  passport_->south_east_lon = basis_->southEast.lon();

  passport_->first_major_parallel = basis_->firstMajorParallel;
  passport_->second_major_parallel = basis_->secondMajorParallel;
  passport_->standard_meridian = basis_->mainPoint.lon();
  passport_->main_point_parallel = basis_->mainPoint.lat();
  passport_->north_offset = basis_->northOffset;
  passport_->east_offset = basis_->eastOffset;

  proj_.a = 6378245.00;
  proj_.e = 0.08181919092890624;
  proj_.la0 = passport_->standard_meridian;
  proj_.k0 = cos(passport_->first_major_parallel)/
    pow((1 - pow(proj_.e,2)*sin(passport_->first_major_parallel)*sin(passport_->first_major_parallel)),0.5);
  proj_.fe = passport_->east_offset;
  proj_.fn = passport_->north_offset;  

  float swy, swx, nwy, nwx, ney, nex, sey, sex;
  if (passport_->map_projection == 36) {
    meteo::f2xy_merc1sp(proj_, passport_->south_west_lat,  passport_->south_west_lon, &swy, &swx);
    meteo::f2xy_merc1sp(proj_, passport_->north_west_lat,  passport_->north_west_lon, &nwy, &nwx);
    meteo::f2xy_merc1sp(proj_, passport_->north_east_lat,  passport_->north_east_lon, &ney, &nex);
    meteo::f2xy_merc1sp(proj_, passport_->south_east_lat,  passport_->south_east_lon, &sey, &sex);
    passport_->south_west_y = swy;   passport_->south_west_x = swx;
    passport_->north_west_y = nwy;   passport_->north_west_x = nwx;
    passport_->north_east_y = ney;   passport_->north_east_x = nex;
    passport_->south_east_y = sey;   passport_->south_east_x = sex;	  
  } else if (passport_->map_projection == 23) {
    proj_.k0 = 0.994;
    meteo::f2xy_stereo_polar(proj_, passport_->south_west_lat,  passport_->south_west_lon, &swy, &swx);
    meteo::f2xy_stereo_polar(proj_, passport_->north_west_lat,	passport_->north_west_lon, &nwy, &nwx);
    meteo::f2xy_stereo_polar(proj_, passport_->north_east_lat,	passport_->north_east_lon, &ney, &nex);
    meteo::f2xy_stereo_polar(proj_, passport_->south_east_lat,	passport_->south_east_lon, &sey, &sex);
    passport_->south_west_y = swy;   passport_->south_west_x = swx;
    passport_->north_west_y = nwy;   passport_->north_west_x = nwx;
    passport_->north_east_y = ney;   passport_->north_east_x = nex;
    passport_->south_east_y = sey;   passport_->south_east_x = sex;	  
  }
  if (!isSaveDegree_) {
    passport_->unit_on_plan = 0;
    passport_->coord_system = 6;
  }

  // debug_log << MnMath::rad2deg(passport_->south_west_lat) << MnMath::rad2deg(passport_->south_west_lon) 
  // 	    << MnMath::rad2deg(passport_->south_east_lat) << MnMath::rad2deg(passport_->south_east_lon);
  // debug_log << MnMath::rad2deg(passport_->north_west_lat) << MnMath::rad2deg(passport_->north_west_lon) 
  // 	    << MnMath::rad2deg(passport_->north_east_lat) << MnMath::rad2deg(passport_->north_east_lon);
  // printf("%f %f %f %f\n", passport_->south_west_y, passport_->south_west_x,
  // 	 passport_->south_east_y, passport_->south_east_x);
	   
    

  return true;
}

bool Document::makeDescription()
{
  if (descr_ != 0) {
    delete descr_;
    descr_ = 0;
  }
  descr_ = new Description();

  descr_->records_count = static_cast<int32_t>(objects_.size());

  return true;
}

bool Document::makeCheckSum(QByteArray* serialized) const
{
  if (serialized == 0 ||
      serialized->size() < (Passport::checksum_offset + Passport::checksum_length)) {
    return false;
  }

  int32_t checkSum = 0;
  for (int i = 0, sz = serialized->size(); i < sz; ++i) {
    checkSum += static_cast<int32_t>(serialized->at(i));
  }
  //var(checkSum);
  serialized->replace(Passport::checksum_offset, Passport::checksum_length, serializeValue(checkSum));
  //var(serializeValue(checkSum).toHex());
  return true;
}

const QByteArray Document::serializePassport() const
{
  QByteArray result;
  result.reserve(Passport::length);

  result += serializeValue(passport_->file_id);
  result += serializeValue(passport_->passport_length);
  result += serializeValue(passport_->format_version);
  result += serializeValue(passport_->check_sum);
  result += serializeValue(passport_->created_date, sizeof(passport_->created_date));
  result += serializeValue(passport_->sheet_nomenclature, sizeof(passport_->sheet_nomenclature));
  result += serializeValue(passport_->sheet_scale);
  result += serializeValue(passport_->sheet_title, sizeof(passport_->sheet_title));
  result += serializeValue(passport_->info_flags);
  result += serializeValue(passport_->epsg_code);
  result += serializeValue(passport_->south_west_x);
  result += serializeValue(passport_->south_west_y);
  result += serializeValue(passport_->north_west_x);
  result += serializeValue(passport_->north_west_y);
  result += serializeValue(passport_->north_east_x);
  result += serializeValue(passport_->north_east_y);
  result += serializeValue(passport_->south_east_x);
  result += serializeValue(passport_->south_east_y);
  result += serializeValue(passport_->south_west_lat);
  result += serializeValue(passport_->south_west_lon);
  result += serializeValue(passport_->north_west_lat);
  result += serializeValue(passport_->north_west_lon);
  result += serializeValue(passport_->north_east_lat);
  result += serializeValue(passport_->north_east_lon);
  result += serializeValue(passport_->south_east_lat);
  result += serializeValue(passport_->south_east_lon);
  result += serializeValue(passport_->ellipsoid_kind);
  result += serializeValue(passport_->height_system);
  result += serializeValue(passport_->map_projection);
  result += serializeValue(passport_->coord_system);
  result += serializeValue(passport_->unit_on_plan);
  result += serializeValue(passport_->unit_on_height);
  result += serializeValue(passport_->frame_kind);
  result += serializeValue(passport_->map_type);
  result += serializeValue(passport_->raw_data_date, sizeof(passport_->raw_data_date));
  result += serializeValue(passport_->raw_data_kind);
  result += serializeValue(passport_->raw_data_type);
  result += serializeValue(passport_->msk_zone_id);
  result += serializeValue(passport_->has_frame);
  result += serializeValue(passport_->magnetic_declination);
  result += serializeValue(passport_->meridian_convergence);
  result += serializeValue(passport_->convergence_variation);
  result += serializeValue(passport_->convergence_date, sizeof(passport_->convergence_date));
  result += serializeValue(passport_->msk_zone_number);
  result += serializeValue(passport_->relief_height);
  result += serializeValue(passport_->angle_local_axis);
  result += serializeValue(passport_->device_resolution);
  result += serializeValue(passport_->device_frame_south_west_x);
  result += serializeValue(passport_->device_frame_south_west_y);
  result += serializeValue(passport_->device_frame_north_west_x);
  result += serializeValue(passport_->device_frame_north_west_y);
  result += serializeValue(passport_->device_frame_north_east_x);
  result += serializeValue(passport_->device_frame_north_east_y);
  result += serializeValue(passport_->device_frame_south_east_x);
  result += serializeValue(passport_->device_frame_south_east_y);
  result += serializeValue(passport_->device_frame_class);
  result += serializeValue(passport_->first_major_parallel);
  result += serializeValue(passport_->second_major_parallel);
  result += serializeValue(passport_->standard_meridian);
  result += serializeValue(passport_->main_point_parallel);
  result += serializeValue(passport_->north_offset);
  result += serializeValue(passport_->east_offset);

  return result;
}

const QByteArray Document::serializeDescription() const
{
  QByteArray result;
  result.reserve(Description::length);

  result += serializeValue(descr_->data_id);
  result += serializeValue(descr_->description_length);
  result += serializeValue(descr_->sheet_nomenclature, sizeof(descr_->sheet_nomenclature));
  result += serializeValue(descr_->records_count);
  result += serializeValue(descr_->info_flags);
  result += serializeValue(descr_->reserve);

  return result;
}

void Document::addObject(const Object& obj)
{
  objects_.append(obj);
}

bool Document::setData(const QByteArray& ba)
{ 
  isValid_ = false;
  data_ = ba; 
  offset_ = 0; 

  if (passport_ != 0) {
    delete passport_;
    passport_ = 0;
  }
  passport_ = new Passport();
  passport_->read(ba);

  uint32_t checkSum = 0;
  for (int i = 0, sz = ba.size(); i < sz; ++i) {
    if (i < 12 || i >= 16) {
      checkSum += static_cast<int32_t>(ba.at(i));
    }
  }

  if (passport_->file_id != 0x00465853 || (uint32_t)passport_->check_sum != checkSum) {
    data_.clear();
    return false;
  }

  if (descr_ != 0) {
    delete descr_;
    descr_ = 0;
  }
  descr_ = new Description();
  descr_->read(ba);

  //TODO это только для WGS84
  proj_.a = 6378245.00;;
  proj_.e = 0.08181919092890624;
  
  proj_.la0 = passport_->standard_meridian;
  proj_.k0 = cos(passport_->first_major_parallel)/
    pow((1 - pow(proj_.e,2)*sin(passport_->first_major_parallel)*sin(passport_->first_major_parallel)),0.5);
  proj_.fe = passport_->east_offset;
  proj_.fn = passport_->north_offset;  

  isValid_ = true;
  return true;
}

//! поиск номеров использующихся объектов (классификационный код, локализация)
bool Document::findObjects(QMap<ObjectKey, ObjectDescription>* objCodes, 
			   QMap<uint8_t, sxf::LayersDescription>* graphicLegend)
{
  if (0 == objCodes) return false;

  QMap<QString, int> layers;

  objCodes->clear();
  int pos = 0;
  //  static const uchar c[] = {0xff, 0x7f, 0xff, 0x7f};
  QByteArray ar = QByteArray::fromRawData((const char*)kSxfObjectIdentification/*(const char*)c*/, 4);
  while(-1 != (pos = data_.indexOf(ar, pos))) {
    if (pos + 21 > data_.size()) return false;


    bool isPuansonWind = false;
    uint32_t len; //размер записи
    uint32_t metrlength;
    ObjectKey val; //классификационный код, локализация
    QDataStream ds(data_.mid(pos + 4, 4+4+4+4+1));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> len;
    ds >> metrlength;
    ds >> val.first;
    ds.skipRawData(4);
    ds >> val.second;
    
    if (len > Object::Header::length + metrlength) {
      QList<sxf::Semantic> semantic;
      Object::readSemantic(data_.mid(pos + Object::Header::length + metrlength, 
				     len - Object::Header::length + metrlength), &semantic);
      if (val.first == 0) { //графический объект, ищем название слоя в семантике
	
	for (int idx = 0; idx < semantic.length(); idx++) {
	  if (semantic.at(idx).code == meteo::sxf::kLayerSemantic) {
	    //TODO соответствие названия слоя и его номера
	    int layNum = layers.count() + 100;
	    if (layers.contains(semantic.at(idx).value)) {
	      layNum = layers.value(semantic.at(idx).value);
	    } else {
	      layers.insert(semantic.at(idx).value, layNum);
	    }
	    if (! graphicLegend->contains(layNum)) {
	      LayersDescription ld(semantic.at(idx).value);
	      sxf::ObjectDescription od(1);
	      od.name = QObject::tr("Графический объект");
	      ld.obj.insertMulti(val, od);
	      graphicLegend->insert(layNum, ld);
	    } else {
	      if (!graphicLegend->operator[](layNum).obj.contains(val)) {
		sxf::ObjectDescription od(1);
		od.name = QObject::tr("Графический объект");
		graphicLegend->operator[](layNum).obj.insert(val, od);
	      } else {
		graphicLegend->operator[](layNum).obj.find(val).value().count += 1;
	      }
	    }
	    break;
	  }
	}
      } else {
	//скорость и направление ветра
	for (int idx = 0; idx < semantic.length(); idx++) {
	  if (semantic.at(idx).code == meteo::sxf::kNameSemantic && 
	      semantic.at(idx).value.contains("Puanson")) {
	    isPuansonWind = true;
	  }
	}
      }
    }

    if (!isPuansonWind) { //объект пропускаем, рисуем по семантике текста
      if (!objCodes->contains(val)) {
	objCodes->insert(val, ObjectDescription(1));
      } else {
	objCodes->operator[](val).count += 1;
      }
    }

    pos += len;    
  }

  return true;
}

//! Поиск следующего подходящего объекта по классификационному коду и локализации
bool Document::readNextObject(sxf::Object* obj, const QList<ObjectKey>* ident)
{
  bool ok = true;
  if (obj == 0 || passport_ == 0) {
    return false;
  }
  
  //  static const uchar kSxfObjectIdentification[] = {0xff, 0x7f, 0xff, 0x7f};
  QByteArray ar = QByteArray::fromRawData((const char*)kSxfObjectIdentification, 4);

  uint32_t len = 0; //размер записи
  while (-1 != (offset_ = data_.indexOf(ar, offset_))) {
    ObjectKey val; //классификационный код, локализация
    QDataStream ds(data_.mid(offset_ + 4, 17));
    ds.setByteOrder(QDataStream::LittleEndian);
    ds >> len;
    ds.skipRawData(4);
    ds >> val.first;
    ds.skipRawData(4);
    ds >> val.second;

    if (0 == ident || ident->contains(val)) {
      obj->read(data_, offset_, passport_->unit_on_plan, passport_->map_projection, proj_);
      offset_ += len;
      break;
    }
    offset_ += len;
  }

  if (offset_ == -1) ok = false;

  return ok;
}

Object::Header::Header() :
  record_begin(0x7FFF7FFF),
  record_length(0),
  metric_length_bytes(0),
  classification_code(0),
  number_in_group(0),
  group_number(0),
  localization(0),
  record_tags(::objectHeaderRecordTags()),
  metric_tags(::objectHeaderMetricTags()),
  generalization_level(::objectGeneralizationLevel()),
  blob_point_count(0),
  subobject_count(0),
  metric_point_count(0)
{
}

void Object::Header::read(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  
  ds >> record_begin;
  ds >> record_length;
  ds >> metric_length_bytes;
  ds >> classification_code;
  ds >> number_in_group;
  ds >> group_number;
  ds >> localization;
  ds >> record_tags;
  ds >> metric_tags;
  ds >> generalization_level;
  ds >> blob_point_count;
  ds >> subobject_count;
  ds >> metric_point_count;
}

void Object::Header::setHasLabelFlag()
{
  localization |= 0x03;  //!< Объект типа Подпись
  metric_tags  |= 0x08;  //!< Метрика содержит текст подписи
}

void Object::Header::setHasGraphicFlag()
{
  metric_tags |= 0x10;   //!< Наличие графического описания объекта следом за метрикой объекта
}

bool Object::Header::hasGraphicFlag()
{
  return 0 != (metric_tags & 0x10);
}

Metric::Metric() :
  x(0.0),
  y(0.0),
  h(0.0)
{
}


//! Чтение метрики из массива данных
/*! 
  \param ba Данные
  \param offset Смещение до начала метрики
  \param msize  Размер элементка метрики
  \param m3d    Наличие третьей координаты
  \return Количество считанных байт
*/
template<class T> uint64_t Metric::read(const QByteArray& data, uint64_t offset, int msize, bool m3d)
{
  T val;
  uint64_t length = 0;

  if ((uint64_t)data.size() < offset + msize*2) return 0;

  memcpy(&val, data.data() + offset + length, msize);
  x = val;
  length += msize;
  memcpy(&val, data.data() + offset + length, msize);
  y = val;
  length += msize;
  if (m3d &&  ((uint64_t)data.size() >= offset + length + msize)) {
    memcpy(&val, data.data() + offset + length, msize);
    h = val;
    length += msize;
  }

  return length;
}

void Metric::toGeodetic(int unit_on_plan, int sxfproj, const meteo::ProjectionParams& proj)
{
  //debug_log << m.x << m.y;
  if (unit_on_plan == 0) {
    // printf("M: %f %f\n", m.x, m.y);
    if (sxfproj == 36 || sxfproj == 8) {
      float vx, vy;
      meteo::xy2f_merc1sp(proj, y, x, &vx, &vy);
      x = vx;
      y = vy;
    } else if (sxfproj == 23) {
      //meteo::xy2f_stereo_polar(proj, y, x, &x, &y);
      error_log << QObject::tr("Преобразование координат не поддерживается");
    } else {
      error_log << QObject::tr("Преобразование координат не поддерживается");
    }
    // printf("M: %f %f\n", m.x, m.y);
  } else if (unit_on_plan == 2) {
    x = MnMath::deg2rad(x);
    y = MnMath::deg2rad(y);
  }
  
  if (y > M_PI) y = M_PI;
  if (y < -M_PI) y = -M_PI;
  //  y = MnMath::PiToPi(y);

  // printf("D: %f %f\n", MnMath::rad2deg(m.x), MnMath::rad2deg(m.y));
  // debug_log << MnMath::rad2deg(m.x) << MnMath::rad2deg(m.y);
}

MetricLabel::MetricLabel() :
  hasLabel_(false)
{
}

const QByteArray MetricLabel::serializeToByteArray() const
{
  const int8_t terminateZero = 0x00;

  QByteArray result;

  if (text.isEmpty() == false) {
    // int8_t labelLength = static_cast<int8_t>(text.length()*2 + 2);
    // result += serializeValue(labelLength);
    // result += serializeValue(text.utf16(), labelLength);
    int8_t labelLength = static_cast<int8_t>(text.length());
    result += serializeValue(labelLength);
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    result += codec->fromUnicode(text);
  }
  else {
    int8_t labelLength = 0;
    result += serializeValue(labelLength);
  }
  result += serializeValue(terminateZero);
  return result;
}

void FullMetric::addLabel(const QByteArray& data)
{
  QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
  label.text = codec->toUnicode(data.data(), strlen(data.data()));
  label.hasLabel_ = true;
}

MetricSeparator::MetricSeparator() :
  reserve(0),
  subobject_point_count(0)
{
}

MetricSeparator::MetricSeparator(int pointCount) :
  reserve(0),
  subobject_point_count(static_cast<int16_t>(pointCount))
{
}

Semantic::Semantic() :
  code(meteo::sxf::kUuidSemantic), //идентификатор объекта
  type(126),
  scale_coef(0xFF),
  length(0)
{
}

Semantic::Semantic(const QString& avalue, int16_t acode) :
  code(acode),
  type(126),
  scale_coef(0xFF),
  length(static_cast<int32_t>(avalue.length() + 1)),
  //  length(static_cast<int32_t>(avalue.length()*2 + 2)),
  value(avalue)
{
  scale_coef = value.length();
}

Semantic::Semantic(const QString& avalue, int16_t acode, int8_t atype, int ascale_coef):
  code(acode),
  type(atype)
{
  value = avalue;
  if (type > 8) {
    scale_coef = value.length();
    length = value.length() + 1;
    if (length > 255) {
      scale_coef = 255;
      length = 256;
      value.truncate(scale_coef);
      error_log << QObject::tr("Семантика данного типа не может превышать 255 символов");
    }
  } else {
    scale_coef = ascale_coef;
  }

}

Document::Description::Description() :
  data_id(0x00544144),
  description_length(Description::length),
  records_count(0),
  info_flags(::descriptionInfoFlags()),
  reserve(0)
{
  fillWithValue(sheet_nomenclature, sizeof(sheet_nomenclature));
}

bool Document::Description::read(const QByteArray& ba)
{
  if (ba.size() < Passport::length + Description::length) {
    return false;
  }

  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);
  ds.skipRawData(Passport::length);
  ds >> data_id;
  ds >> description_length;
  ds.readRawData(sheet_nomenclature, sizeof(sheet_nomenclature));
  ds >> records_count;
  ds >> info_flags;
  ds >> reserve;

  return true;
}

Passport::Passport() :
  file_id(0x00465853),
  passport_length(Passport::length),
  format_version(0x00040000),
  check_sum(0),
  sheet_scale(-1),
  info_flags(::passportInfoFlags()),
  epsg_code(0),
  south_west_x(0.0),
  south_west_y(0.0),
  north_west_x(0.0),
  north_west_y(0.0),
  north_east_x(0.0),
  north_east_y(0.0),
  south_east_x(0.0),
  south_east_y(0.0),
  south_west_lat(0.0),
  south_west_lon(0.0),
  north_west_lat(0.0),
  north_west_lon(0.0),
  north_east_lat(0.0),
  north_east_lon(0.0),
  south_east_lat(0.0),
  south_east_lon(0.0),
  ellipsoid_kind(9),
  height_system(1),
  map_projection(-1),
  coord_system(7),
  //coord_system(10),
  unit_on_plan(64),
  unit_on_height(0),
  frame_kind(0),
  map_type(2),
  raw_data_kind(-1),
  raw_data_type(-1),
  msk_zone_id(0),
  has_frame(0),
  magnetic_declination(0.0),
  meridian_convergence(0.0),
  convergence_variation(0.0),
  msk_zone_number(0),
  relief_height(0.0),
  angle_local_axis(0.0),
  device_resolution(-1),
  device_frame_south_west_x(0),
  device_frame_south_west_y(0),
  device_frame_north_west_x(0),
  device_frame_north_west_y(0),
  device_frame_north_east_x(0),
  device_frame_north_east_y(0),
  device_frame_south_east_x(0),
  device_frame_south_east_y(0),
  device_frame_class(-1),
  first_major_parallel(0.0),
  second_major_parallel(0.0),
  standard_meridian(0.0),
  main_point_parallel(0.0),
  north_offset(0.0),
  east_offset(0.0)
{
  fillWithValue(created_date, sizeof(created_date));
  fillWithValue(sheet_nomenclature, sizeof(sheet_nomenclature));
  fillWithValue(sheet_title, sizeof(sheet_title));
  //  QByteArray ba(QObject::tr("ГМИ").toUtf8());
  QByteArray ba("GMI");
  strncpy(sheet_title, ba.data(), ba.size());
  fillWithValue(raw_data_date, sizeof(raw_data_date));
  fillWithValue(convergence_date, sizeof(convergence_date));
}

bool Passport::read(const QByteArray& ba)
{
  QDataStream ds(ba);
  ds.setByteOrder(QDataStream::LittleEndian);

  ds >> file_id;
  ds >> passport_length;
  ds >> format_version;
  ds >> check_sum;
  ds.readRawData(created_date, sizeof(created_date));
  ds.readRawData(sheet_nomenclature, sizeof(sheet_nomenclature));
  ds >> sheet_scale;
  ds.readRawData(sheet_title, sizeof(sheet_title));
  ds >> info_flags;
  ds >> epsg_code;
  ds >> south_west_x;
  ds >> south_west_y;
  ds >> north_west_x;
  ds >> north_west_y;
  ds >> north_east_x;
  ds >> north_east_y;
  ds >> south_east_x;
  ds >> south_east_y;
  ds >> south_west_lat;
  ds >> south_west_lon;
  ds >> north_west_lat;
  ds >> north_west_lon;
  ds >> north_east_lat;
  ds >> north_east_lon;
  ds >> south_east_lat;
  ds >> south_east_lon;
  ds >> ellipsoid_kind;
  ds >> height_system;
  ds >> map_projection;
  ds >> coord_system;
  ds >> unit_on_plan;
  ds >> unit_on_height;
  ds >> frame_kind;
  ds >> map_type;
  ds.readRawData(raw_data_date, sizeof(raw_data_date));
  ds >> raw_data_kind;
  ds >> raw_data_type;
  ds >> msk_zone_id;
  ds >> has_frame;
  ds >> magnetic_declination;
  ds >> meridian_convergence;
  ds >> convergence_variation;
  ds.readRawData(convergence_date, sizeof(convergence_date));
  ds >> msk_zone_number;
  ds >> relief_height;
  ds >> angle_local_axis;
  ds >> device_resolution;
  ds >> device_frame_south_west_x;
  ds >> device_frame_south_west_y;
  ds >> device_frame_north_west_x;
  ds >> device_frame_north_west_y;
  ds >> device_frame_north_east_x;
  ds >> device_frame_north_east_y;
  ds >> device_frame_south_east_x;
  ds >> device_frame_south_east_y;
  ds >> device_frame_class;
  ds >> first_major_parallel;
  ds >> second_major_parallel;
  ds >> standard_meridian;
  ds >> main_point_parallel;
  ds >> north_offset;
  ds >> east_offset;

  return true;
}


} // sxf
} // meteo
