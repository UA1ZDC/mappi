#include "dataloader.h"
#include "ui_dataloader.h"
#include "obanalstatus.h"
#include "loaderstatus.h"
#include "documentstatus.h"
#include "climatstatus.h"
#include "loader.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/msgparser/tlgparser.h>
#include <meteo/commons/proto/msgparser.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/msgparser/msgmetainfo.h>

#include <qdir.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <qmessagebox.h>
#include <qevent.h>
#include <qtablewidget.h>
#include <qfile.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qmap.h>
#include <qprocess.h>


namespace meteo {

const int kChunkSize = 64*16;
const int kParamCount = 3;
const int kColumnParam = 0;
const int kColumnCount = 1;
const int kColumnList = 2;
const int kRowDirs = 0;
const int kRowFiles = 1;
const int kRowSended = 2;

DataLoader::DataLoader(QWidget* parent) :
  QWidget(parent),
  ui_(new Ui::DataLoader),
  settings_(new QSettings(QDir::homePath() + "/." + MnCommon::applicationName() + "/dataloader.ini", QSettings::IniFormat)),
  dir_(new QDir()),
  dirsBox_(new QComboBox()),
  filesBox_(new QComboBox()),
  obanalAeroProc_(new QProcess(this)),
  obanalOceanProc_(new QProcess(this)),
  obanalSurfProc_(new QProcess(this)),
  obanalGribProc_(new QProcess(this)),
  documentProc_(new QProcess(this)),
  climatProc_(new QProcess(this)),
  obanalStatus_(new ObanalStatus()),
  loaderStatus_(new LoaderStatus()),
  documentStatus_(new DocumentStatus()),
  climatStatus_(new ClimatStatus()),
  loader_(new Loader()),
  thread_(new QThread())
{
  if ( nullptr == gSettings() ){
    auto settings = global::Settings::instance();
    gSettings(settings);
    settings->load();
  }
  ui_->setupUi(this);
  ui_->pathBtn->setIcon(QIcon(":/meteo/icons/file_open.png"));
  ui_->pathBtn->setIconSize(QSize(32,32));
  ui_->yearBox->setValue(QDateTime::currentDateTime().date().year());
  ui_->monthBox->setCurrentIndex(QDateTime::currentDateTime().date().month()-1);
  ui_->dayBox->setValue(QDateTime::currentDateTime().date().day());
  dirsBox_->setToolTip(QObject::tr("Обработанные директории"));
  filesBox_->setToolTip(QObject::tr("Обработанные файлы"));
  auto dt = QDateTime::currentDateTimeUtc();
  dt.setTime(QTime(dt.time().hour(),dt.time().minute(),0));
  ui_->dteEdit->setDateTime(dt);
  ui_->dtsEdit->setDateTime(dt);
  ui_->dteDocEdit->setDateTime(dt);
  ui_->dtsDocEdit->setDateTime(dt);
  ui_->dteClimatEdit->setDateTime(dt);
  ui_->dtsClimatEdit->setDateTime(dt);
  loaderStatus_->setWindowTitle("Загрузка");
  ui_->tableBtn->setIcon(QIcon(":/meteo/icons/arrow/darkarrowdown.png"));
  ui_->toolTipImg->setPixmap(QIcon(":/meteo/icons/help.png").pixmap(20,20));
  ui_->toolTipImg->setToolTip(" Для извлечения информации из имени файла\n"
                              " можно указать следующие шаблоны:\n"
                              "{YYYY}, {YY}, {MM}, {DD},\n"/* {J},*/
                              "{hh}, {mm}, {T1}, {T2}, {A1},\n"
                              "{A2}, {CCCC}, {ii}, {YYGGgg}");
  loadSettings();
  initStatTable();
  loader_->moveToThread(thread_);
  QObject::connect(ui_->dayBtn, SIGNAL(toggled(bool)), SLOT(slotSwitchDayBtn()));
  QObject::connect(ui_->pathBtn, SIGNAL(clicked(bool)), SLOT(slotOpenDirDialog()));
  QObject::connect(ui_->loadBtn, SIGNAL(clicked(bool)), SLOT(slotLoadMsg()));
  QObject::connect(ui_->tableBtn, SIGNAL(clicked(bool)), SLOT(slotSwitchTableBox()));
  QObject::connect(ui_->closeBtn, SIGNAL(clicked(bool)), SLOT(slotClose()));
  QObject::connect(ui_->obanalBtn, SIGNAL(clicked(bool)), SLOT(slotRunObanal()));
  QObject::connect(ui_->docBtn, SIGNAL(clicked(bool)), SLOT(slotRunDocument()));
  QObject::connect(ui_->climatBtn, SIGNAL(clicked(bool)), SLOT(slotRunClimat()));

  QObject::connect(obanalStatus_, SIGNAL(stopObanal()), SLOT(slotStopObanal()) );
  QObject::connect(obanalAeroProc_, SIGNAL(finished(int)), obanalStatus_, SLOT(slotAeroDone()));
  QObject::connect(obanalOceanProc_, SIGNAL(finished(int)), obanalStatus_, SLOT(slotOceanDone()));
  QObject::connect(obanalSurfProc_, SIGNAL(finished(int)), obanalStatus_, SLOT(slotSurfDone()));
  QObject::connect(obanalGribProc_, SIGNAL(finished(int)), obanalStatus_, SLOT(slotGribDone()));
  QObject::connect(obanalAeroProc_, SIGNAL(readyReadStandardOutput()), SLOT(slotReadProcOutput()));
  QObject::connect(obanalAeroProc_, SIGNAL(readyReadStandardError()), SLOT(slotReadProcError()));
  QObject::connect(obanalOceanProc_, SIGNAL(readyReadStandardOutput()), SLOT(slotReadProcOutput()));
  QObject::connect(obanalOceanProc_, SIGNAL(readyReadStandardError()), SLOT(slotReadProcError()));
  QObject::connect(obanalSurfProc_, SIGNAL(readyReadStandardOutput()), SLOT(slotReadProcOutput()));
  QObject::connect(obanalSurfProc_, SIGNAL(readyReadStandardError()), SLOT(slotReadProcError()));
  QObject::connect(obanalGribProc_, SIGNAL(readyReadStandardOutput()), SLOT(slotReadProcOutput()));
  QObject::connect(obanalGribProc_, SIGNAL(readyReadStandardError()), SLOT(slotReadProcError()));

  QObject::connect(documentStatus_, SIGNAL(stopDocument()), SLOT(slotStopDocument()));
  QObject::connect(documentProc_, SIGNAL(finished(int)), documentStatus_, SLOT(slotDocumentDone()));
  QObject::connect(documentProc_, SIGNAL(readyReadStandardOutput()), SLOT(slotReadProcOutput()) );
  QObject::connect(documentProc_, SIGNAL(readyReadStandardError()), SLOT(slotReadProcError()) );

  QObject::connect(climatStatus_, SIGNAL(stopClimat()), SLOT(slotStopClimat()));
  QObject::connect(climatProc_, SIGNAL(finished(int)), climatStatus_, SLOT(slotClimatDone()));
  QObject::connect(climatProc_, SIGNAL(readyReadStandardOutput()), SLOT(slotReadProcOutput()) );
  QObject::connect(climatProc_, SIGNAL(readyReadStandardError()), SLOT(slotReadProcError()) );

  QObject::connect(loader_, SIGNAL(signalAddDirs(QString)), SLOT(slotAddDirs(QString)));
  QObject::connect(loader_, SIGNAL(signalAddFiles(QString)), SLOT(slotAddFiles(QString)));
  QObject::connect(loader_, SIGNAL(signalTableItemIncrement(int)), SLOT(slotTableItemIncrement(int)));
  QObject::connect(loader_, SIGNAL(signalUpdateDT(QDateTime,QDateTime)), SLOT(slotUpdateDT(QDateTime, QDateTime)));
  QObject::connect(loader_, SIGNAL(signalFileIncrement()), loaderStatus_, SLOT(slotIncrementValue()));
  QObject::connect(loaderStatus_, SIGNAL(signalStop()), loader_, SLOT(slotStop()));
  QObject::connect(this, SIGNAL(signalRunLoadMsg()), loader_, SLOT(slotRun()));
  thread_->start();
}

DataLoader::~DataLoader()
{
  delete ui_;
  ui_ = nullptr;
  delete settings_;
  settings_ = nullptr;
  delete filesBox_;
  filesBox_ = nullptr;
  delete dirsBox_;
  dirsBox_ = nullptr;
  delete settings_;
  settings_ = nullptr;
  delete dir_;
  dir_ = nullptr;
  delete obanalAeroProc_;
  obanalAeroProc_ = nullptr;
  delete obanalOceanProc_;
  obanalOceanProc_ = nullptr;
  delete obanalSurfProc_;
  obanalSurfProc_ = nullptr;
  delete obanalGribProc_;
  obanalGribProc_ = nullptr;
  delete documentProc_;
  documentProc_ = nullptr;
  delete obanalStatus_;
  obanalStatus_ = nullptr;
  delete loaderStatus_;
  loaderStatus_ = nullptr;
  delete documentStatus_;
  documentStatus_ = nullptr;
  delete loader_;
  loader_ = nullptr;
  thread_->quit();
  thread_->deleteLater();
  thread_ = nullptr;
}

void DataLoader::slotOpenDirDialog()
{
  QFileDialog dlg;
  dlg.setOption(QFileDialog::Option::ShowDirsOnly);
  dlg.setFileMode(QFileDialog::FileMode::DirectoryOnly);
  if (true == dir_->exists()) {
    dlg.setDirectory(*dir_);
  }
  else {
    dlg.setDirectory(QDir::home());
  }
  if (true == dlg.exec() ) {
//    dir_->setPath(dlg.directory().path());
    dir_->setPath(dlg.selectedFiles().value(0)); // ASTRA fix
    ui_->pathLabel->setText(dir_->path());
  }
}

void DataLoader::slotLoadMsg()
{
  setMaskFileList();
  saveSettings();
  clearStatTable();
  clearDTs();
  if ( false == dir_->exists() ) {
    error_log.msgBox() << "Выбранная директория не существует";
    QApplication::restoreOverrideCursor();
    return;
  }
  countFiles_ = 0;
  recusiveCountFile(*dir_);
  loaderStatus_->slotSetValue(0);
  loaderStatus_->slotSetMaxValue(countFiles_);

  loader_->slotSetDir(*dir_);
  loader_->slotSetYMD(ui_->yearBox->value(),ui_->monthBox->currentIndex()+1,
                    ( ui_->dayBox->isEnabled() ? ui_->dayBox->value() : 0) );
  loader_->slotSetTemplList(templList_);
  loader_->slotSetMaskFileList(maskFileList_);
  emit signalRunLoadMsg();
  loaderStatus_->exec();
  checkObanal();
}

void DataLoader::slotSwitchTableBox()
{
  if (true == ui_->tableBox->isVisible()) {
    ui_->tableBox->hide();
  }
  else {
    ui_->tableBox->show();
  }
  if (true == ui_->tableBox->isVisible()) {
    resize(ui_->groupBox->size()+ui_->tableBox->size());
  }
  else {
    resize(ui_->groupBox->size());
  }
  setTableBtn(ui_->tableBox->isVisible());
  adjustSize();
}

void DataLoader::slotSwitchDayBtn()
{
  if ( true == ui_->dayBtn->isChecked() ) {
    ui_->dayBox->setEnabled(true);
  }
  else {
    ui_->dayBox->setEnabled(false);
  }
}

void DataLoader::slotClose()
{
  QMessageBox dlg;
  dlg.setWindowTitle(QObject::tr("Подтвреждение"));
  dlg.setText(QObject::tr("Выйти?"));
  dlg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  dlg.setDefaultButton(QMessageBox::Yes);
  int ret = dlg.exec();
  if ( ret == QMessageBox::Yes) {
    saveSettings();
    close();
  }
}

void DataLoader::slotRunObanal()
{
  aeroRunObanal();
  surfRunObanal();
  oceanRunObanal();
  gribRunObanal();
  showStatus();
  checkDocument();
}

void DataLoader::slotRunDocument()
{
  if ( nullptr == documentStatus_ ) {
    documentStatus_ = new DocumentStatus();
    QObject::connect(documentStatus_, SIGNAL(stopDocument()), SLOT(slotStopDocument()));
  }
  QString docPath = QString( MnCommon::binPath()+"meteo.map.client "
                             + " --begin " + ui_->dtsDocEdit->dateTime().toString(Qt::ISODate)
                             + " --end " + ui_->dteDocEdit->dateTime().toString(Qt::ISODate));
  documentStatus_->slotSetNotDone();
  documentProc_->start( docPath );
  documentProc_->waitForStarted();
  QPoint center = geometry().center();
  QRect swGeom = documentStatus_->geometry();
  center.setX(center.x()-swGeom.width()/2);
  center.setY(center.y()-swGeom.height()/2);
  documentStatus_->move(this->pos());
  documentStatus_->move(center);
  documentStatus_->exec();
  checkClimat();
}

void DataLoader::slotRunClimat()
{
  if ( nullptr == climatStatus_ ) {
    climatStatus_ = new ClimatStatus();
    QObject::connect(climatStatus_, SIGNAL(stopClimat()), SLOT(slotStopClimat()));

  }
  QString climatPath = QString( MnCommon::binPath()+"maslo.climatsaver "
                             + " -s " + ui_->dtsClimatEdit->dateTime().toString(Qt::ISODate)
                             + " -e " + ui_->dteClimatEdit->dateTime().toString(Qt::ISODate));
  climatStatus_->slotSetNotDone();
  climatProc_->start( climatPath );
  climatProc_->waitForStarted();
  QPoint center = geometry().center();
  QRect swGeom = climatStatus_->geometry();
  center.setX(center.x()-swGeom.width()/2);
  center.setY(center.y()-swGeom.height()/2);
  climatStatus_->move(this->pos());
  climatStatus_->move(center);
  climatStatus_->exec();
}

void DataLoader::slotReadProcOutput()
{
  QProcess* proc = qobject_cast<QProcess*>( sender() );
  if (nullptr != proc) {
   info_log << QString::fromLocal8Bit( proc->readAllStandardOutput());
  }
}

void DataLoader::slotReadProcError()
{
  QProcess* proc = qobject_cast<QProcess*>( sender() );
  if (nullptr != proc) {
   info_log << QString::fromLocal8Bit( proc->readAllStandardError());
  }
}

void DataLoader::slotStopObanal()
{
  obanalAeroProc_->kill();
  obanalOceanProc_->kill();
  obanalSurfProc_->kill();
  obanalGribProc_->kill();
}

void DataLoader::slotStopDocument()
{
  documentProc_->kill();
}

void DataLoader::slotStopClimat()
{
  climatProc_->kill();
}

void DataLoader::slotAddDirs(QString name )
{
  dirsBox_->addItem(name);
}

void DataLoader::slotAddFiles(QString name)
{
  filesBox_->addItem(name);
}

void DataLoader::slotTableItemIncrement(int row)
{
  tableItemIncrement(row);
}

void DataLoader::slotUpdateDT(QDateTime dts, QDateTime dte)
{
  dts_ = dts;
  dte_ = dte;
  updateTextDts();
}

void DataLoader::slotDebug()
{
  debug_log << "DEBUG DEBUG DEBUG DEBUG DEBUG";
}

void DataLoader::keyReleaseEvent(QKeyEvent* event)
{
  if ( nullptr == event ) {
    return;
  }
  switch (event->key()) {
  case Qt::Key::Key_Enter:
  case Qt::Key_Return:
    slotLoadMsg();
    break;
  case Qt::Key_Escape:
    slotClose();
    break;
  default:
    break;
  }
}

void DataLoader::loadSettings()
{
  if ( true == settings_->contains("dirname") ) {
    dir_->setPath(settings_->value("dirname").toString());
    ui_->pathLabel->setText(settings_->value("dirname").toString());
  }
  else {
    ui_->pathLabel->setText("Выбирете директорию с телеграммами");
    dir_->setPath("");
  }
  if ( true == settings_->contains("filenametempl") ) {
    ui_->fileNameTempl->setText(settings_->value("filenametempl").toString() );
  }
  else {
    ui_->fileNameTempl->clear();
  }
  if ( true == settings_->contains("tablevisible") ) {
    bool visible = settings_->value("tablevisible").toBool();
    ui_->tableBox->setVisible(visible);
    setTableBtn(visible);
  }
  else {
    ui_->tableBox->setVisible(false);
  }
  if ( true == settings_->contains("daybtn") ) {
    ui_->dayBtn->setChecked(settings_->value("daybtn").toBool());
  }
  else {
    ui_->dayBtn->setChecked(true);
  }
  slotSwitchDayBtn();
  if ( true == settings_->contains("geometry") ) {
    restoreGeometry( settings_->value("geometry").toByteArray() );
  }
  else {
    this->resize(this->sizeHint());
  }
}

void DataLoader::saveSettings()
{
  settings_->setValue("dirname", dir_->path());
  settings_->setValue("filenametempl", ui_->fileNameTempl->text());
  settings_->setValue("tablevisible", ui_->tableBox->isVisible());
  settings_->setValue("daybtn", ui_->dayBtn->isChecked());
  settings_->setValue("geometry", saveGeometry());
}

void DataLoader::setTableBtn( bool show )
{
  if ( true == show ) {
    ui_->tableBtn->setToolTip(QObject::tr("Скрыть статистику"));
    ui_->tableBtn->setIcon(QIcon(":/meteo/icons/arrow/darkarrowup.png"));
    ui_->tableBtn->setIconSize(QSize(32,32));
  }
  else {
    ui_->tableBtn->setToolTip(QObject::tr("Показать статистику"));
    ui_->tableBtn->setIcon(QIcon(":/meteo/icons/arrow/darkarrowdown.png"));
    ui_->tableBtn->setIconSize(QSize(32,32));
  }
}

void DataLoader::initStatTable()
{
  ui_->statTable->setRowCount(kParamCount);
  QTableWidgetItem* item = new QTableWidgetItem();
  item->setText("Директории");
  ui_->statTable->setItem(kRowDirs, kColumnParam, item);
  item = new QTableWidgetItem();
  item->setData(Qt::UserRole,0);
  QString  text = QString::number( item->data(Qt::UserRole).toInt() );
  item->setText(text);
  ui_->statTable->setItem(kRowDirs, kColumnCount, item);
  dirsBox_->clear();
  ui_->statTable->setCellWidget(kRowDirs, kColumnList, dirsBox_);
  item = new QTableWidgetItem();
  item->setText("Файлы");
  ui_->statTable->setItem(kRowFiles, kColumnParam, item);
  item = new QTableWidgetItem();
  item->setData(Qt::UserRole,0);
  text = QString::number( item->data(Qt::UserRole).toInt() );
  item->setText(text);
  ui_->statTable->setItem(kRowFiles, kColumnCount, item);
  filesBox_->clear();
  ui_->statTable->setCellWidget(kRowFiles, kColumnList, filesBox_);
  item = new QTableWidgetItem();
  item->setText("Отправлено");
  ui_->statTable->setItem(kRowSended, kColumnParam, item);
  item = new QTableWidgetItem();
  item->setData(Qt::UserRole,0);
  text = QString::number( item->data(Qt::UserRole).toInt() );
  item->setText(text);
  ui_->statTable->setItem(kRowSended, kColumnCount, item);
}

void DataLoader::clearStatTable()
{
  dirsBox_->clear();
  filesBox_->clear();

  QTableWidgetItem* item;
  for (int i = 0; i < kParamCount; ++i) {
    item = ui_->statTable->item(i,kColumnCount);
    item->setData(Qt::UserRole,0);
    QString  text = QString::number( item->data(Qt::UserRole).toInt() );
    item->setText(text);
  }
}

void DataLoader::tableItemIncrement(int row)
{
  QTableWidgetItem* item = ui_->statTable->item(row,kColumnCount);
  int data = item->data(Qt::UserRole).toInt();
  ++data;
  item->setData(Qt::UserRole,data);
  QString  text = QString::number( item->data(Qt::UserRole).toInt() );
//  if (row == kRowSended) {
//    debug_log << QObject::tr("Телеграмм переданно ") << data;
//  }
  item->setText(text);
}

void DataLoader::setMaskFileList()
{
  maskFileList_.clear();
  templList_.clear();
  if ( true == ui_->fileNameTempl->text().isEmpty() ) {
    return;
  }
  maskFileList_ = ui_->fileNameTempl->text().simplified().replace(" ","").split(",");
  for (int i =0, size = maskFileList_.size(); i < size; ++i  ) {
    QString& mask = maskFileList_[i];
    foreach ( QString filter, meteo::kLens.keys() ) {
      if ( true == mask.contains(filter) ) {
        templList_.append(mask);
        foreach ( QString key, meteo::kLens.keys() ) {
          mask.replace(key, QString().fill('*', meteo::kLens.value(key)));
        }
        break;
      }
    }
  }
}

bool DataLoader::checkFileName( QString& name)
{
  if ( true == maskFileList_.isEmpty() ) {
    return  true;
  }
  bool result = false;
  QRegExp exp;
  foreach (QString templ, maskFileList_) {
    exp.setPattern(templ);
    exp.setPatternSyntax(QRegExp::Wildcard);
    result = result || exp.exactMatch(name);
  }
  return result;
}

QDate DataLoader::makeDateFromUi()
{
  QDate dt;
  dt.setDate( ui_->yearBox->value(),ui_->monthBox->currentIndex()+1,
             ( ui_->dayBox->isEnabled() ? ui_->dayBox->value() : 0) );
  return dt;
}

void DataLoader::clearDTs()
{
  dts_.setDate(QDate(0,0,0));
  dts_.setTime(QTime(0,0));
  dte_.setDate(QDate(0,0,0));
  dte_.setTime(QTime(0,0));
}

void DataLoader::updateTextDts()
{
  ui_->dtsEdit->setDateTime(dts_);
  ui_->dteEdit->setDateTime(dte_);
  ui_->dtsDocEdit->setDateTime(dts_);
  ui_->dteDocEdit->setDateTime(dte_);
  ui_->dtsClimatEdit->setDateTime(dts_);
  ui_->dteClimatEdit->setDateTime(dte_);
}

void DataLoader::checkObanal()
{
  if ( true == ui_->obanalCheck->isChecked()) {
    slotRunObanal();
  }
}

void DataLoader::checkDocument()
{
  if ( true == ui_->docCheck->isChecked()) {
    slotRunDocument();
  }
}

void DataLoader::checkClimat()
{
  if ( true == ui_->climatCheck->isChecked()) {
    slotRunClimat();
  }
}

void DataLoader::aeroRunObanal()
{
  QString argAeroPath = QString( ::MnCommon::binPath()+"maslo.obanal "
                             + " --aero "
                             + " --begin " + ui_->dtsEdit->dateTime().toString(Qt::ISODate)
                             + " --end " + ui_->dteEdit->dateTime().toString(Qt::ISODate));
  obanalAeroProc_->start( argAeroPath );
  obanalAeroProc_->waitForStarted();
}

void DataLoader::oceanRunObanal()
{
  QString argOceanPath = QString( ::MnCommon::binPath()+"maslo.obanal"
                             + " --ocean "
                             + " --begin " + ui_->dtsEdit->dateTime().toString(Qt::ISODate)
                             + " --end " + ui_->dteEdit->dateTime().toString(Qt::ISODate));
  obanalOceanProc_->start( argOceanPath );
  obanalOceanProc_->waitForStarted();
}

void DataLoader::surfRunObanal()
{
  QString argSurfPath = QString( ::MnCommon::binPath()+"maslo.obanal"
                             + " --surface "
                             + " --begin " + ui_->dtsEdit->dateTime().toString(Qt::ISODate)
                             + " --end " + ui_->dteEdit->dateTime().toString(Qt::ISODate));
  obanalSurfProc_->start( argSurfPath );
  obanalSurfProc_->waitForStarted();
}

void DataLoader::gribRunObanal()
{
  QString argGribPath = QString( ::MnCommon::binPath()+"maslo.obanal"
                             + " --grib ");
  obanalGribProc_->start( argGribPath );
  obanalGribProc_->waitForStarted();
}

void DataLoader::showStatus()
{
  if (nullptr == obanalStatus_) {
    obanalStatus_ = new ObanalStatus();
  }
  obanalStatus_->slotSetAllNotDone();
  obanalStatus_->move(this->pos());
  QPoint center = geometry().center();
  QRect swGeom = obanalStatus_->geometry();
  center.setX(center.x()-swGeom.width()/2);
  center.setY(center.y()-swGeom.height()/2);
  obanalStatus_->move(center);
  obanalStatus_->exec();
}

void DataLoader::recusiveCountFile(const QDir &directory)
{
  QStringList files = directory.entryList(QDir::Filter::Files);
  countFiles_ += directory.entryList(QDir::Filter::Files).count();
  QStringList dirs = directory.entryList(QDir::Filter::Dirs | QDir::NoDotAndDotDot);
  foreach (QString dir, dirs) {
    QDir d;
    d.setPath(directory.absolutePath()+"/"+dir);
    recusiveCountFile(d);
  }
}

}
