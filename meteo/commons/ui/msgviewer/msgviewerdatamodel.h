#ifndef METEO_COMMONS_UI_MSGVIEWER_MSGVIEWERDATAMODEL_H
#define METEO_COMMONS_UI_MSGVIEWER_MSGVIEWERDATAMODEL_H

#include <meteo/commons/ui/customviewer/customviewerdatamodel.h>
#include <meteo/commons/ui/msgviewer/leftpanel.h>

namespace meteo {

class MsgViewerDataModel : public meteo::CustomViewerDataModel
{
  Q_OBJECT
public:
  MsgViewerDataModel(int pageSize, const QString& column_id = QString("id") );

  void updateLastTlgId();

  virtual proto::CustomViewerId tableId() const override;
  virtual QStringList columnMenuValues(const QString& column);

  virtual const QString& stringTemplateTotalRecords() const override;
  virtual const QString& windowTitle() const override;

  virtual QWidget* getLeftPanelWidget() const override;
protected:
  virtual QList<proto::CustomViewerTableRecord> load(QList<Condition> conditions) const override;
private slots:
  void onFiltersChanged();
  void slotChangeCondition();

private:
  long int lastId_;
  LeftPanel* leftPanel_;
  QString column_id_ = QString("id");

};

}
#endif
