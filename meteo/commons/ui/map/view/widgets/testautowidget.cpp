#include "testautowidget.h"
#include <meteo/commons/ui/map/view/widgethandler.h>

namespace meteo {
namespace map {

namespace{
  QWidget* createDialog( QWidget* parent, const QString& options )
  {
    TestAutoWidget* dlg = new TestAutoWidget( parent, options );
    dlg->show();
    return dlg;
  }

  bool registerDialog()
  {
    mapauto::WidgetHandler* hndl = WidgetHandler::instance();
    return hndl->registerWidget( "testautowidget", createDialog );
  }

static bool res = registerDialog();
}

TestAutoWidget::TestAutoWidget(QWidget* parent, const QString& options) :
  MapWidget(parent)
{
  QLabel* l = new QLabel(this);
  l->setText(options);
}

TestAutoWidget::~TestAutoWidget()
{

}

void TestAutoWidget::setOptions(const QString& options)
{
  Q_UNUSED(options)
}

}
}
