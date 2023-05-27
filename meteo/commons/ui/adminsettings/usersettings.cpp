#include "usersettings.h"
#include "ui_usersettings.h"
#include "dictview.h"
#include "userprofile.h"
#include <meteo/commons/auth/auth.h>


namespace meteo {

namespace ui {

static const QString SERVICE_NOT_UNAVAILABLE = QObject::tr("Сервис диагностики не доступен");

UserSettings::UserSettings(QWidget* parent /*=*/, Qt::WindowFlags f /*=*/) :
    QWidget(parent, f),
  ui_(new Ui::UserSettings()),
  sequence_(1),
  model_(new UserModel(this))
{
  ui_->setupUi(this);

  ui_->tView->setAlternatingRowColors(true);
  ui_->tView->setSelectionMode(QAbstractItemView::SingleSelection);
  ui_->tView->setSelectionBehavior(QAbstractItemView::SelectRows);

  ui_->tView->setModel(model_);
  ui_->tView->horizontalHeader()->setSectionResizeMode(UserModel::FIO, QHeaderView::Stretch);
  ui_->tView->hideColumn(DictModel::ID);

  initSequence();

  if (0 < model_->rowCount()) {
    setCurrentIndex(0);
    showProfile(model_->index(0, 0));
    ui_->tbUpdate->setEnabled(true);
  } else
    ui_->tbUpdate->setEnabled(false);

  QObject::connect(ui_->tbDepartment, SIGNAL(clicked()), this, SLOT(depatmentView()));
  QObject::connect(ui_->tbRank, SIGNAL(clicked()), this, SLOT(rankView()));

  QObject::connect(ui_->tbReload, SIGNAL(clicked()), this, SLOT(reload()));
  QObject::connect(ui_->tbUpdate, SIGNAL(clicked()), this, SLOT(updateUser()));
  QObject::connect(ui_->tbAppend, SIGNAL(clicked()), this, SLOT(appendUser()));

  QObject::connect(ui_->tView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(showProfile(const QModelIndex&)));
  QObject::connect(ui_->tView, SIGNAL(activated(const QModelIndex&)), this, SLOT(showProfile(const QModelIndex&)));
}

UserSettings::~UserSettings()
{
  delete ui_;
  ui_ = nullptr;
}

void UserSettings::initSequence()
{
  QMap<QString, app::User> users = TUserSettings::instance()->userList();
  if (users.isEmpty())
    return ;

  QList<int> keys;
  QMapIterator<QString, app::User> it(users);
  while (it.hasNext()) {
    it.next();
    keys.push_back(it.value().id());
  }

  qSort(keys.begin(), keys.end());
  sequence_ = keys.last() + 1;
}

void UserSettings::setCurrentIndex(int row)
{
  if (0 <= row && row < model_->rowCount())
    ui_->tView->setCurrentIndex(model_->index(row, 0));
}

app::User UserSettings::getUser(const QModelIndex& index)
{
  QString username = model_->data(model_->index(index.row(), UserModel::USERNAME)).toString();
  return TUserSettings::instance()->getUser(username);
}

int UserSettings::findRowById(UserModel* model, int id)
{
  int res = 0;
  for (int row = 0; row < model->rowCount(); ++row) {
    if (model->data(model->index(row, UserModel::ID)).toInt() == id)
      return res;

    ++res;
  }

  return res;
}

void UserSettings::depatmentView()
{
  DepartmentView view(this);
  view.exec();
}

void UserSettings::rankView()
{
  RankView view(this);
  view.exec();
}

void UserSettings::reload()
{
  if (TUserSettings::instance()->load()) {
    if (0 < model_->rowCount()) {
      setCurrentIndex(0);
      showProfile(model_->index(0, 0));
      ui_->tbUpdate->setEnabled(true);
    } else
      ui_->tbUpdate->setEnabled(false);

    return ;
  }

  error_log.msgBox() << SERVICE_NOT_UNAVAILABLE;
}

void UserSettings::updateUser()
{
  UserProfile profile(UserProfile::UPDATE, this);

  app::User user;
  user.CopyFrom(getUser(ui_->tView->currentIndex()));
  profile.setUser(&user);

  QStringList list;
  list << user.username().c_str();
  profile.setUserList(list);
  if (profile.exec() == QDialog::Accepted) {
    if (meteo::TUserSettings::instance()->updateUser(user)) {
      showProfile(ui_->tView->currentIndex());
      return ;
    }

    error_log.msgBox() << SERVICE_NOT_UNAVAILABLE;
  }
}

void UserSettings::appendUser()
{
  auth::Auth auth;
  if (!auth.init(auth::Auth::kDirect)) {
    error_log.msgBox() << QObject::tr("Не удается получить список пользователей из системы");
    return ;
  }

  // удаление из системного списка тех пользователей,
  // которые уже есть в программе, исключение повторного добавления
  QStringList list = auth.userList();
  for (auto item : TUserSettings::instance()->usernameList())
    list.removeOne(item);

  UserProfile::gecosList_t gecosList;
  for (auto item : list)
    auth.getGecos(item, gecosList[item]);

  static app::User user;
  user.set_id(sequence_);

  UserProfile profile(UserProfile::APPEND, this);
  profile.setUser(&user);
  profile.setGecosList(gecosList);
  profile.setUserList(list);
  if (profile.exec() == QDialog::Accepted) {
    if (meteo::TUserSettings::instance()->appendUser(user)) {
      model_->update();
      setCurrentIndex(findRowById(model_, user.id()));
      ++sequence_;

      user.Clear();
      return ;
    }

    error_log.msgBox() << SERVICE_NOT_UNAVAILABLE;
  }
}

void UserSettings::showProfile(const QModelIndex& index)
{
  app::User user = getUser(index);

  ui_->lName1Val->setText(user.surname().c_str());
  ui_->lName2Val->setText(user.name().c_str());
  ui_->lName3Val->setText(user.patron().c_str());
  ui_->lBirthdateVal->setText(user.birthdate().c_str());

  QByteArray buffer = QString::fromStdString(user.avatar()).toLocal8Bit();
  if (!buffer.isEmpty()) {
    QPixmap pixmap;
    if (pixmap.loadFromData(QByteArray::fromBase64(buffer)))
      ui_->lAvatar->setPixmap(pixmap.scaled(ui_->lAvatar->frameRect().size()));
    else
      ui_->lAvatar->setText("Не удалось\nзагрузить");

  } else
    ui_->lAvatar->setText("Фото\nне загружено");

  switch (user.gender()) {
    case app::User_GenderType::User_GenderType_Male :
      ui_->tbMale->setEnabled(true);
      ui_->tbFemale->setEnabled(false);
      break ;

    case app::User_GenderType::User_GenderType_Female :
      ui_->tbFemale->setEnabled(true);
      ui_->tbMale->setEnabled(false);
      break ;

    default :
      break;
  }

  ui_->lRoleVal->setText(user.role().name().c_str());
  ui_->lDepartmentVal->setText(user.department().name().c_str());
  ui_->lRankVal->setText(user.rank().name().c_str());
}

}

}
