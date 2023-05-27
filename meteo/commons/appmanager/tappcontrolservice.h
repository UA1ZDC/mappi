#ifndef TAPPCONTROLSERVICE_H
#define TAPPCONTROLSERVICE_H

#include "tappcontrolservicemethodbag.h"
#include <meteo/commons/proto/appconf.pb.h>


namespace meteo {
namespace app {

class Service : public ControlService
{
  public:
    Service();
    ~Service();

    void GetProcList( google::protobuf::RpcController* c,
                              const meteo::app::Dummy* req,
                              meteo::app::AppState* res,
                              google::protobuf::Closure* d );

    void StartProc( google::protobuf::RpcController* c,
                            const meteo::app::AppState::Proc* req,
                            meteo::app::Dummy* res,
                            google::protobuf::Closure* d );

    void StopProc( google::protobuf::RpcController* c,
                           const meteo::app::AppState::Proc* req,
                           meteo::app::Dummy* res,
                           google::protobuf::Closure* d );

    void ProcChanged( google::protobuf::RpcController* c,
                              const meteo::app::Dummy* req,
                              meteo::app::AppState::Proc* res,
                              google::protobuf::Closure* d );

    void GetAppOut( google::protobuf::RpcController* c,
                            const meteo::app::AppOutRequest* req,
                            meteo::app::AppOutReply* res,
                            google::protobuf::Closure* d );

    void SetOperaionStatus(google::protobuf::RpcController *c,
                                   const OperationStatus *req,
                                   Dummy *res,
                                   google::protobuf::Closure *d);

    void AuthUser(google::protobuf::RpcController* ctrl, const UserAuthRequest* req,
      UserAuthResponse* resp, google::protobuf::Closure* done);

    void NewUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void AddUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void RegisterUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void UpdateUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void BlockUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void UnlockUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void HomeDirUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void CheckUser(google::protobuf::RpcController* ctrl, const UserRequest* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void UserList(google::protobuf::RpcController* ctrl, const Dummy* req,
      UserResponse* resp, google::protobuf::Closure* done);

    void RoleList(google::protobuf::RpcController* ctrl, const Dummy* req,
      RoleResponse* resp, google::protobuf::Closure* done);

    void RankList(google::protobuf::RpcController* ctrl, const Dummy* req,
      RankResponse* resp, google::protobuf::Closure* done);

    void PutRank(google::protobuf::RpcController* ctrl, const Rank* req,
      DefaultResponce* resp, google::protobuf::Closure* done);

    void DepartmentList(google::protobuf::RpcController* ctrl, const Dummy* req,
      DepartmentResponse* resp, google::protobuf::Closure* done);

    void PutDepartment(google::protobuf::RpcController* ctrl, const Department* req,
      DefaultResponce* resp, google::protobuf::Closure* done);

    void UserAppointmentAdd(::google::protobuf::RpcController* controller,
                                    const ::meteo::app::UserAppointment* request,
                                    ::meteo::app::DefaultResponce* response,
                                    ::google::protobuf::Closure* done);

    void UserAppointmentSetCurrentUser(::google::protobuf::RpcController* controller,
                                               const ::meteo::app::UserAppointment* request,
                                               ::meteo::app::DefaultResponce* response,
                                               ::google::protobuf::Closure* done);

    void UserAppointmentUnsetCurrentUser(::google::protobuf::RpcController* controller,
                                                 const ::meteo::app::UserAppointment* request,
                                                 ::meteo::app::DefaultResponce* response,
                                                 ::google::protobuf::Closure* done);

    void UserAppointmentsLoad(::google::protobuf::RpcController* controller,
                                      const ::meteo::app::Dummy* request,
                                      ::meteo::app::UserAppointmentsResponce* responce,
                                      ::google::protobuf::Closure* done);

    void UserAppointmentSetUserList (::google::protobuf::RpcController* controller,
                                             const ::meteo::app::UserAppointment* request,
                                             ::meteo::app::DefaultResponce* responce,
                                             ::google::protobuf::Closure* done);

    void UserAppointmentDataUpdate (::google::protobuf::RpcController* controller,
                                            const ::meteo::app::UserAppointment* request,
                                            ::meteo::app::DefaultResponce* responce,
                                            ::google::protobuf::Closure* done);

    void UserAppointmentSetList (::google::protobuf::RpcController* controller,
                                          const ::meteo::app::UserAppointmentListRequest* request,
                                          ::meteo::app::DefaultResponce* responce,
                                          ::google::protobuf::Closure* done);

    void UserAppointmentsGetList (::google::protobuf::RpcController* controller,
                                          const ::meteo::app::UserAppointment* request,
                                          ::meteo::app::UserAppointmentsResponce* responce,
                                          ::google::protobuf::Closure* done);

    void UserAppointmentsGetCurrent (::google::protobuf::RpcController* controller,
                                             const ::meteo::app::UserAppointment* request,
                                             ::meteo::app::UserAppointmentsResponce* responce,
                                             ::google::protobuf::Closure* done);

    void PositionCreate (::google::protobuf::RpcController* controller,
                                 const ::meteo::app::PositionCreateRequest* request,
                                 ::meteo::app::PositionCreateResponce* responce,
                                 ::google::protobuf::Closure* done);

    void PositionsList (::google::protobuf::RpcController* controller,
                                 const ::meteo::app::PositionsListRequest* request,
                                 ::meteo::app::PositionsListResponce* responce,
                                 ::google::protobuf::Closure* done);

    void PositionDelete (::google::protobuf::RpcController* controller,
                                 const ::meteo::app::PositionDeleteRequest* request,
                                 ::meteo::app::PositionDeleteResponce* responce,
                                 ::google::protobuf::Closure* done);

    void PositionEdit (::google::protobuf::RpcController* controller,
                                 const ::meteo::app::PositionEditRequest* request,
                                 ::meteo::app::PositionEditResponce* responce,
                                 ::google::protobuf::Closure* done);

    //!< Получаем настройки погоды для пользователя
    void UserWeatherSetting (::google::protobuf::RpcController* controller,
                                     const ::meteo::app::WeatherSetting* request,
                                     ::meteo::app::WeatherSettingResponce* responce,
                                     ::google::protobuf::Closure* done);
    
    // сохраняем настройки погоды пользователя
    void UserSaveWeatherSetting (::google::protobuf::RpcController* controller,
                                         const ::meteo::app::WeatherSetting* request,
                                         ::meteo::app::DefaultResponce* responce,
                                         ::google::protobuf::Closure* done);


    void SavePreset(::google::protobuf::RpcController* controller,
                  const::meteo::app::FolderEntity *request,
                  ::meteo::app::DefaultResponce* response,
                  ::google::protobuf::Closure* done);

    void GetPreset(::google::protobuf::RpcController* controller,
                 const ::meteo::app::FolderEntity* request,
                 ::meteo::app::FolderEntity* response,
                 ::google::protobuf::Closure* done);

    void SaveIcon(::google::protobuf::RpcController* c,
               const ::meteo::app::EntityIcon* req,
               ::meteo::app::DefaultResponce* resp,
               ::google::protobuf::Closure* d);

    void GetIcon(::google::protobuf::RpcController* c,
               const ::meteo::app::EntityIcon* req,
               ::meteo::app::EntityIcon* resp,
               ::google::protobuf::Closure* d);

    void UserPogodaInputStsSave(::google::protobuf::RpcController* controller,
                                 const ::meteo::app::UserPogodaInputStationsSaveRequest* request,
                                 ::meteo::app::PogodaInputStationsSaveResponce* response,
                                 ::google::protobuf::Closure* done);

    void UserPogodaInputStsLoad(::google::protobuf::RpcController* controller,
                                 const ::meteo::app::UserPogodaInputStationsLoadRequest* request,
                                 ::meteo::app::PogodaInputStationsLoadResponce* response,
                                 ::google::protobuf::Closure* done);

    void SaveUserSetStations(::google::protobuf::RpcController* controller,
                                     const ::meteo::app::UserSetStations* request,
                                     ::meteo::app::DefaultResponce* response,
                                     ::google::protobuf::Closure* done);

    void  GetUserSetStations(::google::protobuf::RpcController* controller,
                                     const ::meteo::app::UserSetStations* request,
                                     ::meteo::app::UserSetStations* response,
                                     ::google::protobuf::Closure* done);

    void  msgviewerUserSettingsSave(::google::protobuf::RpcController* controller,
                                            const ::meteo::app::MsgViewerUserSettingsSaveRequest* request,
                                            ::meteo::app::Dummy* response,
                                            ::google::protobuf::Closure* done);

    void  msgviewerUserSettingsLoad(::google::protobuf::RpcController* controller,
                                            const ::meteo::app::MsgViewerUserSettingsLoadRequest* request,
                                            ::meteo::proto::CustomViewerUserConfig* response,
                                            ::google::protobuf::Closure* done);

    void  ProcState(::google::protobuf::RpcController* controller,
                            const ::meteo::app::AppState::Proc* request,
                            ::meteo::app::AppState::Proc* response,
                            ::google::protobuf::Closure* done);

    void SaveCustomPreset(::google::protobuf::RpcController* controller,
                       const ::meteo::app::CustomSettings* request,
                       ::meteo::app::Result* response,
                       ::google::protobuf::Closure* done);

    void LoadCustomPreset(::google::protobuf::RpcController* controller,
                       const ::meteo::app::CustomSettings* request,
                       ::meteo::app::CustomSettings* response,
                       ::google::protobuf::Closure* done);

  private :
    TapControlServiceMethodBag *method_;
};

}
}

#endif
