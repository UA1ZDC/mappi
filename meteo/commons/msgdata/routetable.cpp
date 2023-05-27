#include "routetable.h"

#include <commons/textproto/pbtools.h>

#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/msgparser/common.h>

namespace meteo {

const QString RouteTable::kDbGateway      = "db";
const QString RouteTable::kSrcDataGateway = "srcdata";

const QString RouteTable::kCircTarget = "_circ_";
const QString RouteTable::kSelfTarget = "_self_";
const QString RouteTable::kAddrTarget = "_addr_";


QStringList RouteTable::reservedGateways()
{
  return { RouteTable::kDbGateway, RouteTable::kSrcDataGateway };
}

RouteTable::RouteTable()
{
  rowId_ = 0;
}

void RouteTable::insert(const Rule& rule)
{
  rules_.append(rule);
}

void RouteTable::insert(const Rules& rules)
{
  rules_.append(rules);
}

Rules RouteTable::merge(const Rules &rules)
{
  Rules answer;
  for ( auto rule : rules ) {
    if ( !rules_.contains(rule) ) {
      rules_.append(rule);
      answer.append(rule);
    }
  }
  return answer;
}

void RouteTable::remove(const Rule &rule)
{
  rules_.remove(rule);
}

void RouteTable::remove(const Rules &rules)
{
  rules_.remove(rules);
}

bool RouteTable::routeToDb(const tlg::MessageNew& msg) const
{
  Rule reqRule;
  reqRule.setTo(RouteTable::kDbGateway);
  reqRule.setMsgFilter(TlgFilter::filterFromMsg(msg));
  return rules_.allow(reqRule);
}

void RouteTable::fromProto(const msgcenter::RuleTable& opt)
{
  rules_.clear();
  for ( int i = 0, sz = opt.rule_size(); i < sz; ++i) {
    rules_.append( opt.rule(i) );
  }
}

bool RouteTable::allow(const Rule& reqRule) const
{
  return rules_.allow(reqRule);
}

}
