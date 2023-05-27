#include "maptoolwidget.h"
#include "ui_maptoolwidget.h"


namespace meteo {

namespace map {

MapToolWidget::MapToolWidget(QWidget* parent /*=*/, Qt::WindowFlags f /*=*/) :
    QWidget(parent, f),
  ui_(new Ui::MapToolWidget)
{
  ui_->setupUi(this);

  ui_->tb_mapOpen->setIcon(QIcon(":/meteo/icons/map.png"));
  ui_->tb_mapOpen->setToolTip(tr("Открыть карту"));
  QObject::connect(ui_->tb_mapOpen, SIGNAL(clicked()), SIGNAL(mapOpen()));

  ui_->tb_docOpen->setIcon(QIcon(":/meteo/icons/jobs/doc.png"));
  ui_->tb_docOpen->setToolTip(tr("Открыть документ"));
  QObject::connect(ui_->tb_docOpen, SIGNAL(clicked()), SIGNAL(docOpen()));

  ui_->tb_docDownload->setIcon(QIcon(":/meteo/icons/jobs/download.png"));
  ui_->tb_docDownload->setToolTip(tr("Скачать последний сформированный документ"));
  QObject::connect(ui_->tb_docDownload, SIGNAL(clicked()), SIGNAL(docDownload()));

  ui_->tb_runJob->setIcon(QIcon(":/meteo/icons/run_proc.png"));
  ui_->tb_runJob->setToolTip(tr("Запустить задание"));
  QObject::connect(ui_->tb_runJob, SIGNAL(clicked()), SIGNAL(runJob()));




}

MapToolWidget::~MapToolWidget()
{
  delete ui_;
  ui_ = nullptr;
}

}

}
