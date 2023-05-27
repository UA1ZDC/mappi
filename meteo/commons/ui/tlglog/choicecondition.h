#ifndef METEO_COMMONS_UI_TLGLOG_CHOICECONDITION_H
#define METEO_COMMONS_UI_TLGLOG_CHOICECONDITION_H

#include <qdialog.h>
#include <qmap.h>
#include <qlistwidget.h>
#include "entergroupname.h"
#include <qtoolbutton.h>
#include <qsplitter.h>
#include <qsettings.h>
#include <qdir.h>

namespace Ui {
class choicecondition;
}

class choicecondition : public QDialog
{
  Q_OBJECT

public:
  explicit choicecondition(QWidget *parent = 0);
  ~choicecondition();
  void setGroupCondition( QMap<QString,QMap<int,QString>> condlst, QString curr_group,
                          QMap<int,QString> conditions,QMap<QString,QMap<int,QString>> condhumlst);

  QString currGroup(){return current_group_;}
  QMap<int,QString> currCondition(){return conditions_;}
  QMap< QString, QMap< int,QString > > groupCondList(){return group_cond_list_;}
  QMap< QString, QMap< int,QString > > groupCondHumList(){return group_cond_human_list_;}

protected:
  void accept();

private:

  Ui::choicecondition *ui_;
  EnterGroupName* enterName_;
  QMap<QString,QMap<int,QString > > group_cond_list_;
  QMap<QString,QMap<int,QString > > group_cond_human_list_;
  QStringList group_names_list_;
  QMap<int,QString> conditions_;
  QMap<int,QString> checkpanelconditions_;
  QString current_group_;
  QSplitter* splitter_;

  QSettings* settings_;

  bool ChoiceGroup();

private slots:
  void slotAddGroup();
  void slotRemoveGroup();
  void slotChoiceGroup();
  void slotHideCondPanel(bool);
  void slotGroupSelect();
  void slotCondEdit();
  void slotCondEditable(bool);
  void slotItemChanged(QString str);
  void slotSaveCond();
  void slotSaveGeometry();
  void slotRestoreGeometry();
};

#endif // CHOICECONDITION_H
