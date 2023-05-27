#ifndef METEO_COMMONS_SERVICES_FIELDATA_TEST_GETFIELDSMDATA_H
#define METEO_COMMONS_SERVICES_FIELDATA_TEST_GETFIELDSMDATA_H

#include "headers.h"

#include <commons/meteo_data/meteo_data.h>

TEST(GetFieldsMDataTest, ManyCenter) {
  meteo::field::DataRequest req;
  req.add_level(1000);
  req.add_center(34);
  req.add_center(250);

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  SurfReplyPtr reply = SurfReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldsMData, req, kTimeout, true));

  EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(), reply->error());
}

TEST(GetFieldsMDataTest, EmptyLevel) {
  meteo::field::DataRequest req;
  req.add_center(34);

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  SurfReplyPtr reply = SurfReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldsMData, req, kTimeout, true));

  EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(), reply->error());
}

TEST(GetFieldsMDataTest, ManyLevel) {
  meteo::field::DataRequest req;
  req.add_center(34);
  req.add_level(0);
  req.add_level(1000);

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  SurfReplyPtr reply = SurfReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldsMData, req, kTimeout, true));

  EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(), reply->error());
}

TEST(GetFieldsMDataTest, MinimunFill) {
  int level  = 1000;
  int center = 34;

  meteo::field::DataRequest req;
  req.add_level (level);
  req.add_center(center);

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  SurfReplyPtr reply = SurfReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldsMData, req, kTimeout, true));

  if(reply->meteodata_size() > 0) {
    QByteArray barr;
    barr.fromStdString(reply->meteodata(0));

    TMeteoData md;
    md << barr;

    debug_log << md.toJson(&level);
    EXPECT_TRUE(reply->error().empty());
  }
  else {
    EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(),
              reply->error());
  }
}

TEST(GetFieldsMDataTest, FullFill) {
  int descr      = 12108;
  int level_type = 100;
  int level      = 1000;
  int hour       = 0;
  int center     = 34;
  std::string dt("2018-04-06T09:00:00Z");

  meteo::field::DataRequest req;
  req.add_meteo_descr(descr);
  req.add_type_level (level_type);
  req.add_level      (level);
  req.add_hour       (hour);
  req.add_center     (center);
  req.set_date_start (dt);
  req.set_date_end   (dt);

  RpcControllerPtr ctrl = RpcControllerPtr(
        meteo::global::serviceController(meteo::settings::proto::kField));
  if(nullptr == ctrl) {
    error_log << QObject::tr("Не удалось установить соединение с сервисом данных");
    return;
  }

  SurfReplyPtr reply = SurfReplyPtr(ctrl->
      remoteCall(&meteo::field::FieldService::GetFieldsMData, req, kTimeout, true));

  if(reply->meteodata_size() > 0) {
    QByteArray barr;
    barr.fromStdString(reply->meteodata(0));

    TMeteoData md;
    md << barr;

    debug_log << md.toJson(&level);
    EXPECT_TRUE(reply->error().empty());
  }
  else {
    EXPECT_EQ(meteo::msglog::kServiceRequestFailedErr.toStdString(),
              reply->error());
  }
}

#endif // METEO_COMMONS_SERVICES_FIELDATA_TEST_GETFIELDSMDATA_H
