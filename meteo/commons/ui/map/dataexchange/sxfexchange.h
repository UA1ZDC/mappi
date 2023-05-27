#ifndef METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFEXCHANGE_H
#define METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFEXCHANGE_H

#include <meteo/commons/ui/map/dataexchange/sxfstructures.h>

#include <QSharedPointer>
#include <QString>

class QByteArray;
template <typename T> class QList;
class QStringList;


namespace meteo {

  class GeoVector;
  
  namespace map {
    class Document;
    class GeoText;
    class Layer;
    class Object;
    class Puanson;
  } // map
  

  namespace sxf {
    class Document;
    class Object;
    class PassportBasis;
  } // sxf
  
  namespace puanson {
    namespace proto {
      class CellRule;
    }
  }

    
class Sxf
{
  Q_DISABLE_COPY(Sxf)

public:
  Sxf();
  ~Sxf();

  void setPassportBasis(const QSharedPointer<sxf::PassportBasis>& basis);
  void setCoordType(bool isDeg) { isSaveDegree_ = isDeg; }

  const QString& lastError() const;

  const QByteArray exportDocument(const map::Document& doc, bool onlyVis = false);
  const QByteArray exportDocument(const QStringList& layerUuids, const map::Document& doc, bool onlyVis = false);

  void addObject(const sxf::Object& obj);
  const QByteArray serializeToByteArray();

  void setTTFSupported(bool isSupport) { isTtfSupported_ = isSupport; }

private:
  void prepareSxfDocument();

  //  void convertMapObjects(const QStringList& objectUuids, const map::Layer& layer);
  const QList<sxf::Object> convertMapObject(const map::Object& obj, const QList<sxf::Metric>& parentMetric) const;


  const QList<sxf::Object> convertMapObject(const map::Object& obj) const;
  void parseMetric(const GeoVector& geo, QList<sxf::Metric>* ometric) const;
  void parseMetric(const GeoVector& geo, const QList<sxf::Metric>& pmetric, QList<sxf::Metric>* ometric) const;
  int findParentPoint(const GeoPoint& gp, const QList<sxf::Metric>& pmetric) const;
  void parseSemantic(const map::Object& from, QList<sxf::Semantic>* semantic) const;
  void parseObject(const map::Object& from, QList<sxf::Metric>* metric, 
		   const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const;
  void parsePolygonObject(const map::Object& from, QList<sxf::Metric>* metric, 
			  const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const;
  void parseTextObject(const map::Object& from, const QList<sxf::Metric>& metric, 
		       const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const;
  void parsePuansonObject(const map::Object& from, const QList<sxf::Metric>& metric, 
			  const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const;
  bool parsePuansonWind(const map::Object& from, const map::Puanson* puan, 
			const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const;
  void parseMeteoObjects(const map::Puanson* puan, bool isWind, const QList<sxf::Metric>& metric,
			 const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const;
  void parseMeteoWithTtf(const map::Puanson* puan, const puanson::proto::CellRule& rule, 
			 sxf::Object* obj, sxf::VectorSign* sign) const;

  void parseRadarObject(const map::Object& from, const QList<sxf::Metric>& metric,
			const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const;
  void parseGroupObject(const map::Object& from, 
			QList<sxf::Object>* to) const;
  void parseCircleObject(const map::Object& from, const QList<sxf::Metric>& metric,
			 const QList<sxf::Semantic>& semantic, QList<sxf::Object>* to) const;

  // int getWindCode(int ff) const;
  // const sxf::FontCode& getFont(const QFont& font) const;

private:
  bool init();

private:
  sxf::Document* sxfdoc_;
  QSharedPointer<sxf::PassportBasis> basis_;

  mutable QString lastError_;

  int pixScale_; //!< Количество пикселов в мм
  //  sxf::SxfCodes* sxfCodes_;

  bool isSaveDegree_;
  bool onlyVis_;
  bool isTtfSupported_;
};


} // meteo

#endif // METEO_COMMONS_UI_MAP_DATAEXCHANGE_SXFEXCHANGE_H
