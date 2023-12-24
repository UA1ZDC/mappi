#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace mappi::Import;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
  ui->setupUi(this);
  ui->cmbFormat->addItem("RGBA(Цветной с прозрачностью)", 0);
  ui->cmbFormat->addItem("RGB(Цветной)", 1);
  ui->cmbFormat->addItem("Grayscale(Градации серого)", 2);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::updatePicture() {
  bool fileIsOpened = ui->textEdit->toPlainText() != "";
  ui->cmbRed->setEnabled(fileIsOpened);
  ui->cmbGreen->setEnabled(fileIsOpened);
  ui->cmbBlue->setEnabled(fileIsOpened);
  ui->cmbAlpha->setEnabled(fileIsOpened);
  ui->cmbFormat->setEnabled(fileIsOpened);
  ui->btnSave->setEnabled(fileIsOpened);

  if(!fileIsOpened) return;
  QPixmap pixmap = QPixmap::fromImage(importer.getImage());
  ui->pic->setPixmap(pixmap);
}

void MainWindow::on_btnOpen_clicked() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Открыть файл"), QDir::homePath(), tr("Файлы изображений (*.*)"));
  if (filePath == "") return;
  if (!importer.load(filePath)){
    QMessageBox::warning(this, "Ошибка", QString("Невозможно получить данные из файла %1").arg(filePath));
    return;
  }
  if (importer.getBandsCount() < 1){
    QMessageBox::warning(this, "Ошибка", QString("В файле %1 нет растровых слоев").arg(filePath));
    return;
  }

  ui->numBands->setText(QString("%1").arg(importer.getBandsCount()));
  ui->cmbRed->clear();
  ui->cmbGreen->clear();
  ui->cmbBlue->clear();
  ui->cmbAlpha->clear();

  ui->cmbAlpha->addItem("Непрозрачный", 0);
  for(unsigned int i=1; i<=importer.getBandsCount(); i++){
    ui->cmbRed->addItem(QString("Канал %1").arg(i), i);
    ui->cmbGreen->addItem(QString("Канал %1").arg(i), i);
    ui->cmbBlue->addItem(QString("Канал %1").arg(i), i);
    ui->cmbAlpha->addItem(QString("Канал %1").arg(i), i);
  }

  ui->textEdit->setText(filePath);

  ui->cmbRed->setCurrentIndex(importer.getChannelFor(ColorChannel::RED) - 1);
  ui->cmbGreen->setCurrentIndex(importer.getChannelFor(ColorChannel::GREEN) - 1);
  ui->cmbBlue->setCurrentIndex(importer.getChannelFor(ColorChannel::BLUE) - 1);
  ui->cmbAlpha->setCurrentIndex(importer.getChannelFor(ColorChannel::ALPHA));
  updatePicture();
}

void MainWindow::on_btnSave_clicked() {
  QString filePath = QFileDialog::getSaveFileName(this, tr("Выберите путь сохранения файла"), QDir::homePath()+"/build_dir/var/meteo/thematics", tr("PNG Image (*.png);;TIFF Image (*.tiff)"));
  if(filePath == "") return;
  if(!(filePath.endsWith(".png") || filePath.endsWith(".tiff"))) filePath += ".tiff";

  if (!importer.saveImage(filePath)) {
    QMessageBox::warning(this, "Ошибка", QString("Ошибка сохранения файла: %1").arg(filePath));
    return;
  }
  if (!importer.createSession(filePath)) {
    QMessageBox::warning(this, "Ошибка", QString("Ошибка сохранения файла: %1").arg(filePath));
    return;
  }
  if (!importer.saveData(filePath)) {
    QMessageBox::warning(this, "Ошибка", QString("Ошибка записи в БД: %1").arg(filePath));
    return;
  }
  QMessageBox::information(this, "Готово", QString("Файл сохранен: %1").arg(filePath));
}

void MainWindow::currentIndexChanged(ColorChannel color, int currentIndex){
  if(ui->textEdit->toPlainText() == "") return;
  if(currentIndex < 0) return;
  if(color != ColorChannel::ALPHA) currentIndex++;

  importer.setBand(color, currentIndex);
  updatePicture();
}
void MainWindow::on_cmbRed_currentIndexChanged(int currentIndex){
  currentIndexChanged(ColorChannel::RED, currentIndex);
}

void MainWindow::on_cmbGreen_currentIndexChanged(int currentIndex){
  currentIndexChanged(ColorChannel::GREEN, currentIndex);
}

void MainWindow::on_cmbBlue_currentIndexChanged(int currentIndex){
  currentIndexChanged(ColorChannel::BLUE, currentIndex);
}

void MainWindow::on_cmbAlpha_currentIndexChanged(int currentIndex){
  currentIndexChanged(ColorChannel::ALPHA, currentIndex);
}

void MainWindow::on_cmbFormat_currentIndexChanged(int currentIndex){
  if(ui->textEdit->toPlainText() == "") return;
  if(currentIndex < 0) return;
  importer.setFormat(static_cast<SaveAs>(currentIndex));
}
