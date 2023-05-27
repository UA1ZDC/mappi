#include "metaedit.h"

#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qmessagebox.h>
#include <qbytearray.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/puanson.pb.h>

#include "ui_metaedit.h"

namespace meteo {
namespace puanson {

QMap< int, QString > loadPunchTypes()
{
  QMap<int, QString> types;
  types.insert( proto::kSurface, QObject::tr("Приземные данные") );
  types.insert( proto::kAero, QObject::tr("Аэрологические данные") );
  types.insert( proto::kSpecial, QObject::tr("Произвольный набор данных") );
  return types;
}


Metaedit::Metaedit( QWidget* p, Qt::WindowFlags fl )
  : QDialog( p, fl ),
  ui_(new Ui::Metaedit)
{
  ui_->setupUi(this);
  ui_->idedit->setPlaceholderText( QObject::tr("[Введите идентификатор]") );
  ui_->nameedit->setPlaceholderText( QObject::tr("[Введите имя]") );
  QMapIterator< int, QString > it(kPunchTypes);
  while ( true == it.hasNext() ) {
    it.next();
    ui_->datatypebox->addItem( it.value(), QVariant( it.key() ) );
  }
}

Metaedit::~Metaedit()
{
  delete ui_; ui_ = 0;
}
    
void Metaedit::setCurrentCodeAndTitleAndType( const QString& code, const QString& title, int typ )
{
  code_ = code;
  title_ = title;
  type_ = typ;
  ui_->idedit->setText(code_);
  ui_->nameedit->setText(title_);
  for ( int i = 0, sz = ui_->datatypebox->count(); i < sz; ++i ) {
    int cmbtype = ui_->datatypebox->itemData(i).toInt();
    if ( cmbtype == typ ) {
      ui_->datatypebox->setCurrentIndex(i);
      break;
    }
  }
}

void Metaedit::accept()
{
  code_ = ui_->idedit->text();
  title_ = ui_->nameedit->text();
  type_ = ui_->datatypebox->itemData( ui_->datatypebox->currentIndex() ).toInt();
  if ( true == code_.isEmpty() ) {
    QMessageBox::warning(
        this,
        QObject::tr("Введите идентификатор шаблона"),
        QObject::tr("Введите идентификатор шаблона")
        );
    return;
  }
  if ( true == title_.isEmpty() ) {
    QMessageBox::warning(
        this,
        QObject::tr("Введите наименование шаблона"),
        QObject::tr("Введите наименование шаблона")
        );
    return;
  }
  QDialog::accept();
}

}
}
