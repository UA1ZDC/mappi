//#include "../tformfreq.h"
//#include "../tbuttonstempl.h"
//#include "../tdurationtempl.h"
#include <tdebug.h>

#include "ui_form.h"

#include <tdebug.h>

#include <qapplication.h>
#include <qtextcodec.h>

//int testCalendar()
//{
//  TPopupCalendar* pc = new TPopupCalendar( 0, 0, QDate::currentDate() );
//  pc->show();
//  int res = pc->exec();
//  if ( QDialog::Accepted == res ) {
//    qDebug() << "Выбранная дата =" << pc->date().toString("dd.MM.yyyy");
//  }
//  return res;
//}

int testForm( QApplication* app )
{
  Ui::Form* tb = new Ui::Form;
  QWidget* widget = new QWidget;
  tb->setupUi(widget);

  // TButtonsTempl* tb = new TButtonsTempl;
  widget->show();

  app->setActiveWindow(widget);
  return app->exec();
}

int main( int argc, char** argv )
{
  QTextCodec::setCodecForCStrings( QTextCodec::codecForLocale() );
  QApplication app( argc, argv );
  return testForm(&app);
//  return testCalendar();
//  QApplication app( argc, argv );
//  TFormFreq* ff = new TFormFreq;
//  ff->show();
//  app.setMainWidget(ff);
//  return app.exec();
}


