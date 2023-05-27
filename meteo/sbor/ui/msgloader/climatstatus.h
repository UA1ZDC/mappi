#ifndef METEO_SBOR_UI_MSGLOADER_CLIMATSTATUS_H
#define METEO_SBOR_UI_MSGLOADER_CLIMATSTATUS_H

#include <QDialog>

class QSettings;

namespace Ui {
class ClimatStatus;
}

namespace meteo {

class ClimatStatus : public QDialog
{
  Q_OBJECT

public:
  explicit ClimatStatus(QWidget *parent = nullptr);
  ~ClimatStatus();

public slots:
  void slotClimatDone();
  void slotSetNotDone();

private slots:
  void slotSaveSettings();

signals:
  void stopClimat();

protected:
  void closeEvent(QCloseEvent* e);

private:
  void loadSettings();
  void checkDone();

private:
  Ui::ClimatStatus *ui_;
  QSettings* settings_ = nullptr;
  bool climatSt_ = false;
};

}
#endif // METEO_SBOR_UI_MSGLOADER_CLIMATSTATUS_H
