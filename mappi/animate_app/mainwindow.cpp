#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "animate.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
  ui->setupUi(this);
  ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Задержка (мс)"));
  ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Путь к файлу"));
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::move(bool up) {
  Q_ASSERT(ui->tableWidget->selectedItems().count() > 0);
  const int sourceRow = ui->tableWidget->row(ui->tableWidget->selectedItems().at(0));
  const int destRow = (up ? sourceRow - 1 : sourceRow + 1);
  Q_ASSERT(destRow >= 0 && destRow < ui->tableWidget->rowCount());

  // take whole rows
  QList < QTableWidgetItem * > sourceItems = takeRow(sourceRow);
  QList < QTableWidgetItem * > destItems = takeRow(destRow);

  // set back in reverse order
  setRow(sourceRow, destItems);
  setRow(destRow, sourceItems);

  ui->tableWidget->selectRow(destRow);
}

// takes and returns the whole row
QList<QTableWidgetItem *> MainWindow::takeRow(int row) {
  QList < QTableWidgetItem * > rowItems;
  for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
    rowItems << ui->tableWidget->takeItem(row, col);
  }
  return rowItems;
}

// sets the whole row
void MainWindow::setRow(int row, const QList<QTableWidgetItem *> &rowItems) {
  for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
    ui->tableWidget->setItem(row, col, rowItems.at(col));
  }
}

void MainWindow::updateButtons() {
  bool hasItems = ui->tableWidget->rowCount() > 0;
  if (!hasItems) {
    ui->btnUp->setEnabled(false);
    ui->btnDown->setEnabled(false);
    ui->btnDelete->setEnabled(false);
    ui->label->clear();
    return;
  }

  int selectedRow = ui->tableWidget->currentRow();
  if (selectedRow < 0) {
    ui->btnUp->setEnabled(false);
    ui->btnDown->setEnabled(false);
    ui->btnDelete->setEnabled(false);
    ui->label->clear();
    return;
  }

  ui->btnDelete->setEnabled(true);
  ui->btnUp->setEnabled(selectedRow != 0);
  ui->btnDown->setEnabled(selectedRow != ui->tableWidget->rowCount() - 1);
}

void MainWindow::on_btnAdd_clicked() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Открыть файл"), "/home", tr("Файлы изображений (*.*)"));
  if (filePath == "") return;
  QImageReader reader(filePath);
  if (!reader.canRead()) {
    QMessageBox::warning(this, "Ошибка", QString("Файл %1 не является изображением").arg(filePath));
    return;
  }

  int rowCount = ui->tableWidget->rowCount();
  ui->tableWidget->insertRow(rowCount);
  ui->tableWidget->setItem(rowCount, 0, new QTableWidgetItem("1000"));
  ui->tableWidget->setItem(rowCount, 1, new QTableWidgetItem(filePath));
  ui->tableWidget->selectRow(rowCount);
  updateButtons();
}

void MainWindow::on_btnUp_clicked() {
  move(true);
  updateButtons();
}

void MainWindow::on_btnDown_clicked() {
  move(false);
  updateButtons();
}

void MainWindow::on_btnDelete_clicked() {
  ui->tableWidget->removeRow(ui->tableWidget->currentRow());
  updateButtons();
}

void MainWindow::on_actionSave_triggered() {
  if (ui->tableWidget->rowCount() < 2) {
    QMessageBox::warning(this, "Ошибка", "Недостаточно кадров для создания GIF");
    return;
  }
  QString filePath = QFileDialog::getSaveFileName(this, tr("Выберите путь сохранения файла"), "/home",
                                                  tr("GIF Image (*.gif)"));
  mappi::Animate::GifBuilder builder(filePath);

  for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
    int delay = ui->tableWidget->item(row, 0)->text().toInt();
    QString imagePath = ui->tableWidget->item(row, 1)->text();
    if (!builder.addFrame(filePath, delay)) {
      QMessageBox::warning(this, "Предупреждение", QString("Пропущен файл: %1").arg(imagePath));
    }
  }
  builder.writeGif();
  QMessageBox::information(this, "Готово", QString("Файл сохранен: %1").arg(filePath));
}

void MainWindow::on_actionAdd_triggered() {
  return on_btnAdd_clicked();
}

void MainWindow::on_actionClear_triggered() {
  ui->tableWidget->clearContents();
  ui->label->clear();
  updateButtons();
}

void MainWindow::on_tableWidget_itemSelectionChanged() {
  int selectedRow = ui->tableWidget->currentRow();
  if(selectedRow >= 0) {
    QString imagePath = ui->tableWidget->item(selectedRow, 1)->text();
    QPixmap pixmap = QPixmap::fromImage(QImage(imagePath));
    ui->label->setPixmap(pixmap);
  }
  updateButtons();
}
