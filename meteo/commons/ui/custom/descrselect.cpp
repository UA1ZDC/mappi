#include "descrselect.h"

#include <qlist.h>
#include <qstring.h>
#include <qdom.h>

#include <cross-commons/debug/tlog.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/ui/custom/filterheader.h>

#include "ui_descrselect.h"

static const int kDescrnameClmn      (0);
static const int kDescrClmn          (1);
//static const int kIndexClmn          (2);
static const int kUnitsClmn          (2);
static const int kD9nClmn            (3);

DescrSelect::DescrSelect( QWidget* p, Qt::WindowFlags f )
  :QDialog( p, f ),
  ui_(new Ui::DescrSelect)
{
  ui_->setupUi(this);
  QObject::connect( ui_->descrtree, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ),
      this, SLOT( slotItemDoubleClicked( QTreeWidgetItem*, int ) ) );

  ui_->descrtree->setSelectionMode( QAbstractItemView::SingleSelection );
  FilterHeader* hdr = new FilterHeader(ui_->descrtree);
  ui_->descrtree->setHeader(hdr);
  hdr->setSectionsClickable(true);

  // виджет будет показан только если установить фильтр setFilter()
  ui_->filterCheck->setVisible(false);
  ui_->filterCheck->setChecked(false);
  connect( ui_->filterCheck, SIGNAL(toggled(bool)), SLOT(slotFilterToggled(bool)) );
}

DescrSelect::~DescrSelect()
{
  delete ui_; ui_ = 0;
}

QTreeWidget* DescrSelect::tree() const
{
  return ui_->descrtree;
}

void DescrSelect::setSelected( const QStringList& selected  )
{
  selected_ = selected;
}

meteo::puanson::proto::Id DescrSelect::selectedDescr()
{
  QList<QTreeWidgetItem*> items = ui_->descrtree->selectedItems();
  if ( items.size() == 0 ) {
    return ::meteo::puanson::proto::Id();
  }

  ::meteo::puanson::proto::Id id;


  QTreeWidgetItem* i = items.first();
  QString name = i->data( kDescrnameClmn, Qt::UserRole).toString();
  QString parent;
  if ( true == name.contains('@') ) {
    QStringList l = name.split('@');
    if ( 2 != l.size() ) {
      error_log << QObject::tr("Неизвестный формат наименования дескриптора. Значение = %1")
        .arg(name);
      return ::meteo::puanson::proto::Id();
    }
    parent = l[0];
    name = l[1];
  }
  id.set_descr( i->text(kDescrClmn).toInt() );
  id.set_name( name.toStdString() );
  if ( 0 != parent.size() ) {
    id.set_parent( parent.toStdString() );
  }
  id.set_description( i->toolTip(kDescrnameClmn).toUtf8().constData() );

  return id;
}

void DescrSelect::loadDescriptors(const QStringList& excludeDescrs)
{
  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();
  if ( nullptr == md ) {
    error_log << QObject::tr("Указатель на meteodescr::TMeteoDescriptor не инициализирован. Невозможно получить описание дескрипторов");
    return;
  }

  QList<QString> descrlist = TMeteoDescriptor::instance()->allNames();
  for ( int i = 0, sz = descrlist.size(); i < sz; ++i ) {
    const QString& descrname = descrlist[i];
    descr_t descr;
    QString description;
    meteodescr::Property prop;


    if ( true == md->isAdditional(descrname) ) {
      meteodescr::Additional add = md->additional(descrname);
      descr = add.descr;
      prop = md->property(descr);
//      addindx = add.index;
      description = add.description;
    }
    else {
      descr = md->descriptor(descrname);
      prop = md->property(descr);
      description = prop.description;
    }
    if ( true == md->isComplex(descrname) ) {
      continue;
    }

    if ( excludeDescrs.contains(descrname) ) {
      continue;
    }

    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->descrtree );
    QString parent;
    if ( true == md->isComponent( descrname, &parent ) ) {
      item->setText( kDescrnameClmn, QString("%1.%3")
          .arg(parent)
          .arg(descrname) );
      item->setData( kDescrnameClmn, Qt::UserRole, QString("%1@%3")
          .arg(parent)
          .arg(descrname) );
    }
    else {
      item->setText( kDescrnameClmn, descrname );
      item->setData( kDescrnameClmn, Qt::UserRole, descrname );
    }
    item->setText( kDescrClmn, QString::number(descr) );
    item->setText( kUnitsClmn, prop.unitsRu );
    item->setText( kD9nClmn, description );
    item->setToolTip( kD9nClmn, description );
    QSize sh = item->sizeHint(0);
    sh.setHeight(32);
    item->setSizeHint( 0, sh );
    if ( true == selected_.contains(descrname) ) {
      item->setSelected(true);
    }
  }
  ui_->descrtree->sortByColumn( 0, Qt::AscendingOrder );
  ui_->descrtree->header()->setSortIndicatorShown(true);
}

void DescrSelect::setFilter(const QStringList& descrs)
{
  filter_ = descrs;

  bool turnOn = filter_.size() > 0;

  ui_->filterCheck->setVisible(turnOn);
  ui_->filterCheck->setChecked(!turnOn);
  // сигнал toggled() не испускается если значение filterCheck было таким же как и переданное,
  // поэтому вызываем слот принудительно
  slotFilterToggled(ui_->filterCheck->isChecked());
}

void DescrSelect::slotItemDoubleClicked( QTreeWidgetItem* i, int col )
{
  Q_UNUSED(col);

  if ( 0 == i ) {
    return;
  }

  QDialog::accept();
}

void DescrSelect::slotFilterToggled(bool checked)
{
  for ( int i=0,isz=ui_->descrtree->topLevelItemCount(); i<isz; ++i ) {
    QTreeWidgetItem* item = ui_->descrtree->topLevelItem(i);
    auto d = item->text(kDescrnameClmn).remove("Ptend.").remove("stream.")
        .remove("cloud.").remove("wind.").remove("wlevel.");
    bool inFilterList = filter_.contains(d);
    bool hidden = false;
    if ( checked ) {
      hidden = false;
    }
    else if ( !checked && !inFilterList ) {
      hidden = true;
    }

    ui_->descrtree->setItemHidden(item, hidden);
  }
}
