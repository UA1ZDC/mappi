#include "pogodainputsettingsperuser.h"
#include "ui_pogodainputsettingsperuser.h"
#include <meteo/commons/settings/tusersettings.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/global/global.h>

namespace meteo {
static const QString& kSettingsName = QObject::tr("Аэродромы для ввода ФП");


PogodaInputSettingsPerUser::PogodaInputSettingsPerUser( const meteo::settings::Location& loc,
                                                        QWidget *parent):
  SettingsWidget(parent),  
  ui_(new Ui::PogodaInputSettingsPerUser),
  loc_(loc)
{
  setObjectName(kSettingsName);
  ui_->setupUi(this);
  QObject::connect( ui_->cbUsersList, SIGNAL(currentIndexChanged(int)), this, SLOT(onUserChanged(int)) );

  auto usersList = TUserSettings::instance()->usernameList();
  for ( auto user: usersList ){
    this->ui_->cbUsersList->addItem(user, user);
    WgtPogodaInputSettings *sett = new WgtPogodaInputSettings(this->loc_, user);
    sett->init();
    sett->setHidden(true);
    this->settings_[user] = sett;
    this->ui_->stackedWidget->addWidget(sett);    
    QObject::connect(sett, &WgtPogodaInputSettings::changed, this, &PogodaInputSettingsPerUser::slotChanged);

    this->ui_->cbUsersList->setCurrentText(user);
    this->ui_->stackedWidget->setCurrentWidget(sett);
  }

}

PogodaInputSettingsPerUser::~PogodaInputSettingsPerUser()
{
  if (nullptr != ui_) {
    delete ui_;
  }
}

void PogodaInputSettingsPerUser::onUserChanged(int index)
{
  this->ui_->stackedWidget->setCurrentIndex(index);
}

void PogodaInputSettingsPerUser::init()
{
}

bool PogodaInputSettingsPerUser::saveRpc()
{  
  meteo::app::UserPogodaInputStationsSaveRequest request;

  for (auto user : this->settings_.keys() ){
    auto sett = this->settings_[user];
    if ( false == sett->isChanged() ){
      continue;
    }
    sett->save();
    auto data = request.add_data();
    data->set_login(user.toStdString());
    auto param = sett->getData();    
    data->mutable_param()->CopyFrom(param);
  }

  auto ch = std::unique_ptr<meteo::rpc::Channel>(global::serviceChannel(meteo::settings::proto::ServiceCode::kDiagnostic));
  if ( nullptr == ch ){
    error_log.msgBox() << meteo::msglog::kServiceConnectFailed;
    return false;
  }

  auto resp = std::unique_ptr<meteo::app::PogodaInputStationsSaveResponce>(ch->remoteCall(&app::ControlService::UserPogodaInputStsSave, request, 1000000));
  if ( nullptr == resp ){
    error_log.msgBox() << meteo::msglog::kServiceRequestFailed;
    return false;
  }
  if ( false == resp->result() ){
    error_log.msgBox() << resp->comment();
  }
  return true;
}

void PogodaInputSettingsPerUser::save()
{
  error_log << __FUNCTION__ << changed_;
  if ( false == this->changed_ ){
    return;
  }  
  if ( true == this->saveRpc() ){
    this->changed_ = false;
  }
}

void PogodaInputSettingsPerUser::load()
{
  meteo::app::UserPogodaInputStationsLoadRequest request;

  for (auto user : this->settings_.keys() ){
    request.add_login(user.toStdString());    
  }

  auto ch = std::unique_ptr<meteo::rpc::Channel>(global::serviceChannel(meteo::settings::proto::ServiceCode::kDiagnostic));
  if ( nullptr == ch ){    
    error_log.msgBox() << meteo::msglog::kServiceConnectFailedSimple.arg(global::serviceTitle(meteo::settings::proto::ServiceCode::kDiagnostic));
    return;
  }

  auto resp = std::unique_ptr<meteo::app::PogodaInputStationsLoadResponce>(ch->remoteCall(&app::ControlService::UserPogodaInputStsLoad, request, 1000000));
  if ( nullptr == resp ){
    error_log.msgBox() << meteo::msglog::kServiceRequestFailed;
    return;
  }

  for ( int i = 0; i < resp->data_size(); ++i ){
    auto data = resp->data(i);
    auto login = QString::fromStdString(data.login());
    auto param = data.param();
    auto sett = this->settings_[login];
    sett->setData(param);
    sett->load();
  }
  if ( false == resp->result() ){
    error_log.msgBox() << resp->comment();
    return;
  }
  this->changed_ = false;
}

void PogodaInputSettingsPerUser::ownStationChanged(const meteo::settings::Location& loc)
{
  this->loc_ = loc;
  for ( auto sett: this->settings_ ){
    sett->ownStationChanged(loc);
  }
}

void PogodaInputSettingsPerUser::slotChanged()
{
  this->changed_ = true;
  emit changed();
}

}
