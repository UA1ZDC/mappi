#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_WGTDOCVIEW_DOCUMENTVIEWERWIDGET_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_WGTDOCVIEW_DOCUMENTVIEWERWIDGET_H

#include <qwidget.h>
#include <qlistwidget.h>
#include <meteo/commons/ui/custom/intervaledit.h>
#include <qpushbutton.h>
#include <qaction.h>
#include <qgraphicsproxywidget.h>
#include <qcombobox.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/documentviewer/faxaction.h>
#include <meteo/commons/ui/documentviewer/custom/wgtdocviewpanel.h>
#include <meteo/commons/ui/documentviewer/datamodel/docviewtypes.h>
#include <meteo/commons/ui/documentviewer/datamodel/pinningstorage.h>
#include <meteo/commons/ui/documentviewer/custom/wgthoveringwidget.h>
#include <meteo/commons/ui/documentviewer/custom/animationcontroller.h>


namespace Ui {
class WgtDocView;
}


namespace meteo {
namespace documentviewer {

class WgtDocView : public QWidget
{
  Q_OBJECT
public:  
  enum DocumentViewerStates{
    kStateDefault = 0,
    kStateDefaultFaxPage,
    kStateDefaultLocalPage,
    kStateDefaultSatelitePage,
    kStateDefaultMapImagePage,
    kStateDefaultFormalDocPage,

    kStateFaxWithPinning,
    kStateSateliteWithPinning,
    kStateMapImageWithPinning

  };

  explicit WgtDocView( DocumentViewerStates state = DocumentViewerStates::kStateDefault );
  ~WgtDocView() override;
  void setPinningAction(meteo::map::Faxaction *action);

  void setJobNameFilter(const QString& jobName, const QString& mapName);


protected:
  virtual bool eventFilter(QObject *watched, QEvent *event) override;

signals:
  void initialized();

private slots:  
  void slotDocumentResized();
  void slotDocumentClassChanged( DocumentHeader::DocumentClass docClass );
  void slotFileLoaded( const QList<QImage>& path );
  void slotDocumentLeftPanelAction( WgtDocViewPanel::PanelAction );  
  void slotDestroyed();
  void slotPinningMenuActionActivated( QAction* act );
  void slotShowContextMenuForHoverWidget( const QPoint& pnt);
  void slotSplitterMoved();
  void slotHoverWidgetAction( WgtHoveringWidget::HoverWidgetActions actions );
  void slotSelectedDocumentsCountUpdate( int current, int total );
  void slotListLoaded( const QList<DocumentHeader*> *documentsInfo);
  void slotConditionAdd( WgtDocViewPanel::TreeWidgetsColumns column, Condition condition);
  void slotConditionRemove( WgtDocViewPanel::TreeWidgetsColumns column );  

  void slotAnimationSlideChanged( int value );
  void slotAnimationStarted();
  void slotAnimationFinished();
  void slotAnimationResumed();
  void slotAnimationPaused();

  void slotPanelLoaded();

private:
  static QImage transparenPixmap(const QImage& original, DataModel::TransparencyMode transp, int value ) ;
  void animationSave();
  void printCurrentDocument();

  void enablePinningFaxToMap();
  void addCurrentDocumentToMap();
  void addImageToMap(const QString& name, const QString& displayName, const QImage& image );
  void pinSumbenuOpen();
  void pinSumbenuExtendedOpen();
  void removeDocumentFromMap( DocumentHeader* doc );
  void removeSelectedDocumentsFromMap();
  void applyPinning( const PinningStorage::Pin& pin );

  void pinningSettingsOpen();

  bool prepareAnimationData( bool withtransparent);

  void animate( bool onMap, bool onOnScene );
  QHash<WgtDocViewPanel::TreeWidgetsColumns, Condition> getCurrentConditions();
  void loadSettings();
  void saveSettings();

private:
  enum PinningActionType {
    kPinSave = -1,
    kPinRemove = -2
  };
  ::Ui::WgtDocView *ui_;

  WgtDocViewPanel *leftPanel_;
  WgtHoveringWidget* hover_;
  QGraphicsProxyWidget* hoveringWidgetProxy_;

  meteo::map::Faxaction* pinningAction_;

  QDialog* pinningDialog_;  
  PinningStorage pinningStorage_;
  bool isLeftWidgetsHidden = false;
  DataModel *model_;  
  bool isInitialized = false;
  //bool animateMap_ = false;
  //bool isAnimationPaused_ = false;

  AnimationController animationController_;
  QList<int> spliteerSizes_;
};


}
}
#endif
