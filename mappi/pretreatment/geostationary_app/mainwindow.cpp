#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimeZone>
#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <sql/dbi/dbi.h>
#include <sql/dbi/dbientry.h>
#include <sql/psql/psql.h>
#include <sql/psql/psqlquery.h>
#include <meteo/commons/global/global.h>
#include <mappi/global/global.h>
#include <meteo/commons/settings/settings.h>
#include <commons/textproto/tprototext.h>
#include <mappi/proto/reception.pb.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

  meteo::gSettings(meteo::global::Settings::instance());
  if ( !meteo::global::Settings::instance()->load() ) {
    QMessageBox::warning(this, "Ошибка", QString("Не удалось загрузить настройки."));
  }
  ui->setupUi(this);
  ui->treeWidget->setColumnWidth(0, 20);

  ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->dateEdit->setDate(QDate::currentDate());

  connect(ui->dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(slotTypeChanged()));
  connect(ui->treeWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(slotShowMapInfo()));
  connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(slotShowMapInfo()));

  db_ = std::unique_ptr<meteo::Dbi>(meteo::global::dbMappi());
  if ( nullptr == db_.get() ) {
    QMessageBox::warning(this, "Ошибка", QString("Нет подключения к базе данных"));
  }else{
    loadGeoSats();
    slotTypeChanged();
  }
}

MainWindow::~MainWindow() {
  delete ui;
}

bool MainWindow::loadGeoSats(){
  mappi::conf::Reception conf;
  QFile file(RECEPTION_FILE);
  if (!file.open(QIODevice::ReadOnly))  return false;

  QString text = QString::fromUtf8(file.readAll());
  file.close();

  if (!TProtoText::fillProto(text, &conf)) return false;
  for (int idx = 0; idx < conf.satellite_size(); idx++) {
    if(conf.satellite(idx).type() == mappi::conf::kGeostatSat){
      QString satName = QString::fromStdString(conf.satellite(idx).name());
      geoSats_ << satName;
    }
  }
  return true;
}

bool MainWindow::loadSessions() {
  ui->treeWidget->clear();
  ui->treeWidget->setHeaderLabels({"Прибор", "Спутник", "Канал", "Время", "Длительность", "Файл"});
  ui->treeWidget->hideColumn(5);
  ui->treeWidget->header()->resizeSection(0, 100);
  ui->treeWidget->header()->resizeSection(1, 100);
  ui->treeWidget->header()->resizeSection(2, 50);
  ui->treeWidget->header()->resizeSection(3, 150);
  ui->treeWidget->header()->resizeSection(4, 100);

  QString queryString = "SELECT satellite, instrument_name, channel_number, channel_alias, date_start, date_end, fpath FROM meteo.pretreatment_view WHERE ";
  if(geoSats_.size() > 0){
    queryString += "satellite IN ('" + geoSats_.join("','") + "') AND ";
  }

  queryString += "date_start > '";
  QDateTime dt = ui->dateEdit->dateTime();
  dt.setTimeZone(QTimeZone::systemTimeZone());
  QTime time(0, 0, 0);
  dt.setTime(time);
  queryString += dt.toUTC().toString(Qt::ISODate);
  queryString += "' AND date_end < '";
  time.setHMS(23, 59, 59);
  dt.setTime(time);
  queryString += dt.toUTC().toString(Qt::ISODate);
  queryString += "'";

  auto query = db_->queryptr(queryString);
  if(nullptr == query) {
    QMessageBox::warning(this, "Ошибка", QString("Нет query"));
    return false;
  }
  if ( false == query->exec() ) {
    QMessageBox::warning(this, "Ошибка", QString("Не удалось выполнить запрос"));
    return false;
  }
  if ( false == query->initIterator() ) {
    QMessageBox::warning(this, "Ошибка", QString("ERR ITER"));
    return false;
  }
  while ( true == query->next() ) {
    const meteo::DbiEntry& doc = query->entry();
    QString fPath = doc.valueString("fpath");
    fPath.replace(".po", ".png");
    if(!QFile::exists(fPath)) continue;

    QDateTime dts = QDateTime::fromString(doc.valueString("date_start"), Qt::ISODate);
    QDateTime dte = QDateTime::fromString(doc.valueString("date_end"), Qt::ISODate);
    dts.setTimeSpec(Qt::UTC);
    dte.setTimeSpec(Qt::UTC);
    QTime time = dts.toLocalTime().time();
    QString timeString = time.toString(Qt::ISODate);
    int secs = dts.secsTo(dte);
    QString lengthString = QDateTime::fromTime_t(secs).toUTC().toString("mm:ss");

    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, doc.valueString("instrument_name"));
    item->setText(1, doc.valueString("satellite"));
    item->setText(2, QString::number(doc.valueInt32("channel_number")));
    item->setText(3, timeString);
    item->setText(4, lengthString);
    item->setText(5, fPath);
    ui->treeWidget->addTopLevelItem(item);
  }

  return true;
}

void MainWindow::slotTypeChanged(){
  loadSessions();
  ui->treeWidget->clearSelection();
}

void MainWindow::slotShowMapInfo(){
  QTreeWidgetItem* item = ui->treeWidget->currentItem();
  QString fPath = item->text(5);


  QImage img;
  if(!img.load(fPath)){
    QMessageBox::warning(this, "Ошибка", QString("Невозможно загрузить изображение: %1").arg(fPath));
    return;
  }
  if(img.width() / ui->picture->width() > img.height() / ui->picture->height()){
    img = img.scaledToWidth(ui->picture->width(), Qt::SmoothTransformation);
  }else{
    img = img.scaledToHeight(ui->picture->height(), Qt::SmoothTransformation);
  }
  ui->picture->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  QPixmap pixmap = QPixmap::fromImage(img);
  ui->picture->setPixmap(pixmap);
}