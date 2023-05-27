#pragma once

#include <meteo/commons/auth/auth.h>
#include <meteo/commons/proto/appconf.pb.h>


namespace meteo {

namespace app {

class AuthHandler
{
public :
  AuthHandler();
  ~AuthHandler();

  QString getUserId(const QString& login);
  QString getUserLogin(const QString& uid);
  QString getUserLogin(const int32_t& uid);

  void AuthUser(const UserAuthRequest* req, UserAuthResponse* resp);

  void NewUser(const UserRequest* req, UserResponse* resp);
  void AddUser(const UserRequest* req, UserResponse* resp);
  void RegisterUser(const UserRequest* req, UserResponse* resp);
  void UpdateUser(const UserRequest* req, UserResponse* resp);

  void HomeDirUser(const UserRequest* req, UserResponse* resp);
  void CheckUser(const UserRequest* req, UserResponse* resp);
  void UserList(const Dummy* req, UserResponse* resp);

  void BlockUser(const UserRequest* req, UserResponse* resp);
  void UnlockUser(const UserRequest* req, UserResponse* resp);

  void RoleList(const Dummy* req, RoleResponse* resp);
  void RankList(const Dummy* req, RankResponse* resp);
  void PutRank(const Rank* req, DefaultResponce* resp);
  void DepartmentList(const Dummy* req, DepartmentResponse* resp);
  void PutDepartment(const Department* req, DefaultResponce* resp);

private :
  void blockUser(const UserRequest* req, UserResponse* resp, bool isBlock);

private :
  auth::Auth* auth_;
  bool isInit_;
};

}

}
