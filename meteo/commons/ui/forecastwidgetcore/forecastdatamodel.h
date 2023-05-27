#ifndef METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTDATAMODEL_H
#define METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTDATAMODEL_H

#include <meteo/commons/ui/customviewer/customviewerdatamodel.h>
namespace meteo {
namespace ui {

class ForecastDataModel : public CustomViewerDataModel
{
public:
  ForecastDataModel(int pageSize);

  virtual QString cellItemText(const QString& column, QHash<int, QString> row);
  virtual QString cellItemTooltip(const QString& column, QHash<int, QString> row);


  virtual const QString& stringTemplateTotalRecords() const override;
  virtual const QString& windowTitle() const override;
  virtual proto::CustomViewerId tableId() const override;
  
private:
  void loadDescr();
  void loadLevelTypes();
  void loadReplaceValues();
  void loadCenters();

private:
  QHash<QString, QString> descrReplaceList_;
  QHash<QString, QString> levelTypeReplaceList_;
  QHash<QString, QString> descrTooltipList_;
  QHash<QString, QHash<QString, QString>> bufrCodes_;
  QHash<QString, QString> centerTooltips_;
  QHash<QString, QString> centerReplaceTexts_;

};

}
}
#endif
