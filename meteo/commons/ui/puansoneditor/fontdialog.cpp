#include "fontdialog.h"

#include <stdint.h>

#include <qfontdatabase.h>

#include <meteo/commons/fonts/weatherfont.h>

#include "ui_fontdialog.h"

namespace meteo {
namespace puanson {

QMap< int , QString > initWeights()
{
  QMap< int, QString > m;
  m.insert( QFont::Light, QObject::tr("Легкий") );
  m.insert( QFont::Normal, QObject::tr("Нормальный") );
  m.insert( QFont::DemiBold, QObject::tr("Полужирный") );
  m.insert( QFont::Bold, QObject::tr("Жирный") );
  m.insert( QFont::Black, QObject::tr("Черный") );
  return m;
}

void setSelection( const QString& name, QTreeWidget* tree )
{
  QTreeWidgetItem* item = 0;
  QList<QTreeWidgetItem*> list = tree->findItems( name, Qt::MatchExactly );
  if ( 0 == list.size() && 0 != tree->topLevelItemCount() ) {
    item = tree->topLevelItem(0);
  }
  else {
    item = list[0];
  }
  if ( 0 != item ) {
    item->setSelected(true);
  }
}

FontDialog::FontDialog( QWidget* p, Qt::WindowFlags fl )
  : QDialog( p, fl ),
  ui_( new Ui::FontDialog ),
  weights_( initWeights() )
{
  ui_->setupUi(this);
  meteo::WeatherFont::instance()->loadFonts();
  QFontDatabase fdb;
  QStringList families = fdb.families();
  for ( int i = 0, sz = families.size(); i < sz; ++i ) {
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->treefont );
    item->setText( 0, families[i] );
  }

  QMapIterator< int, QString > it(weights_);
  while ( true == it.hasNext() ) {
    it.next();
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->treeweight );
    item->setText( 0, it.value() );
  }

  for ( int i = 5; i < 50; ++i ) {
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->treesize );
    item->setText( 0, QString::number(i) );
  }

  QObject::connect( ui_->treefont, SIGNAL( itemSelectionChanged() ), this, SLOT( slotFontChanged() ) );
  QObject::connect( ui_->treeweight, SIGNAL( itemSelectionChanged() ), this, SLOT( slotWeightChanged() ) );
  QObject::connect( ui_->treesize, SIGNAL( itemSelectionChanged() ), this, SLOT( slotSizeChanged() ) );
  QObject::connect( ui_->cbitalic, SIGNAL( toggled( bool ) ), this, SLOT( slotItalicToggled( bool ) ) );
  QObject::connect( ui_->cbunderline, SIGNAL( toggled( bool ) ), this, SLOT( slotUnderlineToggled( bool ) ) );

  setSelection( meteo::WeatherFont::instance()->kFontSynop.family(), ui_->treefont );
  setSelection( weights_[QFont::Normal], ui_->treeweight );
  setSelection( QString::number(10), ui_->treesize);
}

FontDialog::~FontDialog()
{
  delete ui_; ui_ = 0;
}

void FontDialog::setFont()
{
  ui_->tesample->document()->clear();
  QString fname = QString("synop");
  int fweight = QFont::Normal;
  int fsize = 10;
  bool italic = ( ui_->cbitalic->checkState() == Qt::Checked );
  bool underline = ( ui_->cbunderline->checkState() == Qt::Checked );
  QList<QTreeWidgetItem*> list = ui_->treefont->selectedItems();
  if ( 0 != list.size() ) {
    fname = list[0]->text(0);
  }
  list = ui_->treeweight->selectedItems();
  if ( 0 != list.size() ) {
    fweight = weights_.key( list[0]->text(0) );
  }
  list = ui_->treeweight->selectedItems();
  if ( 0 != list.size() ) {
    fweight = weights_.key( list[0]->text(0) );
  }
  list = ui_->treesize->selectedItems();
  if ( 0 != list.size() ) {
    fsize= list[0]->text(0).toInt();
  }
  QFont font( fname, fsize, fweight, italic );
  if ( true == underline ) {
    font.setUnderline(underline);
  }
  ui_->lesymbols->setFont(font);
  ui_->tesample->document()->setDefaultFont(font);
  QString sample;
  uint8_t ch = 0;
  while ( true ) {
    sample.append( QChar(ch) );
    if ( 255 == ch ) {
      break;
    }
    ++ch;
  }
  ui_->tesample->append(sample);
}

void FontDialog::slotFontChanged()
{
  setFont();
}

void FontDialog::slotWeightChanged()
{
  setFont();
}

void FontDialog::slotSizeChanged()
{
  setFont();
}

void FontDialog::slotItalicToggled( bool fl )
{
  Q_UNUSED(fl);
  setFont();
}

void FontDialog::slotUnderlineToggled( bool fl )
{
  Q_UNUSED(fl);
  setFont();
}

}
}
