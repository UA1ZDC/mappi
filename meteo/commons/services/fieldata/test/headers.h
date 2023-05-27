#ifndef METEO_COMMONS_SERVICES_FIELDATA_TEST_HEADERS_H
#define METEO_COMMONS_SERVICES_FIELDATA_TEST_HEADERS_H

#include <memory>

#include <qdatetime.h>

#include <gtest/gtest.h>

#include <meteo/commons/rpc/tcontroller.h>
#include <meteo/commons/proto/services.pb.h>
#include <meteo/commons/proto/field.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/global/log.h>

static int constexpr kTimeout = 10000;

using RpcControllerPtr  = std::unique_ptr<rpc::TController>;
using FieldReplyPtr     = std::unique_ptr<meteo::field::DataReply>;
using FieldManyReplyPtr = std::unique_ptr<meteo::field::ManyDataReply>;
using SurfReplyPtr      = std::unique_ptr<meteo::surf::DataReply>;

#endif // METEO_COMMONS_SERVICES_FIELDATA_TEST_HEADERS_H
