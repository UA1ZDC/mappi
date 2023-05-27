#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_EDITOR_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_EDITOR_H

#include <qobject.h>
#include <qwidget.h>
#include <qtreewidget.h>

#include <commons/meteo_data/tmeteodescr.h>

#include "puansonitem.h"

namespace Ui {
 class PuansonEdit;
}

class QTreeWidgetItem;

namespace meteo {
namespace puanson {

namespace proto {
  class Id;
  class CellRule;
  class Puanson;
}

class Editor : public QWidget
{
  Q_OBJECT
  public:
    Editor( QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~Editor();

    QTreeWidget* descrtree() const ;

    void puansonChangedOnScene();

  protected:
    void closeEvent( QCloseEvent* e );
    void showEvent( QShowEvent* e );

  private:
    Ui::PuansonEdit* ui_;
    QString id_;   //!< идентиикатор шаблона
    QString name_; //!< наименование шаблона
    QString path_; //!< путь к файлу шаблона
    proto::Puanson* puanson_; //!< прото шаблона
    Item* item_;   //!< Элемент сцены, отображающий данные по шаблону
    int recttype_;
    int anchortype_;
    GridItem* grid_;

    void muteSignals();
    void turnSignals();

    void setRectType( int rect );
    void setAnchorType( int anchor );

    int rectType() const { return recttype_; }
    int anchorType() const { return anchortype_; }

    void loadFromProto( const proto::Puanson& proto ); //!< загрузить шаблон на уишку
    void saveToProto( proto::Puanson* proto );   //!< заполнить шаблон по уишке

    void updateFromProto( const proto::Puanson& proto );

    void updateScene();

    void addDescrToTree( const proto::Id& id );        //!< добавить дескриптор в дерево дескрипторов в уишке
    proto::CellRule* addDescrToProto( const proto::Id& id );       //!< добавить дескриптор в шаблон
    void setupTreeItem( proto::CellRule* rule );         //!< настроить treeitem по шаблону

    void save();                                 //!< сохранить изменения

    void saveLastParams();                       //!< :)
    bool loadLastParams();                       //!< :)

    void enableEdit();                            //!< разрешить реадктирование
    void disableEdit();                           //!< запретить редактирование

    void setupFormat( proto::CellRule* rule );
    void setupAlign( proto::CellRule* rule );
    void setupTestValue( proto::CellRule* rule );
    void setupValueView();

    proto::CellRule defaultRule() const ;
    proto::CellRule* currentRule();
    proto::CellRule* itemRule( QTreeWidgetItem* item );

    void setWindValuesFromCells();
    void setCellsFromWindValues();

  private slots:
    void slotSelectPuanson();                       //!< отработка кнопки "открыть шаблон"
    void slotSavePuanson();                       //!< отработка кнопки "сохранить шаблон"

    void slotDrawWindClicked( bool checked );     //!< отработка кнопки "есть ветер"
    void slotDrawFlowClicked( bool checked );     //!< отработка кнопки "есть течение"
    void slotDrawDFClicked( bool checked );     //!< отработка кнопки "Рисовать DF"

    void slotAddSign();                           //!< отработка кнопки "добавить дескриптор"
    void slotRmSign();                            //!< отработка кнопки "удалить дескриптор"

    void slotCellItemDoubleClicked( QTreeWidgetItem* item, int clmn );
    void slotCellItemClicked( QTreeWidgetItem* item, int clmn );
    void slotCellItemChanged( QTreeWidgetItem* item, int clmn );

    void slotBtnRectIndexChanged( int  indx );
    void slotBtnAnchorIndexChanged( int indx );
    void slotBtnGridClicked();

    void slotCenterIndexChanged( int index );
    void slotRadiusSpinValueChanged( int radius );

    void slotCrissCrossIndexChanged( int index );
    void slotTypeChanged( int index );

    void slotFFCanged( int val );
    void slotDDCanged( int val );
    void slotFFQualCanged( int indx );
    void slotDDQualCanged( int indx );

    void slotWindFontChanged( const QFont& fnt );

    void slotXchanged( int x );
    void slotYChanged( int y );
    void slotValueChanged( double value );
    void slotAlignChanged( int align );
    void slotYPlusDfIndexChanged( int indx );

    void slotSelectedDescriptorChanged( QTreeWidgetItem* item, QTreeWidgetItem* previous );
    void slotAbsentLabelClicked();
    void slotAbsentClear();
};

}
}

#endif
