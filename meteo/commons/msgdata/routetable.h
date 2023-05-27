#ifndef METEO_COMMONS_MSGDATA_ROUTETABLE_H
#define METEO_COMMONS_MSGDATA_ROUTETABLE_H

#include <qmap.h>
#include <qhash.h>
#include <qobject.h>
#include <qstringlist.h>

#include <cross-commons/debug/tlog.h>
#include "msgfilter.h"

namespace meteo {
namespace msgcenter {
class RuleTable;
}
}

namespace meteo {

class RouteTable
{
public:
  static const QString kDbGateway;
  static const QString kSrcDataGateway;

  static const QString kCircTarget;
  static const QString kSelfTarget;
  static const QString kAddrTarget;

  static const QStringList kProtocols;

  static QStringList reservedGateways();

  RouteTable();

  void insert(const Rule& rule);
  void insert(const Rules& rules);
  Rules merge( const Rules& rules );
  void remove(const Rule& rule);
  void remove(const Rules& rules);
  bool routeToDb(const tlg::MessageNew& msg) const;

  void addSelfTarget(const QString& target) { selfTargets_ += target; }
  void clear();
  void fromProto(const msgcenter::RuleTable& opt);
  bool allow(const Rule& reqRule) const;
  Rules rules() const { return rules_;}

private:
  int rowId_;
  QStringList selfTargets_;
  Rules rules_;
  friend TLog& operator<< (TLog& log, const RouteTable& tbl);
};


inline TLog& operator<< (TLog& log, const RouteTable& tbl)
{
  QStringList lines;
  lines << "";
  lines << QString("type").leftJustified(8, ' ') + "   " +
           QString("to").leftJustified(30, ' ') + "   " +
           QString("from").leftJustified(30, ' ') + "   " +
           QString("filter").leftJustified(30, ' ') + "   " +
           QString("priority")
           ;
  lines << QString().fill('-', 8) + "   " +
           QString().fill('-', 30) + "   " +
           QString().fill('-', 30) + "   " +
           QString().fill('-', 30) + "   " +
           QString().fill('-', 5)
           ;

  for ( auto rule : tbl.rules() ) {
    QString type;
    switch (rule.type()) {
    case Rule::RuleType::kRequest: {
      type = QString("kRequest").leftJustified(8, ' ') + "   ";
      break;
    }
    case Rule::RuleType::kAllow: {
      type = QString("kAllow").leftJustified(8, ' ') + "   ";
      break;
    }
    case Rule::RuleType::kDeny: {
      type = QString("kDeny").leftJustified(8, ' ') + "   ";
      break;
    }

    }
    lines << type +
             rule.to().leftJustified(30, ' ') + "   " +
             rule.from().leftJustified(30, ' ') + "   " +
             rule.tlgFilter().toString().leftJustified(30, ' ') + "   " +
             QString::number(rule.priority()).leftJustified(5, ' ')
             ;
  }

  return log.nospace() << lines.join("\n").toLocal8Bit().constData();
}

}

#endif
