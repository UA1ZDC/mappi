#ifndef METEO_COMMONS_UI_CUSTOM_CUSTOMHEADER_H
#define METEO_COMMONS_UI_CUSTOM_CUSTOMHEADER_H

#include <qheaderview.h>
#include <qmap.h>
#include <qtreewidget.h>

class FilterHeader : public QHeaderView
{
  Q_OBJECT
  public:
    FilterHeader( QTreeWidget* tree );
    ~FilterHeader();

  void setUnsortedColumn(int col);
  void reloadFilters();
  void setCortColumn(int col, Qt::SortOrder order);
  const QString& getFilter(int column);

signals:
  void sortChanged(int col, Qt::SortOrder order);

  private:
    QTreeWidget* tree_;
    int clickedcolumn_;
    Qt::SortOrder order_;
    int sortcolumn_;
    QMap< int, QString > filters_;
    QMap< int, QString > defnames_;
    QList<int> unsorted_;



  private slots:
    void slotSectionClicked( int indx );
    void slotSortAscent();
    void slotSortDescent();
    void slotSetFilter();
};

#endif
