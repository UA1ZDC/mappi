#include "settingswidget.h"
#include "ui_settings.h"
#include "ui_addisolinewidget.h"


#include <cross-commons/app/paths.h>
#include <commons/meteo_data/tmeteodescr.h>

#include <meteo/commons/fonts/weatherfont.h>
#include <meteo/commons/proto/map_isoline.pb.h>
#include <commons/textproto/tprototext.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/ui/custom/customtreewidgetitem.h>
#include <meteo/commons/ui/custom/gradienteditor.h>
#include <meteo/commons/ui/custom/colorwidget.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/global/weatherloader.h>

#include <meteo/commons/global/global.h>


#include <qsettings.h>

#define MARK_TEXT " (*)"

namespace meteo {
namespace map {

  //! Номера колонок для изолиний по уровням
enum {
  Level = 0,
  Min   = 1,
  Max   = 2,
  Step  = 3,
  AutoStep = 4,
  Fill  = 5,
  Units = 6,
  LevelType = 7,
  LevelTypeName = 8
};

  //! Номера колонок для изолиний
  enum {
    kIsoId       = 0,
    kIsoName     = 1,
    kIsoLine     = 2,
    kIsoFont     = 3,
    kIsoText     = 4
  };

  //static const QStringList kBannedTypes = QStringList() << "3" << "4" << "9" << "20" /*<< "101"*/ << "103"
//						        << "104" << "106" << "111" << "121" << "125"
//						        << "160" << "201" << "210" << "901";
  static const QStringList kBannedTypes = QStringList() << "210" << "901";

  static const QString kSettingsFile = QDir::homePath() + "/.meteo/isoline_gui.ini";

SettingsWidget::SettingsWidget( app::MainWindow* parent) :
  QWidget(parent),
  ui_(new Ui::Settings),
  isoparams_( meteo::global::kIsoParamPath() ),
  leveltree_(nullptr),
  addwgt_(nullptr),
  isChanged_(false)
{

  ui_->setupUi(this);
  resize(500, 550);

  QApplication::setOverrideCursor(Qt::WaitCursor);
  fillLevelTypes();
  QApplication::restoreOverrideCursor();

  leveltree_ = new TreeWidget();
  QStringList labels;
  labels << QObject::tr("Уровень") << QObject::tr("Мин") << QObject::tr("Макс")
         << QObject::tr("Шаг") << QObject::tr("Автошаг") << QObject::tr("Заливка, шаг")
         << QObject::tr("Ед. измерения") << QObject::tr("Тип уровня") << QObject::tr("Тип уровня");
  leveltree_->setHeaderLabels(labels);
  leveltree_->setIconSize(QSize(16,16));
  ui_->leveltree->layout()->addWidget(leveltree_);

  if (!levelTypes_.isEmpty()) {
    leveltree_->hideColumn(LevelType);
  }

  QObject::connect(ui_->colorBtnMin, SIGNAL(clicked()),SLOT(slotColorMinDialog()));
  QObject::connect(ui_->colorBtnMax, SIGNAL(clicked()),SLOT(slotColorMaxDialog()));
  QObject::connect(ui_->colorBtnFill, SIGNAL(clicked()),SLOT(slotColorFillDialog()));
  QObject::connect(ui_->removeButton, SIGNAL(clicked()), SLOT(slotRemoveIsoline()));
  QObject::connect(ui_->saveBtn, SIGNAL(clicked()), SLOT(slotSaveIsoline()));
  QObject::connect(ui_->closeBtn, SIGNAL(clicked()), SLOT(close()));

  QObject::connect(ui_->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotSelectionChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

  QObject::connect(leveltree_, SIGNAL(iconDoubleClicked(QTreeWidgetItem*,int,bool)), SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int,bool)));
  QObject::connect(ui_->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));
  QObject::connect(leveltree_, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(slotItemChanged(QTreeWidgetItem*,int)));
  QObject::connect(ui_->addButton, SIGNAL(clicked()), SLOT(slotAddWidget()));
  ui_->colorBtnMin->setStyleSheet("background: black");
  ui_->colorBtnMax->setStyleSheet("background: black");
  ui_->colorBtnFill->setIcon(QIcon(":/meteo/icons/tools/fill.xpm"));
  ui_->removeButton->setDisabled(true);
  ui_->addButton->setDisabled(true);
  setupTree();
  ui_->lineBox->setToolTip(QObject::tr("Стиль линии"));
  ui_->lineBox->setEditable(false);
  ui_->lineBox->setIconSize(QSize(70,14));
  ui_->lineBox->setMinimumWidth(70);
  QObject::connect(ui_->lineBox, SIGNAL(currentIndexChanged(int)),this,SLOT(slotLineStyleSelected(int)));

  ui_->widthBox->setToolTip(QObject::tr("Толщина линии"));
  ui_->widthBox->setEditable(false);
  ui_->widthBox->setIconSize(QSize(70,14));
  ui_->widthBox->setMinimumWidth(70);
  QObject::connect(ui_->widthBox, SIGNAL(currentIndexChanged(int)),this,SLOT(slotLineWidthSelected(int)));

  setupLineStyle();
  setupLineWidth();
  QObject::connect(ui_->fontBox, SIGNAL(currentIndexChanged(int)), SLOT(slotFontSelected(int)));
  if (ui_->treeWidget->topLevelItemCount() != 0) {
    ui_->treeWidget->setCurrentItem(ui_->treeWidget->topLevelItem(0));
  }

  QObject::connect(ui_->fontBox, SIGNAL(currentIndexChanged(int)), SLOT(slotSetChanged()));
  QObject::connect(ui_->textSize, SIGNAL(currentIndexChanged(int)), SLOT(slotSetChanged()));
  QObject::connect(ui_->lineBox, SIGNAL(currentIndexChanged(int)), SLOT(slotSetChanged()));
  QObject::connect(ui_->widthBox, SIGNAL(currentIndexChanged(int)), SLOT(slotSetChanged()));

  QObject::connect(ui_->boldBtn, SIGNAL(clicked()), SLOT(slotSetChanged()));
  QObject::connect(ui_->italicBtn, SIGNAL(clicked()), SLOT(slotSetChanged()));
  QObject::connect(ui_->underlineBtn, SIGNAL(clicked()), SLOT(slotSetChanged()));
  QObject::connect(ui_->colorBtnMin, SIGNAL(clicked()), SLOT(slotSetChanged()));
  QObject::connect(ui_->colorBtnMax, SIGNAL(clicked()), SLOT(slotSetChanged()));

  QObject::connect(ui_->addLevelBtn, SIGNAL(clicked()), SLOT(addIsoLevel()));
  QObject::connect(ui_->rmLevelBtn,  SIGNAL(clicked()), SLOT(removeIsoLevel()));

  QObject::connect( ui_->spinisomin, SIGNAL( valueChanged(int) ), SLOT( slotSetChanged() ) );
  QObject::connect( ui_->spinisomax, SIGNAL( valueChanged(int) ), SLOT( slotSetChanged() ) );
  QObject::connect( ui_->spinisostep, SIGNAL( valueChanged(double) ), SLOT( slotSetChanged() ) );

}

SettingsWidget::~SettingsWidget()
{
  delete ui_;
  ui_ = 0;
}

void SettingsWidget::saveBannedTypes()
{
  QSettings setts( kSettingsFile, QSettings::IniFormat );
  setts.setValue("leveltypes", kBannedTypes);
  setts.sync();
}

void SettingsWidget::fillLevelTypes()
{
  levelTypes_ = meteo::global::kLevelTypes();

  QStringList banned;
  QSettings setts( kSettingsFile, QSettings::IniFormat );
  if ( false == setts.contains("leveltypes") ) {
    saveBannedTypes();
    banned = kBannedTypes;
  } else {
    banned = setts.value("leveltypes").toStringList();
  }

  bool ok = false;
  for (int idx = 0; idx < banned.size(); idx++) {
    int key = banned.at(idx).toInt(&ok);
    if (ok) {
      levelTypes_.remove(key);
    }
  }
}

void SettingsWidget::setCurrentIsoline( int descr )
{
  QString descrstr = QString::number(descr);
  QTreeWidgetItem* item = 0;
  QList<QTreeWidgetItem*> list = ui_->treeWidget->findItems( descrstr, Qt::MatchContains, kIsoId );
  if( list.count() > 0 ){
    item = list.first();
  }
  if( 0 != item ){
    ui_->treeWidget->setCurrentItem(item);
  }
}

//! Заполнение списка изолиний
void SettingsWidget::setupTree()
{
  leveltree_->clear();
  ui_->treeWidget->clear();

  const proto::FieldColors& colors = isoparams_.protoParams();
  for ( int i = 0, sz = colors.color_size(); i < sz; ++i ) {
    const meteo::map::proto::FieldColor& isoline = colors.color(i);
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->treeWidget);
    item->setText(kIsoId, QString::number(isoline.descr()));
    item->setText(kIsoName, QString::fromStdString(isoline.name()));
    item->setText(kIsoFont, QString::fromStdString( isoline.font().family() ) );
    meteodescr::Property prop = TMeteoDescriptor::instance()->property(isoline.descr());
    item->setText(kIsoText, prop.description);
    QPen pen = pen2qpen( isoline.pen() );
    LineLabel* l = new LineLabel( isoparams_.isoColorMin(isoline),
                                 pen.style(),
                                 pen.width(),
                                 ui_->treeWidget);
    ui_->treeWidget->setItemWidget(item, kIsoLine, l);
  }

  ui_->treeWidget->resizeColumnToContents(kIsoName);
}

//! Отрисовка кнопки выбора стиля линии
void SettingsWidget::setupLineStyle()
{
  int idx = ui_->lineBox->currentIndex();
  if( -1 == idx ){
    idx = 0;
  }
  disconnect(ui_->lineBox, SIGNAL(currentIndexChanged(int)),this,SLOT(slotLineStyleSelected(int)));
  QColor color(colorMin());
  ui_->lineBox->clear();
  for (int i = Qt::SolidLine; i < Qt::CustomDashLine; i++)
  {
    QPixmap pix(70,14);
    pix.fill(Qt::white);

    QBrush brush(color);
    int width_idx = ui_->widthBox->currentIndex();
    if( -1 == width_idx ){
      width_idx = 1;
    }
    QPen pen(brush, width_idx+1, (Qt::PenStyle)i);

    QPainter painter(&pix);
    painter.setPen(pen);
    painter.drawLine(2, 7, 68, 7);
    ui_->lineBox->addItem(QIcon(pix),"");
  }
  ui_->lineBox->setCurrentIndex(idx);
  connect(ui_->lineBox, SIGNAL(currentIndexChanged(int)),this,SLOT(slotLineStyleSelected(int)));
}

//! Отрисовка кнопки выбора толщины линии
void SettingsWidget::setupLineWidth()
{
  int idx = ui_->widthBox->currentIndex();
  if( -1 == idx ){
    idx = 1;
  }
  QObject::disconnect(ui_->widthBox, SIGNAL(currentIndexChanged(int)),this,SLOT(slotLineWidthSelected(int)));
  QColor color(colorMin());
  ui_->widthBox->clear();
  for (int i = 1; i < 7; i++)
  {
    QPixmap pix(70,14);
    pix.fill(Qt::white);

    QBrush brush(color);
    QPen pen(brush,(double)i, (Qt::PenStyle)(ui_->lineBox->currentIndex()+1));

    QPainter painter(&pix);
    painter.setPen(pen);
    painter.drawLine(2, 7, 68, 7);

    ui_->widthBox->addItem(QIcon(pix),"");
  }
  ui_->widthBox->setCurrentIndex(idx);
  QObject::connect(ui_->widthBox, SIGNAL(currentIndexChanged(int)),this,SLOT(slotLineWidthSelected(int)));
}


QColor SettingsWidget::colorMin() const
{
  return ui_->colorBtnMin->palette().color(QPalette::Background);
}

QColor SettingsWidget::colorMax() const
{
  return ui_->colorBtnMax->palette().color(QPalette::Background);
}

double SettingsWidget::minIsoValue() const
{
  return ui_->spinisomin->value();
}

double SettingsWidget::maxIsoValue() const
{
  return ui_->spinisomax->value();
}

double SettingsWidget::isoStep() const
{
  return ui_->spinisostep->value();
}

//! Обработка кнопки выбора цвета минимальных значений
void SettingsWidget::slotColorMinDialog()
{
  IsoColorDialog* dlg = new IsoColorDialog(0, IsoColorDialog::NoOption, this);
  dlg->setCurrentColor(colorMin());
  dlg->show();
  if( dlg->exec() == QDialog::Accepted  ) {
    ui_->colorBtnMin->setStyleSheet(QString("background: rgba(%1,%2,%3,%4)")
                                    .arg(dlg->currentColor().red())
                                    .arg(dlg->currentColor().green())
                                    .arg(dlg->currentColor().blue())
                                    .arg(dlg->currentColor().alpha()));
    setupLineStyle();
    setupLineWidth();
    for ( int i = 0; i < leveltree_->topLevelItemCount(); i++ ){
      CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(leveltree_->topLevelItem(i));
      item->setColorMin( dlg->currentColor());
    }
  }
  delete dlg;

  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if (0 == item) return;

  LineLabel* l = new LineLabel(colorMin(),
                               Qt::PenStyle(ui_->lineBox->currentIndex()+1),
                               ui_->widthBox->currentIndex()+1,
                               ui_->treeWidget);
  ui_->treeWidget->setItemWidget(item, kIsoLine, l);
}

//! Обработка кнопки выбора цвета максимальных значений
void SettingsWidget::slotColorMaxDialog()
{
  IsoColorDialog* dlg = new IsoColorDialog(0, IsoColorDialog::NoOption, this);
  dlg->setCurrentColor(colorMax());
  dlg->show();
  if( dlg->exec() == QDialog::Accepted  ){
    ui_->colorBtnMax->setStyleSheet(QString("background: rgba(%1,%2,%3,%4)")
                                    .arg(dlg->currentColor().red())
                                    .arg(dlg->currentColor().green())
                                    .arg(dlg->currentColor().blue())
                                    .arg(dlg->currentColor().alpha()));
    for ( int i = 0; i < leveltree_->topLevelItemCount(); i++ ){
      CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(leveltree_->topLevelItem(i));
      item->setColorMax(dlg->currentColor());
    }
  }
  delete dlg;
}

//! Обработка кнопки выбора цвета заливки для всех
void SettingsWidget::slotColorFillDialog()
{
  int isoline_id = ui_->treeWidget->currentItem()->text(kIsoId).remove(MARK_TEXT).toInt();

  GradientEditor* dlg = 0;

  proto::FieldColor iso = isoparams_.protoParams(isoline_id);
  if ( 0 != iso.def_grad_size() ) {
    double minval = isoparams_.gradMin(iso);
    double maxval = isoparams_.gradMax(iso);
    double step = iso.step_grad();
    if ( 0 != iso.def_grad_size() ) {
      TColorGradList gradlist;
      for (int i = 0, sz = iso.def_grad_size(); i < sz; ++i )  {
        const proto::ColorGrad& grad = iso.def_grad(i);
        QColor c = QColor::fromRgba( grad.min_color() );
        gradlist.append( TColorGrad( grad.min_value(), grad.max_value(), c, c ) );
      }
      dlg = new GradientEditor( gradlist, this );
    }
    else {
      dlg = new GradientEditor(
          minval,
          colorMin(),
          maxval,
          colorMax(),
          step,
          this );
    }
  }
  else {
    QTreeWidgetItem* item = leveltree_->topLevelItem(0);
    float min = (item != 0) ? item->text(Min).toDouble() : 0;
    float max = (item != 0) ? item->text(Max).toDouble() : 0;
    float step = (item != 0) ? item->text(Step).toDouble() : 0;
    dlg = new GradientEditor(min, colorMin(), max, colorMax(), step, this);
  }
  dlg->setWindowTitle(QObject::tr("Настройка заливки"));
  if ( QDialog::Accepted != dlg->exec() ) {
    delete dlg;
    return;
  }
  for (int i = 0; i < leveltree_->topLevelItemCount(); i++) {
    CustomTreeWidgetItem* item = reinterpret_cast<CustomTreeWidgetItem*>(leveltree_->topLevelItem(i));
    if (item->gradient().size() == 0) {
      item->setText(Fill, QString::number(dlg->step()));
    }
  }
  iso.set_step_grad(dlg->step());
  iso.clear_def_grad();
  TColorGradList pal = dlg->gradient();
  for (int i = 0, sz = pal.size(); i < sz; ++i ) {
    const TColorGrad& srcclr = pal[i];
    proto::ColorGrad* grad = iso.add_def_grad();
    grad->set_min_value( srcclr.begval() );
    grad->set_max_value( srcclr.endval() );
    grad->set_min_color( srcclr.begcolor().rgba() );
    grad->set_max_color( srcclr.endcolor().rgba() );
  }

  isoparams_.setParams(iso);
  slotSetChanged();
  delete dlg;
  if ( 0 != iso.def_grad_size() ) {
    ui_->colorBtnFill->setIcon(QIcon(":/meteo/icons/tools/fill_custom.xpm"));
  }
  else {
    ui_->colorBtnFill->setIcon(QIcon(":/meteo/icons/tools/fill.xpm"));
  }
}

//! Добавление изолинии
void SettingsWidget::slotAddIsoline()
{
  if( nullptr == addwgt_ ){
    return;
  }
  int descr = addwgt_->index();
  QString name = addwgt_->name();
  if (-1 == descr) {
    QMessageBox::warning(0, tr("Ошибка"),
                         msglog::kNameIsolineNotFound.arg(name), tr("Закрыть"));
    return;
  }

  if ( isoparams_.contains(descr) ) {
    int res = QMessageBox::question(
                0,
                QObject::tr("Внимание"),
                msglog::kIsolineAlreadyAdded.arg(descr),
                QObject::tr("Да"),
                QObject::tr("Нет")
                );
    if( 1 == res ){
      return;
    }
  }

  meteo::map::proto::FieldColor iso;
  iso.set_descr(descr);
  iso.set_name(name.toStdString());
  isoparams_.setParams(iso);
  ui_->treeWidget->blockSignals(true);
  setupTree();
  ui_->treeWidget->blockSignals(false);
  QList<QTreeWidgetItem*> list = ui_->treeWidget->findItems(QString::number(descr), Qt::MatchContains, 0);
  if( list.count() > 0 ){
    QTreeWidgetItem* item = list.first();
    ui_->treeWidget->setCurrentItem(item);
    setMarked(item);
  }
}

//! Сохранение настроек в файл
void SettingsWidget::slotSaveIsoline()
{
  isChanged_ = false;
  isoSettingsFromItem( ui_->treeWidget->currentItem() );
  isoparams_.saveParams();

  for( int i = 0; i <= ui_->treeWidget->topLevelItemCount(); i++ ) {
    if( 0 != ui_->treeWidget->topLevelItem(i) ) {
      QString id = ui_->treeWidget->topLevelItem(i)->text(kIsoId).remove(MARK_TEXT);
      ui_->treeWidget->topLevelItem(i)->setText(kIsoId, id);
    }
  }
  internal::WeatherLoader* wl = WeatherLoader::instance();
  wl->reloadTypes();
  emit updateSettings();
}

void SettingsWidget::proto2ui()
{
  isChanged_ = false;
  QTreeWidgetItem* item = 0;
  item = ui_->treeWidget->currentItem();
  int descr = 0;
  if( 0 != item ){
    descr = item->text(kIsoId).remove(MARK_TEXT).toInt();
  }

  ui_->treeWidget->blockSignals(true);
  setupTree();
  ui_->treeWidget->blockSignals(false);

  QList<QTreeWidgetItem*> list = ui_->treeWidget->findItems(QString::number(descr), Qt::MatchContains, 0);
  if( list.count() > 0 ){
    item = list.first();
    ui_->treeWidget->setCurrentItem(item);
  } else if (ui_->treeWidget->topLevelItemCount() != 0) {
    ui_->treeWidget->setCurrentItem(ui_->treeWidget->topLevelItem(0));
  }
}

void SettingsWidget::slotClose()
{
  if (isChanged_) {
    int res = QMessageBox::question(0,
                                    QObject::tr("Внимание"),
                                    msglog::kSettingsChanged,
                                    QObject::tr("Да"),
                                    QObject::tr("Нет")
                                    );
    if (0 == res) {
      slotSaveIsoline();
    }
  }
  isChanged_ = false;

  QWidget::close();
}

void SettingsWidget::keyReleaseEvent( QKeyEvent * e)
{
  if (e->key() == Qt::Key_Escape) {
    slotClose();
  }
}


void SettingsWidget::closeEvent(QCloseEvent *)
{
  slotClose();
}

void SettingsWidget::showEvent(QCloseEvent *)
{
  proto2ui();
}

//! Выбор стиля линии
void SettingsWidget::slotLineStyleSelected(int)
{
  setupLineWidth();

  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if (0 == item) {
    return;
  }

  LineLabel* l = new LineLabel(colorMin(),
             Qt::PenStyle(ui_->lineBox->currentIndex()+1),
             ui_->widthBox->currentIndex()+1,
             ui_->treeWidget);
  ui_->treeWidget->setItemWidget(item, kIsoLine, l);
}

//! Выбор толщины линии
void SettingsWidget::slotLineWidthSelected(int)
{
  setupLineStyle();

  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if (0 == item) {
    return;
  }

  LineLabel* l = new LineLabel(colorMin(),
             Qt::PenStyle(ui_->lineBox->currentIndex()+1),
             ui_->widthBox->currentIndex()+1,
             ui_->treeWidget);
  ui_->treeWidget->setItemWidget(item, kIsoLine, l);
}

void SettingsWidget::slotFontSelected(int)
{
  QTreeWidgetItem* item = ui_->treeWidget->currentItem();
  if (0 == item) {
    return;
  }
  item->setText(kIsoFont, ui_->fontBox->currentText() );
}

//! Удаление настроек изолинии
void SettingsWidget::slotRemoveIsoline()
{
  QTreeWidgetItem* item = 0;
  item = ui_->treeWidget->currentItem();
  if( 0 == item ){
    return;
  }
  int res = QMessageBox::question(
              0,
              QObject::tr("Внимание"),
              msglog::kRemove.arg(item->text(kIsoName)),
              QObject::tr("Да"),
              QObject::tr("Нет")
              );
  if( 1 == res ){
    return;
  }
  isoparams_.removeParams(item->text(kIsoId).remove(MARK_TEXT).toInt());

  ui_->treeWidget->blockSignals(true);
  setupTree();
  ui_->treeWidget->blockSignals(false);

  if (ui_->treeWidget->topLevelItemCount() != 0) {
    ui_->treeWidget->setCurrentItem(ui_->treeWidget->topLevelItem(0));
  }
}

//! Переключение между линиями - сохранение состояния предыдущей, заполнение данными текущей
void SettingsWidget::slotSelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem * previous)
{
  //сохранение предыдущей
  isoSettingsFromItem(previous);

  QTreeWidgetItem* item = current;
  if( 0 == item ){
    ui_->removeButton->setDisabled(true);
  }
  else{
    ui_->removeButton->setEnabled(true);
    uiFromCurrentItem();
  }
}

//! Заполнение списка настроек изолинии по уровням
void SettingsWidget::fillLevelsTree(int isoline_id)
{
  leveltree_->clear();
  leveltree_->blockSignals(true);

  if (!isoparams_.contains(isoline_id)) {
    leveltree_->blockSignals(false);
    return;
  }

  QStringList levels = levelTypes_.values();
  qSort(levels);

  const meteo::map::proto::FieldColor& isoline = isoparams_.protoParams(isoline_id);
  for (int i = 0, sz = isoline.level_size(); i < sz; ++i ) {
    const meteo::map::proto::LevelColor& level = isoline.level(i);
    CustomTreeWidgetItem* item2 = new CustomTreeWidgetItem(leveltree_);
    switch (level.type_level()) {
    case meteodescr::kSurfaceLevel:
    case meteodescr::kOt500to1000:
    case meteodescr::kTropoLevel:
      item2->setText(Level, levelTypes_.value(level.type_level()));
      break;
    default:
      item2->setText(Level, QString::number(level.level()));
      break;
    }

    item2->setText(LevelType, QString::number(level.type_level()));
    ComboBoxItem* cb = new ComboBoxItem(item2, LevelTypeName, leveltree_->visualItemRect(item2).height());
    cb->fill(levels, levelTypes_.value(level.type_level()));

    leveltree_->setItemWidget(item2, LevelTypeName, cb);

    item2->setText(Min, QString::number( isoparams_.isoMin( level.level(), level.type_level(), isoline ) ) );
    item2->setText(Max, QString::number( isoparams_.isoMax( level.level(), level.type_level(), isoline ) ) );
    if (0 != isoline.def_grad_size()) {
      item2->setText(Fill, QString::number( isoline.step_grad() ));
    }
    else {
      item2->setText(Fill, QString::number( level.step_grad() ));
    }
    item2->setText(Step, QString::number( level.step_iso() ));
    item2->setText(Units, QString::fromStdString(isoline.unit()));
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    item2->setColorMin( isoparams_.isoColorMin( level.level(), level.type_level(), isoline ) );
    item2->setColorMax( isoparams_.isoColorMax( level.level(), level.type_level(), isoline ) );
    Qt::CheckState chst = Qt::Unchecked;
    if ( true == level.autostep() ) {
      chst = Qt::Checked;
    }
    item2->setCheckState( AutoStep, chst );
    item2->setFillIcon(Fill, ":/meteo/icons/tools/fill.xpm");
    if ( 0 != level.gradient_size() ) {
      item2->setFillIcon(Fill, ":/meteo/icons/tools/fill_custom.xpm");
      item2->setText( Fill, QString::number( level.step_grad() ) );
      TColorGradList gradlist;
      for ( int j = 0, jsz = level.gradient_size(); j < jsz; ++j ) {
        const proto::ColorGrad& srcgrad = level.gradient(j);
        QColor c = QColor::fromRgba( srcgrad.min_color() );
        gradlist.append( TColorGrad( srcgrad.min_value(), srcgrad.max_value(), c, c ) );
      }
      item2->setGradient(gradlist);
    }
  }

  leveltree_->blockSignals(false);
}

//! Заполнение виджетов в соответствии с настройками текущей изолинии
void SettingsWidget::uiFromCurrentItem()
{
  int isoline_id = ui_->treeWidget->currentItem()->text(kIsoId).remove(MARK_TEXT).toInt();
  if ( false == isoparams_.contains(isoline_id) ) {
    return;
  }

  ui_->fontBox->blockSignals(true);
  ui_->textSize->blockSignals(true);
  ui_->lineBox->blockSignals(true);
  ui_->widthBox->blockSignals(true);
  ui_->spinisomin->blockSignals(true);
  ui_->spinisomax->blockSignals(true);
  ui_->spinisostep->blockSignals(true);

  const meteo::map::proto::FieldColor& iso = isoparams_.protoParams(isoline_id);
  if ( 0 != iso.def_grad_size() ) {
    ui_->colorBtnFill->setIcon(QIcon(":/meteo/icons/tools/fill_custom.xpm"));
  }
  else {
    ui_->colorBtnFill->setIcon(QIcon(":/meteo/icons/tools/fill.xpm"));
  }
  QFont font = font2qfont( iso.font() );
  ui_->addButton->setEnabled(true);
  ui_->fontBox->setCurrentFont( QFont( QString::fromStdString( iso.font().family() ) ) );
  QColor minclr = isoparams_.isoColorMin(iso);
  QColor maxclr = isoparams_.isoColorMax(iso);
  ui_->colorBtnMin->setStyleSheet(QString("background: rgba(%1,%2,%3,%4)")
                                  .arg(minclr.red())
                                  .arg(minclr.green())
                                  .arg(minclr.blue())
                                  .arg(minclr.alpha()));
  ui_->colorBtnMax->setStyleSheet(QString("background: rgba(%1,%2,%3,%4)")
                                  .arg(maxclr.red())
                                  .arg(maxclr.green())
                                  .arg(maxclr.blue())
                                  .arg(maxclr.alpha()));
  ui_->spinisomin->setValue( isoparams_.isoMin(iso) );
  ui_->spinisomax->setValue( isoparams_.isoMax(iso) );
  ui_->spinisostep->setValue( isoparams_.isoStep(iso) );
  QPen pen = pen2qpen( iso.pen() );
  ui_->lineBox->setCurrentIndex( pen.style() - 1 );
  ui_->widthBox->setCurrentIndex( pen.width() - 1 );
  ui_->textSize->setCurrentIndex(ui_->textSize->findText(QString::number(font.pointSize())));
  setupLineStyle();
  setupLineWidth();
  ui_->boldBtn->setChecked(font.bold());
  ui_->italicBtn->setChecked(font.italic());
  ui_->underlineBtn->setChecked(font.underline());

  ui_->fontBox->blockSignals(false);
  ui_->textSize->blockSignals(false);
  ui_->lineBox->blockSignals(false);
  ui_->widthBox->blockSignals(false);
  ui_->spinisomin->blockSignals(false);
  ui_->spinisomax->blockSignals(false);
  ui_->spinisostep->blockSignals(false);

  fillLevelsTree(isoline_id);
}

void SettingsWidget::slotItemDoubleClicked(QTreeWidgetItem* item, int col, bool isicon)
{
  bool flag = false;

  CustomTreeWidgetItem* item2 = static_cast<CustomTreeWidgetItem*>(item);
  if( 0 == item2 ){
    return;
  }

  switch(col){
  case Level: {
    int type = item->text(LevelType).toInt();
    switch (type) {
    case meteodescr::kSurfaceLevel:
    case meteodescr::kOt500to1000:
    case meteodescr::kTropoLevel:
      item2->setFlags(item2->flags() & ~Qt::ItemIsEditable);
      flag = false;
      break;
    default:
      edit_ = item2->text(col);
      leveltree_->editItem(item2, col);
      item2->setFlags(item2->flags() | Qt::ItemIsEditable);
      break;
    }
    break;
  }
  case Min :{
    if( false == isicon ){
      edit_ = item2->text(col);
      leveltree_->editItem(item2, col);
    }else{
      IsoColorDialog* dlg = new IsoColorDialog(0, IsoColorDialog::NoOption, this);
      dlg->setCurrentColor(item2->min());
      dlg->show();
      if( dlg->exec() == QDialog::Accepted  ){
        item2->setColorMin(dlg->currentColor());
        flag = true;
      }
      delete dlg;
    }
    break;
  }
  case Max :{
    if( false == isicon ){
      edit_ = item2->text(col);
      leveltree_->editItem(item2, col);
    }else{
      IsoColorDialog* dlg = new IsoColorDialog(0, IsoColorDialog::NoOption, this);
      dlg->setCurrentColor(item2->max());
      dlg->show();
      if( dlg->exec() == QDialog::Accepted  ){
        item2->setColorMax(dlg->currentColor());
        flag = true;
      }
      delete dlg;
    }
    break;
  }
  case Step: {
    edit_ = item->text(col);
    leveltree_->editItem(item, col);
  }
    break;
  case LevelType: {
    edit_ = item->text(col);
    flag = false;
  }
    break;
  case Fill: {
    GradientEditor* dlg;
    int iso_id = ui_->treeWidget->currentItem()->text(kIsoId).remove(MARK_TEXT).toInt();
    if (item2->gradient().size() != 0) {
      dlg = new GradientEditor(item2->gradient(), this);
    }
    else if ( 0  != isoparams_.protoParams(iso_id).def_grad_size() ) {
      const meteo::map::proto::FieldColor& gradient = isoparams_.protoParams(iso_id);
      TColorGradList gradlist;
      for (int i = 0, sz = gradient.def_grad_size(); i < sz; ++i ) {
        const proto::ColorGrad& grad = gradient.def_grad(i);
        QColor c = QColor::fromRgba(grad.min_color());
        gradlist.append( TColorGrad(grad.min_value(), grad.max_value(), c,c ) );
      }
      dlg = new GradientEditor(gradlist, this);
    }
    else {
      dlg = new GradientEditor(item2->text(Min).toFloat(), item2->min(),
                               item2->text(Max).toFloat(), item2->max(),
                               item2->text(Fill).toFloat(), this);
    }
    if (0 != dlg) {
      dlg->setModal(true);
      dlg->setWindowTitle(QObject::tr("Настройка заливки"));
      dlg->show();
      if (dlg->exec() == QDialog::Accepted) {
        item2->setFillIcon(Fill, ":/meteo/icons/tools/fill_custom.xpm");
        item2->setText(Fill, QString::number(dlg->step()));
        item2->setGradient(dlg->gradient());
        flag = true;
      }
      delete dlg;
    }
  }
    break;
  }

  if (col != Level) {
    leveltree_->blockSignals(true);
    item2->setFlags(item2->flags() | Qt::ItemIsEditable);
    leveltree_->blockSignals(false);
  }

  if (flag) {
    slotSetChanged();
  }
}


void SettingsWidget::slotItemDoubleClicked(QTreeWidgetItem* item, int col)
{
  if( kIsoName == col ) {
    QString name;
    QInputDialog* dlg = new QInputDialog(this);
    dlg->setCancelButtonText(QObject::tr("Отмена"));
    dlg->setOkButtonText(QObject::tr("Принять"));
    dlg->setLabelText(msglog::kSetIsoName);
    dlg->setWindowTitle(QObject::tr("Изменение названия изолинии"));
    dlg->resize(400, dlg->height());
    int res = dlg->exec();
    name = dlg->textValue();
    delete dlg;
    if( QDialog::Rejected == res ) {
      return;
    }
    setMarked(item);

    item->setText(kIsoName, name);
    int32_t id = item->text(kIsoId).remove(MARK_TEXT).toInt();
    if (id != 0) {
      proto::FieldColor clr = isoparams_.protoParams(id);
      clr.set_name(name.toStdString());
      isoparams_.setParams(clr);
    }
  }
}

void SettingsWidget::slotItemChanged(QTreeWidgetItem* item, int col)
{
  bool flag = true;
  switch( col ) {
  case Level: {
    int type = item->text(LevelType).toInt();
    switch (type) {
    case meteodescr::kSurfaceLevel:
    case meteodescr::kOt500to1000:
    case meteodescr::kTropoLevel:
       flag = false;
      break;
    default:
      bool ok = true;
      double val = item->text(col).toDouble(&ok);
      if (!ok || val < 0 || val > 1500) {
        item->setText(col, edit_);
        QMessageBox::warning(0, tr("Ошибка"), msglog::kSetPositiveValue.arg(1100), tr("Закрыть"));
        flag = false;
      }
      break;
    }
    break;
  }
  case Units : {
    for( int i = 0; i <= leveltree_->topLevelItemCount(); i++ ){
      if( 0 != leveltree_->topLevelItem(i) ){
        leveltree_->topLevelItem(i)->setText(col, item->text(col));
      }
    }
    break;
  }
  case LevelType:
    item->setText(col, edit_);
    break;
  case LevelTypeName: {
    leveltree_->blockSignals(true);
    edit_ = item->data(col, Qt::UserRole).toString();
    if (levelTypes_.value(item->text(LevelType).toInt()) != item->data(col, Qt::UserRole).toString()) {
      int type = levelTypes_.key(item->data(col, Qt::UserRole).toString());
      item->setText(LevelType, QString::number(type));
      switch (type) {
      case meteodescr::kSurfaceLevel:
      case meteodescr::kOt500to1000:
      case meteodescr::kTropoLevel:
        item->setText(Level, levelTypes_.value(type));
        break;
      default:
        item->setText(Level, "0");
        break;
      }
      slotSetChanged();
    }
    leveltree_->blockSignals(false);
    break;
  }
  default:
    break;
  }

  if (edit_ != item->text(col) && col != LevelTypeName &&
      !(col == Level && flag == false)) {
    slotSetChanged();
  }
}

void SettingsWidget::slotAddWidget()
{
  if ( nullptr == addwgt_ ){
    addwgt_ = new AddIsolineWidget(this);
    QObject::connect(addwgt_, SIGNAL(accepted()), SLOT(slotAddIsoline()));
  }
  addwgt_->show();
}

void SettingsWidget::isoSettingsFromItem(QTreeWidgetItem* item)
{
  if (0 == item) {
    return;
  }

  int64_t isoline_id = item->text(kIsoId).remove(MARK_TEXT).toInt();
  proto::FieldColor iso;
  if ( isoparams_.contains(isoline_id) ) {
    proto::FieldColor old = isoparams_.protoParams(isoline_id);
    if (old.has_step_grad()) {
      iso.set_step_grad(old.step_grad());
    }
    for (int idx = 0; idx < old.def_grad_size(); idx++) {
      proto::ColorGrad* grad = iso.add_def_grad();
      grad->CopyFrom(old.def_grad(idx));
    }
  }
  isoSettingsFromItem(item, &iso);
  setupGradientParamsFromItems(&iso);
  isoparams_.setParams(iso);
}

//! Заполнение параметров текущей линии
void SettingsWidget::isoSettingsFromItem(const QTreeWidgetItem* lineItem, proto::FieldColor* iso)
{
  if (0 == lineItem || 0 == iso) return;

  int descr = lineItem->text(kIsoId).remove(MARK_TEXT).toInt();
  QString name = lineItem->text(kIsoName);
  iso->set_descr(descr);
  iso->set_name(name.toStdString());
  Font font = qfont2font( ui_->fontBox->currentFont() );
  font.set_bold(ui_->boldBtn->isChecked());
  font.set_italic(ui_->italicBtn->isChecked());
  font.set_underline(ui_->underlineBtn->isChecked());
  font.set_pointsize(ui_->textSize->currentText().toInt());
  iso->mutable_font()->CopyFrom(font);
  isoparams_.setIsoColorMin( colorMin(), iso );
  isoparams_.setIsoColorMax( colorMax(), iso );
  isoparams_.setIsoMin( minIsoValue(), iso );
  isoparams_.setIsoMax( maxIsoValue(), iso );
  isoparams_.setIsoStep( isoStep(), iso );
  Pen pen;
  pen.set_width( ui_->widthBox->currentIndex()+1 );
  pen.set_style( static_cast<PenStyle>(ui_->lineBox->currentIndex()+1) );
  iso->mutable_pen()->CopyFrom(pen);

  meteodescr::Property prop = TMeteoDescriptor::instance()->property(descr);
  int kol_items = leveltree_->topLevelItemCount();

  for(int i = 0; i < kol_items; ++i) {
    CustomTreeWidgetItem* item = reinterpret_cast<CustomTreeWidgetItem*>(leveltree_->topLevelItem(i));
    if ( 0 == item ) {
      continue;
    }
    meteo::map::proto::LevelColor* level = iso->add_level();
    QString lvl = item->text(Level);
    if (item->text(LevelType).toInt() == meteodescr::kOt500to1000) {
      level->set_level(15000);
    } else {
      level->set_level(lvl.toInt());
    }
    level->set_type_level(item->text(LevelType).toInt());
    level->mutable_color()->Clear();
    proto::ColorGrad* grad = level->mutable_color();
    grad->set_min_color( QColor( item->min() ).rgba() );
    grad->set_max_color( QColor( item->max() ).rgba() );
    grad->set_min_value( item->text(Min).toDouble() );
    grad->set_max_value( item->text(Max).toDouble() );
    level->set_step_iso(item->text(Step).toDouble());
    bool autostep = false;
    if ( Qt::Checked == item->checkState( AutoStep ) ) {
      autostep = true;
    }
    level->set_autostep(autostep);
  }

  if( 0 != leveltree_->topLevelItem(0) ){
    QString unit = leveltree_->topLevelItem(0)->text(Units);
    if( true != unit.isEmpty() ){
      iso->set_unit(unit.toStdString());
    }
    else{
      iso->set_unit(prop.units.toStdString());
    }
  }
  else{
    iso->set_unit(prop.units.toStdString());
  }
}

//! Заполнение параметров заливки для текущей линии по уровням
void SettingsWidget::setupGradientParamsFromItems(proto::FieldColor* iso )
{
  if (0 == iso) {
    return;
  }

  int kol_items = leveltree_->topLevelItemCount();

  for(int i = 0; i < kol_items; ++i) {
    CustomTreeWidgetItem* item = static_cast<CustomTreeWidgetItem*>(leveltree_->topLevelItem(i));
    if( 0 == item ) {
      continue;
    }
    if ( true == item->hasGradient() ) {
      proto::LevelColor level = GradientParams::levelProto( item->text(Level).toInt(),item->text(LevelType).toInt(), *iso);
      level.set_step_grad(item->text(Fill).toFloat());
      for (int j = 0, jsz = item->gradient().size(); j < jsz; ++j ) {
        proto::ColorGrad* grad = level.add_gradient();
        grad->set_min_value(item->gradient().at(j).begval());
        grad->set_max_value(item->gradient().at(j).endval());
        grad->set_min_color(item->gradient().at(j).begcolor().rgba());
        grad->set_max_color(item->gradient().at(j).endcolor().rgba());
      }
      GradientParams::setLevelProto( level.level(), level.type_level(), level, iso );
    }
  }
}

void SettingsWidget::slotSetChanged()
{
  setMarked(ui_->treeWidget->currentItem());
}

//! Помечает измененную пользователем изолинию
void SettingsWidget::setMarked(QTreeWidgetItem* item)
{
  if (item == 0) return;
  isChanged_ = true;

  QString text = item->text(kIsoId);
  if (!text.contains(MARK_TEXT)) {
    item->setText(kIsoId, item->text(kIsoId) + MARK_TEXT);
  }
}

void SettingsWidget::addIsoLevel()
{
  if (0 == ui_->treeWidget->currentItem()) {
    return;
  }

  int64_t isoline_id = ui_->treeWidget->currentItem()->text(kIsoId).remove(MARK_TEXT).toInt();
  proto::FieldColor iso = isoparams_.protoParams(isoline_id);

  GradientEditor* dlg = nullptr;
  if ( 0 != iso.def_grad_size() ) {
    TColorGradList gradlist;
    for (int i = 0, sz = iso.def_grad_size(); i < sz; ++i )  {
      const proto::ColorGrad& grad = iso.def_grad(i);
      QColor c = QColor::fromRgba( grad.min_color() );
      gradlist.append( TColorGrad( grad.min_value(), grad.max_value(), c, c ) );
    }
    dlg = new GradientEditor( gradlist, this );
    dlg->accept();
  }

  QStringList levels = levelTypes_.values();
  qSort(levels);

  leveltree_->blockSignals(true);
  CustomTreeWidgetItem* item = new CustomTreeWidgetItem(leveltree_);
  item->setText(Min, QString::number( ui_->spinisomin->value() ));
  item->setText(Max, QString::number( ui_->spinisomax->value() ));
  if (nullptr != dlg) {
    item->setText(Fill, QString::number(dlg->step()));
    item->setGradient(dlg->gradient());
    iso.set_step_grad(dlg->step());
    iso.clear_def_grad();
    TColorGradList pal = dlg->gradient();
    for (int i = 0, sz = pal.size(); i < sz; ++i ) {
      const TColorGrad& srcclr = pal[i];
      proto::ColorGrad* grad = iso.add_def_grad();
      grad->set_min_value( srcclr.begval() );
      grad->set_max_value( srcclr.endval() );
      grad->set_min_color( srcclr.begcolor().rgba() );
      grad->set_max_color( srcclr.endcolor().rgba() );
    }
    isoparams_.setParams(iso);
    delete dlg;
  }
  else {
    item->setText(Fill, QString::number( 1 ));
  }
  item->setText(Step, QString::number( ui_->spinisostep->value() ));
  item->setText(Units, "");
  item->setCheckState( AutoStep, Qt::Unchecked );
  if ( leveltree_->topLevelItemCount() != 0) {
    item->setText(Units, leveltree_->topLevelItem(0)->text(Units));
    item->setCheckState( AutoStep, leveltree_->topLevelItem(0)->checkState(AutoStep));
  }

  item->setText(LevelType, QString::number(meteodescr::kIsobarLevel));
  int level = 0;
  if ( leveltree_->topLevelItemCount() != 0) {
    for (int i = 0, sz = leveltree_->topLevelItemCount(); i<sz; ++i) {
      if ( ( leveltree_->topLevelItem(i)->text(Level).toInt() == level )
           && (leveltree_->topLevelItem(i)->text(LevelType) == QString::number(meteodescr::kIsobarLevel) ) ) {
        ++level;
        i = 0;
      }
    }
    item->setText(Level, QString::number(level));
  }
  else {
    item->setText(Level, QString::number(level));
  }

  ComboBoxItem* cb = new ComboBoxItem(item, LevelTypeName, leveltree_->visualItemRect(item).height());
  cb->fill(levels, levelTypes_.value(meteodescr::kIsobarLevel));
  leveltree_->setItemWidget(item, LevelTypeName, cb);

  item->setFlags(item->flags() | Qt::ItemIsEditable);
  item->setColorMin(colorMin());
  item->setColorMax(colorMax());
  if ( 0 != iso.def_grad_size() ) {
    item->setFillIcon(Fill, ":/meteo/icons/tools/fill_custom.xpm");
    isoparams_.setParams(iso);
  }
  else {
    item->setFillIcon(Fill, ":/meteo/icons/tools/fill.xpm");
  }

  setMarked(ui_->treeWidget->currentItem());

  leveltree_->blockSignals(false);

  leveltree_->setCurrentItem(item);
}

void SettingsWidget::removeIsoLevel()
{
  QTreeWidgetItem* isoItem = ui_->treeWidget->currentItem();
  QTreeWidgetItem* item = leveltree_->currentItem();
  if(0 == item  || 0 == isoItem) {
    return;
  }

  int res = QMessageBox::question(
              0,
              QObject::tr("Внимание"),
              msglog::kRemoveLevel.arg(item->text(Level)).arg(item->data(LevelTypeName, Qt::UserRole).toString()).arg(isoItem->text(kIsoName)),
              QObject::tr("Да"),
              QObject::tr("Нет")
              );
  if( 1 == res ){
    return;
  }

  int level = item->text(Level).toInt();
  int leveltype = item->text(LevelType).toInt();

  proto::FieldColor iso = isoparams_.protoParams(isoItem->text(kIsoId).remove(MARK_TEXT).toInt());
  for( int j = 0, sz = iso.level_size(); j < sz; ++j ){
    if ( iso.level(j).level() == level &&
         iso.level(j).type_level() == leveltype){
      if ( j != iso.level_size() - 1 ) {
        iso.mutable_level()->SwapElements(j, iso.level_size() - 1);
      }
      iso.mutable_level()->RemoveLast();
      break;
    }
  }

  isoparams_.setParams(iso);

  leveltree_->takeTopLevelItem(leveltree_->indexOfTopLevelItem(item));

  setMarked(isoItem);
}


LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent){
    clearButton = new QToolButton(this);
    QPixmap pixmap(":/icons/clear.png");
    clearButton->setIcon(QIcon(pixmap));
    clearButton->setIconSize(pixmap.size());
    clearButton->setCursor(Qt::ArrowCursor);
    clearButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
    clearButton->hide();
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateCloseButton(const QString&)));
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px;} ").arg(clearButton->sizeHint().width() + frameWidth + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2));
}

void LineEdit::resizeEvent(QResizeEvent *)
{
    QSize sz = clearButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    clearButton->move(rect().right() - frameWidth - sz.width(),
                      (rect().bottom() + 1 - sz.height())/2);
}

void LineEdit::updateCloseButton(const QString& text)
{
  clearButton->setVisible(!text.isEmpty());
}

LineLabel::LineLabel(QWidget* parent) :
  QLabel(parent),
  color_(Qt::black),
  style_(Qt::SolidLine),
  width_(2)
{
  paint();
}

LineLabel::LineLabel(QColor color, Qt::PenStyle style, int width, QWidget* parent) :
  QLabel(parent),
  color_(color),
  style_(style),
  width_(width)
{
  paint();
}

void LineLabel::setColor(QColor color)
{
  color_ = color;
  paint();
}

void LineLabel::setStyle(Qt::PenStyle style)
{
  style_ = style;
  paint();
}

void LineLabel::setWidth(int width)
{
  width_ = width;
  paint();
}

const QColor LineLabel::color() const
{
  return color_;
}

Qt::PenStyle LineLabel::style() const
{
  return style_;
}

int LineLabel::width() const
{
  return width_;
}

QPixmap LineLabel::pixmap() const
{
  return pix_;
}

void LineLabel::paint()
{
  pix_ = QPixmap(70,14);
  pix_.fill(Qt::white);
  QBrush brush(color_);
  QPen pen(brush, width_, style_);
  QPainter painter(&pix_);
  painter.setPen(pen);
  painter.drawLine(2, 7, 68, 7);
  setPixmap(pix_);
}

AddIsolineWidget::AddIsolineWidget(QWidget* parent):
  QDialog(parent),
  ui_(new Ui::AddIsolineWidget),
  response_(nullptr)
{
  ui_->setupUi(this);
  setWindowTitle(QObject::tr("Добавление изолинии"));
  ui_->buttonBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("Принять"));
  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setText(QObject::tr("Отмена"));
  ui_->indexTree->header()->show();

  QObject::connect(ui_->indexTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotIndexChanged()));
  QObject::connect(ui_->buttonBox, SIGNAL(accepted()), SLOT(acceptIsoline()));
  QObject::connect(ui_->buttonBox, SIGNAL(rejected()), SLOT(reject()));
  QObject::connect(ui_->indexTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(accept()));
  QObject::connect(ui_->nameEdit, SIGNAL(textEdited(const QString&)), SLOT(slotFindIsoline(const QString&)));

  rpc::Channel* channel = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
  if( nullptr == channel ){
    error_log << msglog::kServiceNotFound.arg(meteo::global::serviceTitle(settings::proto::kSprinf));
    return;
  }
  meteo::sprinf::MeteoParameterRequest request;
  QCursor q;
  q.setShape(Qt::WaitCursor);
  qApp->setOverrideCursor( q );
  response_ = channel->remoteCall(&meteo::sprinf::SprinfService::GetMeteoParametersByBufr, request, 10000);
  if ( nullptr == response_ ) {
    delete channel;
    return;
  }
  for( int i = 0; i < response_->parameter_size(); i++ ) {
    if ( ( true == response_->parameter(i).has_bufr() )
         && ( 0 != response_->parameter(i).bufr() ) ) {
      const meteo::sprinf::MeteoParameter& p = response_->parameter(i);
      QTreeWidgetItem* item = new QTreeWidgetItem( ui_->indexTree);
      item->setText(0, QString::number(p.bufr()));
      item->setText(1, QString::fromStdString(p.name()));
    }
  }
  delete channel;
  qApp->restoreOverrideCursor();
}

AddIsolineWidget::~AddIsolineWidget()
{
  delete response_;
  response_ = nullptr;
}

void AddIsolineWidget::acceptIsoline()
{
  if (ui_->indexTree->topLevelItemCount() == 1) {
    ui_->indexTree->setCurrentItem(ui_->indexTree->topLevelItem(0));
    accept();
    return;
  }

  if (0 != ui_->indexTree->currentItem()) {
    accept();
  } else {
    QMessageBox::information(this, tr("Внимание"), msglog::kIsoSelect,
                             tr("Закрыть"));
  }
}

void AddIsolineWidget::slotIndexChanged()
{
  QTreeWidgetItem* item = 0;
  item = ui_->indexTree->currentItem();
  if( 0 == item ){
    return;
  }
  ui_->nameEdit->setText(item->text(1));
}

void AddIsolineWidget::slotFindIsoline(const QString& text)
{
  ui_->indexTree->clear();
  if (nullptr == response_) {
    return;
  }

  if (text.isEmpty()) {
    for (int i = 0; i < response_->parameter_size(); i++) {
      const meteo::sprinf::MeteoParameter& p = response_->parameter(i);
      QTreeWidgetItem* item = new QTreeWidgetItem(ui_->indexTree);
      item->setText(0, QString::number(p.bufr()));
      item->setText(1, QString::fromStdString(p.name()));
    }
  } else {
    for (int i = 0; i < response_->parameter_size(); i++) {
      const meteo::sprinf::MeteoParameter& p = response_->parameter(i);
      QString name = QString::fromStdString(p.name()).toUpper();
      if (name.contains(text.toUpper())) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui_->indexTree);
        item->setText(0, QString::number(p.bufr()));
        item->setText(1, QString::fromStdString(p.name()));
      }
    }
  }
}

void AddIsolineWidget::setOptions(const QString& options)
{
  Q_UNUSED(options);
}

int AddIsolineWidget::index() const
{
  QTreeWidgetItem* item = 0;
  item = ui_->indexTree->currentItem();
  if( 0 == item ){
    return -1;
  }
  return item->text(0).toInt();
}

QString AddIsolineWidget::name() const
{
  if( false == ui_->nameEdit->text().isEmpty() ){
    return ui_->nameEdit->text();
  }else{
    QTreeWidgetItem* item = ui_->indexTree->currentItem();
    if( 0 == item ){
      return QString();
    }
    return item->text(1);
  }
}

TreeWidget::TreeWidget(QWidget* parent) :
  QTreeWidget(parent)
{
  setRootIsDecorated(false);
}

void TreeWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
  const QPoint clickedPosition    = ev->pos();
  const QRect itemRectangle       = visualItemRect( itemAt( clickedPosition ) );
  const int iconOffset            = itemRectangle.height() - iconSize().height();
  int col = columnAt(clickedPosition.x());
  QTreeWidgetItem* item = itemAt(clickedPosition);
  int  viewportPosition = columnViewportPosition(col) ;
  QRect iconRectangle;
  iconRectangle.setTopLeft( itemRectangle.topLeft() + QPoint( iconOffset/2 + viewportPosition, iconOffset/2 ) );
  iconRectangle.setWidth( iconSize().width() );
  iconRectangle.setHeight( iconSize().height() );
  if ( iconRectangle.contains( clickedPosition ) )
  {
    emit iconDoubleClicked(item, col, true);
  }else{
    emit iconDoubleClicked(item, col, false);
    QTreeWidget::mouseDoubleClickEvent( ev );
  }
}


ComboBoxItem::ComboBoxItem(QTreeWidgetItem *item, int column, int height)
{
  if (0 != item) {
    setParent(item->treeWidget());
  }

  setStyleSheet("QComboBox {	   \
      border: 0px solid gray;		   \
      padding: 1px 18px 1px 3px;	   \
      min-width: 6em;				\
    }						\
    QComboBox::drop-down {			\
      width: 15px;				\
      border-left-width: 0px;			\
    }							\
    QComboBox::down-arrow {					\
      image: url(:/meteo/icons/arrow/ctrlarrowdown.png);	\
  }");

  
  // QPalette palette;
  // QBrush brush(Qt::white);
  // brush.setStyle(Qt::SolidPattern);
  // palette.setBrush(QPalette::Active, QPalette::Base, brush);
  // palette.setBrush(QPalette::Active, QPalette::Window, brush);
  // palette.setBrush(QPalette::Active, QPalette::Button, brush);
  // palette.setBrush(QPalette::Active, QPalette::Shadow, brush);

  // palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
  // palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
  // palette.setBrush(QPalette::Inactive, QPalette::Button, brush);
  // palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush);

  // palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
  // palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
  // palette.setBrush(QPalette::Disabled, QPalette::Button, brush);
  // palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush);

  // setPalette(palette);
  //setAutoFillBackground(true);

  item_ = item;
  column_ = column;
  setFrame(false);
  setFixedHeight(height);
}

void ComboBoxItem::changeItem(int index)
{
  if(index >=0 && 0 != item_) {
    item_->setData(column_, Qt::UserRole, itemText(index));
  }
}

void ComboBoxItem::fill(const QStringList& values, const QString& current)
{
  int curIdx = 0;
  for (int idx = 0; idx < values.size(); idx++) {
    addItem(values.at(idx));
    if (values.at(idx) == current) {
      curIdx = idx;
    }
  }
  setCurrentIndex(curIdx);

  QObject::connect(this, SIGNAL(activated(int)), SLOT(changeItem(int)));
}

}
}

