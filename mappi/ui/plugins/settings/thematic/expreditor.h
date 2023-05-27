#ifndef MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_EXPREDITOR_H
#define MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_EXPREDITOR_H

#include <qdialog.h>
#include <qmap.h>

#include <mappi/proto/thematic.pb.h>

class ExprGuidePaintWidget;
class QPushButton;
class QLineEdit;
class QSignalMapper;

namespace Ui {
  class ExprEditor;
}

namespace mappi {

class VariableEditor;

class ExprEditor : public QDialog
{
  Q_OBJECT
  using Variable = conf::ThematicVariable;

public:
  explicit ExprEditor(QWidget *parent = 0);
  ~ExprEditor();

  void addNewVariable(const Variable& variable);
  void removeVariable(const Variable& variable);

public slots:
  void open(const QString& expr, const  QVector<Variable>& vars);

signals:
  void complite(const QString&, const QMap<QString, Variable>&);

private slots:
  void complited();

  void slotSetVars(const  QMap<QString, Variable>& vars);

  void slotVar(const QString& variable);
  void slotSin();
  void slotCos();
  void slotTan();
  void slotCtg();
  void slotComma();
  void slotAbs();
  void slotLeftBracket();
  void slotSelectionText();

private:
  void loadSettings(const QString &filename);
  void saveSettings(const QString &filename);
  void refreshExpr();

private:
  Ui::ExprEditor*         ui_;
  ExprGuidePaintWidget*   exprVis_;
  VariableEditor*          variableEditor_;

  QMap<QString, Variable> vars_;

  QString                 selectedText_;
  int                     startSelectedText_;
};

}

#endif // MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_EXPREDITOR_H
