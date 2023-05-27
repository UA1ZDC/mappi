#ifndef METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFSTRUCTURES_H
#define METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFSTRUCTURES_H

#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/projectionfunc.h>
#include <meteo/commons/ui/map/dataexchange/sxfobjects.h>
#include <meteo/commons/proto/sxfcodes.pb.h>

#include <QSharedPointer>
#include <QString>
#include <QVector>
#include <QMultiMap>


class QByteArray;
class QColor;

namespace meteo {
 
namespace map {
class Document;
} // map


namespace sxf {
  class PassportBasis;

  typedef QPair<uint32_t, uint8_t> ObjectKey; //!< (классификационный код, серия объектов)

  struct Passport
  {
    static const int length;

    static const int checksum_offset;
    static const int checksum_length;

    int32_t file_id;                   //!< Идентификатор файла (0x00465853 - SXF)
    int32_t passport_length;           //!< Длина записи паспорта в байтах (400)
    int32_t format_version;            //!< Редакция формата (0x00040000)
    int32_t check_sum;                 //!< Контрольная сумма для всего файла
    char    created_date[12];          //!< Дата создания набора данных (ГГГГММДД\0)
    char    sheet_nomenclature[32];    //!< Номенклатура листа ANSI
    int32_t sheet_scale;               //!< Знаменатель масштаба листа
    char    sheet_title[32];           //!< Условное название листа ANSI
    int32_t info_flags;                //!< Информационные флаги
    int32_t epsg_code;                 //!< Код EPSG для системы координат
                                       //!< Прямоугольные координаты углов листа в метрах (X - по вертикали, Y - по горизонтали):
    double  south_west_x;              //!< X - юго-западного угла
    double  south_west_y;              //!< Y - юго-западного угла
    double  north_west_x;              //!< X - северо-западного угла
    double  north_west_y;              //!< Y - северо-западного угла
    double  north_east_x;              //!< X - северо-восточного угла
    double  north_east_y;              //!< Y - северо-восточного угла
    double  south_east_x;              //!< X - юго-восточного угла
    double  south_east_y;              //!< Y - юго-восточного угла
                                       //!< Геодезические координаты углов листа в радианах:
    double  south_west_lat;            //!< Широта - юго-западного угла
    double  south_west_lon;            //!< Долгота - юго-западного угла
    double  north_west_lat;            //!< Широта - северо-западного угла
    double  north_west_lon;            //!< Долгота - северо-западного угла
    double  north_east_lat;            //!< Широта - северо-восточного угла
    double  north_east_lon;            //!< Долгота - северо-восточного угла
    double  south_east_lat;            //!< Широта - юго-восточного угла
    double  south_east_lon;            //!< Долгота - юго-восточного угла
                                       //!< Математическая основа листа:
    int8_t  ellipsoid_kind;            //!< Вид эллипсоида
    int8_t  height_system;             //!< Система высот
    int8_t  map_projection;            //!< Проекция карты
    int8_t  coord_system;              //!< Система координат
    int8_t  unit_on_plan;              //!< Единица измерения в плане
    int8_t  unit_on_height;            //!< Единица измерения по высоте
    int8_t  frame_kind;                //!< Вид рамки
    int8_t  map_type;                  //!< Обобщенный тип карты
                                       //!< Справочные данные об исходном материале:
    char    raw_data_date[12];         //!< Дата съёмки местности или обновления карты (ГГГГММДД\0)
    int8_t  raw_data_kind;             //!< Вид исходного материала
    int8_t  raw_data_type;             //!< Тип исходного материала
    int8_t  msk_zone_id;               //!< Идентификатор зоны МСК-63
    int8_t  has_frame;                 //!< Признак ограничения карты рамкой (1 - карта ограничена рамкой)
    double  magnetic_declination;      //!< Магнитное склонение (в радианах)
    double  meridian_convergence;      //!< Среднее сближение меридианов (в радианах)
    double  convergence_variation;     //!< Годовое изменение магнитного склонения (в радианах)
    char    convergence_date[12];      //!< Дата изменения склонения (ГГГГММДД\0)
    int32_t msk_zone_number;           //!< Номер зоны МСК-63
    double  relief_height;             //!< Высота сечения рельефа (в метрах)
    double  angle_local_axis;          //!< Угол разворота осей для местных систем координат (в радианах по часовой стрелке)
    int32_t device_resolution;         //!< Разрешающая способность прибора (в точках на метр)
                                       //!< Расположение рамки на приборе (в точках в системе прибора):
    int32_t device_frame_south_west_x; //!< X - юго-западного угла
    int32_t device_frame_south_west_y; //!< Y - юго-западного угла
    int32_t device_frame_north_west_x; //!< X - северо-западного угла
    int32_t device_frame_north_west_y; //!< Y - северо-западного угла
    int32_t device_frame_north_east_x; //!< X - северо-восточного угла
    int32_t device_frame_north_east_y; //!< Y - северо-восточного угла
    int32_t device_frame_south_east_x; //!< X - юго-восточного угла
    int32_t device_frame_south_east_y; //!< Y - юго-восточного угла
    int32_t device_frame_class;        //!< Классификационный код рамки объекта
                                       //!< Справочные данные о проекции исходного материала (в радианах):
    double  first_major_parallel;      //!< Первая главная параллель
    double  second_major_parallel;     //!< Вторая главная параллель
    double  standard_meridian;         //!< Осевой меридиан
    double  main_point_parallel;       //!< Параллель главной точки
    double  north_offset;              //!< Смещение на север
    double  east_offset;               //!< Смещение на восток

    Passport();
    bool read(const QByteArray& ba);
  };

    //! Описание бъеков sxf на карте
    struct ObjectDescription {
      QString name;
      uint64_t count;
      ObjectDescription(uint64_t cnt = 0):count(cnt) {}
      ObjectDescription(const ObjectDescription& a):name(a.name), count(a.count) {}
    };

    //! Стурктура слоев и объектов sxf
    struct LayersDescription {
      QString layName;
      QMultiMap<ObjectKey, ObjectDescription> obj;//multi вроде из-за серий объектов
      LayersDescription(const QString& name = QString()):layName(name) {}
    };

  void sxfToMapGeneralization(uint8_t sxfLow, uint8_t sxfHi, int* lowLimit, int* highLimit);

  //!< Структура метрики объекта (подобъекта).
  //!  Представляет собой последовательно расположенные координаты точек контура объекта
  //!  или координаты точки привязки для объектов, не имеющих цифрового контура (точечные объекты, подписи и т.д.)
  //!
  struct Metric
  {
    double  x;                 //!< Координата X (от нижнего левого угла вверх)
    double  y;                 //!< Координата Y (слева направо)
    double  h;                 //!< Высота точки метрики (в метрах)

    Metric();
    template<class T> uint64_t read(const QByteArray& ba, uint64_t offset, int msize, bool m3d);
    void toGeodetic(int unit_on_plan, int sxfproj, const meteo::ProjectionParams& proj);
  };

  struct MetricLabel
  {
    QString text;           //!< Текст подписи

    bool hasLabel_;

    MetricLabel();

    const QByteArray serializeToByteArray() const;
  };

  struct FullMetric {
    QList<Metric> coords;
    MetricLabel label;

    void addLabel(const QByteArray& data);
  };

  //!< Служебное поле для разделения метрик объекта и подобъекта
  //!  Записывается непосредственно перед метрикой подобъекта
  //!
  struct MetricSeparator
  {
    int16_t reserve;                //!< Резерв (0x0000)
    uint16_t subobject_point_count;  //!< Число точек метрики в подобъекте

    MetricSeparator();
    explicit MetricSeparator(int pointCount);
  };

  //! Структура блока данных семантики объекта.
  //!  Блоки записываются непосредственно за метрикой объекта (подобъекта) друг за другом без разделителей.
  //!  Количество блоков данных соответствует количеству характеристик семантики для данного объекта.
  //!
  struct Semantic
  {
    uint16_t code;        //!< Код характеристики
    uint8_t  type;        //!< Тип характеристики (128 - символы произвольной длины)
    uint8_t  scale_coef;  //!< Масштабный коэффициент (0xFF)
    uint32_t length;      //!< Длина значения, включая замыкающие нули
    QString value;       //!< Значение характеристики в кодировке UTF-16

    Semantic();
    explicit Semantic(const QString& value, int16_t code);
    explicit Semantic(const QString& value, int16_t code, int8_t type, int ascale_coef = 0);
  };

class Object
{
public:
  //!< Структура заголовка записи объекта.
  //!
  struct Header
  {
    static const int length;

    uint32_t record_begin;         //!< Идентификатор начала записи (0x7FFF7FFF)
    uint32_t record_length;        //!< Общая длина записи с заголовком
    uint32_t metric_length_bytes;  //!< Длина метрики в байтах
    uint32_t classification_code;  //!< Классификационный код
    uint16_t number_in_group;      //!< Собственный номер объекта в группе
    uint16_t group_number;         //!< Номер группы

    uint8_t  localization;         //!< Характер локализации
    uint8_t  record_tags;          //!< Справочные данные записи
    uint8_t  metric_tags;          //!< Справочные данные метрики
    uint8_t  generalization_level; //!< Уровень генерализации

    uint32_t blob_point_count;     //!< Число точек метрики для больших объектов
    uint16_t subobject_count;      //!< Число подобъектов
    uint16_t metric_point_count;   //!< Число точек метрики (65535 для больших объектов)

    Header();

    void read(const QByteArray& ba);
    void setHasLabelFlag();
    void setHasGraphicFlag();
    bool hasGraphicFlag();
  };



public:

  void clear();

  static const Metric metricFromGeoPoint(const GeoPoint& gp);
  static void readSemantic(const QByteArray& ba, QList<sxf::Semantic>* semantic);

  bool hasMetric() const;

  void setScale();
  void setContract(bool has_contract);
  void setSpline(bool has_spline);

  void addMetric(const Metric& mtr);
  void addMetric(const QList<Metric>& mtr);

  void addSemantic(const Semantic& smt);
  void addSemantic(const QList<Semantic>& smt);

  void addGraphicPrimitive(GraphicsType type, const QColor& color, int width = 0, int dash_len = 0, int space_len = 0);
  void addTextPrimitive(const QString& text, const QFont& font, const QColor& color, int pixScale);
  void addTrueTypeText(int signNum, int fontCode, const QFont& font, const QColor& color, int pixScale);
  void addTextWithClassificator(const QString& text, int32_t code);
  void addWithClassificator(int32_t code, Localization loc);
  sxf::VectorSign* createVectorObject();
  void addGraphicPrimitive(const QSharedPointer<GraphicPrimitive>& gobj, Localization loc);

  void addLabel(const QString& text);
  void addLabel(const QByteArray& text);
  void setGeneralization(int lowLimit, int highLimit);
  void setClassificationCode(int32_t code, Localization local);

  const QByteArray serializeToByteArray(int serialNumber, int groupNumber, Passport* passport, const meteo::ProjectionParams& proj);

  //read
  const Header& header() const { return header_; }
  void read(const QByteArray& data, uint64_t poffset, int unit_on_plan, int sxfproj, const meteo::ProjectionParams& proj);
  void readGraphic(const QByteArray& data);


  bool isScale() const;
  bool isContract() const;
  bool isSpline() const { return (header_.metric_tags & 0x80) != 0; }
  const QList<Metric>& metric() const { return metric_; }
  const QList<Semantic>& semantic() const { return semantic_; }
  const QString& label() const { return label_.text; }
  const QList<FullMetric>& submetric() const { return submetric_; }
  const QSharedPointer<GraphicPrimitive>& graphicObject() const { return graphic_; }

private:
  bool makeHeader(int serialNumber, int groupNumber, int metricLengthBytes, int semanticLengthBytes);

  const QByteArray serializeHeader() const;
  const QByteArray serializeMetrics(Passport* passport, const meteo::ProjectionParams& proj) const;
  const QByteArray serializeSemantics() const;

  template<class T> uint64_t readMetric(const QByteArray& ba, int unit_on_plan, int sxfproj, const meteo::ProjectionParams& proj);
  void readSemantic(const QByteArray& ba);

private:
  Header header_;
  MetricLabel label_;

  QList<Metric> metric_;
  QList<Semantic> semantic_;
  
  QList<FullMetric> submetric_; //! метрика подобъектов

  QSharedPointer<GraphicPrimitive> graphic_;

};


class Document
{
public:
  //! Структура паспорта.
  //!  Все записи размещаются в одном файле, данные метрики и семантики на один объект
  //!  располагаются в одной записи - семантика (характеристики объекта) за метрикой (координаты объекта).
  //!


  //!< Структура дескриптора данных.
  //!
  struct Description
  {
    static const int length;

    int32_t data_id;                //!< Идентификатор данных (0x00544144 - DAT)
    int32_t description_length;     //!< Длина дескриптора (52)
    char    sheet_nomenclature[32]; //!< Номенклатура листа
    int32_t records_count;          //!< Число записей данных
    int32_t info_flags;             //!< Информационные флаги
    int32_t reserve;                //!< Резерв (0x00000000)

    Description();
    bool read(const QByteArray& ba);
  };

  Q_DISABLE_COPY(Document)

public:
  Document();
  ~Document();

  void setCoordType(bool isDeg) { isSaveDegree_ = isDeg; }
  void setGeobasis(const QSharedPointer<PassportBasis>& basis);
  const QByteArray serializeToByteArray(QString* errorMessage);

  void addObject(const Object& obj);
  
  bool setData(const QByteArray& ba);
  bool findObjects(QMap<ObjectKey, ObjectDescription>* objCodes, QMap<uint8_t, sxf::LayersDescription>* graphicLegend);
  bool readNextObject(sxf::Object* obj, const QList<ObjectKey>* layers);

  uint objectsCount() { return descr_ == 0 ? 0 : descr_->records_count; }
  QDate date();
  QString name();
  bool isValid() { return isValid_; }
  float getMapScaleMatch();
  const Passport* passport() { return passport_; }

private:
  bool makePassport();
  bool makeDescription();
  bool makeCheckSum(QByteArray* serialized) const;

  const QByteArray serializePassport() const;
  const QByteArray serializeDescription() const;

private:
  QSharedPointer<PassportBasis> basis_;
  Passport* passport_;
  Description* descr_;
  QList<Object> objects_;

  QByteArray data_; //!< Данные в формате sxf
  int64_t offset_; //!< Текущее положение внутри data_
  bool isValid_; 

  meteo::ProjectionParams proj_; 
  bool isSaveDegree_;
};

} // sxf
} // meteo

#endif // METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFSTRUCTURES_H
