#ifndef METEO_COMMONS_UI_CUSTOMVIEWER_VIEWHEADER_H
#define METEO_COMMONS_UI_CUSTOMVIEWER_VIEWHEADER_H

#include <qstyle.h>
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qheaderview.h>
#include <qlayout.h>
#include <qmenu.h>
#include <qtablewidget.h>
#include <qtoolbutton.h>
#include <qwidgetaction.h>

#include <cross-commons/debug/tmap.h>

namespace meteo {

namespace bank {

typedef QMap<int,QList< QPair<QString, QString> > > MenuItems;

class ViewHeader : public QHeaderView
{
  Q_OBJECT
public:
  static const QString conditionNoConditions_;
  static const QString conditionCustom_;
  ViewHeader(QWidget *parent = nullptr);
  ~ViewHeader();
  void init();
  void showEvent(QShowEvent *e);  
  void setOriginalValueHidden( bool isHidden);

  void buildMenu(QMap<int,QSet< QPair<QString, QString> > > values);
  void buildMenu(MenuItems value );
  void setFilterEnabled(int id, bool on);
  void dropFilters();
  void addFilterColumn(int col);
  void setFilterColumns(QList<int> list);
  void setButtonVisible( int id, bool visible );
  void addMenu(QMenu* menu, int col);
  QMenu* getMenu(int col);
  void setMenuToolTip(int col, const QString& tooltip);
  void clearContent();
  bool restoreState(const QByteArray &state);
  void setHaveCondition(int col, bool ifHaveCondition);
  void setSecondValueDisplayed(int col, bool displayed);

protected:
  virtual void	paintEvent(QPaintEvent *e) override;

private:
  QMap<int, QToolButton*> btn_;
  QMap<int, QMenu*> menu_;
  MenuItems items_;
  QList<int> fcolumns_;
  bool originalValueHidden_ = false;
  QHash<int, bool> isSecondValueDisplayed_;

public slots:
  void fixComboPositions();

private slots:
  void slotFixComboPositions();
  void handleSectionResized(int i);
  void handleSectionMoved(int logical, int oldVisualIndex, int newVisualIndex);
  void slotMenuClicked();
  void slotActivateMenu(QAction* act);
  void slotOutsideMenuClicked();

//  void slotOnCondition();

signals:
  void activate(int col, QAction* act);
  void movecolumn(int oldVisualIndex, int newVisualIndex);
  void menuAboutToShow( int clmn, QMenu* menu );
};


} //bank

} //meteo

#endif // METEO_COMMONS_UI_CUSTOMVIEWER_VIEWHEADER_H
