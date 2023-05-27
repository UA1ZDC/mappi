#include "selectwidget.h"
#include "ui_select.h"

#include <qpushbutton.h>

#include <cross-commons/debug/tlog.h>

#include <meteo/commons/global/global.h>

#include <meteo/commons/global/weatherloader.h>
#include <meteo/commons/proto/sprinf.pb.h>

namespace {

const QMap<meteo::surf::DataType, QString>& dataTypesName()
{
  static QMap<meteo::surf::DataType, QString> types;
  if (types.isEmpty() == true) {
    meteo::sprinf::GmiTypes proto = meteo::global::kGmiTypes();
    for( int i = 0; i < proto.gmi_size(); i++ ){
      if( false == proto.gmi(i).total() ){
        meteo::surf::DataType data = static_cast<meteo::surf::DataType>(proto.gmi(i).type());
        types.insert(data, QString::fromStdString( proto.gmi(i).name() ));
      }
    }
  }
  return types;
}

const QList<int>& basicTypeLevels()
{
  static QList<int> btl;
  if( true == btl.isEmpty() ){
    btl.append(1);
    btl.append(6);
    btl.append(7);
    btl.append(100);
    btl.append(101);
    btl.append(102);
    btl.append(105);
    btl.append(160);
    btl.append(200);
  }
  return btl;
}

const QStringList& basicLayers()
{
  static QStringList bl;
  if( true == bl.isEmpty() ){
    bl.append("0-0");
    bl.append("0-1000");
    bl.append("1000-2000");
    bl.append("2000-3000");
    bl.append("3000-4000");
    bl.append("4000-5000");
    bl.append("5000-6000");
    bl.append("6000-7000");
    bl.append("7000-8000");
    bl.append("8000-9000");
    bl.append("9000-10000");
    bl.append("10000-11000");
  }
  return bl;
}

const QStringList& sigwxLayers()
{
  static QStringList bl;
  if ( true == bl.isEmpty() ) {
    bl.append("0-0");
    bl.append("3050-13720");
    bl.append("3050-19200");
    bl.append("7620-19200");
  }
  return bl;
}

const QStringList& basicTransparency()
{
  static QStringList bt;
  if( true == bt.isEmpty() ){
    bt.append("0");
    bt.append("10");
    bt.append("25");
    bt.append("50");
    bt.append("75");
    bt.append("100");
  }
  return bt;
}

const QMap<meteo::surf::DataType, QString>& basicDataTypeGroup()
{
  static QMap<meteo::surf::DataType, QString> gmi_group;
  if (gmi_group.isEmpty() == true) {
    meteo::sprinf::GmiTypes proto = meteo::global::kGmiTypes();
    for( int i = 0; i < proto.gmi_size(); i++ ){
      if( true == proto.gmi(i).total() ){
        meteo::surf::DataType data = static_cast<meteo::surf::DataType>(proto.gmi(i).type());
        gmi_group.insert(data, QString::fromStdString( proto.gmi(i).name() ));
      }
    }
  }
  return gmi_group;
}

const QMap<meteo::map::proto::ViewMode, QString>& basicViewMode()
{
  static QMap<meteo::map::proto::ViewMode, QString> vm;
  if( true == vm.isEmpty() ){
    vm.insert(meteo::map::proto::kGradient,QString::fromUtf8("Градиентная заливка"));
    vm.insert(meteo::map::proto::kIsoGrad, QString::fromUtf8("Изолинии и градиентная заливка"));
    vm.insert(meteo::map::proto::kIsoline, QString::fromUtf8("Изолинии"));
    vm.insert(meteo::map::proto::kRadar,   QString::fromUtf8("Радары"));
    vm.insert(meteo::map::proto::kPuanson, QString::fromUtf8("Пуансоны"));
    vm.insert(meteo::map::proto::kSigwx,   QString::fromUtf8("Опасные явления"));
  }
  return vm;
}

QList<meteo::surf::DataType> dataTypeByGroup(meteo::surf::DataType gr)
{
  QList<meteo::surf::DataType> group_data_type;
  meteo::sprinf::GmiTypes proto = meteo::global::kGmiTypes();
  for( int i = 0; i < proto.gmi_size(); i++ ){
    if( false == proto.gmi(i).total() ){
      meteo::surf::DataType parent = static_cast<meteo::surf::DataType>(proto.gmi(i).parent());
      meteo::surf::DataType data = static_cast<meteo::surf::DataType>(proto.gmi(i).type());
      if( parent == gr ){
        group_data_type.append(data);
      }
    }
  }
  return group_data_type;
}

}

SelectWidget::SelectWidget(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::Select)
{
  ui_->setupUi(this);
  ui_->buttonBox->button(QDialogButtonBox::Ok)->setText(QObject::tr("Принять"));
  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setText(QObject::tr("Отмена"));
  ui_->buttonBox->button(QDialogButtonBox::Ok)->setMinimumHeight(32);
  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setMinimumHeight(32);
  setStyleSheet(QString("QComboBox,QLineEdit,QLabel{min-height:20px}"));
  connect(ui_->typeLevelBox, SIGNAL(currentIndexChanged(QString)), SLOT(slotTypeLevelChange(QString)));
  connect(ui_->viewModeBox, SIGNAL(currentIndexChanged(int)), SLOT(slotViewModeChange()));
  connect(ui_->groupTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(slotGroupChange(QTreeWidgetItem*)));
  connect(ui_->dataTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(slotCheckDataType(QTreeWidgetItem*)));
  connect(ui_->groupTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(slotCheckGroup(QTreeWidgetItem*)));
  getTypeLevels();
  createList();
  for( const QString& gr : ::basicDataTypeGroup() ){
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->groupTree);
    item->setSizeHint(0, QSize(0, 20));
    item->setText(0, gr);
    item->setToolTip(0, gr);
  }

}

int SelectWidget::level() const
{
  return ui_->levelBox->currentText().toInt();
}

int SelectWidget::type_level() const
{
  QString typeLevelText = ui_->typeLevelBox->currentText();
  if( false == typelevels_.values().contains(typeLevelText) ){
    return -1;
  }
  int type_level = typelevels_.key(typeLevelText);
  return type_level;
}

QString SelectWidget::data_type() const
{
  if( meteo::map::proto::kPuanson == type() ){
    QString checked_types;
    for( const meteo::surf::DataType& type : checked_ ){
      checked_types.append(QString::number(type)).append(";");
    }
    return checked_types;
  }
  else{
    return QString();
  }
}

QString SelectWidget::templateName()
{
  return getTemplates(type()).key(templateNameHuman());
}

meteo::map::proto::ViewMode SelectWidget::type() const
{
  return ::basicViewMode().key(ui_->viewModeBox->currentText());
}

QString SelectWidget::templateNameHuman() const
{
  return ui_->templateBox->currentText();
}

QString SelectWidget::levelHuman() const
{
   return ui_->levelBox->currentText();
}

QString SelectWidget::typeLevelHuman() const
{
  return ui_->typeLevelBox->currentText();
}

QString SelectWidget::hh() const
{
  return ui_->layerBox->currentText();
}

int SelectWidget::transparency() const
{
  return ui_->transparencyBox->currentText().toInt();
}

meteo::map::proto::WeatherLayer SelectWidget::layer()
{
  meteo::map::proto::WeatherLayer l;
  l.set_mode(type());
  l.set_template_name(templateName().toStdString());
  l.set_type_level(type_level());
  l.set_level(level());
  for(auto data_type : checked_ ) {
    l.add_data_type(data_type);
  }
  return l;
}

QString SelectWidget::levelHuman(int level, int type_level)
{
  QMap< int, QStringList > levels = meteo::global::kStandardLevels();
  if( false == levels.keys().contains(type_level) ){
    return QString::number(level);
  }
  QStringList level_human = levels.value(type_level);
  if( level_human.isEmpty() ){
    return meteo::global::kLevelTypes().value(type_level);
  }
  return QString::number(level);
}

QString SelectWidget::typeLevelHuman(int type_level)
{
 return meteo::global::kLevelTypes().value(type_level);
}

void SelectWidget::fillForm(const meteo::map::proto::WeatherLayer& layer)
{
  QMap<QString, QString> templates = getTemplates(layer.mode());

  ui_->viewModeBox->setCurrentIndex(ui_->viewModeBox->findText(::basicViewMode().value(layer.mode())));

  if( meteo::map::proto::kRadar == layer.mode() ){
    ui_->layerBox->clear();
    ui_->layerBox->addItems(::basicLayers());
    ui_->layerBox->setCurrentIndex(ui_->layerBox->findText(QString("%1-%2").arg(layer.h1()).arg(layer.h2())));
  }
  else if ( meteo::map::proto::kSigwx == layer.mode() ) {
    ui_->layerBox->clear();
    ui_->layerBox->addItems(::sigwxLayers());
    ui_->layerBox->setCurrentIndex(ui_->layerBox->findText(QString("%1-%2").arg(layer.h1()).arg(layer.h2())));
  }

  QString t_name = QString::fromStdString(layer.template_name());
  QString template_human = templates.value(t_name);
  ui_->templateBox->setCurrentIndex(ui_->templateBox->findText(template_human));

  QString type_level_human = typelevels_.value(layer.type_level());
  ui_->typeLevelBox->setCurrentIndex(ui_->typeLevelBox->findText(type_level_human));

  ui_->transparencyBox->setCurrentIndex(ui_->transparencyBox->findText(QString::number(layer.transparency())));

  QMap< int, QStringList > levels = meteo::global::kStandardLevels();
  if( true == levels.keys().contains(layer.type_level()) ){

    QStringList level_human = levels.value(layer.type_level());
    if( 1 == level_human.count() ){
      ui_->levelBox->setCurrentIndex(0);
    }
    else{
      int index = level_human.indexOf(QString::number(layer.level()));
      if( -1 != index ){
        ui_->levelBox->setCurrentIndex(index);
      }
    }
  }
  checked_.clear();
  for( int i = 0; i < ui_->groupTree->topLevelItemCount(); i++ ){
    QTreeWidgetItem* item = ui_->groupTree->topLevelItem(i);
    QList<meteo::surf::DataType> datatypelist = ::dataTypeByGroup(::basicDataTypeGroup().key(item->text(0)));
    bool ok = false;
    for( const auto& datatype: datatypelist ){
      if( true == checkDataType(layer, datatype) ){
        item->setCheckState(0, Qt::Checked);
        checked_.append(datatype);
        ok = true;
      }
    }
    if( false == ok ){
      item->setCheckState(0, Qt::Unchecked);
    }
  }
  ui_->groupTree->setCurrentItem(ui_->groupTree->topLevelItem(0));
  auto item = ui_->groupTree->currentItem();
  if ( nullptr != item) {
    slotGroupChange(item);
  }
}

void SelectWidget::clearForm()
{
  ui_->templateBox->setCurrentIndex(0);
  ui_->typeLevelBox->setCurrentIndex(0);
  ui_->levelBox->setCurrentIndex(0);
  for( int i = 0; i < ui_->groupTree->topLevelItemCount(); i++ ){
    ui_->groupTree->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
  }
  for( int i = 0; i < ui_->dataTree->topLevelItemCount(); i++ ){
    ui_->dataTree->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
  }
  checked_.clear();
}

void SelectWidget::createList()
{
  ui_->typeLevelBox->addItems(typelevels_.values());
  ui_->layerBox->addItems(::basicLayers());
  ui_->viewModeBox->addItems(::basicViewMode().values());
  ui_->transparencyBox->addItems(::basicTransparency());
}

void SelectWidget::getTypeLevels()
{
  for( int key : meteo::global::kLevelTypes().keys() ){
    if( ::basicTypeLevels().contains(key) ){
      typelevels_.insert(key, meteo::global::kLevelTypes().value(key));
    }
  }
}

QMap<QString, QString> SelectWidget::getTemplates(meteo::map::proto::ViewMode mode)
{
  QMap<QString, QString> templates;
  switch( mode ){
    case meteo::map::proto::kGradient:
    case meteo::map::proto::kIsoGrad:
    case meteo::map::proto::kIsoline: {
      QMap< QString, meteo::map::proto::FieldColor > library = meteo::map::WeatherLoader::instance()->isolibrary();
      for( const QString& str : library.keys() ){
        templates.insert(str, QString::fromStdString(library[str].name()));
      }
      break;
    }
    case meteo::map::proto::kRadar: {
      QMap< QString, meteo::map::proto::RadarColor > library = meteo::map::WeatherLoader::instance()->radarlibrary();
      for( const QString& str : library.keys() ){
        templates.insert(str, QString::fromStdString(library[str].name()));
      }
      break;
    }
    case meteo::map::proto::kPuanson: {
      QMap< QString, meteo::puanson::proto::Puanson > library = meteo::map::WeatherLoader::instance()->punchlibrary();
      for( const QString& str : library.keys() ){
        templates.insert(str, QString::fromStdString(library[str].name()));
      }
      break;
    }
    default:
      break;
  }
  return templates;
}

bool SelectWidget::checkDataType(const meteo::map::proto::WeatherLayer& layer, meteo::surf::DataType datatype)
{
  for( int i = 0; i < layer.data_type_size(); i++ ){
    if( datatype == layer.data_type(i) ){
      return true;
    }
  }
  return false;
}

QString SelectWidget::groupByDataType(meteo::surf::DataType datatype) const
{
  QString group_name;
  meteo::surf::DataType parent = meteo::surf::kUnknownDataType;
  meteo::sprinf::GmiTypes proto = meteo::global::kGmiTypes();
  for( int i = 0; i < proto.gmi_size(); i++ ){
    if( false == proto.gmi(i).total() ){
      meteo::surf::DataType data = static_cast<meteo::surf::DataType>(proto.gmi(i).type());
      if( data == datatype ){
        parent = static_cast<meteo::surf::DataType>(proto.gmi(i).parent());
      }
    }
  }
  if( meteo::surf::kUnknownDataType != parent ){
    QMap<meteo::surf::DataType, QString>  list = ::basicDataTypeGroup();
    if( true == list.keys().contains(parent) ){
      group_name = list.value(parent);
    }
  }
  return group_name;
}

void SelectWidget::slotTypeLevelChange(const QString& text)
{
  ui_->levelBox->clear();
  int type_level;
  if( typelevels_.values().contains(text) ){
    type_level = typelevels_.key(text);
  }
  QMap< int, QStringList > levels = meteo::global::kStandardLevels();
  if( false == levels.keys().contains(type_level) ){
    return;
  }
  QStringList level_human = levels.value(type_level);
  if( true == level_human.isEmpty() ){
    level_human.append(meteo::global::kLevelTypes().value(type_level));
  }
  ui_->levelBox->addItems(level_human);
}

void SelectWidget::slotViewModeChange()
{
  trc;
  ui_->templateBox->clear();
  meteo::map::proto::ViewMode mode = ::basicViewMode().key(ui_->viewModeBox->currentText());
  ui_->templateBox->addItems(getTemplates(mode).values());
  switch( mode ){
    case meteo::map::proto::kSigwx:
      ui_->selectDataType->setVisible(false);
      ui_->layerBox->setVisible(true);
      ui_->layerLabel->setVisible(true);
      ui_->typeLevelBox->setVisible(false);
      ui_->typeLevelLabel->setVisible(false);
      ui_->levelBox->setVisible(false);
      ui_->levelLabel->setVisible(false);
      ui_->transparencyBox->setVisible(false);
      ui_->transparencyLabel->setVisible(false);
      ui_->label->setVisible(false);
      ui_->templateBox->setVisible(false);
      ui_->layerBox->clear();
      ui_->layerBox->addItems(::sigwxLayers());
      break;
    case meteo::map::proto::kGradient:
    case meteo::map::proto::kIsoGrad:
    case meteo::map::proto::kIsoline:
      ui_->selectDataType->setVisible(false);
      ui_->layerBox->setVisible(false);
      ui_->layerLabel->setVisible(false);
      ui_->typeLevelBox->setVisible(true);
      ui_->typeLevelLabel->setVisible(true);
      ui_->levelBox->setVisible(true);
      ui_->levelLabel->setVisible(true);
      ui_->transparencyBox->setVisible(true);
      ui_->transparencyLabel->setVisible(true);
      ui_->label->setVisible(true);
      ui_->templateBox->setVisible(true);
      break;
    case meteo::map::proto::kRadar:
      ui_->selectDataType->setVisible(false);
      ui_->layerBox->setVisible(true);
      ui_->layerLabel->setVisible(true);
      ui_->typeLevelBox->setVisible(false);
      ui_->typeLevelLabel->setVisible(false);
      ui_->levelBox->setVisible(false);
      ui_->levelLabel->setVisible(false);
      ui_->transparencyBox->setVisible(false);
      ui_->transparencyLabel->setVisible(false);
      ui_->label->setVisible(true);
      ui_->templateBox->setVisible(true);
      ui_->layerBox->clear();
      ui_->layerBox->addItems(::basicLayers());
      break;
    case meteo::map::proto::kPuanson:
      ui_->selectDataType->setVisible(true);
      ui_->layerBox->setVisible(false);
      ui_->layerLabel->setVisible(false);
      ui_->typeLevelBox->setVisible(true);
      ui_->typeLevelLabel->setVisible(true);
      ui_->levelBox->setVisible(true);
      ui_->levelLabel->setVisible(true);
      ui_->transparencyBox->setVisible(false);
      ui_->transparencyLabel->setVisible(false);
      ui_->label->setVisible(true);
      ui_->templateBox->setVisible(true);
      break;
    default:
      break;
  }
}

void SelectWidget::slotGroupChange(QTreeWidgetItem* item)
{
  ui_->dataTree->clear();
  meteo::surf::DataType gr = ::basicDataTypeGroup().key(item->text(0));
  QList<meteo::surf::DataType> list = ::dataTypeByGroup(gr);
  for( const meteo::surf::DataType& type : list ){
    QTreeWidgetItem* item = new QTreeWidgetItem(ui_->dataTree);
    QString text = ::dataTypesName().value(type);
    item->setText(0, text);
    item->setToolTip(0, text);
    item->setSizeHint(0, QSize(0, 20));
    if( true == checked_.contains(type) ){
      item->setCheckState(0, Qt::Checked);
    }
    else{
      item->setCheckState(0, Qt::Unchecked);
    }
  }
}

void SelectWidget::slotCheckDataType(QTreeWidgetItem* item)
{
  if( 0 == item ){
    return;
  }
  meteo::surf::DataType type = ::dataTypesName().key(item->text(0));

  if( Qt::Checked == item->checkState(0) ){
    checked_.append(type);
  }
  else{
    checked_.removeAll(type);
  }
  int count = 0;
  for( int i = 0; i < ui_->dataTree->topLevelItemCount(); i++ ){
    if( ui_->dataTree->topLevelItem(i)->checkState(0) == Qt::Checked ){
      count++;
    }
  }
  QString group = groupByDataType(type);
  for( int i = 0; i < ui_->groupTree->topLevelItemCount(); i++ ){
    QTreeWidgetItem* item2 = ui_->groupTree->topLevelItem(i);
    if( item2->text(0) == group ){
      if( count > 0 ){
        item2->setCheckState(0, Qt::Checked);
      }
      else{
        item2->setCheckState(0, Qt::Unchecked);
      }
    }
  }
}

void SelectWidget::slotCheckGroup(QTreeWidgetItem* item)
{
  meteo::surf::DataType gr = ::basicDataTypeGroup().key(item->text(0));
  QList<meteo::surf::DataType> list = dataTypeByGroup(gr);
  for ( const meteo::surf::DataType& type : list ){
    if( item->checkState(0) == Qt::Checked ){
      QList<meteo::surf::DataType> datatypelist = ::dataTypeByGroup(::basicDataTypeGroup().key(item->text(0)));
      bool ok = false;

      for( int i = 0; i < ui_->dataTree->topLevelItemCount(); i++ ){
        if( Qt::Checked == ui_->dataTree->topLevelItem(i)->checkState(0) ){
          ok = true;
          break;
        }
      }
      if( false == checked_.contains(type) && false == ok ){
        checked_.append(type);
      }
    }
    else if( item->checkState(0) == Qt::Unchecked ){
      checked_.removeAll(type);
    }
  }
  if( ui_->groupTree->currentItem() != item ){
    ui_->groupTree->setCurrentItem(item);
  }
  else{
    slotGroupChange(item);
  }
}
