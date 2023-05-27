#include "ui_city.h"
#include "citywidget.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <cross-commons/app/paths.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/map/object.h>
#include <meteo/commons/global/loadcity.h>
#include <commons/textproto/tprototext.h>
#include <qevent.h>

#include <fstream>
#include "displaysettingseditdialog.h"
//#include "city_display_settings.h"

namespace meteo {
namespace map {

const QString kMapSettingsFile( MnCommon::etcPath("meteo") + "/map.settings.conf" );
//столбцы в treewidget'ах
const int kVisibilityColumn = 0;
const int kCityMarkColumn = 1;
const int kCityTitleColumn = 2;
const int kCityGeneralizationLevelColumn = 3;
const int kPriorityColumn = 4;
const int kCityScaleColumn = 5;

void CityWidget::init()
{
  //cityMapModified = false;
  //displaySettingListModified = false;

  //задаем состав combobox'ов (здесь, т.к. в дизайнере не прописать data в item)
  ui_->modeLeftComboBox->addItem( QObject::tr("Все"), QVariant(-1) );
  ui_->modeLeftComboBox->addItem( QObject::tr("Столицы"), QVariant(meteo::map::proto::Capital) );
  ui_->modeLeftComboBox->addItem( QObject::tr("Столица РФ"), QVariant(meteo::map::proto::RussianCapital) );
  ui_->modeLeftComboBox->addItem( QObject::tr("Областные центры"), QVariant(meteo::map::proto::Regional) );
  ui_->modeLeftComboBox->addItem( QObject::tr("Города"), QVariant(meteo::map::proto::City) );
  ui_->modeLeftComboBox->addItem( QObject::tr("Села"), QVariant(meteo::map::proto::Village) );
  ui_->modeLeftComboBox->addItem( QObject::tr("Свои пункты"), QVariant(meteo::map::proto::Location) );

  ui_->modeRightComboBox->addItem( QObject::tr("Нераспределенные"), QVariant(-2) );
  ui_->modeRightComboBox->addItem( QObject::tr("Столицы"), QVariant(meteo::map::proto::Capital) );
  ui_->modeRightComboBox->addItem( QObject::tr("Столица РФ"), QVariant(meteo::map::proto::RussianCapital) );
  ui_->modeRightComboBox->addItem( QObject::tr("Областные центры"), QVariant(meteo::map::proto::Regional) );
  ui_->modeRightComboBox->addItem( QObject::tr("Города"), QVariant(meteo::map::proto::City) );
  ui_->modeRightComboBox->addItem( QObject::tr("Села"), QVariant(meteo::map::proto::Village) );
  ui_->modeRightComboBox->addItem( QObject::tr("Свои пункты"), QVariant(meteo::map::proto::Location) );

  ui_->cityMarkComboBox->addItem( QObject::tr("Без знака"), QVariant(0) );

  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kCircleCityMark)), "", QVariant(1) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kCircleFilledCityMark)), "", QVariant(2) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kSquareCityMark)), "", QVariant(3) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kSquareFilledCityMark)), "", QVariant(4) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kStarCityMark)), "", QVariant(5) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kStarFilledCityMark)), "", QVariant(6) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kAirCityMark)), "", QVariant(7) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kFlagCityMark)), "", QVariant(8) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kFlagRedCityMark)), "", QVariant(9) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kFlagRectRedCityMark)), "", QVariant(10) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kFlagBlueCityMark)), "", QVariant(11) );
  ui_->cityMarkComboBox->addItem( QIcon(cityMarkIconByIndex(meteo::map::proto::kFlagRectBlueCityMark)), "", QVariant(12) );

  
  //ui_->cityMarkComboBox->he
  //    ui_->cityMarkComboBox->insertSeparator(1);
  //    ui_->cityMarkComboBox->insertSeparator(3);
  //    ui_->cityMarkComboBox->insertSeparator(5);
  //    ui_->cityMarkComboBox->insertSeparator(7);
  //    ui_->cityMarkComboBox->insertSeparator(9);
  //    ui_->cityMarkComboBox->insertSeparator(11);

  QMap <Position, bool> aligns;
  aligns.insert(kTopCenter, true);
  aligns.insert(kTopLeft, true);
  aligns.insert(kTopRight, true);
  ui_->fontWidget->setAllowedAligns(aligns);

  dsOperator_ = new DisplaySettingsOperator();
  if (dsOperator_->loadedOK() == false){
    return;
  }

  fillCityListsFromMap();
  QStringList header;
  header.append(QObject::tr(""));//header.append(QObject::tr("Отображать"));
  header.append("");// усл. знак
  header.append(QObject::tr("Название"));
  header.append(QObject::tr("Уровень генерализации"));
  header.append(QObject::tr("Приоритет"));
  header.append(QObject::tr("Размер"));
  ui_->citiesLeftTreeWidget->setHeaderLabels(header);
  ui_->citiesLeftTreeWidget->setRootIsDecorated(false);
  ui_->citiesRightTreeWidget->setHeaderLabels(header);
  ui_->citiesRightTreeWidget->setRootIsDecorated(false);
  ui_->citiesLeftTreeWidget->headerItem()->setTextAlignment(kCityGeneralizationLevelColumn, Qt::AlignLeft |Qt::AlignVCenter);
  ui_->citiesLeftTreeWidget->headerItem()->setTextAlignment(kPriorityColumn, Qt::AlignLeft |Qt::AlignVCenter);
  ui_->citiesLeftTreeWidget->headerItem()->setTextAlignment(kCityScaleColumn, Qt::AlignLeft |Qt::AlignVCenter);
  ui_->citiesRightTreeWidget->headerItem()->setTextAlignment(kCityGeneralizationLevelColumn, Qt::AlignLeft |Qt::AlignVCenter);
  ui_->citiesRightTreeWidget->headerItem()->setTextAlignment(kPriorityColumn, Qt::AlignLeft |Qt::AlignVCenter);
  ui_->citiesRightTreeWidget->headerItem()->setTextAlignment(kCityScaleColumn, Qt::AlignLeft |Qt::AlignVCenter);

  updateModeWidget();

  ui_->toggleTwoPanelsPushButton->setChecked(false);
  toggleTwoPanels();

  ui_->citiesLeftTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui_->citiesRightTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
  resizeTreeWidgetsColumns();

  filterList(ui_->leftFilterLineEdit, ui_->citiesLeftTreeWidget, ui_->modeLeftComboBox);
  filterList(ui_->rightFilterLineEdit,  ui_->citiesRightTreeWidget, ui_->modeRightComboBox);

  ui_->fontWidget->setSpecialVisible(false);//уберем "только специальные шрифты"
  ui_->fontWidget->setSpecFontEnabled(false);

  //ui_->fontWidget->al

  connect(ui_->citiesLeftTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(editCity(QTreeWidgetItem*)));

  connect(ui_->clearLeftFilterToolButton, SIGNAL(clicked()), ui_->leftFilterLineEdit, SLOT(clear()));
  connect(ui_->clearRightFilterToolButton, SIGNAL(clicked()), ui_->rightFilterLineEdit, SLOT(clear()));

  connect(ui_->leftFilterLineEdit, SIGNAL(textChanged(QString)), this, SLOT(filterLists()));
  connect(ui_->rightFilterLineEdit, SIGNAL(textChanged(QString)), this, SLOT(filterLists()));

  connect(ui_->moveLeftToolButton, SIGNAL(clicked()), this, SLOT(changeItemMode()));
  connect(ui_->moveRightToolButton, SIGNAL(clicked()), this, SLOT(changeItemMode()));

  connect(ui_->addNewToolButton, SIGNAL(clicked()), this, SLOT(addNewCity()));


  connect(ui_->modeLeftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(filterLists()));
  connect(ui_->modeRightComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(filterLists()));

  connect(ui_->modeLeftComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModeWidget()));

  connectModeDisplaySettingsWidgets();

  connect(ui_->toggleTwoPanelsPushButton, SIGNAL(clicked()), this, SLOT(toggleTwoPanels()));

  connect(ui_->citiesLeftTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(toggleCityVisibility(QTreeWidgetItem*,int)));

  connect(ui_->closePushButton, SIGNAL(clicked()), this, SLOT(close()));

  connect(ui_->acceptPushButton, SIGNAL(clicked()), this, SLOT(flushSettings()));
}

void CityWidget::initMenu()
{
  contextMenu_ = new QMenu(this);
  contextMenu_->addAction(QObject::tr("Изменить параметры..."), this, SLOT(editCityTranslator()));
  QMenu * moveToMenu = new QMenu(QObject::tr("Переместить в группу"), contextMenu_);
  moveToMenu->addAction(QObject::tr("Столицы"), this, SLOT(changeSelectedItemsModeFromMenu()));
  moveToMenu->addAction(QObject::tr("Столица РФ"), this, SLOT(changeSelectedItemsModeFromMenu()));
  moveToMenu->addAction(QObject::tr("Областные центры"), this, SLOT(changeSelectedItemsModeFromMenu()));
  moveToMenu->addAction(QObject::tr("Города"), this, SLOT(changeSelectedItemsModeFromMenu()));
  moveToMenu->addAction(QObject::tr("Села"), this, SLOT(changeSelectedItemsModeFromMenu()));
  moveToMenu->addAction(QObject::tr("Свои пункты"), this, SLOT(changeSelectedItemsModeFromMenu()));
  contextMenu_->addMenu(moveToMenu);
  contextMenu_->addAction(QObject::tr("Сделать видимым"), this, SLOT(setSelectedCitiesVisibility()));
  QAction * alwaysShowAction = contextMenu_->addAction(QObject::tr("Отображать всегда"), this, SLOT(toggleAlwaysVisible()));
  alwaysShowAction->setCheckable(true);
}

void CityWidget::updateMenu(bool allowEditCityAction, bool visibilityActionShow, bool alwaysVisibleActionChecked)
{
  QList <QAction*> list = contextMenu_->actions();
  for (int i = 0; i < list.size(); i++){
    if ( list.at(i)->text() == QObject::tr("Изменить параметры...") )
      list[i]->setEnabled(allowEditCityAction);

    if ( list.at(i)->text() == QObject::tr("Сделать видимым") ||
         list.at(i)->text() == QObject::tr("Скрыть") ){
      if (visibilityActionShow == true )
        list[i]->setText(QObject::tr("Сделать видимым"));
      else
        list[i]->setText(QObject::tr("Скрыть"));
    }

    if ( list.at(i)->text() == QObject::tr("Отображать всегда") ) {
      if ( alwaysVisibleActionChecked == true )
        list.at(i)->setChecked(true);
      else
        list.at(i)->setChecked(false);
      if ( visibilityActionShow == true ) {
        list.at(i)->setEnabled(false);
      }
      else {
        list.at(i)->setEnabled(true);
      }
    }
  }
}

void CityWidget::editCity(QTreeWidgetItem *item)
{
  QString cityName = item->text(kCityTitleColumn);
  int cityIndex = item->data(kCityMarkColumn, Qt::UserRole).toInt();
  QMap <int, meteo::map::proto::CitySetting> * cityMap = dsOperator_->citiesMap();
  meteo::map::proto::CitySetting citySetting = cityMap->value(cityIndex);

  meteo::map::proto::DisplaySetting cityDisplaySetting = dsOperator_->filledCityDisplaySetting(cityIndex);

  DisplaySettingsEditDialog * dialog = new DisplaySettingsEditDialog(this);
  dialog->setProperty("city_name", cityName);
  dialog->setProperty("city_index", cityIndex);

  dialog->setCitySettings(citySetting);
  dialog->setDisplaySetting(cityDisplaySetting);
  dialog->adjustSize();
  dialog->show();
//  if (dialog->exec() != QDialog::Accepted) {
//    delete dialog;
//    return;
//  }

  connect( dialog, SIGNAL(finished(int)), SLOT(slotEditFinished(int)) );
  connect( dialog, SIGNAL(finished(int)), dialog, SLOT(deleteLater()) );
}

void CityWidget::editCityTranslator()
{
  editCity(ui_->citiesLeftTreeWidget->selectedItems().at(0));
}

void CityWidget::filterLists()
{
  QLineEdit * filterLE = NULL;
  QTreeWidget* filteredTW = NULL;
  QComboBox* filterCB = NULL;
  //QListWidget * filteredLW = NULL;

  //для левого списка городов
  if ( (sender() == ui_->leftFilterLineEdit) || (sender() == ui_->modeLeftComboBox)  ){
    filterLE = ui_->leftFilterLineEdit;
    filteredTW = ui_->citiesLeftTreeWidget;
    //filteredLW = ui_->citiesLeftListWidget;
    filterCB = ui_->modeLeftComboBox;
  }

  //для правого списка городов
  if ( (sender() == ui_->rightFilterLineEdit) || (sender() == ui_->modeRightComboBox) ){
    filterLE = ui_->rightFilterLineEdit;
    filteredTW = ui_->citiesRightTreeWidget;
    //filteredLW = ui_->citiesRightListWidget;
    filterCB = ui_->modeRightComboBox;
  }

  if (!filterLE || !filteredTW  || !filterCB) return;

  filterList(filterLE, filteredTW, filterCB);

}

void CityWidget::flushSettings()
{
  dsOperator_->flushSettingsToFiles();

  for (int idx = 0; idx < ui_->citiesLeftTreeWidget->topLevelItemCount(); idx++) {
    for( int col = 0; col <= ui_->citiesLeftTreeWidget->columnCount(); col++ ) {
      ui_->citiesLeftTreeWidget->topLevelItem(idx)->setBackgroundColor(col, Qt::white);
    }
  }
}

void CityWidget::filterList(QLineEdit * filterLE,
                            QTreeWidget* filteredTW,
                            QComboBox* filterCB)
{
  QString filterStr = filterLE->text();

  int modeId = filterCB->itemData(filterCB->currentIndex(), Qt::UserRole).toInt();

  int qty = filteredTW->topLevelItemCount();
  for (int i = 0; i < qty; i++){
    QTreeWidgetItem * item = filteredTW->topLevelItem(i);

    if ( ( item->text(kCityTitleColumn).contains(filterStr, Qt::CaseInsensitive) ) &&
         ( item->data(kCityTitleColumn ,Qt::UserRole).toInt() == modeId  ||//показываем выбранную группу
           modeId == -1   || //или показываем все города
           (modeId == -2 &&
            item->data(kCityTitleColumn, Qt::UserRole).toInt() == -1) ) )//или показываем нераспределенные города (для правой панели)
      item->setHidden(false);
    else item->setHidden(true);
  }
}


void CityWidget::fillCityListsFromMap()
{
  ui_->citiesLeftTreeWidget->clear();
  ui_->citiesRightTreeWidget->clear();
  cityItemMap_.clear();
  //Добавить данные в оба списка городов из cityMap
  QMap<int, meteo::map::proto::CitySetting>::Iterator mapRecord;
  QMap <int, meteo::map::proto::CitySetting> *cityMap = dsOperator_->citiesMap();
  mapRecord = cityMap->begin();
  while (mapRecord != cityMap->end()){
    QString cityTitle = QString::fromStdString(mapRecord.value().title());
    QTreeWidgetItem *leftTItem = new QTreeWidgetItem;
    leftTItem->setText(kCityTitleColumn, cityTitle);
    //в data хранится группа, к которой относится город. Если группа в прото-файле не задана, ставим -1
    if (mapRecord.value().has_city_mode())
      leftTItem->setData(kCityTitleColumn, Qt::UserRole, QVariant(mapRecord.value().city_mode()));
    else
      leftTItem->setData(kCityTitleColumn, Qt::UserRole, QVariant(-1));

    ui_->citiesLeftTreeWidget->addTopLevelItem(leftTItem);

    // индекс города будет храниться в data столбца с условным знаком
    leftTItem->setData(kCityMarkColumn, Qt::UserRole, QVariant(mapRecord.key()));

    //ищем настройки отображения для города
    meteo::map::proto::DisplaySetting cityDS;
    cityDS = dsOperator_->filledCityDisplaySetting(mapRecord.key());

    if ( cityDS.visible() == true ) {
      if ( cityDS.has_always_visible() &&
           cityDS.always_visible() == true) {
        leftTItem->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible_locked.png"));
      }
      else {
        leftTItem->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
      }
    }
    else {
      leftTItem->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
    }

    QString cityMarkIconStr = cityMarkIconByIndex( cityDS.city_mark() );
    if (cityMarkIconStr.isEmpty() == false ) {
      leftTItem->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
    }
   
    QFont cityFont = font2qfont(cityDS.font());
    QPen cityPen = pen2qpen(cityDS.pen());
    leftTItem->setTextColor(kCityTitleColumn, cityPen.color());
    leftTItem->setFont(kCityTitleColumn, cityFont);

    leftTItem->setText(kCityGeneralizationLevelColumn, QVariant(cityDS.generalization_level()).toString() );
    leftTItem->setText(kPriorityColumn, QVariant(cityDS.priority()).toString() );
    //leftTItem->setText(kCityScaleColumn, QVariant(cityDS.city_scale()).toString() );
    leftTItem->setText(kCityScaleColumn, QString::number(cityDS.city_scale(), 'f', 2) );

    leftTItem->setTextAlignment(kVisibilityColumn, Qt::AlignHCenter |Qt::AlignVCenter);
    leftTItem->setTextAlignment(kCityGeneralizationLevelColumn, Qt::AlignLeft |Qt::AlignVCenter);
    leftTItem->setTextAlignment(kPriorityColumn, Qt::AlignLeft |Qt::AlignVCenter);
    leftTItem->setTextAlignment(kCityScaleColumn, Qt::AlignLeft |Qt::AlignVCenter);

    leftTItem->setSizeHint(kVisibilityColumn, QSize(24, 24));

    QTreeWidgetItem * rightTItem = leftTItem->clone();
    ui_->citiesRightTreeWidget->addTopLevelItem(rightTItem);
    cityItemMap_.insert(mapRecord.key(), QPair <QTreeWidgetItem*, QTreeWidgetItem*> (leftTItem, rightTItem));

    mapRecord++;
  }
  ui_->citiesLeftTreeWidget->sortByColumn(kCityTitleColumn, Qt::AscendingOrder);
  ui_->citiesRightTreeWidget->sortByColumn(kCityTitleColumn, Qt::AscendingOrder);

}

void CityWidget::contextMenuEvent(QContextMenuEvent *event)
{
  QList <QTreeWidgetItem*> list = ui_->citiesLeftTreeWidget->selectedItems();
  if ( list.size() < 1 )
    return;

  QPoint globalCursorPoint = event->globalPos();
  bool allowEditCityAction, visibilityActionShow, alwaysVisibleActionChecked;

  if ( list.size() > 1 )//вызвать диалоговое окно можно только для одного города
    allowEditCityAction = false;
  else
    allowEditCityAction = true;

  visibilityActionShow = false;//исходим из того, что выделено несколько городов
  //тогда пункт меню "скрыть" будет доступен только если ВСЕ города видимы
  //в противном случае на этом месте будет пункт меню "сделать видимым"
  alwaysVisibleActionChecked = true;
  for (int i = 0; i < list.size(); i++){
    //QString cityTitle = list.at(i)->text(kCityTitleColumn);
    int cityIndex = list.at(i)->data(kCityMarkColumn, Qt::UserRole).toInt();
    meteo::map::proto::DisplaySetting cityDS = dsOperator_->filledCityDisplaySetting(cityIndex);
    if ( cityDS.has_always_visible() == false )
      alwaysVisibleActionChecked = false;

    //if ( cityDS.has_always_visible() )
    //continue;//это чтобы элементы с параметром always_visible не влияли на следующие три строки

    if ( cityDS.has_visible() &&
         cityDS.visible() == false )
      visibilityActionShow = true;
  }

  for (int i = 0; i < list.size(); i++){
    QPoint topLeftViewport = ui_->citiesLeftTreeWidget->visualItemRect(list.at(i)).topLeft();
    QPoint bottomRightVewport = ui_->citiesLeftTreeWidget->visualItemRect(list.at(i)).bottomRight();
    QPoint topLeftGlobal = ui_->citiesLeftTreeWidget->viewport()->mapToGlobal(topLeftViewport);
    QPoint bottomRightGlobal = ui_->citiesLeftTreeWidget->viewport()->mapToGlobal(bottomRightVewport);
    QRect mappedToGlobalRect(topLeftGlobal, bottomRightGlobal);

    if ( mappedToGlobalRect.contains(globalCursorPoint) ) {
      updateMenu(allowEditCityAction, visibilityActionShow, alwaysVisibleActionChecked);
      contextMenu_->exec(event->globalPos());
      break;
    }
  }
  event->accept();
}

void CityWidget::resizeEvent(QResizeEvent *event)
{
  resizeTreeWidgetsColumns();
  event->accept();
}

void CityWidget::resizeTreeWidgetsColumns()
{
  if ( ui_->citiesRightTreeWidget->isVisible() ){
    ui_->citiesRightTreeWidget->setColumnWidth(kVisibilityColumn, 32);
    ui_->citiesRightTreeWidget->setColumnWidth(kCityMarkColumn, 32);
    ui_->citiesRightTreeWidget->setColumnWidth(kCityTitleColumn, 250);
    ui_->citiesRightTreeWidget->setColumnWidth(kCityGeneralizationLevelColumn, 175);
  }
  ui_->citiesLeftTreeWidget->setColumnWidth(kVisibilityColumn, 32);
  ui_->citiesLeftTreeWidget->setColumnWidth(kCityMarkColumn, 32);
  ui_->citiesLeftTreeWidget->setColumnWidth(kCityTitleColumn, 250);
  ui_->citiesLeftTreeWidget->setColumnWidth(kCityGeneralizationLevelColumn, 175);
  //ui_->citiesLeftTreeWidget->setColumnWidth(3, 75);
}

void CityWidget::refitCitiesItems(/*QString cityTitle*/)
{//корректирует данные городов в списках citiesRightTreeWidget и citiesLeftTreeWidget


  QMap <int, meteo::map::proto::CitySetting> * cityMap;
  meteo::map::proto::CitySetting citySetting;
  meteo::map::proto::DisplaySetting displaySetting;
  cityMap = dsOperator_->citiesMap();

  QMap <int, QPair<QTreeWidgetItem*, QTreeWidgetItem*> >::Iterator cityItemMapRecord;
  cityItemMapRecord = cityItemMap_.begin();
  while (cityItemMapRecord != cityItemMap_.end()) {
    if (cityMap->contains(cityItemMapRecord.key()) == false) {
      continue;
    }

    citySetting = cityMap->value(cityItemMapRecord.key());
    displaySetting = dsOperator_->filledCityDisplaySetting(cityItemMapRecord.key());

    QPair <QTreeWidgetItem*, QTreeWidgetItem*> pair = cityItemMapRecord.value();
    QTreeWidgetItem *left = pair.first;
    QTreeWidgetItem *right = pair.second;

    //на основе даннных из dsOperator_ заполняются: видимость, видимость всегда, группа, приоритет,
    //уровень генерализации
    if (citySetting.has_city_mode()) {
      left->setData(kCityTitleColumn, Qt::UserRole, QVariant(citySetting.city_mode()));
      right->setData(kCityTitleColumn, Qt::UserRole, QVariant(citySetting.city_mode()));
    }
    else {
      left->setData(kCityTitleColumn, Qt::UserRole, QVariant(-1));
      right->setData(kCityTitleColumn, Qt::UserRole, QVariant(-1));
    }

    if ( displaySetting.visible() == true ) {
      left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
      right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
    }
    else {
      left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
      right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
    }

    if ( displaySetting.has_always_visible() &&
         displaySetting.always_visible() == true) {
      left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible_locked.png"));
      right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible_locked.png"));
    }

    QFont cityFont = font2qfont(displaySetting.font());
    QPen cityPen = pen2qpen(displaySetting.pen());
    left->setTextColor(kCityTitleColumn, cityPen.color());
    left->setFont(kCityTitleColumn, cityFont);
    right->setTextColor(kCityTitleColumn, cityPen.color());
    right->setFont(kCityTitleColumn, cityFont);

    left->setText(kCityGeneralizationLevelColumn, QVariant(displaySetting.generalization_level()).toString());
    left->setText(kPriorityColumn, QVariant(displaySetting.priority()).toString());
    left->setText(kCityScaleColumn, QVariant(displaySetting.city_scale()).toString());
    right->setText(kCityGeneralizationLevelColumn, QVariant(displaySetting.generalization_level()).toString());
    right->setText(kPriorityColumn, QVariant(displaySetting.priority()).toString());
    right->setText(kCityScaleColumn, QVariant(displaySetting.city_scale()).toString());

    cityItemMapRecord++;
  }
}

void CityWidget::refitCitiesItems(int cityMode, QWidget *sender)
{//cityMode имеет тип int, т.к. может иметь значение -1, что сответствует значениям по умолчанию

  QMap <int, meteo::map::proto::CitySetting> *citiesMap = dsOperator_->citiesMap();
  QMap <int, meteo::map::proto::CitySetting>::Iterator citiesMapIterator;// = citySettingsMap->begin();

  //citySettingsMapIterator--;//компенсация первого ++ в цикле
  //while (citySettingsMapIterator != citySettingsMap->end()) {
  QMap <int, meteo::map::proto::DisplaySetting> *cityDisplaySettingMap = dsOperator_->cityDisplaySettingMap();
  QMap <meteo::map::proto::CityMode, meteo::map::proto::DisplaySetting> *cityModeDisplaySettingMap = dsOperator_->cityModeDisplaySettingMap();
  meteo::map::proto::DisplaySetting defaultDisplaySetting = dsOperator_->defaultDisplaySetting();

  for (citiesMapIterator = citiesMap->begin();
       citiesMapIterator != citiesMap->end(); citiesMapIterator++) {

    //QString cityTitle = citySettingsMapIterator.key();
    int cityIndex = citiesMapIterator.key();
    if ( false == cityItemMap_.contains(cityIndex) ) {
      return;
    }
    QPair <QTreeWidgetItem*, QTreeWidgetItem*> pair = cityItemMap_.value(cityIndex);
    QTreeWidgetItem *left = pair.first;
    QTreeWidgetItem *right = pair.second;
    if (left->data(kCityTitleColumn, Qt::UserRole).toInt() != cityMode &&
        cityMode != -1) {//если менялись настройки по умолчанию, то все-таки придется перебрать все
      continue;
    }

    //meteo::map::proto::DisplaySetting cityDisplaySetting = dsOperator_->filledCityDisplaySetting(cityTitle);
    //если использовать средства класса DisplaySettingsOperator, т.е. для каждого города полностью вычислять унаследованные настройки
    //отображения, то программа начинает слишком долго думать
    //поэтому здесь придется работать вручную по конкретным изменившимся настройкам

    meteo::map::proto::DisplaySetting cityDS, modeDS;
    if (cityDisplaySettingMap->contains(cityIndex) == true) {
      cityDS = cityDisplaySettingMap->value(cityIndex);
    }//else будет пустая
    if (left->data(kCityTitleColumn, Qt::UserRole).toInt() != -1 &&
        cityModeDisplaySettingMap->contains(static_cast <meteo::map::proto::CityMode> (left->data(kCityTitleColumn, Qt::UserRole).toInt())) == true) {
      modeDS = cityModeDisplaySettingMap->value(static_cast <meteo::map::proto::CityMode> (left->data(kCityTitleColumn, Qt::UserRole).toInt()));
    }

    if (sender == ui_->fontWidget) {
      meteo::map::proto::DisplaySetting cityDisplaySetting = dsOperator_->filledCityDisplaySetting(cityIndex);//TODO но похоже в этом случае - только так
      //Font cityProtoFont = fillFont()
      QFont cityFont = font2qfont(cityDisplaySetting.font());
      QPen cityPen = pen2qpen(cityDisplaySetting.pen());
      left->setTextColor(kCityTitleColumn, cityPen.color());
      left->setFont(kCityTitleColumn, cityFont);
      right->setTextColor(kCityTitleColumn, cityPen.color());
      right->setFont(kCityTitleColumn, cityFont);
      continue;
    }

    if (sender == ui_->visibilityComboBox) {
      if (cityDS.has_always_visible() == true &&
          cityDS.always_visible() == true) {
        left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible_locked.png"));
        right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible_locked.png"));
        continue;
      }
      if (cityDS.has_visible() == true){
        if (cityDS.visible() == true){
          left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
          right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
          continue;
        }
        if (cityDS.visible() == false){
          left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
          right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
          continue;
        }
      }

      if (modeDS.has_visible() == true) {
        if (modeDS.visible() == true) {
          left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
          right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
          continue;
        }
        if (modeDS.visible() == false) {
          left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
          right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
          continue;
        }
      }

      if (defaultDisplaySetting.has_visible() == true) {
        if (defaultDisplaySetting.visible() == true) {
          left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
          right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
          continue;
        }
        if (defaultDisplaySetting.visible() == false) {
          left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
          right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
          continue;
        }
      }
      continue;
    }

    if (sender == ui_->cityMarkComboBox) {
      QString cityMarkIconStr;
      if ( cityDS.has_city_mark() == true ) {
        cityMarkIconStr = cityMarkIconByIndex( cityDS.city_mark() );
        if (cityMarkIconStr.isEmpty() == false ) {
          left->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
          right->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
        }
        else {
          left->setIcon(kCityMarkColumn, QIcon() );
          right->setIcon(kCityMarkColumn, QIcon() );
        }
        continue;
      }
      if ( modeDS.has_city_mark() == true ) {
        cityMarkIconStr = cityMarkIconByIndex( modeDS.city_mark() );
        if (cityMarkIconStr.isEmpty() == false ) {
          left->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
          right->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
        }
        else {
          left->setIcon(kCityMarkColumn, QIcon() );
          right->setIcon(kCityMarkColumn, QIcon() );
        }
        continue;
      }
      if ( defaultDisplaySetting.has_city_mark() == true ) {
        cityMarkIconStr = cityMarkIconByIndex( defaultDisplaySetting.city_mark() );
        if (cityMarkIconStr.isEmpty() == false ) {
          left->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
          right->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
        }
        else {
          left->setIcon(kCityMarkColumn, QIcon() );
          right->setIcon(kCityMarkColumn, QIcon() );
        }
        continue;
      }
      //else
      left->setIcon(kCityMarkColumn, QIcon() );
      right->setIcon(kCityMarkColumn, QIcon() );
      continue;
    }


    if (sender == ui_->cityScaleSpinBox) {
      if (cityDS.has_city_scale()) {
        left->setText(kCityScaleColumn, QVariant(cityDS.city_scale()).toString() );
        right->setText(kCityScaleColumn, QVariant(cityDS.city_scale()).toString() );
        continue;
      }
      if (modeDS.has_city_scale()) {
        left->setText(kCityScaleColumn, QVariant(modeDS.city_scale()).toString() );
        right->setText(kCityScaleColumn, QVariant(modeDS.city_scale()).toString() );
        continue;
      }
      if (defaultDisplaySetting.has_city_scale()) {
        left->setText(kCityScaleColumn, QVariant(defaultDisplaySetting.city_scale()).toString() );
        right->setText(kCityScaleColumn, QVariant(defaultDisplaySetting.city_scale()).toString() );
        continue;
      }
      continue;
    }
    

    if (sender == ui_->prioritySpinBox) {
      if (cityDS.has_priority()) {
        left->setText(kPriorityColumn, QVariant(cityDS.priority()).toString() );
        right->setText(kPriorityColumn, QVariant(cityDS.priority()).toString() );
        continue;
      }
      if (modeDS.has_priority()) {
        left->setText(kPriorityColumn, QVariant(modeDS.priority()).toString() );
        right->setText(kPriorityColumn, QVariant(modeDS.priority()).toString() );
        continue;
      }
      if (defaultDisplaySetting.has_priority()) {
        left->setText(kPriorityColumn, QVariant(defaultDisplaySetting.priority()).toString() );
        right->setText(kPriorityColumn, QVariant(defaultDisplaySetting.priority()).toString() );
        continue;
      }
      continue;
    }

    if (sender == ui_->generalizationLevelSpinBox) {

      if (cityDS.has_generalization_level()) {
        left->setText(kCityGeneralizationLevelColumn, QVariant(cityDS.generalization_level()).toString() );
        right->setText(kCityGeneralizationLevelColumn, QVariant(cityDS.generalization_level()).toString() );
        continue;
      }
      if (modeDS.has_generalization_level()) {
        left->setText(kCityGeneralizationLevelColumn, QVariant(modeDS.generalization_level()).toString() );
        right->setText(kCityGeneralizationLevelColumn, QVariant(modeDS.generalization_level()).toString() );
        continue;
      }
      if (defaultDisplaySetting.has_generalization_level()) {
        left->setText(kCityGeneralizationLevelColumn, QVariant(defaultDisplaySetting.generalization_level()).toString() );
        right->setText(kCityGeneralizationLevelColumn, QVariant(defaultDisplaySetting.generalization_level()).toString() );
        continue;
      }
      continue;
    }
  }
}

void CityWidget::refitCityItems(int cityIndex)
{
  QMap <int, meteo::map::proto::CitySetting> * cityMap;
  meteo::map::proto::CitySetting citySetting;
  meteo::map::proto::DisplaySetting displaySetting;
  cityMap = dsOperator_->citiesMap();
  citySetting.CopyFrom(cityMap->value(cityIndex));
  displaySetting.CopyFrom(dsOperator_->filledCityDisplaySetting(cityIndex));

  QTreeWidgetItem *left = cityItemMap_.value(cityIndex).first;
  QTreeWidgetItem *right = cityItemMap_.value(cityIndex).second;

  if (citySetting.has_city_mode()) {
    left->setData(kCityTitleColumn, Qt::UserRole, QVariant(citySetting.city_mode()));
    right->setData(kCityTitleColumn, Qt::UserRole, QVariant(citySetting.city_mode()));
  }
  else {
    left->setData(kCityTitleColumn, Qt::UserRole, QVariant(-1));
    right->setData(kCityTitleColumn, Qt::UserRole, QVariant(-1));
  }

  if ( displaySetting.visible() == true ) {
    left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
    right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
  }
  else {
    left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
    right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
  }

  if ( displaySetting.has_always_visible() &&
       displaySetting.always_visible() == true) {
    left->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible_locked.png"));
    right->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible_locked.png"));
  }

  QString cityMarkIconStr = cityMarkIconByIndex( displaySetting.city_mark() );
  if (cityMarkIconStr.isEmpty() == false ) {
    left->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
    right->setIcon(kCityMarkColumn, QIcon(cityMarkIconStr) );
  }
  else {
    left->setIcon(kCityMarkColumn, QIcon() );
    right->setIcon(kCityMarkColumn, QIcon() );
  }

  QFont cityFont = font2qfont(displaySetting.font());
  QPen cityPen = pen2qpen(displaySetting.pen());
  left->setTextColor(kCityTitleColumn, cityPen.color());
  left->setFont(kCityTitleColumn, cityFont);
  right->setTextColor(kCityTitleColumn, cityPen.color());
  right->setFont(kCityTitleColumn, cityFont);

  left->setText(kCityGeneralizationLevelColumn, QVariant(displaySetting.generalization_level()).toString());
  left->setText(kPriorityColumn, QVariant(displaySetting.priority()).toString());
  left->setText(kCityScaleColumn, QVariant(displaySetting.city_scale()).toString());
  right->setText(kCityGeneralizationLevelColumn, QVariant(displaySetting.generalization_level()).toString());
  right->setText(kPriorityColumn, QVariant(displaySetting.priority()).toString());
  right->setText(kCityScaleColumn, QVariant(displaySetting.city_scale()).toString());
}

void CityWidget::connectModeDisplaySettingsWidgets()
{
  connect(ui_->fontWidget, SIGNAL(paramChanged(QFont,int,QRgb)), this, SLOT(updateModeDisplaySetting()));
  connect(ui_->fontWidget, SIGNAL(colorChanged(QRgb)), this, SLOT(updateModeDisplaySetting()));
  connect(ui_->visibilityComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModeDisplaySetting()));
  connect(ui_->prioritySpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateModeDisplaySetting()));
  connect(ui_->cityScaleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateModeDisplaySetting()));
  connect(ui_->generalizationLevelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateModeDisplaySetting()));
  connect(ui_->cityMarkComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModeDisplaySetting()));
}

void CityWidget::disconnectModeDisplaySettingsWidgets()
{
  disconnect(ui_->fontWidget, SIGNAL(paramChanged(QFont,int,QRgb)), this, SLOT(updateModeDisplaySetting()));
  disconnect(ui_->fontWidget, SIGNAL(colorChanged(QRgb)), this, SLOT(updateModeDisplaySetting()));
  disconnect(ui_->visibilityComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModeDisplaySetting()));
  disconnect(ui_->prioritySpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateModeDisplaySetting()));
  disconnect(ui_->cityScaleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(updateModeDisplaySetting()));
  disconnect(ui_->generalizationLevelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateModeDisplaySetting()));
  disconnect(ui_->cityMarkComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(updateModeDisplaySetting()));
}

void CityWidget::changeItemMode()
{
  //int modeFrom_int = -1;
  int modeTo_int = -1;
  //meteo::map::proto::CityMode modeTo;
  QList <QTreeWidgetItem*> itemsList;

  //проверяем, кто послал сигнал
  QComboBox *toCombo = NULL;
  QComboBox *fromCombo = NULL;

  if ( sender() == ui_->moveRightToolButton ){
    itemsList = ui_->citiesLeftTreeWidget->selectedItems();
    toCombo = ui_->modeRightComboBox;
    fromCombo = ui_->modeLeftComboBox;
  }
  if ( sender() == ui_->moveLeftToolButton ){
    itemsList = ui_->citiesRightTreeWidget->selectedItems();
    toCombo = ui_->modeLeftComboBox;
    fromCombo = ui_->modeRightComboBox;
  }

  if ( (fromCombo == NULL) || //если условие выполнилось - это не кнопки
       (toCombo == NULL) ){
    return;
  }

  int toComboIndex = toCombo->currentIndex();
  //    if (toComboIndex == -1){
  //        return;//вообще такого быть не должно - в комбобоксах хоть что-то, но выбрано
  //    }

  //QString cityTitle("");

  if ( (itemsList.count() < 1) ||
       /*(fromComboIndex == -1) ||*///обратите внимание - пока говорим про индексы в combBox'е, т.е. выбрано что-то вообще или не выбрано, а не про itemData, т.е. номер группы
       (toComboIndex == -1) ) {
    return; //если ничего не выделено
  }
  QMap <int, meteo::map::proto::CitySetting> * cityMap;
  cityMap = dsOperator_->citiesMap();

  for (int i = 0; i < itemsList.size(); i++){
    modeTo_int = toCombo->itemData(toComboIndex, Qt::UserRole).toInt();


    //cityTitle = itemsList.at(i)->text(kCityTitleColumn);
    int cityIndex = itemsList.at(i)->data(kCityMarkColumn, Qt::UserRole).toInt();
    meteo::map::proto::CitySetting cSetts;
    if (cityMap->contains(cityIndex) == false){
      return;
    }
    cSetts.CopyFrom(cityMap->value(cityIndex));
    if ( modeTo_int == -1 ||//если выбрана группа "Все" или "Нераспределенные" - убираем из текущей записи указания на группы вообще
         modeTo_int == -2 ) {
      cSetts.clear_city_mode();
    }
    else {
      meteo::map::proto::CityMode mode = static_cast <meteo::map::proto::CityMode> (modeTo_int);
      cSetts.set_city_mode(mode);
    }
    cityMap->insert(cityIndex, cSetts);
    refitCityItems(cityIndex);

    for( int col = 0; col <= ui_->citiesLeftTreeWidget->columnCount(); col++ ) {
      itemsList.at(i)->setBackgroundColor(col, Qt::yellow);
    }
  }

  filterList(ui_->leftFilterLineEdit, ui_->citiesLeftTreeWidget, ui_->modeLeftComboBox);
  filterList(ui_->rightFilterLineEdit, ui_->citiesRightTreeWidget, ui_->modeRightComboBox);
  dsOperator_->setCityMapModified(true);
}

void CityWidget::addNewCity()
{
  QMap <int, meteo::map::proto::CitySetting> * cityMap = dsOperator_->citiesMap();
  meteo::map::proto::CitySetting citySetting = cityMap->value(0);

  int cityIndex = cityMap->size();
  while (true  == cityMap->contains(cityIndex)) {
    ++cityIndex;
  }

  QString cityName = QString("Новый город");

  citySetting.set_title(cityName.toStdString().c_str());

  cityMap->insert(cityIndex, citySetting);
  meteo::map::proto::DisplaySetting cityDisplaySetting = dsOperator_->filledCityDisplaySetting(cityIndex);

  DisplaySettingsEditDialog * dialog = new DisplaySettingsEditDialog(this);
  dialog->setProperty("city_name", cityName);
  dialog->setProperty("city_index", cityIndex);

  dialog->setCitySettings(citySetting);
  dialog->setDisplaySetting(cityDisplaySetting);
  dialog->adjustSize();
  dialog->show();
  connect( dialog, SIGNAL(finished(int)), SLOT(slotAddFinished(int)) );
  connect( dialog, SIGNAL(finished(int)),  dialog, SLOT(deleteLater()) );
}

void CityWidget::changeSelectedItemsModeFromMenu()
{
  QAction * senderAction = static_cast <QAction *> (sender());
  meteo::map::proto::CityMode modeTo;

  bool ok = false;
  if ( senderAction->text() == QObject::tr("Столицы") ){
    modeTo = meteo::map::proto::Capital;
    ok = true;
  }
  if ( senderAction->text() == QObject::tr("Столица РФ") ){
    modeTo = meteo::map::proto::RussianCapital;
    ok = true;
  }
  if ( senderAction->text() == QObject::tr("Областные центры") ){
    modeTo = meteo::map::proto::Regional;
    ok = true;
  }
  if ( senderAction->text() == QObject::tr("Города") ){
    modeTo = meteo::map::proto::City;
    ok = true;
  }
  if ( senderAction->text() == QObject::tr("Села") ){
    modeTo = meteo::map::proto::Village;
    ok = true;
  }
  if ( senderAction->text() == QObject::tr("Свои пункты") ){
    modeTo = meteo::map::proto::Location;
    ok = true;
  }

  if ( ok == false )
    return;

  QList <QTreeWidgetItem*> list = ui_->citiesLeftTreeWidget->selectedItems();

  QMap <int, meteo::map::proto::CitySetting> * cityMap;
  cityMap = dsOperator_->citiesMap();
  for ( int i = 0; i < list.size(); i++ ){
    //QString cityTitle = list.at(i)->text(kCityTitleColumn);
    int cityIndex = list.at(i)->data(kCityMarkColumn, Qt::UserRole).toInt();

    for( int col = 0; col <= ui_->citiesLeftTreeWidget->columnCount(); col++ ) {
      list.at(i)->setBackgroundColor(col, Qt::yellow);
    }
    
    if (cityMap->contains(cityIndex) == false){
      return;
    }
    meteo::map::proto::CitySetting citySetting = cityMap->value(cityIndex);
    citySetting.set_city_mode(modeTo);
    cityMap->insert(cityIndex, citySetting);
    refitCityItems(cityIndex);
  }

  filterList(ui_->leftFilterLineEdit, ui_->citiesLeftTreeWidget, ui_->modeLeftComboBox);
  filterList(ui_->rightFilterLineEdit,  ui_->citiesRightTreeWidget, ui_->modeRightComboBox);

  //cityMapModified = true;
  dsOperator_->setCityMapModified(true);
}

void CityWidget::closeEvent(QCloseEvent *event)
{
  if (dsOperator_->cityMapModified() == true ||
      dsOperator_->displaySettingListModified() == true){
    int res = QMessageBox::question(this, QObject::tr("Настройки населенных пунктов"),
                                    QObject::tr("Сохранить изменения?"), QObject::tr("Сохранить"),
                                    QObject::tr("Не сохранять"));//.exec();
    if (res == 0){
      dsOperator_->flushSettingsToFiles();
    }
  }

  event->accept();
}

void CityWidget::updateModeWidget()
{
  disconnectModeDisplaySettingsWidgets();
  // сейчас функция будет писать в виджеты значения настроек отображения, на это время слот, обрабатывающий изменения параметров, надо отсоединить

  meteo::map::proto::CityMode modeId = static_cast <meteo::map::proto::CityMode> (ui_->modeLeftComboBox->itemData(ui_->modeLeftComboBox->currentIndex(), Qt::UserRole).toInt());

  meteo::map::proto::DisplaySetting modeDS;// = modeDisplaySetting(modeId);

  if ( ui_->modeLeftComboBox->itemData(ui_->modeLeftComboBox->currentIndex(), Qt::UserRole).toInt() == -1 ) {
    modeDS.CopyFrom(dsOperator_->defaultDisplaySetting());
    //обратите внимание! чтобы не писать два варианта, я здесь копирую умолчальные настройки и дальше работаю с modeDS
  }
  else {
    modeDS.CopyFrom(dsOperator_->filledCityModeDisplaySetting(modeId));
  }

  Position pos = modeDS.position();
  Font font = modeDS.font();
  QFont qFont = font2qfont(font);
  Pen pen = modeDS.pen();
  QPen qPen = pen2qpen(pen);
  ui_->fontWidget->setFont(qFont, pos, qPen.color().rgba());
  ui_->prioritySpinBox->setValue(modeDS.priority());
  ui_->cityScaleSpinBox->setValue(modeDS.city_scale());
  ui_->generalizationLevelSpinBox->setValue(modeDS.generalization_level());
  int cityMarkIndex = static_cast <int> (modeDS.city_mark());
  ui_->cityMarkComboBox->setCurrentIndex(cityMarkIndex);

  if ( modeDS.visible() == true ) {
    ui_->visibilityComboBox->setCurrentIndex(0);
    ui_->priorityLabel->show();
    ui_->prioritySpinBox->show();
    ui_->generalizationLevelLabel->show();
    ui_->generalizationLevelSpinBox->show();
    ui_->cityMarkLabel->show();
    ui_->cityMarkComboBox->show();
    ui_->cityScaleLabel->show();
    ui_->cityScaleSpinBox->show();
  }
  else {
    ui_->visibilityComboBox->setCurrentIndex(1);
    ui_->priorityLabel->hide();
    ui_->prioritySpinBox->hide();
    ui_->generalizationLevelLabel->hide();
    ui_->generalizationLevelSpinBox->hide();
    ui_->cityMarkLabel->hide();
    ui_->cityMarkComboBox->hide();
    ui_->cityScaleLabel->hide();
    ui_->cityScaleSpinBox->hide();
  }

  connectModeDisplaySettingsWidgets();
}

void CityWidget::updateModeDisplaySetting()
{
  QRgb qRgb = ui_->fontWidget->color();
  QFont qFont = ui_->fontWidget->font();
  Position pos = static_cast <Position> (ui_->fontWidget->align());
  int modeId = ui_->modeLeftComboBox->itemData(ui_->modeLeftComboBox->currentIndex(), Qt::UserRole).toInt();
  bool visible;
  if (ui_->visibilityComboBox->currentIndex() == 0)
    visible = true;
  else
    visible = false;
  meteo::map::proto::CityMark cityMark = static_cast <meteo::map::proto::CityMark> (ui_->cityMarkComboBox->itemData(ui_->cityMarkComboBox->currentIndex(), Qt::UserRole).toInt());

  if ( modeId == -1 ) {//модифицируем настройки по умолчанию
    meteo::map::proto::DisplaySetting defaultDS;
    defaultDS.set_is_default(true);
    Font * font = defaultDS.mutable_font();
    font->CopyFrom(qfont2font(qFont));
    Pen * pen = defaultDS.mutable_pen();
    pen->set_color(qRgb);
    defaultDS.set_generalization_level(ui_->generalizationLevelSpinBox->value());
    defaultDS.set_position(pos);
    defaultDS.set_priority(ui_->prioritySpinBox->value());
    defaultDS.set_city_scale(ui_->cityScaleSpinBox->value());
    defaultDS.set_visible(visible);
    defaultDS.set_city_mark(cityMark);
    dsOperator_->setDefaultDisplaySetting(defaultDS);
  }
  else {

    meteo::map::proto::DisplaySetting modeDS;
    modeDS.set_city_mode(static_cast <meteo::map::proto::CityMode> (modeId));
    Font * font = modeDS.mutable_font();
    font->CopyFrom(qfont2font(qFont));
    Pen * pen = modeDS.mutable_pen();
    pen->set_color(qRgb);
    modeDS.set_generalization_level(ui_->generalizationLevelSpinBox->value());
    modeDS.set_position(pos);
    modeDS.set_priority(ui_->prioritySpinBox->value());
    modeDS.set_city_scale(ui_->cityScaleSpinBox->value());
    modeDS.set_visible(visible);
    modeDS.set_city_mark(cityMark);

    modeDS.CopyFrom(dsOperator_->differedCityModeDisplaySetting(modeDS));

    if ( isEmpty(modeDS) == true ){ //надо бы удалить пустой набор настроек
      dsOperator_->cityModeDisplaySettingMap()->remove(static_cast <meteo::map::proto::CityMode> (modeId));
    }
    else {
      dsOperator_->cityModeDisplaySettingMap()->insert(static_cast <meteo::map::proto::CityMode> (modeId), modeDS);
    }
  }

  if ( ui_->visibilityComboBox->currentIndex() == 0){
    ui_->priorityLabel->show();
    ui_->prioritySpinBox->show();
    ui_->cityScaleLabel->show();
    ui_->cityScaleSpinBox->show();
    ui_->generalizationLevelLabel->show();
    ui_->generalizationLevelSpinBox->show();
    ui_->cityMarkLabel->show();
    ui_->cityMarkComboBox->show();
  }
  else {
    ui_->priorityLabel->hide();
    ui_->prioritySpinBox->hide();
    ui_->cityScaleLabel->hide();
    ui_->cityScaleSpinBox->hide();
    ui_->generalizationLevelLabel->hide();
    ui_->generalizationLevelSpinBox->hide();
    ui_->cityMarkLabel->hide();
    ui_->cityMarkComboBox->hide();
  }

  dsOperator_->setDisplaySettingListModified(true);
  refitCitiesItems(modeId,  static_cast <QWidget*> (sender()));
}

void CityWidget::toggleTwoPanels()
{
  if ( ui_->toggleTwoPanelsPushButton->isChecked() == true ){
    ui_->modeRightComboBox->show();
    ui_->rightFilterLineEdit->show();
    ui_->clearRightFilterToolButton->show();
    ui_->citiesRightTreeWidget->show();
    ui_->moveLeftToolButton->show();
    ui_->moveRightToolButton->show();
    resizeTreeWidgetsColumns();
  }
  else{
    ui_->modeRightComboBox->hide();
    ui_->rightFilterLineEdit->hide();
    ui_->clearRightFilterToolButton->hide();
    ui_->citiesRightTreeWidget->hide();
    ui_->moveLeftToolButton->hide();
    ui_->moveRightToolButton->hide();
  }
}

void CityWidget::toggleCityVisibility(QTreeWidgetItem *item, int column)
{
  if ( column != kVisibilityColumn  )
    return;

  //QString cityTitle = item->text(kCityTitleColumn);
  int cityIndex = item->data(kCityMarkColumn, Qt::UserRole).toInt();

  meteo::map::proto::DisplaySetting cityDS;
  cityDS.CopyFrom(dsOperator_->filledCityDisplaySetting(cityIndex));

  if ( cityDS.has_visible() == false )
    return; //что-то пошло не так
  //if ( cityDS.has_always_visible() &&
  //cityDS.always_visible() == true)
  //return;//если задано "отображать всегда", то давайте ничего больше не менять, чтоб не путаться

  if ( cityDS.visible() == true ) {
    cityDS.set_visible(false);
    cityDS.clear_always_visible();
  }
  else {
    cityDS.set_visible(true);
  }

  dsOperator_->cityDisplaySettingMap()->insert(cityIndex, cityDS);

  refitCityItems(cityIndex);
  //filterList(ui_->leftFilterLineEdit, ui_->citiesLeftTreeWidget, ui_->modeLeftComboBox);
  //filterList(ui_->rightFilterLineEdit,  ui_->citiesRightTreeWidget, ui_->modeRightComboBox);

  dsOperator_->setDisplaySettingListModified(true);
}

void CityWidget::setSelectedCitiesVisibility()
{
  QList <QTreeWidgetItem*> list = ui_->citiesLeftTreeWidget->selectedItems();
  bool setVisible = false;
  for (int i = 0; i < list.size(); i++){//выясняем, ставить параметр в true или в false
    //QString cityTitle = list.at(i)->text(kCityTitleColumn);
    int cityIndex = list.at(i)->data(kCityMarkColumn, Qt::UserRole).toInt();
    meteo::map::proto::DisplaySetting cityDS;
    cityDS.CopyFrom(dsOperator_->filledCityDisplaySetting(cityIndex));
    //if ( cityDS.has_always_visible() &&
    //cityDS.always_visible() == true )//с такими даже пробовать ничего не будем
    //continue;

    if ( cityDS.has_visible() == false ||
         ( cityDS.has_visible() == true && cityDS.visible() == false ) )
      setVisible = true;
  }


  for (int i = 0; i < list.size(); i++){
    //QString cityTitle = list.at(i)->text(kCityTitleColumn);
    int cityIndex = list.at(i)->data(kCityMarkColumn, Qt::UserRole).toInt();
    meteo::map::proto::DisplaySetting cityDS;
    cityDS.CopyFrom(dsOperator_->filledCityDisplaySetting(cityIndex));
    if ( setVisible == false ) {
      cityDS.set_visible(false);
      cityDS.clear_always_visible();
      list.at(i)->setIcon(0, QIcon(":/meteo/icons/tools/layer_hidden.png"));
    }
    else {
      cityDS.set_visible(true);
      list.at(i)->setIcon(0, QIcon(":/meteo/icons/tools/layer_visible.png"));
    }

    //bool found = false;
    cityDS.CopyFrom(dsOperator_->differedCityDisplaySetting(cityDS));
    if ( isEmpty(cityDS) == false ) {
      dsOperator_->cityDisplaySettingMap()->insert(cityIndex, cityDS);
    }
    else {
      dsOperator_->cityDisplaySettingMap()->remove(cityIndex);
    }
  }
  dsOperator_->setDisplaySettingListModified(true);
}

void CityWidget::toggleAlwaysVisible()
{
  QList <QTreeWidgetItem*> list = ui_->citiesLeftTreeWidget->selectedItems();
  bool alwaysVisible = true;

  for (int i = 0; i < list.size(); i++){//выясняем, ставить параметр в true или в false
    //QString cityTitle = list.at(i)->text(kCityTitleColumn);
    int cityIndex = list.at(i)->data(kCityMarkColumn, Qt::UserRole).toInt();
    meteo::map::proto::DisplaySetting cityDS;
    cityDS.CopyFrom(dsOperator_->filledCityDisplaySetting(cityIndex));
    if ( cityDS.has_always_visible() == false )
      alwaysVisible = false;
  }
  if ( alwaysVisible == true ) //надо инвертировать так, иначе условие проверки будет сложнее
    alwaysVisible = false;
  else
    alwaysVisible = true;

  for (int i = 0; i < list.size(); i++){
    //QString cityTitle = list.at(i)->text(kCityTitleColumn);
    int cityIndex = list.at(i)->data(kCityMarkColumn, Qt::UserRole).toInt();
    meteo::map::proto::DisplaySetting cityDS;
    cityDS.CopyFrom(dsOperator_->filledCityDisplaySetting(cityIndex));
    if ( alwaysVisible == false ) {
      cityDS.clear_always_visible();//очищаем, тут либо true, либо нет ничего
      if ( cityDS.has_visible() ){
        if ( cityDS.visible() == true )
          list.at(i)->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible.png"));
        else
          list.at(i)->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_hidden.png"));
      }
    }
    else {
      if ( cityDS.visible() == true ) {
        cityDS.set_always_visible(true);
        list.at(i)->setIcon(kVisibilityColumn, QIcon(":/meteo/icons/tools/layer_visible_locked.png"));
      }
    }


    cityDS.CopyFrom(dsOperator_->differedCityDisplaySetting(cityDS));
    if ( isEmpty(cityDS) == false ) {
      dsOperator_->cityDisplaySettingMap()->insert(cityIndex, cityDS);
    }
    else {
      dsOperator_->cityDisplaySettingMap()->remove(cityIndex);
    }
  }
  dsOperator_->setDisplaySettingListModified(true);
}

void CityWidget::slotEditFinished(int res)
{
  if ( QDialog::Accepted != res ) { return; }

  DisplaySettingsEditDialog* dialog = qobject_cast<DisplaySettingsEditDialog*>(sender());
  if ( 0 == dialog ) {
    return;
  }

  QString cityName = dialog->property("city_name").toString();
  int cityIndex = dialog->property("city_index").toInt();

  QMap<int,meteo::map::proto::CitySetting>* cityMap = dsOperator_->citiesMap();
  meteo::map::proto::CitySetting citySetting = cityMap->value(cityIndex);

  meteo::map::proto::CitySetting returnedCitySetting;
  returnedCitySetting.CopyFrom(dialog->getCitySetting());
  meteo::map::proto::DisplaySetting returnedDisplaySetting;
  returnedDisplaySetting.CopyFrom(dialog->getDisplaySetting());

  QPair <QTreeWidgetItem*, QTreeWidgetItem*> pair = cityItemMap_.value(cityIndex);
  QString newCityName = QString::fromStdString(returnedCitySetting.title());
  if ( newCityName != cityName ) {
    citySetting.set_title(newCityName.toStdString().c_str());
    cityMap->insert(cityIndex, citySetting);
    
    dsOperator_->setCityMapModified(true);

    pair.first->setText(kCityTitleColumn, newCityName);
    pair.second->setText(kCityTitleColumn, newCityName);
  }
  
  for( int i = 0; i <= ui_->citiesLeftTreeWidget->columnCount(); i++ ) {
    pair.first->setBackgroundColor(i, Qt::yellow);
  }

  if ( isEqual(citySetting.geopoint(), returnedCitySetting.geopoint()) == false ){
    citySetting.mutable_geopoint()->CopyFrom(returnedCitySetting.geopoint());
    cityMap->insert(cityIndex, citySetting);//предыдущее значение citySetting в cityMap заменится новым
    dsOperator_->setCityMapModified(true);
  }

  returnedDisplaySetting.CopyFrom(dsOperator_->differedCityDisplaySetting(returnedDisplaySetting));

  if ( isEmpty(returnedDisplaySetting) == false ){
    dsOperator_->cityDisplaySettingMap()->insert(cityIndex, returnedDisplaySetting);
  }
  else {
    dsOperator_->cityDisplaySettingMap()->remove(cityIndex);
  }

  dsOperator_->setDisplaySettingListModified(true);
  refitCityItems(cityIndex);
  filterList(ui_->leftFilterLineEdit, ui_->citiesLeftTreeWidget, ui_->modeLeftComboBox);
  filterList(ui_->rightFilterLineEdit,  ui_->citiesRightTreeWidget, ui_->modeRightComboBox);
}

void CityWidget::slotAddFinished(int res)
{
  if ( QDialog::Accepted != res ) { return; }
  DisplaySettingsEditDialog* dialog = qobject_cast<DisplaySettingsEditDialog*>(sender());
  if ( 0 == dialog ) {
    return;
  }

  QMap <int, meteo::map::proto::CitySetting> * cityMap = dsOperator_->citiesMap();
  meteo::map::proto::CitySetting citySetting = cityMap->value(0);

  QString cityName = dialog->property("city_name").toString();
  int cityIndex = dialog->property("city_index").toInt();
  
  meteo::map::proto::CitySetting returnedCitySetting;
  returnedCitySetting.CopyFrom(dialog->getCitySetting());
  meteo::map::proto::DisplaySetting returnedDisplaySetting;
  returnedDisplaySetting.CopyFrom(dialog->getDisplaySetting());

  //  QString cityName = QString::fromStdString(citySetting.title());
  QString newCityName = QString::fromStdString(returnedCitySetting.title());

  //  returnedDisplaySetting.set_title(CityName.toStdString().c_str());

  if ( newCityName != cityName )
  {
    citySetting.set_title(newCityName.toStdString().c_str());
    cityMap->insert(cityIndex, citySetting);

    returnedCitySetting.set_title(newCityName.toStdString().c_str());
    //          QString cityTitle = QString::fromStdString(newCityName);

    QTreeWidgetItem *leftTItem = new QTreeWidgetItem;
    //          QTreeWidgetItem *rightTItem = new QTreeWidgetItem;

    leftTItem->setText(kCityTitleColumn, newCityName);
    //          if (mapRecord.value().has_city_mode())
    //              leftTItem->setData(kCityTitleColumn, Qt::UserRole, QVariant(mapRecord.value().city_mode()));
    //          else
    leftTItem->setData(kCityTitleColumn, Qt::UserRole, QVariant(-1));
    leftTItem->setData(kCityMarkColumn, Qt::UserRole, cityIndex);

    //    ui_->citiesLeftTreeWidget->addTopLevelItem(leftTItem);
    ui_->citiesLeftTreeWidget->insertTopLevelItem(0, leftTItem);
    for( int i = 0; i <= ui_->citiesLeftTreeWidget->columnCount(); i++ ) {
      leftTItem->setBackgroundColor(i, Qt::yellow);
    }

    QTreeWidgetItem * rightTItem = leftTItem->clone();
    ui_->citiesRightTreeWidget->addTopLevelItem(rightTItem);
    cityItemMap_.insert(cityIndex, QPair <QTreeWidgetItem*, QTreeWidgetItem*> (leftTItem, rightTItem));

    dsOperator_->setCityMapModified(true);

    if ( isEqual(citySetting.geopoint(), returnedCitySetting.geopoint()) == false ){
      citySetting.mutable_geopoint()->CopyFrom(returnedCitySetting.geopoint());
      cityMap->insert(cityIndex, citySetting);//предыдущее значение citySetting в cityMap заменится новым
      dsOperator_->setCityMapModified(true);
    }

    returnedDisplaySetting.CopyFrom(dsOperator_->differedCityDisplaySetting(returnedDisplaySetting));

    if ( isEmpty(returnedDisplaySetting) == false ){
      dsOperator_->cityDisplaySettingMap()->insert(cityIndex, returnedDisplaySetting);
    }
    else {
      dsOperator_->cityDisplaySettingMap()->remove(cityIndex);
    }

    dsOperator_->setDisplaySettingListModified(true);
    refitCityItems(cityIndex);

    filterList(ui_->leftFilterLineEdit, ui_->citiesLeftTreeWidget, ui_->modeLeftComboBox);
    filterList(ui_->rightFilterLineEdit,  ui_->citiesRightTreeWidget, ui_->modeRightComboBox);
  }
}

CityWidget::CityWidget( app::MainWindow* parent) :
  QWidget(parent),
  ui_(new Ui::CityWidget)
{
  ui_->setupUi(this);
  init();
  initMenu();
}

CityWidget::~CityWidget()
{
  delete contextMenu_;
  delete dsOperator_;

  delete ui_;
  ui_ = 0;
}

}
}
