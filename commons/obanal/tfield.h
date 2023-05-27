#ifndef COMMONS_OBANAL_TFIELD_H
#define COMMONS_OBANAL_TFIELD_H

#include <qfile.h>
#include <qmap.h>
#include <qvector.h>
#include <qbitarray.h>

//#include <cross-commons/funcs/mn_funcs.h>
#include <cross-commons/funcs/mn_errdefs.h>

#include <commons/proto/tfield.pb.h>
#include <commons/mathtools/mnmath.h>
#include <commons/geobasis/geopoint.h>
#include <commons/geobasis/geovector.h>
#include <commons/geobasis/geodata.h>


#include "obanal_struct.h"

constexpr int   DATA_PERENOS_ERROR = 56;
constexpr float kPR_DOPUSK         = 12.0f * meteo::kPI180; //!< нижняя широта для геостроф. ветра
constexpr int   GEOSTROF_ERROR     = 43;

constexpr int   kZyclone           = 0;
constexpr int   kLozbina           = 10;
constexpr int   kAntiZyclone       = 20;
constexpr int   kGreben            = 30;
constexpr int   kLowGradient       = 40;
constexpr int   kSaddle            = 50;
constexpr int   kUncBar            = -100;

constexpr int   kSevVost           = 1;
constexpr int   kSevZap            = 2;
constexpr int   kUgVost            = 3;
constexpr int   kUgZap             = 4;
constexpr int   kCalm              = 5;
constexpr int   kUncNapr           = -100;


namespace obanal{

class TFieldParams;

//! будем считать, что поле представляет собой сетку покрывающую всю поверхность планеты
//! значение в точке сопровождается маской наличия данных
class TField
{
  public:
    TField();
    TField(const TField& f);

    virtual ~TField();

    bool isEmpty() const;
    void clear();

    int num(int i, int j) const { return i * size_la_ + j; }
    int cycle_num(int i, int j) const;

    float pointValue (const meteo::GeoPoint& point,  bool* ok = nullptr) const;
    virtual float pointValueF(const meteo::GeoPoint& point,  bool* ok = nullptr) const;
    float radarValue (const meteo::GeoPoint& point,  bool* ok = nullptr) const;

    bool oprSynSit5(const meteo::GeoPoint&, QString*, QString*, int*);
    static QString sitName(int anum_sit);
    void str_Syn(int, QString*, QString*, int*);
    int calcSynSit(const meteo::GeoPoint& fl);
    int calcSynSit(int, int);

    const RegionParam& getRegPar() const { return reg_param_; }

    void setMasks(bool f);
    void setMasksPoData(const meteo::GeoData &, float);
    void setMasks(int i, bool f);
    void setMasks(int i, int j, bool f);
    void setNetMask(const QVector<bool>& m);

    int           kolFi()      const { return size_fi_; }
    int           kolLa()      const { return size_la_; }
    int           kolData()    const { return size_data_; }
    float        netFi(int i) const;
    float        netLa(int i) const;
    const float* netFi()      const { return pnet_fi_; }
    const float* netLa()      const { return pnet_la_; }
    const float* data()       const { return pdata_; }
    const bool*   mask()       const { return pdatamask_; }
    const bool*   netMask()    const { return pnetmask_; }

    meteo::GeoPoint begin() const ;
    meteo::GeoPoint end() const ;

    //bool controlGradient( float rast, int kolstan, int kolstan_min, float predel_grad );

    int  setNet(const RegionParam& areg_par, float astep_fi, float astep_la);
    int  setNet(const RegionParam& areg_par, NetType type_net);
    int  setNet(const QVector<RegionParam>& areg_pars, NetType type_net);
    bool setDecartNet(float fi0, float la0, float fi1, float la1, float astep_fi, float astep_la);

    int  setRadarNet(const RadarParam& areg_par);
    bool setNetMask(const RegionParam& areg_par);
    bool setData(int, float, bool);
    bool setData(int, int, float, bool);
    void setSrcPointCount(int);
    void setMinMaxVal(float amin, float amax);
    void setTypeNet(NetType type_net)  { net_type_ = type_net;}

    int prognTrack(const meteo::GeoPoint& g0, meteo::GeoVector* g, int time, int step, int* real_time, float koef = 1.0) const;
    int prognTrackObj(const meteo::GeoVector& g0, QVector<meteo::GeoVector>* g, int time, int step, int* real_time, float koef = 1.0) const;
    int prognToStep(const meteo::GeoPoint& g0, meteo::GeoPoint* g, int step, float koef) const;

    QVector<fieldExtremum> calcExtremum() const;
    laplaValue lapla(int i, int j, float* lapla = nullptr) const;
    laplaValue laplaEasy(int i, int j) const;

    bool laplainPoint(const meteo::GeoPoint&, float*) const;
    bool gradient(int i, int j, float* grad_fi, float* grad_la) const;
    bool gradient2(int i, int j, float* grad_fi, float* grad_la) const;
    bool gradientKm(int ii, int jj, float step, float *grad_fi,float *grad_la) const;
    bool gradientKmByCoord(float fi, float la, float step, float *grad_fi,float *grad_la) const;
    bool gradient2Km(int ii, int jj, float step, float *grad_fi,float *grad_la) const;

    bool geostrofWind(int i, int j, float* u, float* v) const;

    bool    getNumPoFila(const meteo::GeoPoint& geo, int* num_fi, int* num_la) const;
    NetType typeNet()                 const { return net_type_; }
    bool    stepFi(float*)           const;
    bool    stepLa(float*)           const;
    bool    stepFiDeg(float*)        const;
    bool    stepLaDeg(float*)        const;
    float   getData(int i, int j)     const;
    bool    getData(int i, int j, float*) const;
    bool    getData(int i, int j, double*) const;
    float   getData(int ij)           const;
    bool    getMask(int ij)           const;
    bool    getMask(int i, int j)     const;
    bool    getNetMask(int i, int j)  const;
    float   getFi(int i)              const;
    float   getLa(int j)              const;
    int     getSrcPointCount()        const;
    int     getIsolines(QVector<meteo::GeoVector>* isolines, float val);
    TField* getCleanCopy()            const;
    TField* getCopy()                 const;

    void setValueType(int descr, int value_level, int /*value_type*/);


    bool oprDisp(float* field_Disp, float* field_MO) const;
    bool oprDisp(int start_i, int start_j, int kol_i, int kol_j, float* field_Disp, float* field_MO) const;
    bool oprMatOz(float* field_MO) const;
    bool oprMatOz(int start_i, int start_j, int kol_i, int kol_j, float* field_MO) const;

    bool minus(const TField*, TField** result) const;
    bool plus(const TField* right, TField** result) const;
    bool mnoz_plus_mnoz(const TField* right, TField** result, float mnoz1 = 1., float mnoz2 = 1.) const;
    bool delta_fabs(const TField* right, TField** result, float mnoz = 1.) const;
    bool calcDeltaPorog(const TField* right, TField** result, float porog = 0.) const;
    bool oprKoefKor(const TField* right, float* result, int di, int dj) const;
    bool oprKoefKor(const TField* right, const RegionParam& rpar,  float* result) const;
    int  smootchField(float  stepen);
    void mnoz(float);
    bool sinDiff(const TField* right, float k, float b, TField** result) const;
    static bool sumLinear(const QList<const TField*>& right, const QList<float>& k, const QList<float>& b, TField** result);

    QString unit() const { return unit_; }
    QString name() const { return name_; }

    int    getDescr()           const { return meteodescr_; }
    int    getLevel()           const { return level_; }
    int    getLevelType()       const { return level_type_; }
    const  QDateTime& getDate() const { return date_; }
    int    getHour()            const { return hour_; }
    int    getModel()           const { return model_; }
    int    getCenter()          const { return center_; }
    float getSmootch()         const { return smootch_stepen_; }
    const  QString& getCenterName()    const { return center_name_; }
    const  QString& getLevelTypeName() const { return level_type_name_; }

    void setDescr(int val)      { meteodescr_ = val; }
    void setLevel(int val)      { level_ = val; }
    void setLevelType(int val)  { level_type_ = val; }
    void setDate(const QDateTime& val) { date_ = val; }
    void setHour(int val)       { hour_ = val; }
    void setModel(int val)      { model_ = val; }
    void setCenter(int val)     { center_ = val; }
    void setCenterName(const QString& val) { center_name_ = val; }
    void setLevelTypeName(const QString& val) { level_type_name_ = val; }


    float dPx (int i, int j, float d_1) const;
    float dPy (int i, int j, float d_1) const;
    float dPyy(int i, int j, float d_1) const;
    float dPxx(int i, int j, float d_1) const;
    float dPxy(int i, int j, float dfi_1, float dla_1)     const;
    virtual bool  radKriv(int i, int j, float* rad)               const;
    bool  radKrivPlos(int i, int j, float* rad)           const;
    bool  radKriv(const meteo::GeoPoint& geo, float* rad) const;

    bool  lapltest(int i, int j, float* rad) const;
    virtual bool  isValidSheme(int i, int j) const;
    QString dbId() const { return db_id_; }
    void  setDbId(const QString& id) { db_id_ = id; }

    int sizeofBuffer() const;


    /*!
     * \brief - получить максимальное значение поля, заданное настройками
     * \return - максимальное значение, заданное настройкми
     */
    float max_val() const { return max_val_; }
    /*!
     * \brief - получить минимальное значение поля, заданное настройками
     * \return - минимальное значение заданное настройками
     */
    float min_val() const { return min_val_; }

    /*!
     * \brief - получить максимальное значение, имеющееся в поле
     * \return - максимальное значение, имеющееся в поле
     */
    float max_field_value() const ;

    /*!
     * \brief - получить минимальное значение, имеющееся в поле
     * \return - минимальное значение, имеющееся в поле
     */
    float min_field_value() const ;

    bool fromBuffer(QByteArray* ba);
    bool fromStdString(const ::std::string& );
    bool fromFile(const QString& fileName);
    //bool fromGridFile(const nosql::GridFile& file);

    void getBuffer(QByteArray* ba) const;
    bool save(const QString& fileName) const;

    void copyFrom(const TField& f);
    TField& operator=(const TField& f);

  protected:
    bool init(const int& asize_fi, const int& asize_la);
    void clearData();
    void resetPointers();

    bool loadHeader(QIODevice* dev, int* dataOffset = nullptr);
    proto::TFieldHeader header() const;

    virtual bool ensureDataLoaded(int idx) const;
    virtual bool ensureDataLoaded(int i0, int i1, int i2, int i3) const;

    virtual bool ensureNetMaskLoaded(int idx) const;
    virtual bool ensureNetMaskLoaded(int i0, int i1, int i2, int i3) const;

    virtual bool ensureDataMaskLoaded(int idx) const;
    virtual bool ensureDataMaskLoaded(int i0, int i1, int i2, int i3) const;

    virtual bool ensurePageLoaded(int n) const;




  private:
    //! Загружает данные из файла в память, если они не были загружены. Вызов данной функции должен предшествовать
    //! обращению к полям data_, datamask_ и т.д.
    bool loadToMemory();

 protected:
    int dataOffset_     = 0; // смещение блока данных относительно начала файла
    int dataMaskOffset_ = 0; // смещение блока маски данных относительно начала файла
    int netMaskOffset_  = 0; // смещение блока маски сетки относительно начала файла
    bool  inMemory_     = false; // флаг, указывающий загружены ли узлы сетки и др. данные в память или хранятся в файле
    QFile dataFile_;

    QVector<float> data_;        //! данные в узлах рег сетки
    float*         pdata_;       //! данные в узлах рег сетки

    QVector<float> net_fi_;      //! координаты узлов сетки
    float*         pnet_fi_;     //! координаты узлов сетки
    QVector<float> net_la_;      //! координаты узлов сетки
    float*         pnet_la_;     //! координаты узлов сетки
    QVector<bool>  datamask_;    //! маска
    bool*          pdatamask_;   //! маска
    QVector<bool>  netmask_;     //! маска
    bool*          pnetmask_;    //! маска
    int         size_fi_;
    int         size_la_;
    int         size_data_;
    float      max_val_;
    float      min_val_;
    float      dfi_;
    float      dla_;
  private:
    // при работа с QFile содежит указатель на начало области памяти, в которую
    // отображён файл
    uchar* dataArr_ = nullptr;

    RegionParam reg_param_;       //! параметры области
    NetType     net_type_;        //! тип сетки
    int         src_point_count_; //! количество исходных станций, по которым проводился анализ


    QMap<float, QVector<meteo::GeoVector>> iso_;

    float      step_val_;
    QString     unit_;
    QString     name_;

    int         level_;            //!< изобарический уровень данных
    int         level_type_;       //!< тип уровня
    QDateTime   date_ ;            //!< срок данных в формате Qt::ISODate
    int         meteodescr_;       //!< дескриптор
    int         hour_;             //!< заблаговременность прогноза
    int         model_;            //!< модель обработки (250, 251 - свой анализ)
    int         center_;           //!< центр обработки
    QString     center_name_;      //!< центр обработки название
    QString     level_type_name_;  //!< тип уровня название
    QString     db_id_;            //!< номер записи в бд
    float      smootch_stepen_;

    friend class TFieldParams;
  };
}
#endif
