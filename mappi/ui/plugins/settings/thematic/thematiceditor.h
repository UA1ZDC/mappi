#ifndef MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICEDITOR_H
#define MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICEDITOR_H

#include <qdialog.h>
#include <qmap.h>

#include <mappi/proto/thematic.pb.h>

class ExprGuidePaintWidget;

namespace Ui {
  class ThematicEditor;
}

namespace mappi {

class ExprEditor;

class ThematicEditor : public QDialog
{
  Q_OBJECT

  using Thematic = conf::ThematicProc;
  using Variable = conf::ThematicVariable;

  enum ExprType { kRed, kGreen, kBlue };

public:
  explicit ThematicEditor(QWidget *parent = 0);
  ~ThematicEditor();

  void open(const Thematic& thematic);

signals:
  void complite(const Thematic& thematic);

private slots:
  void loadSettings(const QString& filename);
  void saveSettings(const QString& filename) const;

  void slotSave();

  void slotChangeType(const QString& type);

  void slotEditRed();
  void slotEditGreen();
  void slotEditBlue();
  void slotSetExpr(const QString& expr, const QMap<QString, Variable>& vars);

private:
  void openExprEditor(const QString& expr);
  void fillEditor();
  void changeType(::mappi::conf::ThemType type);

private:
  Ui::ThematicEditor*                   ui_;
  ExprEditor*                           exprEditor_;
  ExprType                              exprType_;
  QMap<ExprType, ExprGuidePaintWidget*> exprsVis_;
  Thematic                              thematic_;
  bool                                  loadedSettings_;
};

} // mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_THEMATICEDITOR_H
