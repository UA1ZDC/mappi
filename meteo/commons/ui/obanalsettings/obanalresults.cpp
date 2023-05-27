#include "obanalresults.h"
#include "ui_obanalresults.h"

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/proto/field.pb.h>
//#include <meteo/commons/rpc/selfchecked/selfcheckedcontroller.h>
#include <meteo/commons/settings/settings.h>
#include <meteo/commons/global/dateformat.h>
#include <commons/obanal/obanal_struct.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <QApplication>
#include <QCursor>
#include <QDateTime>
#include <QList>
#include <QTableWidgetItem>
#include <QTableWidget>

namespace {

int requestTimeout() { return 30000; }

}

namespace meteo {

ObanalResults::ObanalResults(QWidget* parent) :
  QWidget(parent),
  ui_(new Ui::ObanalResultsWidget())
{
  ui_->setupUi(this);
  ui_->startDateTimeEdit->setDisplayFormat(meteo::dtHumanFormatDateOnly);
  ui_->endDateTimeEdit->setDisplayFormat(meteo::dtHumanFormatDateOnly);

  ui_->dataTableWidget->setStyleSheet("QTableView::item { height: 36px; }");
  ui_->dataTableWidget->setColumnCount(MAX_COLUMN_NUMBER);
  ui_->dataTableWidget->horizontalHeader()->setStretchLastSection(true);
  ui_->dataTableWidget->setSelectionBehavior(QTableWidget::SelectRows);


  QDateTime cdt = QDateTime( QDate::currentDate(), QTime(23,59,59) );
  ui_->endDateTimeEdit->setDateTime(cdt);
  ui_->startDateTimeEdit->setDateTime( QDateTime( cdt.date(), QTime(0,0,0) ) );

  connect(ui_->updateButton, SIGNAL(clicked()), SLOT(slotUpdate()));

  connect(ui_->startDateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)), SLOT(slotUpdate()));
  connect(ui_->endDateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)), SLOT(slotUpdate()));
}

ObanalResults::~ObanalResults()
{
  delete ui_;
  ui_ = 0;
}

void ObanalResults::showEvent( QShowEvent* e )
{
  QWidget::showEvent(e);
  slotUpdate();
}

void ObanalResults::slotUpdate()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

  QList<field::DataDesc> descDatas;

  //settings::TMeteoSettings::instance()->load();
  rpc::Channel* ctrl = meteo::global::serviceChannel(meteo::settings::proto::kField);
  if ( nullptr == ctrl ) {
    error_log.msgBox() << meteo::msglog::kNoConnect.arg(meteo::global::serviceTitle(meteo::settings::proto::kField));
    return;
  }
  field::DataRequest req;
  req.set_date_start(ui_->startDateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
  req.set_date_end(ui_->endDateTimeEdit->dateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());

  field::DataDescResponse* res = ctrl->remoteCall(&field::FieldService::GetAvailableData, req, ::requestTimeout());
  if (res != nullptr) {
    for (int i = 0, sz = res->descr_size(); i < sz; ++i) {
      descDatas.append(res->descr(i));
    }
    ui_->countLineEdit->setText(QString::number(res->descr_size()));
    debug_log << QString("result: [%1] %2").arg((res->result() ? "True" : "False")).arg(QString::fromStdString(res->comment()));
  }
  delete res;

  renderDatas(descDatas);

  QApplication::restoreOverrideCursor();
}

void ObanalResults::renderDatas(const QList<field::DataDesc>& datas)
{
  ui_->dataTableWidget->clearContents();
  ui_->dataTableWidget->setRowCount(datas.size());

  for (int row = 0, sz = datas.size(); row < sz; ++row) {
    const field::DataDesc& each = datas.at(row);
    QDateTime dt = QDateTime::fromString(QString::fromStdString(each.date()), Qt::ISODate);
    auto item = new QTableWidgetItem(meteo::dateToHumanTimeShort(dt));
    item->setFlags(item->flags() & ~ Qt::ItemIsEditable);
    ui_->dataTableWidget->setItem(row, DATE, item);
    auto descrname = TMeteoDescriptor::instance()->name(each.meteodescr());
    auto name =  TMeteoDescriptor::instance()->description(descrname);
    if ( name.isEmpty()) {
      if ( !descrname.isEmpty() ) {
        name = descrname;
      }
      else {
        name = QString::number(each.meteodescr());
      }
    }
    item = new QTableWidgetItem(name);
    item->setFlags(item->flags() & ~ Qt::ItemIsEditable);
    ui_->dataTableWidget->setItem(row, DESCR, item);
    item = new QTableWidgetItem(QString::number(each.hour()/3600));
    item->setFlags(item->flags() & ~ Qt::ItemIsEditable);
    ui_->dataTableWidget->setItem(row, HOUR, item);
    item = new QTableWidgetItem(QString::number(each.level()));
    item->setFlags(item->flags() & ~ Qt::ItemIsEditable);
    ui_->dataTableWidget->setItem(row, LEVEL, item);
    item = new QTableWidgetItem(QString::fromStdString(each.center_name()));
    item->setFlags(item->flags() & ~ Qt::ItemIsEditable);
    ui_->dataTableWidget->setItem(row, CENTER, item);
    item = new QTableWidgetItem(obanal::netTypetoStr(::NetType(each.net_type())));
    item->setFlags(item->flags() & ~ Qt::ItemIsEditable);
    ui_->dataTableWidget->setItem(row, TYPE_NET, item);
    item = new QTableWidgetItem(QString::fromStdString(each.level_type_name()));
    item->setFlags(item->flags() & ~ Qt::ItemIsEditable);
    ui_->dataTableWidget->setItem(row, TYPE_LEVEL, item);
    item = new QTableWidgetItem(QString::number(each.count_point()));
    item->setFlags(item->flags() & ~ Qt::ItemIsEditable);
    ui_->dataTableWidget->setItem(row, COUNT_POINT, item);
  }

  ui_->dataTableWidget->resizeColumnsToContents();
}

} // meteo
