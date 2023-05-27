#ifndef METEO_SBOR_UI_MSGLOADER_LOADERSTATUS_H
#define METEO_SBOR_UI_MSGLOADER_LOADERSTATUS_H
#include <QDialog>
#include <qsettings.h>

namespace Ui {
class LoaderStatus;
}

namespace meteo {

class LoaderStatus : public QDialog
{
  Q_OBJECT

public:
  explicit LoaderStatus(QWidget *parent = nullptr);
  ~LoaderStatus();

public slots:
  void slotSetValue(int value);
  void slotSetMaxValue(int maxValue);
  void slotIncrementValue();

private slots:
  void slotClose();
  void slotSaveSettings();

signals:
  void signalStop();
  void signalFinished();
  void signalPrepareToClose();

protected:
  void closeEvent(QCloseEvent* e);

private:
  void loadSettings();

private:
  Ui::LoaderStatus *ui_ = nullptr;
  QSettings* settings_ = nullptr;

};

}

#endif // METEO_SBOR_UI_MSGLOADER_LOADERSTATUS_H
