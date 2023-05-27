#ifndef METEO_COMMONS_UI_TLGLOG_REGEXPCONDITIONDIALOG_H
#define METEO_COMMONS_UI_TLGLOG_REGEXPCONDITIONDIALOG_H

#include <qdialog.h>
#include <meteo/commons/global/global.h>

namespace Ui
{
  class SqlCondition2;
}

class RegExpConditionDialog : public QDialog
{
  Q_OBJECT

public:
  RegExpConditionDialog(QWidget *parent = 0);
  void setCondition(const QString& condition, const QString& datatype = 0);
  QString condition() const { return condition_; }
  QString sql() const { return sql_; }
  QString checkpanelcond() const { return checkpanelcond_; }

protected:
  void accept();

private:
  Ui::SqlCondition2* ui_;
  QString condition_;
  QString sql_;
  QString checkpanelcond_;
  QString datatype_;

private slots:
  void slotCurrentIndexChanged(int index);
  void slotCurrentIndexChanged2(int index);
  void slotConditionEditChanged(QString str);

};

#endif // REGEXPCONDITIONDIALOG_H
