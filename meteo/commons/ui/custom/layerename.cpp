#include "layerename.h"
#include "ui_layerename.h"

LayerRename::LayerRename( const QString& t, QWidget* p, Qt::WindowFlags fl )
  : QDialog( p, fl ),
  ui_( new Ui::LayerRename )
{
  ui_->setupUi(this);

  ui_->lename->setText(t);

  QObject::connect( ui_->okbtn, SIGNAL( clicked() ), this, SLOT( accept() ) );
  QObject::connect( ui_->nobtn, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

LayerRename::~LayerRename()
{
  delete ui_; ui_ = 0;
}

QString LayerRename::name() const
{
  return ui_->lename->text();
}
