#ifndef METEO_NOVOST_UI_COLORALERT_COLORALERTDLG_H
#define METEO_NOVOST_UI_COLORALERT_COLORALERTDLG_H

#include <qdialog.h>

#include <meteo/commons/proto/meteotablo.pb.h>

class QTreeWidgetItem;

namespace Ui {
class ColorAlertDlg;
}

namespace meteo {
namespace sprinf {
class MeteoParams;
} // sprinf
} // meteo

namespace meteo {
namespace tablo {
class ConditionItem;
} // tablo
} // meteo

namespace meteo {

class ColorAlertDlg : public QDialog
{
  Q_OBJECT

public:
  explicit ColorAlertDlg(QWidget *parent = 0);
  ~ColorAlertDlg();

  void loadSettings();
  void setCurrent(const QString& descrName);
  void saveSettings() const;

  void setStatus(const QString& text);

  QString currentDescr() const;

public slots:
  void slotContextMenu(const QPoint& pos);
  void slotItemChanged(QTreeWidgetItem* item, int column);
  void slotEditItem(QTreeWidgetItem* item, int column);
  void slotParamChanged(int index);
  void slotApply();

private:
  tablo::ConditionItem* findItem(const tablo::Condition& cond) const;
  tablo::ColorAlert toProto() const;

private:
  tablo::Settings opt_;

  Ui::ColorAlertDlg* ui_;
};

}

#endif
