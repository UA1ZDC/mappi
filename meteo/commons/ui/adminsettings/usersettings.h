#pragma once

#include "usermodel.h"
#include <qwidget.h>


namespace Ui {
  class UserSettings;
}

namespace meteo {

namespace ui {

class UserSettings :
   public QWidget
{
  Q_OBJECT
public :
  explicit UserSettings(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  virtual ~UserSettings();

private :
  void initSequence();
  void setCurrentIndex(int row);

  app::User getUser(const QModelIndex& index);
  int findRowById(UserModel* model, int id);

private slots :
  void depatmentView();
  void rankView();

  void reload();
  void updateUser();
  void appendUser();

  void showProfile(const QModelIndex& index);

private :
  Ui::UserSettings* ui_;

  int sequence_;
  UserModel* model_;
};

}

}
