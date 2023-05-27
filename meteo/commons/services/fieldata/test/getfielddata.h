#ifndef METEO_COMMONS_SERVICES_FIELDATA_TEST_GETFIELDDATA_H
#define METEO_COMMONS_SERVICES_FIELDATA_TEST_GETFIELDDATA_H

#include "headers.h"

TEST(GetFieldDataTest, EmptyMeteoDescr) {
  meteo::field::DataRequest req;
  req.add_level(1000);
  req.add_hour(0);
  req.set_date_start("2018-04-06T09:00:00Z");

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  FieldReplyPtr reply = FieldReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldData, req, kTimeout, true));

  EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(), reply->error());
}

TEST(GetFieldDataTest, EmptyHour) {
  meteo::field::DataRequest req;
  req.add_meteo_descr(12108);
  req.add_level(1000);
  req.set_date_start("2018-04-06T09:00:00Z");

  RpcControllerPtr ctrl = RpcControllerPtr(meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  FieldReplyPtr reply = FieldReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldData, req, kTimeout, true));

  EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(), reply->error());
}

TEST(GetFieldDataTest, EmptyLevel) {
  meteo::field::DataRequest req;
  req.add_meteo_descr(12108);
  req.add_hour       (0);
  req.set_date_start ("2018-04-06T09:00:00Z");

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  FieldReplyPtr reply = FieldReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldData, req, kTimeout, true));

  EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(), reply->error());
}

TEST(GetFieldDataTest, EmptyDateStart) {
  meteo::field::DataRequest req;
  req.add_meteo_descr(12108);
  req.add_level      (1000);
  req.add_hour       (0);

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  FieldReplyPtr reply = FieldReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldData, req, kTimeout, true));

  EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(), reply->error());
}

TEST(GetFieldDataTest, HasDateEnd) {
  meteo::field::DataRequest req;
  req.add_meteo_descr(12108);
  req.add_level      (1000);
  req.add_hour       (0);
  req.set_date_start ("2018-04-06T09:00:00Z");
  req.set_date_end   ("2018-04-06T09:00:00Z");
  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }
  FieldReplyPtr reply = FieldReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldData, req, kTimeout, true));
  EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(), reply->error());
}

TEST(GetFieldDataTest, MinimunFill) {
  int descr = 12108;
  int hour  = 0;
  int level = 1000;
  std::string dt("2018-04-06T09:00:00Z");

  meteo::field::DataRequest req;
  req.add_meteo_descr(descr);
  req.add_level      (level);
  req.add_hour       (hour);
  req.set_date_start (dt);

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  FieldReplyPtr reply = FieldReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldData, req, kTimeout, true));

  EXPECT_EQ(descr, reply->descr().meteodescr());
  EXPECT_EQ(hour , reply->descr().hour());
  EXPECT_EQ(level, reply->descr().level());
  EXPECT_EQ(dt   , reply->descr().date());
  EXPECT_EQ(meteo::msglog::kServerAnswerOK.arg("GetFieldData").toStdString(),
            reply->error());
}

TEST(GetFieldDataTest, FullFill) {
  int descr      = 12108;
  int level_type = 100;
  int level      = 1000;
  int hour       = 0;
  int center     = 34;
  int model      = 4;
  std::string dt("2018-04-06T09:00:00Z");

  meteo::field::DataRequest req;
  req.add_meteo_descr(descr);
  req.add_type_level (level_type);
  req.add_level      (level);
  req.add_hour       (hour);
  req.add_center     (center);
  req.set_model      (model);
  req.set_date_start (dt);

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  FieldReplyPtr reply = FieldReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldData, req, kTimeout, true));

  EXPECT_EQ(descr     , reply->descr().meteodescr());
  EXPECT_EQ(level_type, reply->descr().level_type());
  EXPECT_EQ(level     , reply->descr().level());
  EXPECT_EQ(hour      , reply->descr().hour());
  EXPECT_EQ(center    , reply->descr().center());
  EXPECT_EQ(model     , reply->descr().model());
  EXPECT_EQ(dt        , reply->descr().date());
  EXPECT_EQ(meteo::msglog::kServerAnswerOK.arg("GetFieldData").toStdString(),
            reply->error());
}

#endif // METEO_COMMONS_SERVICES_FIELDATA_TEST_GETFIELDDATA_H
