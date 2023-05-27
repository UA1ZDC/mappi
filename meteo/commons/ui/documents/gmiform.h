#ifndef DOCUMENTSHORTGMI36FORM_H
#define DOCUMENTSHORTGMI36FORM_H

#include <qdialog.h>
#include <qdatetime.h>
#include <meteo/commons/global/formaldocsaver.h>
#include <meteo/commons/ui/custom/userselectform.h>

namespace Ui {
class GmiForm;
}

class GmiForm : public QWidget
{
  Q_OBJECT

public:
  explicit GmiForm(QWidget *parent = nullptr);
  ~GmiForm();


private slots:
  void initialLoad();
  void slotCreateDocumentButtonClicked();

private:
  void onDateTimeBeginChanged();
  void onSelectUsersBtnClicked();

private:
  Ui::GmiForm *ui;

  QDateTime dtBegin_;
  QDateTime dtEnd_;
  meteo::FormalDocSaver doc_;
  meteo::UserSelectForm *usersWidget_;
};

#endif
