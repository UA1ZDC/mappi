#ifndef METEO_COMMONS_UI_METEOTABLO_STATIONDLG_H
#define METEO_COMMONS_UI_METEOTABLO_STATIONDLG_H

#include <qdialog.h>
#include <meteo/commons/proto/sprinf.pb.h>

namespace Ui {
class StationDlg;
}

namespace meteo {

class StationDlg : public QDialog
{
  Q_OBJECT

public:
  explicit StationDlg(QWidget *parent = 0);
  virtual ~StationDlg();

  void init();
  void setStatus(const QString& text, int timeoutSec = -1);

  sprinf::Station getStation() const;

signals:
  void addClicked();

private slots:
  void slotClearStatus();

protected:
  virtual void closeEvent(QCloseEvent *e);

private:
  Ui::StationDlg* ui_;
};

} // meteo

#endif // METEO_NOVOST_UI_METEOTABLO_STATIONDLG_H
