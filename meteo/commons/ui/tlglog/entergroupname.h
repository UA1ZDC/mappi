#ifndef METEO_COMMONS_UI_TLGLOG_ENTERGROUPNAME_H
#define METEO_COMMONS_UI_TLGLOG_ENTERGROUPNAME_H

#include <qdialog.h>
#include <qdebug.h>
#include <qdialogbuttonbox.h>

namespace Ui {
class EnterGroupName;
}

class EnterGroupName : public QDialog
{
  Q_OBJECT

public:
  explicit EnterGroupName(QWidget *parent = 0);
  ~EnterGroupName();

  QString groupName(){return group_name_;}

private:
  Ui::EnterGroupName *ui;
  QString group_name_;
  void accept();
};

#endif // ENTERGROUPNAME_H
