#ifndef METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFTOMAP_H
#define METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFTOMAP_H

#include <meteo/commons/ui/map/dataexchange/sxfstructures.h>

class QDateTime;

namespace meteo {
  
  namespace map {
    class Document;
    class Map;
  }

    namespace sxf {
      class Document;
      class Object;
    } // sxf
    namespace rsc {
      class Document;
    }
    
    struct SxfDescription {
      QString name;
      QDate date;
      uint32_t objCnt;
      QStringList layers;
      SxfDescription() { objCnt = 0; }
    };
    
    class SxfToMap : public QObject {
      Q_DISABLE_COPY(SxfToMap)
      Q_OBJECT
      
      public:
      SxfToMap();
      ~SxfToMap();

      void clear();

      bool readDescription(const QByteArray& ba, SxfDescription* descr);
      bool readSxf(const QByteArray& ba);
      bool setRscData(const QByteArray& ba);
      bool fillLegend(QMap<uint8_t, sxf::LayersDescription>* legend);

      bool import(const QList<QPair<QString,QString> >& layerNums, const QList<sxf::ObjectKey>& checked, map::Document* doc);
      bool import(const QStringList& layerNums, map::Document* doc );

      bool isValid();

    signals:
      void importedObjects(int, int);

    private:
      bool findObjects(const QString& sxfFile, QList<QPair<uint32_t, uint8_t> >* objCodes);
      bool checkIfMissObject(uint32_t code, uint8_t localization);
      bool addLayers(const QList<QPair<QString,QString> >& layerNums, QMap<QString, QString>* layersUuids, map::Document* doc);
      bool addLayers(const QStringList& layerNums, QMap<QString, map::Layer*>* layersUuids, map::Document* doc );
    private:
      sxf::Document* sxfdoc_;
      rsc::Document* rscdoc_;
      QMap<sxf::ObjectKey, uint8_t> objLayers_; //!< (ключ объекта, номер слоя)
      
    };



}

#endif
