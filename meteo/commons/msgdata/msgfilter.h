#ifndef METEO_COMMONS_MSGPARSER_MSGFILTER_H
#define METEO_COMMONS_MSGPARSER_MSGFILTER_H

#include <qregexp.h>
#include <qstringlist.h>
#include <qmap.h>

namespace meteo {
namespace tlg {
class MessageNew;
}

namespace msgcenter {

class DistributeMsgRequest;
class Rule;
class TlgFilter;

}

}

namespace meteo {

class TlgFilter {

public:
  TlgFilter(){}
  TlgFilter(const tlg::MessageNew& msg);
  void sett1(const std::string& t1) { t1_ = QString::fromStdString(t1); }
  void sett1(const QString& t1) { t1_ = t1; }
  QString  t1() const  { return t1_; }

  void  sett2(const std::string& t2) { t2_ = QString::fromStdString(t2); }
  void  sett2(const QString& t2) { t2_ = t2; }
  QString  t2() const  { return t2_; }

  void  seta1(const std::string& a1) { a1_ = QString::fromStdString(a1); }
  void  seta1(const QString& a1) { a1_ = a1; }
  QString  a1() const  { return a1_; }

  void  seta2(const std::string& a2) { a2_ = QString::fromStdString(a2); }
  void  seta2(const QString& a2) { a2_ = a2; }
  QString  a2() const  { return a2_; }

  void  setii(const std::string& ii) { ii_ = QString::fromStdString(ii); }
  void  setii(const QString& ii) { ii_ = ii; }
  QString  ii() const  { return ii_; }

  void  setBBB(const std::string& BBB) { BBB_ = QString::fromStdString(BBB); }
  void  setBBB(const QString& BBB) { BBB_ = BBB; }
  QString  BBB() const  { return BBB_; }

  void  setCCCC(const std::string& CCCC) { CCCC_ = QString::fromStdString(CCCC); }
  void  setCCCC(const QString& CCCC) { CCCC_ = CCCC; }
  QString  CCCC() const  { return CCCC_; }

  void  setFormat(const std::string& format) { format_ = QString::fromStdString(format); }
  void  setFormat(const QString& format) { format_ = format; }
  QString  format() const  { return format_; }

  void  setYYGGgg(const std::string& YYGGgg) { YYGGgg_ = QString::fromStdString(YYGGgg); }
  void  setYYGGgg(const QString& YYGGgg) { YYGGgg_ = YYGGgg; }
  QString  YYGGgg() const  { return YYGGgg_; }

  void  setTlgPriority(const std::string& Priority) { TlgPriority_ = QString::fromStdString(Priority); }
  void  setTlgPriority(const QString& Priority) { TlgPriority_ = Priority; }
  QString  TlgPriority() const  { return TlgPriority_; }

  void  setChannel(const std::string& channel) { channel_ = QString::fromStdString(channel); }
  void  setChannel(const QString& channel) { channel_ = channel; }
  QString  channel() const  { return channel_; }

  void  setMsgType(const std::string& MsgType) { msgType_ = QString::fromStdString(MsgType); }
  void  setMsgType(const QString& MsgType) { msgType_ = MsgType; }
  QString  msgType() const  { return msgType_; }
  void setExternal( bool external ) { external_ = external; }
  bool external() const { return external_; }

  bool isEmpty() const;
  bool check(const TlgFilter& anotherFilter) const;
  bool operator==(const TlgFilter& anotherFilter) const;
  static TlgFilter fromProto(const msgcenter::TlgFilter& protoFilter );
  static TlgFilter filterFromMsg(const tlg::MessageNew& msg);
  QString toString();

private:
  QString t1_;
  QString t2_;
  QString a1_;
  QString a2_;
  QString ii_;
  QString BBB_;
  QString CCCC_;
  QString format_;
  QString YYGGgg_;
  QString TlgPriority_;
  QString channel_;
  QString msgType_;
  bool external_ = false;
};




class Rule {
public:
  enum RuleType {
    kRequest,
    kAllow,
    kDeny
  };

 public:
  void setMsgFilter( const TlgFilter filter ) { tlgFilter_ = filter; }
  TlgFilter tlgFilter() const { return tlgFilter_; }
  TlgFilter* mutableTlgFilter() { return &tlgFilter_; }
  void setType( RuleType type ) { type_ = type; }
  RuleType type() const { return type_; }
  void setTo( const QString& to ) { to_ = to; }
  void setTo( const std::string& to ) { to_ = QString::fromStdString(to); }
  QString to() const { return  to_; }
  void setFrom( const QString& from ) { from_ = from; }
  void setFrom( const std::string& from ) { from_ = QString::fromStdString(from); }
  QString from() const { return from_; }
  void setPriority( int priority ) { priority_ = priority; }
  int priority() const { return priority_; }
  static Rule fromproto( const msgcenter::Rule& protoRule );
  bool operator==( const Rule& anotherRule ) const;
//  toProto

private:
  RuleType type_;
  TlgFilter tlgFilter_;
  QString to_;
  QString from_;
  int priority_ = 0;


};

class Rules : public QList<Rule>
{
public:
  Rules() {}
  Rules( const msgcenter::DistributeMsgRequest* req );
  Rules( const Rules& rules );
  bool allow( const Rule& reqRule ) const;
  void append( const msgcenter::Rule& protoRule );
  void append( const Rule& rule );
  void append( const Rules& rules );
  void remove( const Rule& rule );
  void remove( const Rules& rules );
};
}

#endif // METEO_COMMONS_MSGPARSER_MSGFILTER_H
