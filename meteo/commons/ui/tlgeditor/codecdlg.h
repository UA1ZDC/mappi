#ifndef METEO_NOVOST_UI_TLGEDITOR_LIB_CODECDLG_H
#define METEO_NOVOST_UI_TLGEDITOR_LIB_CODECDLG_H

#include <qdialog.h>

class QListWidgetItem;

namespace Ui {
class CodecDlg;
}

namespace meteo {

//!
class CodecDlg : public QDialog
{
  Q_OBJECT

public:
  enum Button { kReloadButton, kSaveButton, kCancelButton };

  explicit CodecDlg(QWidget *parent = 0);
  ~CodecDlg();

  //!
  QString codecName() const;
  //!
  void setCurrentCodecName(const QString& name);

  //!
  Button run();

public slots:
  void slotCurrentChanged(QListWidgetItem* current, QListWidgetItem* previous);
  void slotReload();
  void slotSave();

private:
  Ui::CodecDlg* ui_;
  Button button_;
};

} // meteo

#endif // METEO_NOVOST_UI_TLGEDITOR_LIB_CODECDLG_H
