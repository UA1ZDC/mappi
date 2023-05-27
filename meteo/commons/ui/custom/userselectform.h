#ifndef METEO_PRODUCT_UI_PLUGINS_DOCUMENTPLUGIN_USERSELECTFORM_H
#define METEO_PRODUCT_UI_PLUGINS_DOCUMENTPLUGIN_USERSELECTFORM_H

#include <QWidget>
#include <meteo/commons/proto/web.pb.h>

namespace Ui {
class UserSelectForm;
}

namespace meteo {

class UserSelectForm : public QWidget
{
  Q_OBJECT
  enum ColumnRole {
    kLogin = 32,
    kName,
    kSurname,
    kPatron,
    kRank,
    kDepartment
  };

public:
  explicit UserSelectForm(QWidget *parent = nullptr);
  ~UserSelectForm();

  QStringList getCurrentUsers();

private slots:
  void slotOk();
  void slotRefresh();

private:
  void getUsers();

private:
  Ui::UserSelectForm* ui_ = nullptr;
};

}

#endif // METEO_PRODUCT_UI_PLUGINS_DOCUMENTPLUGIN_USERSELECTFORM_H
