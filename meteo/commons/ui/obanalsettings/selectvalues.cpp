#include "selectvalues.h"
#include "ui_selectvalues.h"

#include <QApplication>
#include <QString>
#include <QTreeWidgetItemIterator>

namespace meteo {

SelectValuesWidget::SelectValuesWidget(QWidget* parent) :
  QDialog(parent),
  ui_(new Ui::SelectValuesWidget())
{
  ui_->setupUi(this);

  connect(ui_->applyButton, SIGNAL(clicked()), SLOT(accept()));
  connect(ui_->cancelButton, SIGNAL(clicked()), SLOT(reject()));

  connect(ui_->allLineEdit, SIGNAL(textChanged(const QString&)), SLOT(slotSearch(const QString&)));
  connect(ui_->selectedLineEdit, SIGNAL(textChanged(const QString&)), SLOT(slotSearch(const QString&)));

  connect(ui_->allButton, SIGNAL(clicked()), SLOT(slotSelectAll()));
  connect(ui_->selectedButton, SIGNAL(clicked()), SLOT(slotSelectAll()));

  connect(ui_->includeButton, SIGNAL(clicked()), SLOT(slotMoveToSelected()));
  connect(ui_->excludeButton, SIGNAL(clicked()), SLOT(slotMoveToAll()));
}

SelectValuesWidget::~SelectValuesWidget()
{
  delete ui_;
  ui_ = 0;
}

void SelectValuesWidget::init()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  selectedValues_.clear();
  availableValues_ = loadAllValuesList();

  ui_->selectedTreeWidget->clear();
  ui_->allTreeWidget->clear();

  installValuesList(ui_->allTreeWidget, availableValues_);
  QApplication::restoreOverrideCursor();
}

void SelectValuesWidget::init(const QString& values)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QStringList all = loadAllValuesList();
  resetValuesList(ui_->allTreeWidget, all);
  resetValuesList(ui_->selectedTreeWidget, all);

  if (values == QString::fromUtf8("Все")) {
    availableValues_.clear();
    selectedValues_ = all;
    installValuesList(ui_->selectedTreeWidget, selectedValues_);
  }
  else {
    selectedValues_ = loadValuesList(values);
    availableValues_ = all;
    for (int i = 0, sz = selectedValues_.size(); i < sz; ++i) {
      availableValues_.removeAll(selectedValues_[i]);
    }
    installValuesList(ui_->selectedTreeWidget, selectedValues_);
    installValuesList(ui_->allTreeWidget, availableValues_);
  }
  QApplication::restoreOverrideCursor();
}

void SelectValuesWidget::resetValuesList(QTreeWidget* target, const QStringList& allvalues)
{
  Q_CHECK_PTR(target);

  target->clear();

  typedef QStringList::const_iterator Iter;
  for (Iter it = allvalues.constBegin(), end = allvalues.constEnd(); it != end; ++it) {
    QTreeWidgetItem* item = new QTreeWidgetItem(target);
    item->setText(0, *it);
    item->setHidden(true);
    target->addTopLevelItem(item);
  }
}

void SelectValuesWidget::installValuesList(QTreeWidget* target, const QStringList& values)
{
  Q_CHECK_PTR(target);

  for (int i = 0, sz = target->topLevelItemCount(); i < sz; ++i) {
    if (values.contains(target->topLevelItem(i)->text(0)) == true) {
      target->topLevelItem(i)->setHidden(false);
    }
  }
}

QString SelectValuesWidget::valuesToString() const
{
  return selectedValues_.isEmpty() ? QString() : getValues();
}

const QStringList& SelectValuesWidget::availableValues() const
{
  return availableValues_;
}

const QStringList& SelectValuesWidget::selectedValues() const
{
  return selectedValues_;
}

void SelectValuesWidget::slotSearch(const QString& str)
{
  QTreeWidget* target = 0;
  if(sender() == ui_->allLineEdit) {
    target = ui_->allTreeWidget;
  }
  else if (sender() == ui_->selectedLineEdit) {
    target = ui_->selectedTreeWidget;
  }
  else {
    return;
  }

  if (str.isEmpty() == true) {
    QTreeWidgetItemIterator it(target);
    while (*it != 0) {
      (*it)->setHidden(false);
      ++it;
    }
  }
  else {
    QList<QTreeWidgetItem*> founded = target->findItems(str, Qt::MatchContains);
    QTreeWidgetItemIterator it(target);
    while (*it != 0) {
      (*it)->setHidden(!founded.contains(*it));
      ++it;
    }
  }
}

void SelectValuesWidget::slotMoveToAll()
{
  if (ui_->selectedTreeWidget->selectionModel()->hasSelection() == false) {
    return;
  }

  foreach (QTreeWidgetItem* movedItem, ui_->selectedTreeWidget->selectedItems()) {
    movedItem->setHidden(true);
    availableValues_.append(selectedValues_.takeAt(selectedValues_.indexOf(movedItem->text(0))));
    foreach (QTreeWidgetItem* targetItem, ui_->allTreeWidget->findItems(movedItem->text(0), Qt::MatchFixedString)) {
      targetItem->setHidden(false);
    }
  }
}

void SelectValuesWidget::slotMoveToSelected()
{
  if (ui_->allTreeWidget->selectionModel()->hasSelection() == false) {
    return;
  }

  foreach (QTreeWidgetItem* movedItem, ui_->allTreeWidget->selectedItems()) {
    movedItem->setHidden(true);
    selectedValues_.append(availableValues_.takeAt(availableValues_.indexOf(movedItem->text(0))));
    foreach (QTreeWidgetItem* targetItem, ui_->selectedTreeWidget->findItems(movedItem->text(0), Qt::MatchFixedString)) {
      targetItem->setHidden(false);
    }
  }
}

void SelectValuesWidget::slotSelectAll()
{
  QTreeWidget* target = 0;
  if (sender() == ui_->allButton) {
    target = ui_->allTreeWidget;
  }
  else if (sender() == ui_->selectedButton) {
    target = ui_->selectedTreeWidget;
  }
  else {
    return;
  }

  target->selectAll();
}

} // meteo
