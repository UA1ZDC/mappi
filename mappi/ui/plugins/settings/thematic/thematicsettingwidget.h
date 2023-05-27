#ifndef MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICSETTINGWIDGET_H
#define MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICSETTINGWIDGET_H

#include <qmap.h>

#include <mappi/ui/plugins/settings/base/settingwidget.h>

#include <mappi/proto/thematic.pb.h>

class ExprGuidePaintWidget;
class QTableWidget;
class QTableWidgetItem;
class QPushButton;
class QPushButton;

namespace Ui {
  class ThematicSettingWidget;
}

class TColorGradList;

namespace mappi {

class ThematicEditor;
class ExprEditor;

class ThematicSettingWidget : public meteo::app::SettingWidget
{
  Q_OBJECT

  using Thematic  = conf::ThematicProc;

  enum ExprType { kRed, kGreen, kBlue };
  enum Column { kEnable, kName, kType, kPalette };

public:
  explicit ThematicSettingWidget(QWidget *parent = nullptr);
  ~ThematicSettingWidget();

public slots:
  void slotLoadConf();
  void slotSaveConf() const;

private slots:
  void slotChangeThematic(QTableWidgetItem* current);
  void slotDoubleClickThematic(int row, int col);
  void slotRemoveThematic();

  void slotEditRed();
  void slotEditGreen();
  void slotEditBlue();
  void slotSetExpr(const QString& expr, const QMap<QString, conf::ThematicVariable>& vars);

  void slotSaveThematic(const Thematic& them);

private:
  void loadSettings(const QString& filename);
  void saveSettings(const QString& filename) const;

  void fillTable();
  void fillContent();

  void openExprEditor(const QString& expr);

  QString cellText(int row, Column col);

private:
  Ui::ThematicSettingWidget*             ui_;
  ThematicEditor*                        thematicEditor_;
  ExprEditor*                            exprEditor_;
  QMap<ExprType, ExprGuidePaintWidget*>  exprsVis_;

  QMap<std::string, TColorGradList>          grads_;//имя -

  QMap<QString, Thematic>                thematics_;
  Thematic                               thematic_;
  ExprType                               exprType_;

  bool                                   loadedSettings_;

private:
  static QString getThematicType(conf::ThemType type);

  static void insertRow(QTableWidget* table, const Thematic& them);
  static void fillRow(QTableWidget* table, int row, const Thematic& them);

  static QTableWidgetItem* createCell(Column col, const Thematic& them);
  static void fillCell(QTableWidgetItem* item, Column col, const Thematic& them);
};

} // mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICSETTINGWIDGET_H
