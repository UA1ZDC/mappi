#ifndef METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTACCURACYDATAMODEL_H
#define METEO_COMMONS_UI_PLUGINS_FORECASTWIDGETCORE_FORECASTACCURACYDATAMODEL_H
#include <meteo/commons/ui/customviewer/customviewerdatamodel.h>

namespace meteo {
namespace ui {

class ForecastAccuracyDataModel : public CustomViewerDataModel
{
public:
  ForecastAccuracyDataModel(int page);

  virtual QString cellItemText(const QString& column, QHash<int, QString> row);
  virtual QString cellItemTooltip(const QString& column, QHash<int, QString> row);


  virtual const QString& stringTemplateTotalRecords() const override;
  virtual const QString& windowTitle() const override;
  virtual proto::CustomViewerId tableId() const override;
  
private:  
  void loadDescr();
  void loadLevelTypes();
  void loadPunkts();
  void loadCenters();
  // загшружаем названия методов
  void loadMethodNames();

private:
  QHash<QString, QString> centerTooltips_;
  QHash<QString, QString> centerReplaceTexts_;
  QHash<QString, QString> punktIdsTexts_;
  QHash<QString, QString> punktIdsTooltips_;
  QHash<QString, QString> valDescrTexts_;
  QHash<QString, QString> valDescrTooltips_;
  QHash<QString, QString> typeLevelTexts_;
  QHash<QString, QString> methodDescrTexts_;  

};


}
}
#endif
