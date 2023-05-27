#ifndef METEO_NOVOST_UI_COLORALERT_CONDITIONITEM_H
#define METEO_NOVOST_UI_COLORALERT_CONDITIONITEM_H

#include <qtreewidget.h>
#include <meteo/commons/proto/meteotablo.pb.h>

namespace meteo {
namespace tablo {

enum ColumnNum {
  kColumnGreen,
  kColumnYellow,
  kColumnRed,
  kColumnValue
};

enum Role {
  kRoleOperation = Qt::UserRole,
  kRoleOperandA,
  kRoleOperandB,
  kRoleOperandCode,
  kRoleValueDescription,
};

class ConditionItem : public QTreeWidgetItem
{
public:
  ConditionItem(QTreeWidget* parent = 0);

  void fromProto(const Condition& cond);
  Condition toProto() const;

  Color color() const;
  void setColor(tablo::Color color);
};

} // tablo
} // meteo

#endif // METEO_NOVOST_UI_COLORALERT_CONDITIONITEM_H
