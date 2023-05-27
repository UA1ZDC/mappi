#ifndef ADDFILTERDIALOG_H
#define ADDFILTERDIALOG_H

#include <QtWidgets>

namespace Ui{
  class AddFilterDialog;
}

class TAddFilterDialog : public QDialog
{
  Q_OBJECT
 public:
  struct ItemInfo {
    QString id;
    QString name;
    QString t1;
    QString t2;
    QString a1;
    QString a2;
    QString groupID;
  };

  TAddFilterDialog(QWidget *parent = 0);

  QString id() const ;
  QString name() const;
  QString t1() const;
  QString t2() const;
  QString a1() const;
  QString a2() const;
  QString groupID() const;

  void fillForm(const ItemInfo& info);

private:
  Ui::AddFilterDialog* ui_;
  QMap<QString, ItemInfo> infomap_;
  QMap<QString, QString> name_id_;
  QString id_;

  QString findGroupName(const QString& group_id);

  void createList();
  bool checkFill();

private slots:
 void slotAnyChange();
 void slotGroupChange();

};

#endif //ADDFILTERDIALOG_H
