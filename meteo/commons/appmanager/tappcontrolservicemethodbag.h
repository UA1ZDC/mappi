#ifndef TAPCONTROLSERVICEMETHODBAG_H
#define TAPCONTROLSERVICEMETHODBAG_H

#include "authhandler.h"
#include <appconf.pb.h>
#include <qstring.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace meteo {
class Document;
class DbiEntry;

namespace app {
class TapControlServiceMethodBag
{
  public:
    TapControlServiceMethodBag();
    ~TapControlServiceMethodBag();

    void AuthUser(const UserAuthRequest* req, UserAuthResponse* resp);
    void NewUser(const UserRequest* req, UserResponse* resp);
    void AddUser(const UserRequest* req, UserResponse* resp);
    void RegisterUser(const UserRequest* req, UserResponse* resp);
    void UpdateUser(const UserRequest* req, UserResponse* resp);
    void BlockUser(const UserRequest* req, UserResponse* resp);
    void UnlockUser(const UserRequest* req, UserResponse* resp);
    void HomeDirUser(const UserRequest* req, UserResponse* resp);
    void CheckUser(const UserRequest* req, UserResponse* resp);
    void UserList(const Dummy* req, UserResponse* resp);

    void RoleList(const Dummy* req, RoleResponse* resp);
    void RankList(const Dummy* req, RankResponse* resp);
    void PutRank(const Rank* req, DefaultResponce* resp);
    void DepartmentList(const Dummy* req, DepartmentResponse* resp);
    void PutDepartment(const Department* req, DefaultResponce* resp);

    void UserAppointmentLoad( const Dummy* request,
                              UserAppointmentsResponce* responce );
    void UserAppointmentsGetList( const UserAppointment* request,
                                  UserAppointmentsResponce* responce );
    void UserAppointmentsGetCurrent( const UserAppointment* request,
                                     UserAppointmentsResponce* responce );

    void UserAppointmentAdd( const UserAppointment *request, DefaultResponce* responce );
    void UserAppointmentSetCurrentUser( const UserAppointment *request, DefaultResponce* responce );
    void UserAppointmentUnsetCurrentUser( const UserAppointment *request, DefaultResponce* responce );
    void UserAppointmentSetUserList( const ::meteo::app::UserAppointment* request, ::meteo::app::DefaultResponce* responce );
    void UserAppointmentDataUpdate( const UserAppointment *request, DefaultResponce* responce );
    void UserAppointmentSetList (const ::meteo::app::UserAppointmentListRequest* request,
                                 ::meteo::app::DefaultResponce* responce);
    void  ProcState(const ::meteo::app::AppState::Proc* request,
                    ::meteo::app::AppState::Proc* response);

    void SavePreset( const FolderEntity* req, DefaultResponce* resp );
    void GetPreset( const FolderEntity* req, FolderEntity* resp );

    void SaveIcon( const EntityIcon* req, DefaultResponce* resp );
    void GetIcon( const EntityIcon* req, EntityIcon* resp );

    //!< Получаем настройки погоды для пользователя
    void UserWeatherSetting( const ::meteo::app::WeatherSetting* request,
                                   ::meteo::app::WeatherSettingResponce* responce );

    // сохраняем настройки погоды пользователя
    void UserSaveWeatherSetting( const ::meteo::app::WeatherSetting* request,
                                   ::meteo::app::DefaultResponce* responce  );

    void PositionCreate (const ::meteo::app::PositionCreateRequest* request,
                                 ::meteo::app::PositionCreateResponce* responce);
    void PositionsList (const ::meteo::app::PositionsListRequest* request,
                                 ::meteo::app::PositionsListResponce* responce);
    void PositionDelete (const ::meteo::app::PositionDeleteRequest* request,
                         ::meteo::app::PositionDeleteResponce* responce);
    void PositionEdit(const ::meteo::app::PositionEditRequest* request,
                      ::meteo::app::PositionEditResponce* responce);

    void UserPogodaInputStsSave(const ::meteo::app::UserPogodaInputStationsSaveRequest* request,
                                ::meteo::app::PogodaInputStationsSaveResponce* response);

    void UserPogodaInputStsLoad(const ::meteo::app::UserPogodaInputStationsLoadRequest* request,
                                ::meteo::app::PogodaInputStationsLoadResponce* response);

    void SaveUserSetStations(const ::meteo::app::UserSetStations* request, ::meteo::app::DefaultResponce* response);

    void GetUserSetStations(const ::meteo::app::UserSetStations* request, ::meteo::app::UserSetStations* response);

    void  msgviewerUserSettingsSave( const ::meteo::app::MsgViewerUserSettingsSaveRequest* request,
                                     ::meteo::app::Dummy* response);

    void  msgviewerUserSettingsLoad( const ::meteo::app::MsgViewerUserSettingsLoadRequest* request,
                                     ::meteo::proto::CustomViewerUserConfig* response );

    void SaveCustomPreset( const ::meteo::app::CustomSettings* req, ::meteo::app::Result* resp );

    void LoadCustomPreset( const ::meteo::app::CustomSettings* req, ::meteo::app::CustomSettings* resp );

  private :
    QString createMapJson(const FolderEntity& entity);
    QString createFolderJson(const FolderEntity& folder);
    QString createJobJson(const FolderEntity& job);
    QString createDelimiterJson(const FolderEntity& delimiter);
    QString createDateFormatJson(const DateFormat& format);
    QString createDateFormatElementJson(const DateFormatElement& element);
    FolderEntity entityFromJson(const meteo::DbiEntry &doc);

  private :
    AuthHandler* auth_;
};

}
}
#endif
