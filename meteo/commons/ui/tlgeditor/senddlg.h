#ifndef METEO_NOVOST_UI_TLGEDITOR_LIB_SENDDLG_H
#define METEO_NOVOST_UI_TLGEDITOR_LIB_SENDDLG_H

#include <qdialog.h>
#include <qdatetime.h>

namespace Ui {
class SendDlg;
}


namespace meteo {

//!
class SendDlg : public QDialog
{
  Q_OBJECT

public:
  explicit SendDlg(QWidget *parent = 0);
  ~SendDlg();

  QByteArray ahead() const;

  QDateTime dateTime() const;

private:
  Ui::SendDlg* ui_;
};

} // meteo

#endif // METEO_NOVOST_UI_TLGEDITOR_LIB_SENDDLG_H
