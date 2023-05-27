#ifndef METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFOBJECTS_H
#define METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFOBJECTS_H

/*!
  \file   sxfobjects.h
  \brief  В документации скудное описание примитивов, полное в mapgdi.h ГИС
*/


#include <QSharedPointer>
#include <QString>
#include <QVector>
#include <QPair>

#include <stdint.h>

class QByteArray;
class QColor;
class QFont;

namespace meteo {
  namespace map {
    class Object;
    class Layer;
  }
  
    namespace rsc {
      class ObjectView;
    }

    namespace sxf {
      
      class FontCode;

      //! Коды типов примитивов
      enum GraphicsType {    
	EMPTY_OBJECT = 127,     //!< Пустой примитив
	SOLID_LINE  = 128,      //!< Сплошная линия
	DASH_LINE   = 129,      //!< Пунктирная линия
	AREA_OBJECT = 135,      //!< Площадной объект
	CIRCLE_OBJECT = 140,    //!< Окружность
	TEXT_OBJECT = 142,      //!< Текст, шрифт из классификатора
	OBJECTS_SET = 147,      //!< Набор примитивов
	VECTOR_SIGN   = 149,    //!< Векторный знак
	TRUE_TYPE   = 151,      //!< Шрифт true-type
	CUSTOMTEXT_OBJECT = 152,//!< Текст, шрифт по названию
	AREA_SHADE  = 153,      //!< Заштрихованная площадь
	DECORATE_LINE = 157,    //!< Декорированная линия
	SECTIONS_LINE = 158,    //!< Наборная линия (вместо нее используется пунктир)
	TEXT_VECT   = 160,      //!< Текст для векторного знака
	AREA_GLASS  = 167,      //!< Площадной - цветное стекло с величиной прозрачности
	GLASS_LINE  = 168,      //!< Линия - цветное стекло с величиной прозрачности
	USER_LINE   = 250       //!< Объект пользователя
      };


      //! Заголовок графического описания
      struct GraphicHeader 
      {
	static const int length;  //!< Базовая длина записи в байтах (исключая длину описания примитива)
	
	int32_t graphic_begin;    //!< Идентификатор начала записи (0x7FFF7FFE)
	int32_t length_bytes;     //!< Общая длина записи примитива в байтах
	int32_t primitive_count;  //!< Число примитивов
	
	GraphicHeader():graphic_begin(0),length_bytes(0), primitive_count(0) {}
	GraphicHeader(int32_t primitive_length, int32_t count);
	const QByteArray serializeToByteArray() const;
	uint32_t read(const QByteArray& ba);
      };
      
      //! Графическое описание объекта.
      struct GraphicPrimitive
      {
	
	int16_t primitive_length; //!< Длина описания примитива в байтах
	int16_t type;             //!< Код типа примитива
	
	GraphicPrimitive();
	virtual ~GraphicPrimitive() = 0;
	
	virtual const QByteArray serializeToByteArray(bool full = true) const;
	uint64_t readHeader(const QByteArray& ba);
	virtual uint32_t read(const QByteArray& , bool  = true) {  return false; }

	static QSharedPointer<GraphicPrimitive> create(uint16_t type);
	static QSharedPointer<GraphicPrimitive> readPrimitive(const QByteArray& ba, int length, int offset, int primitive);
      };
      
      struct LineGraphicObject : public GraphicPrimitive
      {
	uint32_t color;         //!< цвет линии (RGB)
	uint32_t width;         //!< толщина линии (в микронах)

	LineGraphicObject();
	virtual ~LineGraphicObject();

	virtual const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      struct DashLineGraphicObject : public LineGraphicObject
      {
	uint32_t dash_length;   //!< длина штриха (в микронах)
	uint32_t space_length;  //!< длина пробела (в микронах)

	DashLineGraphicObject();

	const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      //! Линия, цветное стекло, примитив 168
      struct LineGlassGraphicObject : public GraphicPrimitive
      {
	uint32_t color;         //!< цвет линии (RGB)
	uint32_t width;         //!< толщина линии (в микронах)
	int32_t bright;		//!< Процент изменения яркости (+/- 100)
	int32_t contrast;		//!< Процент изменения контраста (+/- 100)
	uint32_t transp;	//!< Степень прозрачности (от 0 - не отображается, до 100 - сплошной цвет)

	LineGlassGraphicObject();
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      //! Линия, примитив 158, 250 
      struct LineDummyObject : public GraphicPrimitive {
	uint32_t length;	//!< Полная длина параметров

	LineDummyObject();
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      struct AreaGraphicObject : public GraphicPrimitive
      {
	uint32_t color;         //!< Цвет площади (RGB)

	AreaGraphicObject();

	const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      //! Заштрихованная площадь, код примитива 153
      struct AreaShadingObject : public GraphicPrimitive {
	uint32_t length;
	uint32_t angle;
	uint32_t step;
	uint32_t line_type;
	QSharedPointer<GraphicPrimitive> line;

	AreaShadingObject();
	~AreaShadingObject();
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      //!Площадной - цветное стекло с величиной прозрачности
      struct AreaGlassObject : public GraphicPrimitive {
	uint32_t color;		//!< Цвет тонирования
	int32_t bright;		//!< Процент изменения яркости (+/- 100)
	int32_t contrast;	//!< Процент изменения контраста (+/- 100)
	uint32_t transp;	//!< Степень прозрачности (от 0 - не отображается, до 100 - сплошной цвет)

	AreaGlassObject();
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };
	  
      struct CircleGraphicObject : public GraphicPrimitive
      {
	uint32_t color;         //!< цвет линии (RGB)
	uint32_t width;         //!< толщина линии (в микронах)
	uint32_t radius;        //!< радиус (в микронах)

	CircleGraphicObject();
	virtual ~CircleGraphicObject();

	virtual const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      //! Шрифт из классификатора. Код примитива 142
      struct TextGraphicObject : public GraphicPrimitive
      {
	uint32_t color;             //!< Цвет текста (RGB)
	uint32_t background_color;  //!< Цвет фона (RGB)
	uint32_t shadow_color;      //!< Цвет тени (RGB)
	int32_t height;            //!< Высота символа (в микронах)
	int32_t weight;            //!< Тип контура
	int16_t align;             //!< Выравнивание
	int8_t  reserved1;         //!< Тут почему-то должен быть задан номер шрифта
	int8_t  reserved2;
	int8_t  wide;              //!< Ширина символа (0 - нормальный, 1 - суженный)
	int8_t  horizontal;        //!< Признак горизонтальности
	int8_t  italic;            //!< Признак курсива
	int8_t  underline;         //!< Признак подчёркивания
	int8_t  strikeline;        //!< Признак зачёркивания
	int8_t  fontcode;          //!< Номер шрифта в таблице шрифтов
	int8_t  code;              //!< Кодовая страница (Windows константы: Win32::API::RUSSIAN_CHARSET = 204, ANSI_CHARSET = 0 и пр.)
	int8_t  scaleFlag;	       //!< 1 - масштабировать по метрике
    
	TextGraphicObject();

	virtual const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      //! Пользовательский шрифт, fontcode - произвольный. Код примитива 152
      struct CustomTextGraphicObject : public TextGraphicObject
      {
	char    font_name[32];     //!< Название шрифта (ASCII)
	CustomTextGraphicObject();

	virtual const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      //! Текст для векторного знака, код примитива 160 
      // (semcode - задает название шрифта, reserved1 - шрифт, которым отрисуется, установленно экспериментально)
      struct VectorTextGraphicObject : public TextGraphicObject {
	int32_t semcode;   //!< Код семантики объекта для выбора переменного текста, 0 - для постоянного
	char text[32]; //!< Постоянный текст

	VectorTextGraphicObject();
	virtual const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);

	void setText(const QString& atext, const sxf::FontCode& font, const QColor& acolor);
      };

      //! Фрагмент векторного объекта
      struct VectorObject {
	uint8_t chain_type;		//!< Тип цепочки
	uint8_t param_type;		//!< Тип параметров
	int16_t length;		//!< Длина параметров
	QSharedPointer<GraphicPrimitive> obj; //!< Параметры цепочки по ее типу
	QList<QPair<int32_t, int32_t> > xy;	//!< X,Y – координаты точки
    
	VectorObject(int8_t chain, GraphicsType param);
	VectorObject();
	virtual ~VectorObject();
	void calcLength();
	virtual const QByteArray serializeToByteArray() const;
	virtual uint32_t read(const QByteArray& ba);
      };

      //! Векторный знак, код примитива 149
      struct VectorSign : public GraphicPrimitive
      {
	int32_t fulllength;		//!< Длина параметров
	int32_t pnt_vertic;		//!< Точка привязки (вертикаль)
	int32_t pnt_hor;		//!< Точка привязки (горизонталь)
	int32_t pnt_base;		//!< Длина базы знака
	int32_t mark_vertic_start;	//!< Начало отметки по вертикали
	int32_t mark_vertic_end;	//!< Конец отметки по вертикали
	int32_t size_vertic;	//!< Требуемый размер по вертикали
	int32_t mark_hor_start;	//!< Начало отметки по горизонтали
	int32_t mark_hor_end;	//!< Конец отметки по горизонтали
	int32_t size_hor;		//!< Требуемый размер по горизонтали
	int8_t align;		//!< Флаг расположения (1- по горизонтали, 0 - по метрике)
	int8_t mirror;		//!< Флаг зеркального отображения
	int8_t contract;		//!< Флаг запрета сжатия знака (1 - не сжимать)
	int8_t strech;		//!< Признак растягивания по метрике (1 - база знака совмещается с  первыми двумя точками метрики)
	int8_t centering;		//!< Признак центрирования по метрике (1 - центрировать по метрике)
	int8_t  reserved1;		//!< Резерв
	int16_t reserved2;		//!< Резерв
	int32_t max_dim;		//!< Максимальный габарит знака
	QList<QSharedPointer<VectorObject> > obj;	//!< Описание фрагментов

	static const int32_t baselength = 56;

	VectorSign();
	virtual ~VectorSign();
	virtual const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);
	void calcLength();
	QSharedPointer<VectorObject> addObject(int8_t chain_type, GraphicsType param_type);
      };

      //! Знак шрифта true-type, код примитива 151
      struct TrueTypeSign : public CustomTextGraphicObject 
      {
	int32_t sign_num;		//!< Номер знака в шрифте
	int32_t top_offset;	//!< Габариты знака относительно точки привязки, отклонение вверх
	int32_t left_offset;	//!< Габариты знака относительно точки привязки, отклонение влево
	int32_t bot_offset;	//!< Габариты знака относительно точки привязки, отклонение вниз
	int32_t right_offset;	//!< Габариты знака относительно точки привязки, отклонение вправо
	int32_t pnt_vertic;	//!< Точка привязки знака по вертикали
	int32_t pnt_hor;		//!< Точка привязки знака по горизонтали
      
	TrueTypeSign();
	const QByteArray serializeToByteArray(bool full = true) const;
	virtual uint32_t read(const QByteArray& ba, bool full = true);
      };

      struct SetOfObjects : public GraphicPrimitive {
	GraphicHeader header;
	QList<QSharedPointer<GraphicPrimitive> > obj;
	
	SetOfObjects() { type = OBJECTS_SET; primitive_length = header.length; }
	~SetOfObjects();
	virtual uint32_t read(const QByteArray& ba, bool full = false);
      };

      //! Декорированная линия, код примитива 157
      struct DecoratedLine : public GraphicPrimitive
      {
	int32_t length;		        //!< Длина параметров
	int32_t length_start;	        //!< Длина, занимаемая знаком  в начале линии
	int32_t length_end;		//!< Длина, занимаемая знаком в конце линии
	int32_t length_first;	        //!< Длина, занимаемая знаком в начале отрезка
	int32_t length_last;	        //!< Длина, занимаемая знаком в конце отрезка
	int32_t length_firstSign;	//!< Длина, занимаемая первым заполняющим знаком
	int32_t length_secondSign;	//!< Длина, занимаемая вторым заполняющим знаком
	bool flagUniform;		//!< Флаг равномерного размещения заполняющих знаков
	int8_t reserved1;		//!< Резерв
	int16_t reserved2;		//!< Резерв
	int32_t base_length;	        //!< Длина параметров базовой линии
	int32_t base_num;		//!< Номер примитива базовой линии, любой линейный примитив
	QSharedPointer<GraphicPrimitive> base;	//!< Параметры по типу примитива
	int32_t start_length;		//!< Длина параметров векторного объекта начинающего линию
	int32_t start_num;		//!< Номер примитива (149)
	QSharedPointer<GraphicPrimitive> start;	//!< Параметры по типу примитива
	int32_t end_length;	        //!< Длина параметров векторного объекта заканчивающего линию
	int32_t end_num;		//!< Номер примитива (149)
	QSharedPointer<GraphicPrimitive> end;	//!< Параметры по типу примитива
	int32_t left_length;		//!< Длина параметров векторного объекта отображающегося на каждой точке метрики слева	
	int32_t left_num;		//!< Номер примитива (149)						
	QSharedPointer<GraphicPrimitive> left;	//!< Параметры по типу примитива
	int32_t right_length;	        //!< Длина параметров векторного объекта отображающегося на каждой точке метрики справа 
	int32_t right_num;		//!< Номер примитива (149)
	QSharedPointer<GraphicPrimitive> right;	//!< Параметры по типу примитива
	int32_t fill1_length;	        //!< Длина параметров первого векторного объекта, заполняющего отрезки метрики
	int32_t fill1_num;		//!< Номер примитива (149)
	QSharedPointer<GraphicPrimitive> fill1;	//!< Параметры по типу примитива
	int32_t fill2_length;	        //!< Длина параметров второго векторного объекта, заполняющего отрезки метрики
	int32_t fill2_num;		//!< Номер примитива (149)
	QSharedPointer<GraphicPrimitive> fill2;	//!< Параметры по типу примитива

	DecoratedLine();
	virtual ~DecoratedLine();
	void calcLength();
	const QByteArray serializeToByteArray(bool full = true) const;
      };

    
    }

}

#endif
