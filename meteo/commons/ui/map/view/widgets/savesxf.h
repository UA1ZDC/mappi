#ifndef METEO_COMMONS_UI_MAP_VIEW_WIDGETS_SAVESXF_H
#define METEO_COMMONS_UI_MAP_VIEW_WIDGETS_SAVESXF_H

#include <meteo/commons/ui/map/view/widgets/mapwidget.h>
#include <commons/geobasis/geopoint.h>
#include <meteo/commons/ui/map/dataexchange/passportbasis.h>


class QTreeWidgetItem;

namespace Ui {
  class ExportDoc;
}

namespace meteo {
namespace sxf {
  //  class AbstractSaver;
  class PassportBasis;
} // dataexchange

namespace map {

  class Document;
  class MapRubber;
  class LayersSelect;
  class MapScene;

  class SaveSxf : public MapWidget

{
  Q_OBJECT

public:
  explicit SaveSxf(MapWindow* parent);
  ~SaveSxf();

protected:
  void keyPressEvent( QKeyEvent * e);

private slots:
  void slotAccept();
  void slotCancel();
  void slotSceneChanged(MapScene* scene);
  void getSaveFileName();
  void slotOnlyVis(int state);

private:
  Document* currentDocument() const;
  QString mapname() const;
  void setDefaultName();

  bool eventFilter(QObject* watched, QEvent* event);
  void closeEvent(QCloseEvent *);
  void showEvent(QShowEvent *);

  bool makePassportBasis();
  void checkMercatorBorders();

  int currentScale();

  void resetWatchedDocument();
  bool installWatchedDocument();

  bool execute( const QStringList& layerUuids, const map::Document& doc);
  QString readDefaultDir();
  void saveDefaultDir(const QString& dir);

private:
  Ui::ExportDoc* ui_;
  MapRubber* rubber_;
  LayersSelect* layers_;

  QObject* watchedEventHandler_;
  QSharedPointer<sxf::PassportBasis> basis_;

  QString defaultDir_;
  bool savefs_;
  bool savedb_;
};

} // map
} // meteo

#endif // METEO_COMMONS_UI_MAP_VIEW_WIDGETS_SAVEDOC_H
