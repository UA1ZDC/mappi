#ifndef METEO_COMMONS_DOCUMENTVIEWER_CUSTOM_PINDIALOG_H
#define METEO_COMMONS_DOCUMENTVIEWER_CUSTOM_PINDIALOG_H

#include <qdialog.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/documentviewer/faxaction.h>
#include <meteo/commons/ui/documentviewer/custom/wgtdocviewpanel.h>
#include <meteo/commons/ui/documentviewer/datamodel/pinningstorage.h>
#include <meteo/commons/ui/documentviewer/datamodel/docviewtypes.h>
#include <meteo/commons/ui/conditionmaker/conditiongeneric.h>
#include <meteo/commons/ui/documentviewer/datamodel/pinningstorage.h>


namespace Ui {
class PinDialog;
}

namespace meteo {
namespace documentviewer {

class PinDialog : public QDialog
{
  Q_OBJECT

public:

  explicit PinDialog( DocumentHeader::DocumentClass docClass, PinningStorage* globalStorage );
  explicit PinDialog( DocumentHeader::DocumentClass docClass,
                      PinningStorage* globalStorage,
                      const meteo::GeoVector &target,
                      const QPolygon &source,
                      const QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> &conditions  );
  ~PinDialog();

  const PinningStorage::Pin& getPinning();

private:
  void fillSavedPinningsList();
  QPoint imagePoint( int index );
  meteo::GeoPoint mapPoint( int index );
  void setMapPoint( int index, const meteo::GeoPoint& point );
  void setImagePoint( int index, const QPoint& mapPoint );

  void setConditions( const QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> &conditions );
  void updatePinningInfo();

  void serialize();
  void deserialize();

private slots:
  void slotSavePinning();
  void slotTreeWidgetSavedPinningItemSelectionChanged();
  void slotPinningNameChanged(const QString&);
  void slotRemovePinning();

private:
  ::Ui::PinDialog *ui;
  DocumentHeader::DocumentClass docClass_;  
  PinningStorage::Pin currentPin;
  PinningStorage* globalStorage_;
  QHash<QPushButton*, QTreeWidgetItem*> buttonToItemHash_;



};

}
}
#endif
