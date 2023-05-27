#ifndef MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_CHANNELEDITOR_H
#define MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_CHANNELEDITOR_H

#include <qdialog.h>
#include <qmap.h>
#include <qset.h>

#include <mappi/proto/thematic.pb.h>

#include "channeleditor.h"

class QListWidgetItem;
class QTableWidgetItem;
class QTableWidget;

namespace Ui {
  class VariableEditor;
}
namespace mappi {

class VariableEditor : public QDialog
{
  Q_OBJECT
  enum Column { kSat = 0, kInstr = 1, kChannel = 2 };

  using InstrType = conf::InstrumentType;
  using Variable = conf::ThematicVariable;
  using Channel = conf::ThematicChannel;
  using ChKey = QPair<QString, InstrType>;

public:
  explicit VariableEditor(QWidget *parent = 0);
  ~VariableEditor();

public slots:
  void open(const  QMap<QString, Variable>& vars);

signals:
  void compliteVars(const QMap<QString, Variable>&);

private:
  void init();
  void loadSettings(const QString& filename);
  void saveSettings(const QString& filename);

private slots:
  void slotAddVar();
  void slotRemoveVar();

  void slotAddCh(const ChannelEditor::Channel& channel);
  void slotRemoveCh();

  void slotVarChange(QListWidgetItem *current);
  void slotChChange(QTableWidgetItem* current);

  void slotCompleteEdit();

private:
  Ui::VariableEditor*            ui_;
  ChannelEditor*                 channelEditor_;
  QMap<QString, Variable>        vars_;             //!< Список переменных
  Variable*                      currentVariable_;  //!< Редактируемая переменная


private:
  static void fillTable(QTableWidget* table, const Variable& variable);

  static void insertRow(QTableWidget* table, const Channel& ch);
  static void fillRow  (QTableWidget* table, int row, const Channel& ch);

  static QTableWidgetItem* createCell(Column col, const Channel& ch);
  static void fillCell(QTableWidgetItem* item, Column col, const Channel& ch);

  static bool isValid(const conf::Instrument& instrument);

private:
  static QMap<InstrType, QString> instrumentsName_;   //!< Отображение имени инструмента в его тип
};

} //mappi

#endif // MAPPI_UI_PLUGINS_SETTINGS_THEMATIC_CHANNELEDITOR_H
