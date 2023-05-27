#ifndef METEO_COMMONS_UI_CUSTOM_DESCRSELECT_H
#define METEO_COMMONS_UI_CUSTOM_DESCRSELECT_H

#include <qdialog.h>
#include <qtreewidget.h>

#include <commons/meteo_data/meteo_data.h>
#include <meteo/commons/proto/puanson.pb.h>

class QTreeWidgetItem;

namespace Ui {
  class DescrSelect;
}

class Editor;

class DescrSelect : public QDialog
{
  Q_OBJECT
  public:
    DescrSelect( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~DescrSelect();

    QTreeWidget* tree() const ;

    void setSelected( const QStringList& selected );

    meteo::puanson::proto::Id selectedDescr();
    void loadDescriptors(const QStringList& excludeDescrs = QStringList());

    void setFilter(const QStringList& descrs);

  private:
    Ui::DescrSelect* ui_;

    QStringList selected_;
    QStringList filter_;

  private slots:
    void slotItemDoubleClicked( QTreeWidgetItem* item, int column );
    void slotFilterToggled(bool checked);

  friend class Editor;
};

#endif
