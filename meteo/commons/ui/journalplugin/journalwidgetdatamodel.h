#ifndef METEO_MASLO_UI_JOURNALPLUGIN_JOURNALWIDGETDATAMODEL_H
#define METEO_MASLO_UI_JOURNALPLUGIN_JOURNALWIDGETDATAMODEL_H

#include <meteo/commons/ui/customviewer/customviewerdatamodel.h>
#include <qthread.h>

#include <meteo/commons/ui/journalplugin/journalleftpanel.h>

namespace meteo {

class JournalWidgetDataModel : public CustomViewerDataModel
{
  Q_OBJECT
public:
  JournalWidgetDataModel(int pageSize);

  virtual void initDataModel() override;
  virtual proto::CustomViewerId tableId() const override;

  virtual const QString& stringTemplateTotalRecords() const override;
  virtual const QString& windowTitle() const override;
  void forceUpdate();

protected:  
  virtual QWidget* getLeftPanelWidget() const override;


private slots:
  void slotOnLeftPanelStateChanged();
  void slotFiltersChanged();
  void slotTimeout();

private:
  QTimer updateTimer_;
  QDateTime lastDt_;
  journal::JournalLeftPanel *leftPanel_;
};

}

#endif
