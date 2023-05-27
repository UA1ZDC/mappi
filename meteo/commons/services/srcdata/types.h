#ifndef METEO_COMMONS_SERVICES_SRCDATA_TYPES_H
#define METEO_COMMONS_SERVICES_SRCDATA_TYPES_H

#include <meteo/commons/global/requestcard.h>
#include <meteo/commons/proto/surface_service.pb.h>

#include <meteo/commons/global/requestcard.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/proto/msgparser.pb.h>

namespace meteo {

//! Данный тип содержит параметры запроса ProcessMsg.
typedef RequestCard<const msgcenter::ProcessMsgRequest, msgcenter::ProcessMsgReply> ProcessMsgCard;
//! Данный тип содержит параметры запроса CreateTelegram.
typedef RequestCard<const tlg::MessageNew, msgcenter::CreateTelegramReply> CreateTelegramCard;
//! Данный тип содержит параметры запроса DistributeMsg.
typedef RequestCard<const msgcenter::DistributeMsgRequest, tlg::MessageNew> DistributeMsgCard;
//! Данный тип содержит параметры запроса SetDecode.
typedef RequestCard<const msgcenter::DecodeRequest, msgcenter::Dummy> SetDecodeCard;
//! Данный тип содержит параметры запроса GetTelegram.
typedef RequestCard<const msgcenter::GetTelegramRequest, msgcenter::GetTelegramResponse> GetTelegramCard;
//! Данный тип содержит параметры запроса SearchNode.
typedef RequestCard<const msgcenter::SearchNodeRequest, msgcenter::SearchNodeResponse> SearchNodeCard;

}

namespace meteo {
namespace surf {

//! Данный тип содержит параметры запроса SaveMsg.
typedef RequestCard<const ::meteo::tlg::MessageNew, SaveMsgReply> SaveMsgCard;
//! Данный тип содержит параметры запроса GetMsgMulti.
typedef RequestCard<const ::meteo::msgcenter::DistributeMsgRequest, ::meteo::tlg::MessageNew> GetMsgMulti;

} // surf
} // meteo

#endif // METEO_COMMONS_SERVICES_SRCDATA_TYPES_H
