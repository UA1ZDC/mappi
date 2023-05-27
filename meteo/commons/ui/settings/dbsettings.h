#ifndef METEO_UI_SETTINGS_DBSETTINGS_H
#define METEO_UI_SETTINGS_DBSETTINGS_H

#include <sql/proto/dbsettings.pb.h>
#include <qdialog.h>

namespace Ui {
   class DbSettings;
}

namespace meteo {

class DbSettings : public QDialog
{
  Q_OBJECT
public:
  explicit DbSettings(const ::meteo::settings::DbConnection& originalSettings,
                      QWidget* parent = nullptr);
  ~DbSettings();  

  bool isChanged();

  QString getHumanName();
  QString getName();
  QString getHost();
  int getPort();
  QString getLogin();
  QString getPassword();

private slots:
  void slotChanged();

private:
  Ui::DbSettings* ui_;
  bool isChanged_;
};

}

#endif
