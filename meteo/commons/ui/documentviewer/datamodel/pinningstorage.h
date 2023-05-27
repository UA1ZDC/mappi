#ifndef METEO_COMMONS_UI_DOCUMENTVIEWER_DATAMODEL_PINNINGSTORAGE_H
#define METEO_COMMONS_UI_DOCUMENTVIEWER_DATAMODEL_PINNINGSTORAGE_H

#include <qhash.h>
#include <qstring.h>
#include <qpoint.h>
#include <meteo/commons/ui/documentviewer/custom/wgtdocviewpanel.h>
#include <meteo/commons/ui/conditionmaker/templates/conditiongenericprivate.h>
#include <meteo/commons/ui/documentviewer/faxaction.h>

namespace meteo {
namespace documentviewer {

class PinningStorage
{
public:
  struct Pin{
    QString name;
    meteo::GeoPoint mapPoints[meteo::map::Faxaction::markersCount];
    QPoint imagePoints[meteo::map::Faxaction::markersCount];
    QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> conditions;
  };

  PinningStorage();
  ~PinningStorage();

  const QHash<QString, Pin*>& getStorage( DocumentHeader::DocumentClass docClass );
  const Pin& getPin( DocumentHeader::DocumentClass docClass, const QString& name );
  void addPinning( const DocumentHeader::DocumentClass docClass, Pin* pin );
  void removePinning( const DocumentHeader::DocumentClass docClass, const QString& name );
  bool contains( const DocumentHeader::DocumentClass docClass, const QString& name );

private:
  void deserialize();
  void serialize();

private:
  QHash<DocumentHeader::DocumentClass, QHash<QString, Pin*>> savedPinnings;

};

}
}

#endif
