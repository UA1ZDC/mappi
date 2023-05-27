#ifndef METEO_COMMONS_UI_CUSTOMVIEWER_CUSTOMVIEWER_H
#define METEO_COMMONS_UI_CUSTOMVIEWER_CUSTOMVIEWER_H

#include <QtGui>
#include <meteo/commons/ui/custom/checkpanel.h>
#include <meteo/commons/ui/custom/selectdatewidget.h>
#include "customvieweritem.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/viewheader/viewheader.h>

#include "customviewerdatamodel.h"
#include "viewtablewidget.h"
#include <meteo/commons/ui/conditionmaker/conditiondialogs/conditionaldialogstring.h>
#include <meteo/commons/ui/conditionmaker/conditiondialogs/conditioncreationdialogint.h>

namespace Ui {
  class CustomViewer;
}


namespace meteo {

enum ButtonType
{
  Back,
  Next
};

class CustomViewer : public QWidget
{
  Q_OBJECT

  friend class CustomTreeViewer;
  
public:
  CustomViewer(QWidget *parent, meteo::CustomViewerDataModel *model = nullptr);
  virtual ~CustomViewer() override;

  virtual bool init();
  void reload();

  void addWidgetToPanel(int index, QWidget* w);

  //filters  
  void initFilters();


  int logicalIndexByName(const QString& column_name);
  QString lastError() { return lastError_; }

  QMap<int, QCheckBox*>& checkmap() { return checkmap_; }

  meteo::bank::ViewHeader* header() { return header_; }
  QSettings* settings() { return settings_; }
  CheckPanel* checkpanel();
  ViewTableWidget* tablewidget();
  QMenu* cellMenu() { return cellMenu_; }

  void setColumnId( const QString& column_name ) { id_column_ = column_name; }
  const QString& getColumnId() const { return id_column_; }

protected:
  virtual void setDefaultSectionSize();
  virtual void setDefaultSectionOrder();
  virtual void setDefaultSectionVisible();

protected slots:
  virtual void slotSaveGeometryAndState();
  virtual void slotRestoreGeometryAndState();
  virtual void slotPanelClosed();  

  void slolAllRecordsCountUpdate(long int value);

private slots:
  void slotHideLeftPanel(bool);

private:
  static const int minimumColumnWidth = 10;  

  Ui::CustomViewer* ui_;  

  QString lastError_;


  QMenu*                   columnMenu_;
  QMenu*                   cellMenu_;
  QAction*                 rm_filter_action_;
  QAction*                 rm_sort_action_;
  QAction*                 actionFilterBySelect_;
  QAction*                 actionSortAsc_;
  QAction*                 actionSortDes_;
  QSettings*               settings_;  
  //QHeaderView*           header_;
  meteo::bank::ViewHeader* header_;  
  SelectDateWidget*        timeWidget_;

  QList<QAction*>          actlist_;
  QList<QCheckBox*>        checklist_;

  QMap<int,QList< QPair<QString, QString> > > menu_;
  QMap<int,       QCheckBox*> checkmap_;

  meteo::CustomViewerDataModel* dataModel_;

  QString id_column_ = QString("_id");

protected:
  void setDataModel(meteo::CustomViewerDataModel* model ) { this->dataModel_ = model; }
  meteo::CustomViewerDataModel* getDataModel() { return  this->dataModel_; }

private:
  void addItem(int row, const proto::CustomViewerTableRecord& data);
  void setBtnDisabled(ButtonType btn, bool state) const;
  void createBottomPanel();
  void createTable();
  void createMenu();
  void initConnect();
  void createHeader();
  void createAdditionalMenu();
  void createConditionTime(int col);
  void createCondition(int col);
  bool checkBeforeChaningCondition(int col);
  void fillTableHeaders();
  void setConditionEnabled(int col, bool on);
  void addItemToMenu(int columnIndex, const QString& value, const QString& itemText);

protected slots:
  virtual void slotUpdate();

private slots:
  void slotSectionMoved(int pos, int index);
  void slotDataLoaded( const QList<proto::CustomViewerTableRecord> * data, const int &page);
  void slotFiltersChanged();
  void slotLock(bool state);
  void slotColumnsClicked();
  void slotColumnCheckToggled(bool on);
  void slotSqlConditionClicked(bool checked);
  void slotMoveColumn(int oldVisualIndex, int newVisualIndex);
  void slotDateChecked(bool);
  void slotContextMenu();
  void slotCopyToClipboard();
  void slotFilterBySelect();
  void slotRemoveFilterBySelect();
  void slotSortUp();
  void slotSortDown();
  void slotRemoveSort();
  void slotSortResult();
  void slotBeginClicked();
  void slotBackClicked();
  void slotNextClicked();
  void slotEndClicked();

  void slotHeaderGeometryChanged();
  void slotMenuActivated(int col, QAction* act);
  void slotPageSizeChanged();


};

}

#endif
