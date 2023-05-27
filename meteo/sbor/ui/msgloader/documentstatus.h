#ifndef METEO_SBOR_UI_MSGLOADER_DOCUMENTSTATUS_H
#define METEO_SBOR_UI_MSGLOADER_DOCUMENTSTATUS_H

#include <QDialog>

class QSettings;

namespace Ui {
class DocumentStatus;
}

namespace meteo {

class DocumentStatus : public QDialog
{
  Q_OBJECT

public:
  explicit DocumentStatus(QWidget *parent = nullptr);
  ~DocumentStatus();

public slots:
  void slotDocumentDone();
  void slotSetNotDone();

private slots:
  void slotSaveSettings();

signals:
  void stopDocument();

protected:
  void closeEvent(QCloseEvent* e);

private:
  void loadSettings();
  void checkDone();

private:
  Ui::DocumentStatus* ui_ = nullptr;
  QSettings* settings_ = nullptr;
  bool documentSt_ = false;
};

}
#endif // METEO_SBOR_UI_MSGLOADER_DOCUMENTSTATUS_H
