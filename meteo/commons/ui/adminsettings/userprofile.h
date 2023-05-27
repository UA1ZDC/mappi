#pragma once

#include "dictmodel.h"
#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/auth/gecos.h>
#include <qdialog.h>
#include <qbuttongroup.h>
#include <qcalendarwidget.h>


namespace Ui {
  class UserProfile;
}

namespace meteo {

namespace ui {



class UserProfile :
  public QDialog
{
  Q_OBJECT

public :
  typedef enum {
    APPEND = 0,
    UPDATE,

    COUNT
  } role_t;

  typedef QMap<QString, auth::Gecos> gecosList_t;

public :
  explicit UserProfile(role_t role, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~UserProfile();

  void setUser(app::User* user);
  void setUserList(const QStringList& list);
  void setGecosList(const gecosList_t& list);

private :
   int findRowById(DictModel* model, int id);
   int findIdByRow(DictModel* model, int row);

   void switchAvatar(const QByteArray& buffer);

private slots :
  void save();
  void loadAvatar();
  void clearAvatar();
  void switchUser(const QString& username);

private :
  Ui::UserProfile* ui_;
  DictModel* rankModel_;
  DictModel* departmentModel_;
  QCalendarWidget* calendar_;
  QButtonGroup* bg_;

  app::User* user_;
  gecosList_t gecosList_;
};

}

}
