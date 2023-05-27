//#include <tdebug.h>

#include "ui_form.h"

#include <qapplication.h>
#include <qtextcodec.h>
#include <QStandardItem>

#include <meteo/commons/ui/custom/multichoosedlg.h>
#include <meteo/commons/ui/custom/multichoosebtn.h>
#include <meteo/commons/ui/custom/timesheet.h>
#include <meteo/novost/ui/customtableview/tableviewmodel.h>
#include <meteo/novost/ui/customtableview/comboboxdelegate.h>
#include <meteo/novost/ui/customtableview/timesheetdelegate.h>

int testForm( QApplication* app )
{
  Ui::Form* f = new Ui::Form;
  QWidget* widget = new QWidget;
  f->setupUi(widget);

  TableViewModel* model = new TableViewModel;
  model->init(10,5);

  QList< QStandardItem* > list;
  for(int k = 0; k < 3; ++k){
    QStandardItem* item = new QStandardItem;
    item->setData(QString::number(k)+"val", Qt::EditRole);
    item->setData(k, Qt::UserRole);
    list.append(item);
  }

  CustomDelegate* delegateType = new CustomDelegate(list);
  f->tb->setItemDelegateForColumn(2,delegateType);

  TimesheetDelegate* tdelegate = new TimesheetDelegate;
  f->tb->setItemDelegateForColumn(0,tdelegate);

  for ( int i=0; i<10; i++ )
  {
    QModelIndex index = model->index(i, 0, QModelIndex());
    model->setData(index, "12,13 22,23 * * *", TableViewModel::TimesheetRole);

    index = model->index(i, 1, QModelIndex());
    model->setData(index, "text1", TableViewModel::TextRole);

    index = model->index(i, 2, QModelIndex());
    model->setData(index, 2, TableViewModel::TypeIndexRole);
    model->setData(index, "2val", TableViewModel::TypeTextRole);

    index = model->index(i, 3, QModelIndex());
    model->setData(index, "text3", TableViewModel::TextRole);

    index = model->index(i, 4, QModelIndex());
    model->setData(index, "text4", TableViewModel::TextRole);
  }

  f->tb->setModel(model);

  widget->show();

  app->setActiveWindow(widget);
  return app->exec();
}

int main( int argc, char** argv )
{
  //QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app( argc, argv );
  return testForm(&app);
}


