#include "msgfilter.h"
#include <meteo/commons/msgparser/common.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>


namespace meteo {

const QString kAll = QObject::tr("all");

TlgFilter::TlgFilter(const tlg::MessageNew& msg)
{
  *this = filterFromMsg(msg);
}

bool TlgFilter::isEmpty() const
{
  return (t1_.isEmpty() && t2_.isEmpty() && a1_.isEmpty()
          && a2_.isEmpty() && ii_.isEmpty() && BBB_.isEmpty()
          && CCCC_.isEmpty() && format_.isEmpty() && YYGGgg_.isEmpty()
          && TlgPriority_.isEmpty() && channel_.isEmpty() && msgType_.isEmpty());
}

bool TlgFilter::check(const TlgFilter& anotherFilter) const
{
  bool answer = true;
  if ( !t1_.isEmpty()) {
    answer &= (t1_ == anotherFilter.t1());
  }
  if ( !t2_.isEmpty()) {
    answer &= (t2_ == anotherFilter.t2() );
  }
  if ( !a1_.isEmpty()) {
    answer &= (a1_ == anotherFilter.a1() );
  }
  if ( !a2_.isEmpty()) {
    answer &= (a2_ == anotherFilter.a2() );
  }
  if ( !ii_.isEmpty()) {
    answer &= (ii_ == anotherFilter.ii() );
  }
  if ( !BBB_.isEmpty()) {
    answer &= (BBB_ == anotherFilter.BBB() );
  }
  if ( !CCCC_.isEmpty()) {
    answer &= (CCCC_ == anotherFilter.CCCC() );
  }
  if ( !format_.isEmpty()) {
    answer &= (format_ == anotherFilter.format() );
  }
  if ( !YYGGgg_.isEmpty()) {
    answer &= (YYGGgg_ == anotherFilter.YYGGgg() );
  }
  if ( !TlgPriority_.isEmpty()) {
    answer &= (TlgPriority_ == anotherFilter.TlgPriority() );
  }
  if ( !channel_.isEmpty()) {
    answer &= (channel_ == anotherFilter.channel() );
  }
  if ( !msgType_.isEmpty()) {
    answer &= (msgType_ == anotherFilter.msgType() );
  }
  return answer;
}

bool TlgFilter::operator==(const TlgFilter& anotherFilter) const
{
  return ( t1_ == anotherFilter.t1()
      && t2_ == anotherFilter.t2()
      && a1_ == anotherFilter.a1()
      && a2_ == anotherFilter.a2()
      && ii_ == anotherFilter.ii()
      && BBB_ == anotherFilter.BBB()
      && CCCC_ == anotherFilter.CCCC()
      && format_ == anotherFilter.format()
      && YYGGgg_ == anotherFilter.YYGGgg()
      && TlgPriority_ == anotherFilter.TlgPriority()
      && channel_ == anotherFilter.channel()
      && msgType_ == anotherFilter.msgType() );
}

TlgFilter TlgFilter::fromProto(const msgcenter::TlgFilter &protoFilter)
{
  TlgFilter filter;
  if ( protoFilter.has_t1() ) {
    filter.sett1(protoFilter.t1());
  }
  if ( protoFilter.has_t2() ) {
    filter.sett2(protoFilter.t2());
  }
  if ( protoFilter.has_a1() ) {
    filter.seta1(protoFilter.a1());
  }
  if ( protoFilter.has_a2() ) {
    filter.seta2(protoFilter.a2());
  }
  if ( protoFilter.has_ii() ) {
    filter.setii(protoFilter.ii());
  }
  if ( protoFilter.has_bbb() ) {
    filter.setBBB(protoFilter.bbb());
  }
  if ( protoFilter.has_cccc() ) {
    filter.setCCCC(protoFilter.cccc());
  }
  if ( protoFilter.has_format() ) {
    filter.setFormat(protoFilter.format());
  }
  if ( protoFilter.has_yygggg() ) {
    filter.setYYGGgg(protoFilter.yygggg());
  }
  if ( protoFilter.has_priority() ) {
    filter.setTlgPriority(protoFilter.priority());
  }
  if ( protoFilter.has_channel() ) {
    filter.setChannel(protoFilter.channel());
  }
  if ( protoFilter.has_msgtype() ) {
    filter.setMsgType(protoFilter.msgtype());
  }
  if ( protoFilter.has_external() ) {
    filter.setExternal( protoFilter.external() );
  }
  return filter;
}

Rule Rule::fromproto(const msgcenter::Rule &protoRule)
{
  Rule rule;
  if ( protoRule.type() == msgcenter::RuleType::kAllow ) {
    rule.setType(RuleType::kAllow);
  }
  else {
    rule.setType(RuleType::kDeny);
  }
  if ( protoRule.has_to() ) {
    rule.setTo(protoRule.to());
  }
  if ( protoRule.has_from() ) {
    rule.setFrom(protoRule.from());
  }
  rule.setMsgFilter( TlgFilter::fromProto(protoRule.tlgfilter()));
  if ( protoRule.has_priority() ) {
    rule.setPriority( protoRule.priority() );
  }
  return rule;
}

bool Rule::operator==(const Rule& rule) const
{
  return ( type_ == rule.type() && tlgFilter_ == rule.tlgFilter()
           && to_ == rule.to() && from_ == rule.from()
           && priority_ == rule.priority());
}

Rules::Rules(const msgcenter::DistributeMsgRequest* req )
{
  for (int i = 0, sz = req->rule_size(); i < sz; ++i  ) {
    append(Rule::fromproto(req->rule(i)));
  }
}

Rules::Rules(const Rules& rules) : QList<Rule>()
{
  for ( auto rule : rules ) {
    append(rule);
  }
}

bool Rules::allow(const Rule& reqRule) const
{
  if ( 0 == count() ) {
    return true;
  }
  bool answer = true;
  for ( auto rule : *this) {
    bool checkFilter = true;
    bool checkTo = true;
    bool checkFrom = true;
    bool ruleNotEmpty = false;
    if ( rule.type() == Rule::RuleType::kAllow ) {
      answer = true;
    }
    else if ( rule.type() == Rule::RuleType::kDeny){
      answer = false;
    }
    else {
      error_log << QObject::tr("Неверное правило в таблице");
      return false;
    }
    if (!rule.tlgFilter().isEmpty() && !reqRule.tlgFilter().isEmpty()) {
      if ( rule.tlgFilter().check( reqRule.tlgFilter() ) ) {
        checkFilter = answer;
        ruleNotEmpty = true;
      }
      else {
        continue;
      }
    }
    if ( !rule.to().isEmpty() ) {
      if ( rule.to() == kAll || rule.to() == reqRule.to() ) {
        checkTo = answer;
        ruleNotEmpty = true;
      }
      else {
        continue;
      }
    }
    if ( !rule.from().isEmpty() ) {
      if ( rule.from() == kAll || rule.from() == reqRule.from() ) {
        checkFrom = answer;
        ruleNotEmpty = true;
      }
      else {
        continue;
      }
    }
    if ( ruleNotEmpty ) {
      return  ( checkFilter && checkTo && checkFrom);
    }
  }
  return true;
}

void Rules::append( const msgcenter::Rule& protoRule )
{
  append( Rule::fromproto( protoRule ) );
}

void Rules::append( const Rule &rule )
{
  if ( contains(rule) ) {
    return;
  }
  int priority = rule.priority();
  for (int i = 0, sz = this->count(); i < sz; ++i ) {
    if ( this->at(i).priority() > priority ) {
      QList::insert(i, rule);
      return;
    }
  }
  QList::append(rule);
}

void Rules::append( const Rules &rules )
{
  for ( auto rule : rules ) {
    append(rule);
  }
}

void Rules::remove(const Rule& rule)
{
  removeOne(rule);
}

void Rules::remove(const Rules& rules)
{
  for ( auto rule : rules) {
    remove(rule);
  }
}

TlgFilter TlgFilter::filterFromMsg(const tlg::MessageNew &msg)
{
  TlgFilter filter;
  if ( msg.header().has_t1() ) {
    filter.sett1( QString::fromUtf8(msg.header().t1().c_str()) );
  }
  if ( msg.header().has_t2() ) {
    filter.sett2 ( QString::fromUtf8(msg.header().t2().c_str()) );
  }
  if ( msg.header().has_a1() ) {
    filter.seta1( QString::fromUtf8(msg.header().a1().c_str()) );
  }
  if ( msg.header().has_a2() ) {
    filter.seta2( QString::fromUtf8(msg.header().a2().c_str()) );
  }
  if ( msg.header().has_cccc() ) {
    filter.setCCCC( QString::fromUtf8(msg.header().cccc().c_str()) );
  }
  if ( msg.header().has_cccc() ) {
    filter.setBBB( QString::fromUtf8(msg.header().bbb().c_str()) );
  }
  if ( msg.has_format() ) {
    filter.setFormat( QString::fromUtf8(tlg::Format_Name(msg.format()).c_str()) );
  }
  if ( msg.header().has_ii() ) {
    filter.setii( QString::number(msg.header().ii()) );
  }
  if (msg.header().has_yygggg()) {
    filter.setYYGGgg( QString::fromUtf8(msg.header().yygggg().c_str()) );
  }
  filter.setTlgPriority( QString::number(tlg::priority(msg)) );

  if ( msg.has_metainfo() && msg.metainfo().has_from() ) {
    filter.setChannel( QString::fromUtf8(msg.metainfo().from().c_str()) );
  }

  if ( msg.has_msgtype() ) {
    filter.setMsgType( QString::fromUtf8(msg.msgtype().c_str()) );
  }
  if ( msg.has_metainfo() && msg.metainfo().has_external() ) {
    filter.setExternal(msg.metainfo().external());
  }
  return filter;
}

QString TlgFilter::toString()
{
  QString string;
  if ( !t1_.isEmpty() ) {
    string += QString("t1: %1 ").arg(t1_);
  }
  if ( !t2_.isEmpty() ) {
    string += QString("t2: %1 ").arg(t2_);
  }
  if ( !a1_.isEmpty() ) {
    string += QString("a1: %1 ").arg(a1_);
  }
  if ( !a2_.isEmpty() ) {
    string += QString("a2: %1 ").arg(a2_);
  }
  if ( !ii_.isEmpty() ) {
    string += QString("ii: %1 ").arg(ii_);
  }
  if ( !BBB_.isEmpty() ) {
    string += QString("BBB: %1").arg(BBB_);
  }
  if ( !CCCC_.isEmpty() ) {
    string += QString("CCCC: %1 ").arg(CCCC_);
  }
  if ( !format_.isEmpty() ) {
    string += QString("format: %1 ").arg(format_);
  }
  if ( !YYGGgg_.isEmpty() ) {
    string += QString("YYGGgg: %1 ").arg(YYGGgg_);
  }
  if ( !TlgPriority_.isEmpty() ) {
    string += QString("TlgPriority: %1 ").arg(TlgPriority_);
  }
  if ( !channel_.isEmpty() ) {
    string += QString("channel: %1 ").arg(channel_);
  }
  if ( !msgType_.isEmpty() ) {
    string += QString("msgType: %1 ").arg(msgType_);
  }
  return string;
}

} // meteo
