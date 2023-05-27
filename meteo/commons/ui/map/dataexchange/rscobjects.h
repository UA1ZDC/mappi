#ifndef METEO_COMMONS_UI_MAP_DATAEXCHANGE_RSCOBJECTS_H
#define METEO_COMMONS_UI_MAP_DATAEXCHANGE_RSCOBJECTS_H

#include <meteo/commons/ui/map/dataexchange/sxfstructures.h>

#include <QByteArray>
#include <QSharedPointer>
#include <QMap>
#include <QPair>

#include <stdint.h>

namespace meteo {
  namespace map {
    class Document;
    class Layer;
    class GeoPolygon;
    class Object;
    class GeoText;
  }

    namespace sxf {
      class GraphicPrimitive;
      class Object;
      class Semantic;
    }
    
    namespace rsc {

      //!Структура заголовка классификатора
      struct Header {
	static const int header_size;  //!< Размер структуры

	uint32_t id;		//!< Идентификатор файла. 0x00435352 (RSC)
	uint32_t length;	//!< Длина файла
	uint32_t vers;		//!< Версия структуры RSC
	uint32_t coding;	//!< Кодировка
	uint32_t num_state;	//!< Номер состояния файла
	uint32_t num_mdif;	//!< Номер модификации состояния
	uint32_t language;	//!< Используемый язык
	uint32_t max_id;        //!< Максимальный идентификатор  таблицы  объектов
	char dt[8];		//!< Дата создания файла
	uint8_t some1[32+32+8+4+4]; //!< Набор скучных параметров
	uint32_t obj_offset;	//!< Смещение на таблицу объектов
	uint32_t obj_len;	//!< Длина таблицы объектов	 
	uint32_t obj_cnt;	//!< Число записей		 
	uint32_t sem_offset;	//!< Смещение на таблицу семантики 
	uint32_t sem_len;	//!< Длина таблицы семантики
	uint32_t sem_cnt;	//!< Число записей
	uint32_t clsem_offset;	//!< Смещение на таблицу классификатора семантики
	uint32_t clsem_len;	//!< Длина таблицы классификатора семантики
	uint32_t clsem_cnt;	//!< Число записей
	uint32_t default_offset; //!< Смещение на таблицу умолчаний
	uint32_t default_len;	//!< Длина таблицы умолчаний
	uint32_t default_cnt;	//!< Число записей
	uint32_t probsem_offset; //!< Смещение на таблицу возможных семантик
	uint32_t probsem_len;	//!< Длина таблицы возможных семантик
	uint32_t probsem_cnt;	//!< Число записей
	uint32_t layers_offset;	//!< Смещение на таблицу сегментов (слоев)
	uint32_t layers_len;	//!< Длина таблицы сегментов (слоев)
	uint32_t layers_cnt;	//!< Число записей
	uint32_t lim_offset;	//!< Смещение на таблицу порогов
	uint32_t lim_len;	//!< Длина таблицы порогов
	uint32_t lim_cnt;	//!< Число записей
	uint32_t param_offset;	//!< Смещение на таблицу параметров
	uint32_t param_len;	//!< Длина таблицы параметров
	uint32_t param_cnt;	//!< Число записей
	uint32_t print_offset;	//!< Смещение на таблицу параметров печати
	uint32_t print_len;	//!< Длина таблицы параметров печати
	uint32_t print_cnt;	//!< Число записей печати
	uint32_t pal_offset;	//!< Смещение на таблицу палитр
	uint32_t pal_len;	//!< Длина таблицы палитр
	uint32_t pal_cnt;	//!< Число записей
	uint32_t font_offset;	//!< Смещение на таблицу шрифтов
	uint32_t font_len;	//!< Длина таблицы шрифтов
	uint32_t font_cnt;	//!< Число записей
	
	uint32_t lib_offset;	//!< Смещение на таблицу библиотек
	uint32_t lib_len;	//!< Длина таблицы библиотек
	uint32_t lib_cnt;	//!< Число записей
	uint32_t imsem_offset;	//!< Смещение на таблицу изображений семантики
	uint32_t imsem_len;	//!< Длина таблицы изображений семантики
	uint32_t imsem_cnt;	//!< Число записей
	uint32_t tbl_offset;	//!< Смещение на таблицу таблиц
	uint32_t tbl_len;	//!< Длина таблицы таблиц
	uint32_t tbl_cnt;	//!< Число записей
	
	uint8_t some2[3*4 + 20 +4 + 4]; //!< Ещё набор параметров

	bool isValid;

	Header();
	Header(const QByteArray& ba);
	void clear();
	bool read(const QByteArray& ba);
	const QByteArray serializeToByteArray() const;
      };

      //! Структура таблиц параметров экрана
      struct ObjectView {
	static const uint32_t header_length;

	uint32_t length;	//!< Длина записи объекта
	uint16_t incode;	//!< Внутренний код объекта
	uint16_t primitive;     //!< Код типа примитива
	QSharedPointer<sxf::GraphicPrimitive> obj; //!< Параметры примитива

	ObjectView() { length = 0; }
	ObjectView(const QByteArray& ba);
	~ObjectView();

	bool readHeader(const QByteArray& ba);
	bool readPrimitive(const QByteArray& ba);

	const QByteArray serializeToByteArray() const;
      };

      //! Структура таблицы объектов  классификатора
      struct Object {
	uint32_t length;	//!< Длина записи объекта
	uint32_t code;		//!< Классификационный код
	uint32_t incode;	//!< Внутренний код объекта
	uint32_t ident;		//!< Идентификационный код
	char short_name[32];	//!< Короткое имя объекта
	char name[32];		//!< Название
	uint8_t localization;	//!< Характер локализации
	uint8_t layer;		//!< Номер слоя (сегмента)
	uint8_t scale_flag;	//!< Признак масштабируемости
	uint8_t vis_lo;		//!< Нижняя граница видимости
	uint8_t vis_hi;		//!< Верхняя граница видимости
	uint8_t ext_loc;	//!< Расширение локализации
	uint8_t direct;		//!< Направление цифрования
	uint8_t sem_flag;	//!< Отображение с учетом семантики
	uint16_t snumber;	//!< Номер расширения (для объекта из серии)
	uint8_t text_cnt;	//!< Количество связанных подписей
	uint8_t contract_flag;	//!< Признак сжатия объекта
	uint8_t max_scale;	//!< Максимальное увеличение
	uint8_t min_scale;	//!< Максимальное уменьшение
	uint8_t visible_flag;	//!< Флаг включения границ
	uint8_t reserved;	//!< Резерв

	Object() { length = 0; code = 0; localization = 0; name[0] = 0; short_name[0] = 0; }
	Object(const QByteArray& ba);
	const QByteArray serializeToByteArray() const;
      };

      //! Таблица палитр
      struct Palette {
	uint32_t rgb[256]; //!< Палитра в формате RGB
	char name[32]; //!< Название палитры

	Palette() { name[0] = 0; }
      };

      //! Таблица слоев
      struct Layer {
	uint32_t length;	//!< Длина записи
	char name[32];		//!< Название слоя
	char short_name[16];	//!< Короткое название слоя
	uint8_t number;		//!< Номер слоя (сегмента)
	uint8_t order;		//!< Порядок отображения объектов слоя (меньший номер будет отображаться раньше)
	uint8_t sem_cnt;	//!< Количество семантик слоя
	QList<uint8_t> sem;	//!< Коды семантик

	Layer():length(0) { name[0] = 0; short_name[0] = 0; }
	Layer(const QByteArray& ba);
      };

      //! Таблица порогов (для серий объектов, меняющих вид в зависимости от значения семантики)
      struct Limits {
	uint32_t length;	//!< Длина записи
	uint32_t code;		//!< Классификационный код
	uint8_t localization;	//!< Локализация
	uint8_t reserved1;	//!< Резерв
	uint16_t reserved2;	//!< Резерв
	uint32_t reserved3;	//!< Резерв
	uint32_t semcode1;	//!< Код первой семантики
	uint16_t cnt1;		//!< Количество ограничителей по первой семантике
	uint16_t default1;	//!< Номер умалчиваемого ограничителя по первой семантике
	uint32_t semcode2;	//!< Код второй семантики 
	uint16_t cnt2;		//!< Количество ограничителей по второй семантике
	uint16_t default2;	//!< Номер умалчиваемого ограничителя по второй семантике
	QList<double> sem1;	//!< Ограничители по первой семантике
	QList<double> sem2;	//!< Ограничители по второй семантике
	QList<uint8_t> snumber;	//!< Матрица расширений

	Limits() { length = 0; }
	Limits(const QByteArray& ba);
      };

      //! Структура шрифта
      struct Font {
	QString file;		//!< Название шрифта (имя файла), 32 байта
	QString name;		//!< Условное название, 32 байта
	uint32_t code;		//!< Код (номер в классификаторе)
	int8_t sym;		//!< Номер символа для уточнения высоты шрифта
	uint8_t code_page;	//!< Кодовая страница
	uint16_t reserve;	//!< Резерв

	Font() { code  = 0; }
	Font(const QByteArray& ba);
      };

      //! Структура таблицы таблиц
      struct Tables {
	uint32_t color_offset;	//!< Смещение на таблицу цветов печати
	uint32_t color_length;	//!< Длина таблицы
	uint32_t color_cnt;	//!< Число записей
	char reserved[36];      //!< Резерв
	uint32_t unk_offset;	//!< Смещение на таблицу фиг знает какую
	uint32_t unk_length;	//!< Длина таблицы
	uint32_t unk_cnt;	//!< Число записей
	char reserved1[12];	//!< Резерв

	Tables() { color_offset = 0; reserved[0] = 0; unk_offset = 0; reserved1[0] = 0; }
	Tables(const QByteArray& ba);
	const QByteArray serializeToByteArray() const;
      };

      struct Semantics {
	static const uint32_t size;

	char some[68];	//!< Набор параметров
	uint32_t sem_offset;	//!< Смещение на описание классификатора семантики
	uint32_t sem_cnt;	//!< Число записей
	uint32_t defsem_offset;	//!< Смещение на умалчиваемые значения семантик (значения по умолчанию, но в доке отчего-то так зовутся)
	uint32_t defsem_cnt;	//!< Число записей

	Semantics() { some[0] = 0; sem_offset = 0; defsem_offset = 0; }
	Semantics(const QByteArray& ba);
	const QByteArray serializeToByteArray() const;
      };

	

      class Document {
      public:
	Document(const QByteArray& ba);
	~Document();

	const Object& object(uint32_t code, uint8_t loc, const QList<sxf::Semantic>& semantic) const;
	bool hasLayer(uint number) { return layers_.contains(number); }
	const Layer& layer(uint number) { return layers_.find(number).value(); }

	bool fillObjectsLayers(const QMap<sxf::ObjectKey, sxf::ObjectDescription>& objCodes,
			       QMap<sxf::ObjectKey, uint8_t>* objLayers,
			       QMap<uint8_t, sxf::LayersDescription>* legend);

	uint64_t drawObject(const sxf::Object& sxf, const rsc::Object& rsc, map::Layer* lay, float origScale) const;
	uint64_t drawObject(const sxf::Object& sxf, map::Layer* lay, float origScale) const;
	
	int pixScale() { return pixScale_; }
	bool isValid() { return isValid_; }

	QByteArray viewba() { return viewba_; }
	void changeMeteoSign(uint incode, sxf::VectorSign* sign);

      private:
	void readPalette(const QByteArray& ba);
	void readLayers(const QByteArray& ba);
	void readObjects(const QByteArray& ba);
	void readViews(const QByteArray& ba);
	void readAndChangeViews(const QByteArray& ba);
	void readLimits(const QByteArray& ba);
	void readFonts(const QByteArray& ba);

	meteo::map::Object* drawObject(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, uint16_t primitive,
	map::Layer* lay, int sub) const;
	meteo::map::GeoPolygon* drawArea(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, 
					 map::Layer* lay, int sub) const;
	meteo::map::GeoPolygon* drawSolidLine(const sxf::Object& sxf, sxf::GraphicPrimitive* obj,
					      map::Layer* lay )const;
	meteo::map::GeoPolygon* drawDashLine(const sxf::Object& sxf, sxf::GraphicPrimitive* obj,
					     map::Layer* lay ) const;
	map::GeoPolygon* drawGlassLine(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, map::Layer* lay) const;
	meteo::map::Object* drawObjectsSet(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, 
					   map::Layer* lay) const;
	map::GeoPolygon* drawAreaShade(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, 
				       map::Layer* lay) const;
	map::Object* drawText(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, 
			      map::Layer* lay, int sub) const;
	map::GeoPolygon* drawAreaGlass(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, map::Layer* lay) const;
	map::GeoPolygon* drawDummyLine(const sxf::Object& /*sxf*/, sxf::GraphicPrimitive* obj, map::Layer* lay) const;
	map::Object* drawCustomText(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, 
				    map::Layer* lay, int sub) const;
	map::Object* drawVectorSign(const sxf::Object& sxf, sxf::GraphicPrimitive* obj, map::Layer* lay) const;
	map::GeoPolygon* drawOrnamentLine(const QString& ornament, map::Layer* lay) const;

	int findSerialNumber(const Limits& lim, const QList<sxf::Semantic>& sem) const;
	int findSemanticValue(const QList<sxf::Semantic>& sem, int semCode, int defaultValue) const;
	void fillTextProp(map::GeoText* gp, const sxf::TextGraphicObject* gobj, const QString& fontname = QString()) const;

	double calcAzimuthFromNorth(const GeoPoint& geo1, const GeoPoint& geo2) const;
      private:

	//	QByteArray data_;
	int pixScale_; //!< Количество пикселов в мм
	Header header_;

	QMultiMap<sxf::ObjectKey, Object> objs_; //!< (классификационный код, серия объектов)
	QMap<QPair<uint32_t, uint8_t>, Limits> limits_; //!< (классификационный код, серия объектов)
	QMap<uint, ObjectView> views_;
	QMap<uint, Layer> layers_; //!< (Номер слоя (сегмента), слой)
	Palette pal_;
	QMap<uint32_t, Font> fonts_; //!< (номер, шрифт)
	
	bool isValid_;

	QByteArray viewba_;
      };

    } //namespace rsc
}

#endif
