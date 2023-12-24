#ifndef MAPPI_THEMATIC_ALGS_THEMALG_H
#define MAPPI_THEMATIC_ALGS_THEMALG_H

#include <mappi/thematic/thematiccalc/thematiccalc.h>

#include <cross-commons/debug/tlog.h>
#include <sat-commons/satellite/satviewpoint.h>

#include <mappi/proto/thematic.pb.h>
#include <mappi/settings/mappisettings.h>
#include <mappi/ui/pos/pos.h>
#include <mappi/landmask/landmask.h>

#include <qmap.h>
#include <qsharedpointer.h>
#include <qimage.h>



namespace mappi {
  namespace to {

    class Channel;
    class DataStore;

    class ThemAlg {
    public:
      ThemAlg(mappi::conf::ThemType type, const std::string &them_name, QSharedPointer<to::DataStore>& ds);
      virtual ~ThemAlg();
      enum pixelType {
        CLOUD =0 ,
        CLOUD_LOWER = 1,
        CLOUD_MIDL  = 2,
        CLOUD_HI    = 3,
        SNOW        = 4,
        ICE         = 5,
        OPEN_WATER  = 6,
        OPEN_HYDRO  = 7,
        OPEN_LAND   = 8,
        NOT_CLOUD   = 9,
        UNKNOW      = 255
      };
      enum {
      	    Type = conf::kThemUnk
      };
      virtual conf::ThemType type() const { return _themType; }
      const std::string &name() const { return _them_name; }
      QString description() const { return QString::fromStdString(config_.description()); }

      void clear();
      int channel_size() {return channel_size_;}
      int rows()    { return rows_; }
      int columns() { return cols_; }

      bool init(const QDateTime& start, const QString& satname);
      bool loadData();

      //обработка сеанса
      virtual bool process() =0;

      void normalize(int min, int max);
      virtual bool saveImg();
      bool saveData();

    protected:
      void fillPalette();
      void imageTransform(int rows, int cols );
      void normalizeImg(int min, int max, int nmin, int nmax);

      bool initProjection(const QSharedPointer<Channel>& channel);
      bool initLandMask();
      void getVars();
      static bool isValid(const mappi::conf::ThematicProc& thematic);
      int getLandMask(  ){ return 1;}
      virtual bool readConfig(); //!< чтение конфиг файла
      int instrConfIndex(conf::InstrumentType instrtype);


      const QMap<std::string, QSharedPointer<Channel>>& channels() { return _ch; }
      
      QSharedPointer<DataStore> store() { return  _ds; }
      void clearChannels();

      void getSavePath(QString* path, QString* templName);
      
      bool saveData(const QString& );
      virtual bool saveImage(const QString& name, const QSharedPointer<Channel>& channel, QImage::Format format);


      pixelType cloudTestWithLandMask(float a1, float a2, float t3, float t4, int);
      pixelType cloudTestWithLandMask2(float a1, float a2, float t3, float t4, int);
      pixelType openWaterTest(float a1, float a2, int landmask);
      pixelType openSurfaceTest(float a1, float ndv, int landmask);
      pixelType snowIceTest(float t4, float ndv, pixelType pixtype);


      int channel_size_ = 0;
      int rows_ = 0;
      int cols_ = 0;
      QVector<float>  data_;
      QVector<uchar>  bitmap_;

     // QVector<QRgb> palette_;
      TColorGradList gradient_;
      SatViewPoint sat_view_;

      protected:
      //QList<conf::InstrumentType> _validInstr; //!< приборы для которых есть настройка для обработки
      
      QMap<std::string, QSharedPointer<Channel>> _ch;
      QSharedPointer<DataStore> _ds;


      conf::ThematicProc config_;
      conf::InstrCollect instruments_;
      conf::ThematicCoef them_coef_; //!<коэффициенты для методов тематической обработки
      conf::ThematicThresholds them_thresholds_ ; //!<пороговые значения для методов тематической обработки


      QMap<std::string, QMultiMap< mappi::conf::InstrumentType, std::string> > variables_; //имя переменной , < название канала, прибор >


      QDateTime start_;
      QString satname_;
      mappi::conf::ThemType _themType;
      std::string _them_name;
      meteo::POSproj *projection_ = nullptr;
      LandMask *landmask_ = nullptr;
    };

  }
}

#endif
