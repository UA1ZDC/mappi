#ifndef METEO_NOVOST_UI_COLORALERT_CONDITIONEDIT_H
#define METEO_NOVOST_UI_COLORALERT_CONDITIONEDIT_H

#include <qwidget.h>

#include <meteo/commons/proto/meteotablo.pb.h>

namespace Ui {
class ConditionEdit;
}

namespace meteo {

class ConditionEdit : public QWidget
{
  Q_OBJECT

public:
  static QString toText(const tablo::Condition& cond);

  explicit ConditionEdit(QWidget *parent = 0);
  ~ConditionEdit();

  void fromProto(const tablo::Condition& condition);
  tablo::Condition toProto() const;

  int result() const { return result_; }

  void setVisible(bool visible);

signals:
  void visibilityChanged(bool visible);

private slots:
  void slotOperationChanged(int idx);
  void slotOkClicked();

private:
  void setupUi(tablo::Operation op);

private:
  Ui::ConditionEdit* ui_;

  int result_;
};

} // meteo

#endif // METEO_NOVOST_UI_COLORALERT_CONDITIONEDIT_H
