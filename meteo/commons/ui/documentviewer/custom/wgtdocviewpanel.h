#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_CUSTOM_WGTDOCVIEWPANEL_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTVIEWERPLUGIN_CUSTOM_WGTDOCVIEWPANEL_H

#include <qwidget.h>
#include <meteo/commons/ui/documentviewer/datamodel/docviewtypes.h>
#include <qtimer.h>
#include <qtreewidget.h>
#include <meteo/commons/ui/documentviewer/datamodel/wgtdocviewModel.h>
#include <qlistwidget.h>
#include <meteo/commons/ui/map/view/mapwindow.h>
#include <meteo/commons/ui/documentviewer/datamodel/docviewtypes.h>
#include <meteo/commons/ui/viewheader/viewheader.h>
#include <meteo/commons/ui/custom/selectdatewidget.h>
#include <meteo/commons/proto/documentviewer.pb.h>

namespace Ui {
class WgtDocViewPanel;
}

namespace meteo {
namespace documentviewer {

class WgtDocViewPanel : public QWidget
{
  Q_OBJECT
public:
  explicit WgtDocViewPanel( DocumentHeader::DocumentClass defaultClass, bool pinningEnabled, QWidget *parent = nullptr);
  ~WgtDocViewPanel();  
  void setPinningEnabled( );
  bool isPinningEnabled();  

  DocumentHeader::DocumentClass getCurrentDocumentClass();
  DocumentHeader* getDocumentById(const QString& id);
  const QDateTime getDateBegin();
  const QDateTime getDateEnd();
  void addDocuments(const QList<DocumentHeader*> &documents);
  void clearDocumentList();  
  DocumentHeader* currentDocument();
  QStringList selectedDocuments();
  bool isToolbarAnimationActive();

  void setToolbarEnabled( bool enabled );
  void setAnimationToolbarEnabled( bool enable );

  enum PanelAction {
    kPinToMapStart,    
    kPinningAddCurrentDocumentToMap,
    kPinningRemoveCurrentDocumentFromMap,
    kPinningRemoveSelectedDocumentsFromMap,
    kPinningEnable,    
    kCurrentDocumentChanged,
    kSelectedDocumentsChanged,
    kTransparencyChanged,
    kReloadButtonClicked
  };

  enum TreeWidgetsColumns {
    kHiddenDataColumn = 0,
    /* FAXES */
    kFaxTT ,
    kFaxAA ,
    kFaxCCCC ,
    kFaxII ,
    kFaxDateColumn,

    /* SATELITE IMAGES */
    kSateliteTT,
    kSateliteAA,
    kSateliteCCCC,
    kSateliteII,
    kSateliteDateColumn,

    /* MAPS */
    kMapImageDateTime,
    kMapImageName,
    kMapImageHour,
    kMapImageCenter,
    kMapImageModel,
    kMapImageJobName,

    /* FORMAL DOCUMENTS */
    kFormalDocumentDateTime,
    kFormalDocumentName,
    kFormalDocumentHour,
    kFormalDocumentCenter,
    kFormalDocumentModel,
    kFormalDocumentJobName
  };
  void addConditionString(TreeWidgetsColumns column, const QString& value);

  static const int kColumnCount = kFormalDocumentJobName + 1;

  void removeAllConditions();
  void addCondition( TreeWidgetsColumns column, Condition condition );
  QString getDisplayFilterForColumn( TreeWidgetsColumns col);
  void removeCondition( TreeWidgetsColumns column );
  void setCurrentDocument( const QString& id );

  DataModel::TransparencyMode getTranspacencyMode();
  int getTransparencyValue();

  void setCurrentSlide( int slide );    

  void setDocumentList(const QList<DocumentHeader*> *documentsInfo);
  void setDocumentListEnabled( bool enabled );  

public slots:
  void slotImagePinningRemoved( const QString& name );
  void slotImagePinningAdded( const QString& name);


signals:  
  void signalDocumentClassChanged( DocumentHeader::DocumentClass docClass );
  void signalAction( PanelAction action );
  void signalSlidesCountUpdate( int current, int total );
  void signalConditionAdd( TreeWidgetsColumns column, Condition condition );
  void signalConditionRemove( TreeWidgetsColumns column );
  void signalPanelDone();

private slots:
  void slotComboBoxSelectionChanged();  
  void slotCurrentDocumentChanged(QTreeWidgetItem *, QTreeWidgetItem *);
  void slotFileSelected(const QString& fileName);    


  void slotPinToMapEnable();

  void slotPinToMapAddImageToMap();  
  void slotPinRemoveImageFromMap();  

  void slotInitialLoad();

  void slotFilterActivated(int col, QAction* act);
  void slotCheckActivated();
  void slotConditionsPanelClosed();
  void slotGbTransparencySelectionChanged(int);
  void slotSliderTransparencyValueChanged();
  void slotTreeWidgetSelectionChanged();
  void slotSliderAnimationSlideValueChanged(int);


  void slotShowContextMenuForDocumentWidget( const QPoint& pnt);
  void slotActionAnimateAll();
  void slotActionAnimateNone();
  void slotRemoveCurrentDocumentFromMap();
  void slotHeaderGeometryChanged();
  void slotOnPbReloadClicked();

private:
  static QList<TreeWidgetsColumns> columnsForDocumentClass( DocumentHeader::DocumentClass docClass );
  bool isControlsHidden();
  void restoreControlsVisibility();
  void restoreTreeControlsVisibility();
  void restoreButtonIcons();
  bool isItemsAnyMarked( const QList<QTreeWidgetItem*> items );
  bool isItemMarked( QTreeWidgetItem* item );
  void markTreeWidgetItem( QTreeWidgetItem* item, bool mark );
  void markItemByName( DocumentHeader::DocumentClass docClass, const QString& name, bool mark );
  void setItemsSelected( const QList<QTreeWidgetItem*> items, bool isSelected );
  QList<QTreeWidgetItem*> selectedItems();
  QTreeWidgetItem* currentItem();
  void setCurrentItem(QTreeWidgetItem* item);

  void disableCondition( TreeWidgetsColumns column );  

  void generateCustomCondition(TreeWidgetsColumns col);  


private:
  void treeStateSave();
  void treeStatesSaveToFS();
  void treeStateRestore();
  void treeStatesRestoreFromFs();
  void updateItemsMarks();

  enum TreeItemRoles {
    kTreeItemDataRole = Qt::UserRole,
    kTreeItemMarkRole
  };
  Ui::WgtDocViewPanel *ui;
  QHash<QString, DocumentLocal*> localDocuments_;

  bool isPinningEnabled_ = false;


  SelectDateWidget *dateSelectWidget_ = nullptr;

  QHash<TreeWidgetsColumns, QCheckBox*> conditionsCheckBoxes_;

  QHash<TreeWidgetsColumns, Condition> conditions_;
  QList<QTreeWidgetItem*> oldSelectedItems_;

  QHash<int, QPair<QByteArray,QByteArray>> savedStates_;

  QMap<QString, DocumentFax> faxDocuments_;
  QMap<QString, DocumentMapImage> mapImageDocuments_;
  QMap<QString, DocumentFormal> formalDocuments_;
  QMap<QString, DocumentSatelite> satelliteDocuments_;
  DocumentHeader dummy_ = DocumentHeader();

  QStringList markedItems_;
};

}
}

#endif

