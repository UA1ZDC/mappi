#include "journalleftpanel.h"
#include "ui_journalleftpanel.h"

#include <meteo/commons/global/global.h>

namespace meteo {
namespace journal {

enum JournalLeftPanelColumns {
  kCheckBoxColumn = 0,
  kNameColumn
};

JournalLeftPanel::JournalLeftPanel(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::JournalLeftPanel)
{
  ui->setupUi(this);

  auto names = meteo::global::moduleNames();

  for ( auto name: names.values().toSet() ){
    auto item = new QTreeWidgetItem();    
    item->setCheckState(kCheckBoxColumn, Qt::CheckState::Unchecked);
    item->setData(kNameColumn, Qt::DisplayRole, name);
    item->setData(kNameColumn, Qt::UserRole, name);

    this->ui->senderList->addTopLevelItem(item);
  }

  this->ui->senderList->resizeColumnToContents(kCheckBoxColumn);

  QObject::connect(this->ui->senderList, &QTreeWidget::itemChanged,
                   this, &JournalLeftPanel::slotColumnChecked);
}

JournalLeftPanel::~JournalLeftPanel()
{
  delete ui;
}

void JournalLeftPanel::slotColumnChecked()
{
  emit signalFilterChanged();
}

QStringList JournalLeftPanel::selectedSources()
{
  auto list = this->ui->senderList;
  auto rowCount = list->topLevelItemCount();
  QStringList checkList;
  for ( int i = 0; i < rowCount; ++i ){
    auto item = list->topLevelItem(i);
    auto state = item->checkState(kCheckBoxColumn);
    switch(state){
    case Qt::CheckState::Checked:{
      checkList << item->data(kNameColumn, Qt::DisplayRole).toString();
      break;
    }
    default:{
      break;
    }
    }
  }
  return checkList;
}

void JournalLeftPanel::unsetAllCheckboxes()
{
  auto list = this->ui->senderList;
  auto rowCount = list->topLevelItemCount();
  QStringList checkList;
  this->blockSignals(true);
  bool filtersChanged = false;
  for ( int i = 0; i < rowCount; ++i ){
    auto item = list->topLevelItem(i);
    auto state = item->checkState(kCheckBoxColumn);
    if ( state != Qt::Unchecked ) {
      item->setCheckState(kCheckBoxColumn, Qt::Unchecked);
      filtersChanged = true;
    }
  }
  this->blockSignals(false);
  if ( true == filtersChanged ){
    emit signalFilterChanged();
  }
}

void JournalLeftPanel::setList(const QStringList& checkList)
{
  auto list = this->ui->senderList;
  auto rowCount = list->topLevelItemCount();

  this->blockSignals(true);
  bool itemsChanged = false;
  for ( int i = 0; i < rowCount; ++i ){
    auto item = list->topLevelItem(i);
    auto name = item->data(kNameColumn, Qt::DisplayRole).toString();
    auto state =  true == checkList.contains(name) ? Qt::Checked:
                                                     Qt::Unchecked;
    auto oldState = item->checkState(kCheckBoxColumn);
    if ( state != oldState ){
      item->setCheckState(kCheckBoxColumn, state);
      itemsChanged = true;
    }
  }
  this->blockSignals(false);
  if ( true == itemsChanged ){
    emit signalFilterChanged();
  }
}

}
}
