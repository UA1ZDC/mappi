#ifndef METEO_UI_SETTINGS_MSGSETTINGS_H
#define METEO_UI_SETTINGS_MSGSETTINGS_H

#include "settingswidget.h"
#include <meteo/commons/proto/msgsettings.pb.h>
#include <commons/geobasis/geopoint.h>

namespace Ui {
   class MsgSettings;
}
namespace meteo {
enum MsgType
{
  kDebug,
  kInfo,
  kWarning,
  kError,
  kCritical
};

enum MsgCol
{
  kName = 0,
  kReg,
  kShow
};

class MsgSettings : public SettingsWidget
{
  Q_OBJECT

public:
  explicit MsgSettings(QWidget* parent = nullptr);
  ~MsgSettings();

private slots:
  void slotChangeState();

private:
  void save();
  void load();
  meteo::settings::MsgSettings msg();
  bool setMsg(const meteo::settings::MsgSettings& opt);


  QPixmap setCellIcon(bool status) const;
  void setCellPixmap(const QPixmap& pixmap, int row, int column);

  void setRowIcons(meteo::settings::RegLevel level, int row);
  bool eventFilter(QObject* object, QEvent* event);

  meteo::settings::RegLevel level(meteo::MsgType type);
  meteo::settings::RegLevel debug();
  meteo::settings::RegLevel info();
  meteo::settings::RegLevel warning();
  meteo::settings::RegLevel error();
  meteo::settings::RegLevel critical();


private:
  Ui::MsgSettings* ui_;
};

} // meteo

#endif // METEO_UI_SETTINGS_MSGSETTINGS_H
