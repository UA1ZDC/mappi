#ifndef METEO_UI_SETTINGS_APPSTART_EDITOR_H
#define METEO_UI_SETTINGS_APPSTART_EDITOR_H

#include <QDialog>

class QEvent;
class QStringList;
class QTreeWidgetItem;

namespace Ui {
  class AppStartEditor;
}

namespace meteo {

namespace app {
  class Application;
} // app

class AppStartEditorWidget : public QDialog
{
  Q_OBJECT

public:
  explicit AppStartEditorWidget(const QStringList& usedIds = QStringList(), QWidget* parent = nullptr);
  ~AppStartEditorWidget();

  void init(const app::Application* const config = nullptr);
  app::Application config() const;

protected:
  bool eventFilter(QObject* object, QEvent* event);

private slots:
  void slotOpenDialog();
  void slotViewArgs();
  void slotAddArg();
  void slotEditArg();
  void slotEditArg(QTreeWidgetItem* item);
  void slotRemoveArg();
  void slotRemoveArg(QTreeWidgetItem* item);
  void slotValidateConfig();
  void slotEnableButtons();

private:
  void addArguments(const QStringList& args);
  QStringList arguments() const;

  bool validatePath();
  bool validateId();

private:
  Ui::AppStartEditor* ui_;
  const QStringList& usedIds_;

  enum ArgColumn {
    KEY = 0,
    VALUE
  };

};

} //meteo

#endif // METEO_UI_SETTINGS_APPSTART_EDITOR_H
