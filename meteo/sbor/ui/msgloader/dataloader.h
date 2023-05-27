#ifndef METEO_SBOR_UI_MSGLOADER_DATALOADER_H
#define METEO_SBOR_UI_MSGLOADER_DATALOADER_H


#include <qwidget.h>
#include <qcombobox.h>
#include <qdatetime.h>

#include <meteo/commons/rpc/rpc.h>


class QSettings;
class QDir;
class QProcess;

namespace Ui {
class DataLoader;
}
namespace meteo {
namespace msgcenter {
class ProcessMsgReply;
}

class ObanalStatus;
class LoaderStatus;
class DocumentStatus;
class ClimatStatus;
class Loader;

class DataLoader : public QWidget
{
  Q_OBJECT

public:
  explicit DataLoader(QWidget* parent = nullptr);
  ~DataLoader();

private slots:
  void slotOpenDirDialog();
  void slotLoadMsg();
  void slotSwitchTableBox();
  void slotSwitchDayBtn();
  void slotClose();
  void slotRunObanal();
  void slotRunDocument();
  void slotRunClimat();
  void slotReadProcOutput();
  void slotReadProcError();
  void slotStopObanal();
  void slotStopDocument();
  void slotStopClimat();

  void slotAddDirs(QString name);
  void slotAddFiles(QString name);
  void slotTableItemIncrement(int);
  void slotUpdateDT(QDateTime, QDateTime);
  void slotDebug();

signals:
  void signalRunLoadMsg();


protected:
 virtual void keyReleaseEvent(QKeyEvent* event);

private:
  void loadSettings();
  void saveSettings();
  void setTableBtn(bool show);
  void initStatTable();
  void clearStatTable();
  void tableItemIncrement(int row);
  void setMaskFileList();
  bool checkFileName(QString& name);
  QDate makeDateFromUi();
  void clearDTs();
  void updateTextDts();
  void checkObanal();
  void checkDocument();
  void checkClimat();
  void aeroRunObanal();
  void oceanRunObanal();
  void surfRunObanal();
  void gribRunObanal();
  void showStatus();
  void recusiveCountFile(const QDir &directory);


private:
  Ui::DataLoader* ui_ = nullptr;
  QSettings* settings_ = nullptr;
  QDir* dir_ = nullptr;
  QComboBox* dirsBox_ = nullptr;
  QComboBox* filesBox_ = nullptr;
  QStringList maskFileList_;
  QStringList templList_;
  QDateTime dts_;
  QDateTime dte_;
  QProcess* obanalAeroProc_ = nullptr;
  QProcess* obanalOceanProc_ = nullptr;
  QProcess* obanalSurfProc_ = nullptr;
  QProcess* obanalGribProc_ = nullptr;
  QProcess* documentProc_ = nullptr;
  QProcess* climatProc_ = nullptr;
  ObanalStatus* obanalStatus_ = nullptr;
  LoaderStatus* loaderStatus_ = nullptr;
  DocumentStatus* documentStatus_ = nullptr;
  ClimatStatus* climatStatus_ = nullptr;
  Loader* loader_ = nullptr;
  int countFiles_ = 0;
  QThread* thread_ = nullptr;
};

}

#endif // METEO_SBOR_UI_MSGLOADER_DATALOADER_H
