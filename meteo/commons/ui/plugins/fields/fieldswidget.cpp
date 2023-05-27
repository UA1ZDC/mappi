#include "fieldswidget.h"
#include <meteo/commons/ui/mainwindow/widgethandler.h>

namespace meteo {
namespace map {

namespace{
  QWidget* createDialog( QWidget* parent, const QString& options = 0 )
  {
    Q_UNUSED(options);
    FieldsWidget* dlg = new FieldsWidget( parent );
  //  dlg->initWidget();
    dlg->show();
    return dlg;
  }

  bool registerDialog()
  {
    mapauto::WidgetHandler* hndl = WidgetHandler::instance();
    return hndl->registerWidget( "fieldswidget", createDialog );
  }

static bool res = registerDialog();
}

FieldsWidget::FieldsWidget(QWidget *parent) :
CoordWidget(parent)
{

  initWidget();

}

void FieldsWidget::initWidget(){
  layout_ = new QHBoxLayout(this);
  layout_->setSpacing(0);
  layout_->setContentsMargins(1,1,1,11);
  table_ = new QTableWidget; 
  table_->setColumnCount(kColumnCount);
  // setHorizontalHeaderLabels(QStringList()<<"Величина"<<"Значение");
  table_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  table_->horizontalHeader()->hide();
  table_->verticalHeader()->hide();
  table_->setShowGrid(false);
  table_->setSelectionMode(QAbstractItemView::NoSelection);
  table_->setFrameShape(QFrame::NoFrame);
  layout_->addWidget(table_);
  layout_->addWidget(lock_);
  layout_->addWidget(close_);
}


FieldsWidget::~FieldsWidget()
{
}

void FieldsWidget::setText(const QList<valLabel>& text){
  table_->setColumnCount(kColumnCount);
  for(int j=0;j<table_->columnCount();++j ){
    table_->setColumnWidth(j,0);  
    table_->setColumnHidden(j,true);  
  }
  
  table_->setRowCount(0);
  vlist_ = text;
  for(int i=0;i< text.count();++i){
    table_->insertRow(table_->rowCount());
    
    if(!text.at(i).date.isNull()&&!text.at(i).date.isEmpty()){
      table_->setColumnHidden(kDate,false);
      table_->setItem(i,kDate,new QTableWidgetItem(text.at(i).date));
      table_->item(i,kDate)->setFlags ( Qt::ItemIsEnabled );
    }
    if(!text.at(i).center.isNull()&&!text.at(i).center.isEmpty()){
      table_->setColumnHidden(kCenter,false);
      table_->setItem(i,kCenter,new QTableWidgetItem(text.at(i).center));
      table_->item(i,kCenter)->setFlags ( Qt::ItemIsEnabled );
    }
    if(!text.at(i).name.isNull()&&!text.at(i).name.isEmpty()){
      table_->setColumnHidden(kName,false);
      table_->setItem(i,kName,new QTableWidgetItem(text.at(i).name));
      table_->item(i,kName)->setFlags ( Qt::ItemIsEnabled );
    } 
    if(!text.at(i).level.isNull()&&!text.at(i).level.isEmpty()){
      table_->setColumnHidden(kLevel,false);
      table_->setItem(i,kLevel,new QTableWidgetItem(text.at(i).level));
      table_->item(i,kLevel)->setFlags ( Qt::ItemIsEnabled );
    }
    if(!text.at(i).val.isNull()&&!text.at(i).val.isEmpty()){
      table_->setColumnHidden(kValue,false);
      table_->setItem(i,kValue,new QTableWidgetItem(text.at(i).val));
      table_->item(i,kValue)->setFlags ( Qt::ItemIsEnabled );
    }
    if(!text.at(i).unit.isNull()&&!text.at(i).unit.isEmpty()){
      table_->setColumnHidden(kUnit,false);
      table_->setItem(i,kUnit,new QTableWidgetItem(text.at(i).unit));
      table_->item(i,kUnit)->setFlags ( Qt::ItemIsEnabled );
    }
    if(!text.at(i).hour.isNull()&&!text.at(i).hour.isEmpty() && 0 != text.at(i).hour){
      table_->setColumnHidden(kHour,false);
      QString ht = QString ("%1 ч.").arg(text.at(i).hour.toInt());
      table_->setItem(i,kHour,new QTableWidgetItem(ht));
      table_->item(i,kHour)->setFlags ( Qt::ItemIsEnabled );
    }
  }
  if(0 == text.count()) {
    //hide();
    table_->setColumnCount(1);
    table_->insertRow(table_->rowCount());
    table_->setColumnHidden(0,false);
    table_->setItem(0,0,new QTableWidgetItem("Нет данных"));
  } //else show();
  
  table_->resizeColumnsToContents();
  table_->resizeRowsToContents();
  int width = 0;//verticalHeader()->width()+contentsMargins().left();
  width += table_->contentsMargins().right();
  width += table_->contentsMargins().left();
  int height = 0;// horizontalHeader()->height()+contentsMargins().top();
  height += table_->contentsMargins().bottom();
  height += table_->contentsMargins().top();
  for(int i=0;i<table_->rowCount();++i ){
    height += table_->rowHeight(i);  
  }
  
  for(int j=0;j<table_->columnCount();++j ){
    width += table_->columnWidth(j);  
  }
  table_->setMinimumSize(width,height);
  table_->setMaximumSize(width,height);
  table_->resize(width,height);
  table_->adjustSize();
  adjustSize(width,height);
}


const QList<valLabel> FieldsWidget::vallist() const
{  
  return vlist_;
}

}
}

