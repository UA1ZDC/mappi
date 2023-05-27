#include <qmessagebox.h>

#include "userselectform.h"
#include "ui_userselectform.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/channel.h>
#include <meteo/commons/global/global.h>


namespace meteo {

UserSelectForm::UserSelectForm(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::UserSelectForm)
{
  ui_->setupUi(this);
  setWindowTitle("Выбор пользователей для отправки сообщений");
  ui_->userList->setSelectionMode(QAbstractItemView::MultiSelection);
  ui_->refreshBtn->setIcon(QIcon(":/meteo/icons/tools/print/refresh.png"));
  slotRefresh();
  QObject::connect( ui_->okBtn, SIGNAL(clicked()), this, SLOT(slotOk()));
  QObject::connect( ui_->refreshBtn, SIGNAL(clicked()), this, SLOT(slotRefresh()));
}

UserSelectForm::~UserSelectForm()
{
  delete ui_;
  ui_ = nullptr;
}

QStringList UserSelectForm::getCurrentUsers()
{
  QStringList logins;
  auto userItems = ui_->userList->selectedItems();
  for ( auto item : userItems ) {
    logins << item->data(kLogin).toString();
  }
  return logins;
}

void UserSelectForm::slotOk()
{
  close();
}

void UserSelectForm::slotRefresh()
{

  ui_->userList->clear();

  auto ch = std::unique_ptr<meteo::rpc::Channel>(meteo::global::serviceChannel( meteo::settings::proto::kInterMessagesService ));
  if ( nullptr == ch ) {
    critical_log.msgBox() << QObject::tr("Ошибка. Не удалось подключиться к web сервису ");
    return;
  }

  meteo::web::UserListRequest req;
  req.set_username(meteo::global::currentUserLogin().toStdString());

  auto resp = std::unique_ptr<meteo::web::UserListResponce>(ch->remoteCall(&meteo::web::InterService::GetUserListForUser, req, 30000));
  if ( nullptr == resp ) {
    critical_log.msgBox() << QObject::tr("Ошибка. Не удалось получить список пользователей");
    return;
  }
  for (int i = 0, sz = resp->users_size(); i < sz; ++i ) {
    auto user = resp->users(i);
    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(QSize( item->sizeHint().width(), 35 ) );
    auto font = item->font();
    font.setPointSize(12);
    item->setFont(font);
    QString login = QString::fromStdString( user.login() );
    QString rank = QString::fromStdString( user.ranks() );
    QString name = QString::fromStdString( user.name() );
    QString surname = QString::fromStdString( user.surname() );
    QString patron = QString::fromStdString( user.patron() );
    QString department = QString::fromStdString( user.department() );
    item->setText(rank + " " + surname + " " + name + " " + patron);
    item->setData(kLogin, login);
    item->setData(kRank, rank);
    item->setData(kName, name);
    item->setData(kSurname, surname);
    item->setData(kPatron, patron);
    item->setData(kDepartment, department);
    ui_->userList->insertItem(0, item);
  }
}

}
