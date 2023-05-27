#include "procmonitorwidget.h"
#include "ui_procmonitorwidget.h"

#include <qdir.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qscrollbar.h>

#include <cross-commons/debug/tlog.h>

namespace meteo {

ProcMonitorWidget::ProcMonitorWidget(QWidget *parent) :
  QWidget(parent),
  ui_(new Ui::ProcMonitorWidget)
{
  ui_->setupUi(this);
  setObjectName("ProcParam");

  running_ = false;

  icoState_[app::OperationState_NONE] = QIcon(":/meteo/appclient/icons/ledgrey.png");
  icoState_[app::OperationState_NORM] = QIcon(":/meteo/appclient/icons/ledgreen");
  icoState_[app::OperationState_WARN] = QIcon(":/meteo/appclient/icons/ledyellow.png");
  icoState_[app::OperationState_ERROR] = QIcon(":/meteo/appclient/icons/ledred.png");

  new TextHightlighter(ui_->logbrowser->document());

  settings_ = new QSettings(QDir::homePath() + "/.meteo/" + this->objectName() + ".ini", QSettings::IniFormat);

  connect( ui_->startBtn, SIGNAL(clicked()), SIGNAL(start()) );
  connect( ui_->stopBtn, SIGNAL(clicked()), SIGNAL(stop()) );
  connect( ui_->tabs, SIGNAL(currentChanged(int)), this, SIGNAL(currentTabChange(int)));
  connect( ui_->tabs, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentTabChange(int)));
  connect( ui_->linesSpin, SIGNAL(editingFinished()), SLOT(updateSizeScrollback()));
  connect( ui_->linesSpin, SIGNAL(editingFinished()), SLOT(slotEditingFinished()));

  connect( ui_->clearBtn, SIGNAL(clicked()), ui_->logbrowser, SLOT(clear()) );
  connect( ui_->zoomInBtn,  SIGNAL(clicked()), SLOT(slotZoomIn()) );
  connect( ui_->zoomOutBtn, SIGNAL(clicked()), SLOT(slotZoomOut()) );

  ui_->paramTree->header()->setStretchLastSection( true );

  if( 0 == ui_->tabs->currentIndex() ){
    slotCurrentTabChange(0);
  }
  restoreState();
}

ProcMonitorWidget::~ProcMonitorWidget()
{
}

void ProcMonitorWidget::update(const app::AppState::Proc& proc)
{
  running_ = (proc.state() == app::AppState_ProcState_PROC_RUNNING);

  int needItems = proc.status().param_size() + 2; // +2 - start/stop time

  // добавляем элементы если их мало
  for ( int i=ui_->paramTree->topLevelItemCount(); i<needItems; ++i ) {
    QTreeWidgetItem* item  = new QTreeWidgetItem(ui_->paramTree);
    item->setData(0, kRoleParamState, -1);
  }

  // удаляем элементы если их много
  // удаляем, только если процесс выполняется, что при остановке не терять контролируемые параметры
  for ( int count = ui_->paramTree->topLevelItemCount(); count > needItems && running_; --count ) {
    delete ui_->paramTree->topLevelItem(0);
  }

  if ( proc.has_startdt() ) {
    QTreeWidgetItem* item  = ui_->paramTree->topLevelItem(0);
    setStateIcon( item, app::OperationState_NORM );
    item->setText( 0, QObject::tr("Время запуска") );
    item->setText( 1, QString::fromUtf8(proc.startdt().c_str()) );
    item->setText( 2, QString::fromUtf8(proc.startdt().c_str()) );
  }
  if ( proc.has_stopdt() ) {
    QTreeWidgetItem* item  = ui_->paramTree->topLevelItem(ui_->paramTree->topLevelItemCount() - 1);
    setStateIcon( item, app::OperationState_ERROR );
    item->setText( 0, QObject::tr("Время остановки") );
    item->setText( 1, QString::fromUtf8(proc.stopdt().c_str()) );
    item->setText( 2, QString::fromUtf8(proc.stopdt().c_str()) );
  }

  for ( int i = 0, isz = proc.status().param_size(); i < isz; ++i ) {
    const app::OperationParam& param = proc.status().param(i);
    QTreeWidgetItem* item  = ui_->paramTree->topLevelItem(i + 1);
    setStateIcon( item, param.state() );
    item->setText(0, QString::fromUtf8(param.title().c_str()));
    item->setText(1, QString::fromUtf8(param.value().c_str()));
    item->setText(2, QString::fromUtf8(param.dt().c_str()));
  }

  ui_->startBtn->setEnabled(!running_);
  ui_->stopBtn->setEnabled(running_);

  emit stateUpdate(proc);
}

void ProcMonitorWidget::setCurrentTab(int index)
{
  ui_->tabs->setCurrentIndex(index);
}

void ProcMonitorWidget::setScrollBackSize(int value)
{
  ui_->logbrowser->document()->setMaximumBlockCount(value);
  ui_->linesSpin->setValue(value);
}

void ProcMonitorWidget::setLogFontSize(int size)
{
  QFont f = ui_->logbrowser->font();
  f.setPointSize(size);
  ui_->logbrowser->setFont(f);
}

void ProcMonitorWidget::logReceived(const app::AppOutReply& res)
{
  QScrollBar* sb = ui_->logbrowser->verticalScrollBar();
  bool needAutoScroll = ( sb->value() == sb->maximum() );
  int v = sb->value();

  for ( int i = 0, sz = res.logstring_size(); i < sz; ++i ) {
    ui_->logbrowser->appendPlainText( QString::fromLocal8Bit(res.logstring(i).c_str(), res.logstring(i).size() - 1) );
  }

  if ( needAutoScroll ) { sb->setValue(sb->maximum()); } else { sb->setValue(v); }
}

void ProcMonitorWidget::slotAdjustParamColumns()
{
  for ( int j = 0, jsz = ui_->paramTree->columnCount(); j < jsz; ++j ) {
    ui_->paramTree->resizeColumnToContents(j);
  }
}

void ProcMonitorWidget::slotZoomIn()
{
  QFont f = ui_->logbrowser->font();
  f.setPointSize(f.pointSize() + 1);
  ui_->logbrowser->setFont(f);
}

void ProcMonitorWidget::slotZoomOut()
{
  QFont f = ui_->logbrowser->font();
  f.setPointSize(qMax(1, f.pointSize() - 1));
  ui_->logbrowser->setFont(f);
}

void ProcMonitorWidget::restoreState()
{
  int size = settings_->value(objectName()+".state").toInt();
  if( 0 == size ){
    size = 250;
  }
  ui_->linesSpin->setValue(size);
  ui_->logbrowser->document()->setMaximumBlockCount(size);
}

void ProcMonitorWidget::setStateIcon(QTreeWidgetItem* item, app::OperationState state) const
{
  if ( item->data(0,kRoleParamState).toInt() != state ) {
    item->setIcon(0, icoState_[state]);
    item->setData(0, kRoleParamState, int(state));
  }
}

void ProcMonitorWidget::slotCurrentTabChange(int index)
{
  if ( 0 == index ) {
    ui_->logToolsLayout->hide();
    slotAdjustParamColumns();
  }
  if ( 1 == index ) {
    ui_->logToolsLayout->show();
  }
}

void ProcMonitorWidget::updateSizeScrollback()
{
  ui_->logbrowser->document()->setMaximumBlockCount(ui_->linesSpin->value());
  slotSaveState();
}

void ProcMonitorWidget::slotSaveState()
{
  settings_->setValue(objectName()+".state", ui_->linesSpin->value());
}

void ProcMonitorWidget::slotEditingFinished()
{
  emit scrollbackChange(ui_->linesSpin->value());
}

} // meteo
