#ifndef METEO_SBOR_UI_MSGLOADER_OBANALSTATUS_H
#define METEO_SBOR_UI_MSGLOADER_OBANALSTATUS_H

#include <QDialog>
#include <qsettings.h>

namespace Ui {
class ObanalStatus;
}

namespace meteo {

class ObanalStatus : public QDialog
{
  Q_OBJECT

public:
  explicit ObanalStatus(QDialog *parent = nullptr);
  ~ObanalStatus();

public slots:
  void slotAeroDone();
  void slotOceanDone();
  void slotSurfDone();
  void slotGribDone();
  void slotSetAllNotDone();

private slots:
  void slotSaveSettings();

signals:
  void stopObanal();
  void finished();

protected:
  void closeEvent(QCloseEvent* e);

private:
  void loadSettings();
  void checkAllDone();

private:
  Ui::ObanalStatus* ui_ = nullptr;
  QSettings* settings_ = nullptr;
  bool aeroSt_ = false;
  bool oceanSt_ = false;
  bool surfSt_ = false;
  bool gribSt_ = false;
};

}

#endif // METEO_SBOR_UI_MSGLOADER_OBANALSTATUS_H
