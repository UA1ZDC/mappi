#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <qtreewidget.h>

#include "lineedit.h"

namespace Ui{
  class LeftPanel;
}

namespace meteo {

typedef QMap<int, QString> GroupInfo;
typedef QMap<int, QString> FilterInfo;

class LeftPanel : public QWidget
{
  Q_OBJECT
public:
  LeftPanel(QWidget *parent = 0);
  ~LeftPanel();
  void clear();
  void clearT2();

  QStringList selectedT1();
  QStringList selectedT2();

  QTreeWidget* getT1Tree();
  QTreeWidget* getT2Tree();

  void selectT1(const QString& t1);
  void selectTT(const QString& t1, const QStringList& t2);
  void hideEditFilterButton(bool hide);
  void setT1Enabled(const QStringList list);
  void addItemToChecked(const QString& id);

private:
  Ui::LeftPanel* ui_;
  LineEdit* lineEdit_;
  QStringList checked_id_;
  QMap<QString,QString> items_group_;

  bool tryCreateList();
  void createList();
  void updateItemTree(const QString& group_id);

private slots:
  void slotUpdate();  

  void slotSearch(const QString& text);
  void slotCheckItem(QTreeWidgetItem* item);
  void slotCheckGroup(QTreeWidgetItem* item );
  void showParentItems(const QString& id);
  void showAllItems();

signals:
  void changeCondition();  

private slots:

private:
  bool hasSelectedT1();


  QTreeWidgetItem* itemByT1(const QString& t1);

  QString lastT1_;
  QSet<QString> lastT2_;

};

}
#endif
