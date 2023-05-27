#include "userprofile.h"
#include "ui_userprofile.h"
#include <cross-commons/app/paths.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/authconf.pb.h>
#include <qfiledialog.h>
#include <qstringlistmodel.h>
#include <qmessagebox.h>


namespace meteo {

namespace ui {

static const QString DATE_FORMAT = "dd.MM.yyyy";
static const int ROLE_DEFAULT = 3;

UserProfile::UserProfile(UserProfile::role_t role, QWidget* parent /*=*/, Qt::WindowFlags f /*=*/) :
    QDialog(parent, f),
  ui_(new Ui::UserProfile()),
  rankModel_(new DictModel(TUserSettings::instance()->ranks(), this)),
  departmentModel_(new DictModel(TUserSettings::instance()->departments(), this)),
  calendar_(new QCalendarWidget()),
  bg_(new QButtonGroup(this))
{
  ui_->setupUi(this);

  switch (role) {
    case UserProfile::UPDATE :
      setWindowTitle(QObject::tr("Обновить профиль пользователя"));
      break ;

    case UserProfile::APPEND :
      setWindowTitle(QObject::tr("Добавить пользователя"));
      break ;

    default :
      break ;
  }

  auth::AuthConf conf;
  QString path = QString("%1/os-auth.conf").arg(MnCommon::etcPath());
  if (!TProtoText::fromFile(path, &conf))
    error_log << QObject::tr("Ошибка разбора конфигурации: %1").arg(path);

  ui_->deBirthdate->setCalendarPopup(true);
  ui_->deBirthdate->setCalendarWidget(calendar_);
  calendar_->setGridVisible(true);

  bg_->addButton(ui_->tbMale);
  bg_->addButton(ui_->tbFemale);

  ui_->cbRank->setModel(rankModel_);
  ui_->cbRank->setModelColumn(DictModel::NAME);

  ui_->cbDepartment->setModel(departmentModel_);
  ui_->cbDepartment->setModelColumn(DictModel::NAME);

  ui_->pbLoad->setText(QObject::tr("Загрузить"));
  ui_->pbClear->setText(QObject::tr("Сбросить"));

  ui_->leName1->setValidator(new QRegExpValidator(QRegExp("\\w+"), this));
  ui_->leName2->setValidator(new QRegExpValidator(QRegExp("\\w+"), this));
  ui_->leName3->setValidator(new QRegExpValidator(QRegExp("\\w+"), this));

  QObject::connect(ui_->pbSave, SIGNAL(clicked()), this, SLOT(save()));
  QObject::connect(ui_->pbCancel, SIGNAL(clicked()), this, SLOT(reject()));
  QObject::connect(ui_->pbLoad, SIGNAL(clicked()), this, SLOT(loadAvatar()));
  QObject::connect(ui_->pbClear, SIGNAL(clicked()), this, SLOT(clearAvatar()));
  QObject::connect(ui_->cbUser, SIGNAL(currentTextChanged(const QString&)), this, SLOT(switchUser(const QString&)));
}

UserProfile::~UserProfile()
{
  delete ui_;
  ui_ = nullptr;

  delete calendar_;
  calendar_ = nullptr;

  user_ = nullptr;
}

void UserProfile::setUser(app::User* user)
{
  user_ = user;

  ui_->leName1->setText(user_->surname().c_str());
  ui_->leName2->setText(user_->name().c_str());
  ui_->leName3->setText(user_->patron().c_str());

  if (user_->has_birthdate())
    ui_->deBirthdate->setDate(QDate::fromString(user_->birthdate().c_str(), DATE_FORMAT));
  else
    ui_->deBirthdate->setDate(QDate::currentDate());

  if (user_->gender() == app::User_GenderType::User_GenderType_Female)
    ui_->tbFemale->setChecked(true);
  else
    ui_->tbMale->setChecked(true);

  if (user_->has_rank())
    ui_->cbRank->setCurrentIndex(findRowById(rankModel_, user_->rank().id()));

  if (user_->has_department())
    ui_->cbDepartment->setCurrentIndex(findRowById(departmentModel_, user_->department().id()));

  QByteArray buffer = QString::fromStdString(user_->avatar()).toLocal8Bit();
  switchAvatar(QByteArray::fromBase64(buffer));
}

void UserProfile::setUserList(const QStringList& list)
{
  ui_->cbUser->setModel(new QStringListModel(list, this));
  if (list.isEmpty())
    ui_->pbSave->setEnabled(false);
}

void UserProfile::setGecosList(const gecosList_t& list)
{
  gecosList_ = list;
}

int UserProfile::findRowById(DictModel* model, int id)
{
  int res = 0;
  for (int row = 0; row < model->rowCount(); ++row) {
    if (model->data(model->index(row, DictModel::ID)).toInt() == id)
      return res;

    ++res;
  }

  return res;
}

int UserProfile::findIdByRow(DictModel* model, int row)
{
  return model->data(model->index(row, DictModel::ID)).toInt();
}

void UserProfile::switchAvatar(const QByteArray& buffer)
{
  ui_->pbLoad->setVisible(false);
  ui_->pbClear->setVisible(false);

  if (buffer.isEmpty()) {
    ui_->lAvatar->setText("Фото\nне загружено");
    ui_->pbLoad->setVisible(true);
    return ;
  }

  QPixmap pixmap;
  if (pixmap.loadFromData(buffer)) {
    ui_->lAvatar->setPixmap(pixmap.scaled(ui_->lAvatar->frameRect().size()));
    ui_->pbClear->setVisible(true);
    return ;
  }

  ui_->lAvatar->setText("Не удалось\nзагрузить");
  ui_->pbClear->setVisible(true);

  return ;
}

void UserProfile::save()
{
  user_->set_username(ui_->cbUser->currentText().toStdString());

  user_->set_surname(ui_->leName1->text().toStdString());
  user_->set_name(ui_->leName2->text().toStdString());
  user_->set_patron(ui_->leName3->text().toStdString());

  // WARNING не редактируется
  if (!user_->has_role())
    user_->mutable_role()->set_id(ROLE_DEFAULT);

  if (ui_->tbMale->isChecked())
    user_->set_gender(app::User_GenderType::User_GenderType_Male);
  else
    user_->set_gender(app::User_GenderType::User_GenderType_Female);

  user_->set_birthdate(calendar_->selectedDate().toString(DATE_FORMAT).toStdString());

  if (0 < ui_->cbRank->count()) {
    user_->mutable_rank()->set_name(ui_->cbRank->currentText().toStdString());
    user_->mutable_rank()->set_id(findIdByRow(rankModel_, ui_->cbRank->currentIndex()));
  }

  if (0 < ui_->cbDepartment->count()) {
    user_->mutable_department()->set_name(ui_->cbDepartment->currentText().toStdString());
    user_->mutable_department()->set_id(findIdByRow(departmentModel_, ui_->cbDepartment->currentIndex()));
  }

  // qDebug() << "username:" << user_->username().c_str();
  // qDebug() << "name:" << user_->name().c_str();
  // qDebug() << "surname:" << user_->surname().c_str();
  // qDebug() << "patron:" << user_->patron().c_str();
  // qDebug() << "gender:" << int(user_->gender());
  // qDebug() << "birthdate:" << user_->birthdate().c_str();
  // qDebug() << "rank:" << user_->rank().id() << " - " << user_->rank().name().c_str();
  // qDebug() << "department :" << user_->department().id() << " - " << user_->department().name().c_str();

  accept();
}

void UserProfile::loadAvatar()
{
  QString path = QFileDialog::getOpenFileName(this,
    QObject::tr("Open Image"),
    QDir::homePath(),
    QObject::tr("Image Files (*.png *.jpg *.bmp)")
  );

  if (!path.isEmpty()) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
      error_log.msgBox() << QObject::tr("Не удается открыть файл: %1").arg(path);
      return ;
    }

    QByteArray buffer(file.readAll());
    user_->set_avatar(buffer.toBase64().toStdString());
    switchAvatar(buffer);
  }
}

void UserProfile::clearAvatar()
{
  int ret = QMessageBox::question(this,
    QObject::tr("Профиль пользователя"),
    QObject::tr("Фотография будет удалена, продолжить?"),
    QMessageBox::Ok | QMessageBox::Cancel
  );

  if (ret == QMessageBox::Ok) {
    user_->set_avatar("");
    switchAvatar(QString::fromStdString(user_->avatar()).toLocal8Bit());
  }
}

void UserProfile::switchUser(const QString& username)
{
  if (!gecosList_.isEmpty()) {
    const auth::Gecos& gecos = gecosList_[username];
    ui_->leName1->setText(gecos.surname());
    ui_->leName2->setText(gecos.name());
    ui_->leName3->setText(gecos.patron());
  }
}

}

}
