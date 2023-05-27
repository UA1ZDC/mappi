#include "thematicsettingwidget.h"
#include "ui_thematicsettingwidget.h"
#include "exprguidepaintwidget.h"
#include "thematiceditor.h"
#include "expreditor.h"

#include <qsettings.h>
#include <qmessagebox.h>

#include <meteo/commons/ui/custom/gradienteditor.h>

#include <mappi/settings/mappisettings.h>

namespace mappi {

static QMap<mappi::conf::ThemType, QString> kThemTypes;
//static const QString kThematicsConf = "/thematics.conf";
static const QString kSettings      = QDir::homePath() + "/.meteo/mappi/settings.ini";

// static const QPixmap kEnableThem    = QPixmap(":/mappi/icons/restart_green.png");
// static const QPixmap kDisableThem   = QPixmap(":/mappi/icons/restart_red.png");

ThematicSettingWidget::ThematicSettingWidget(QWidget *parent)
  : SettingWidget(parent)
  , ui_(new Ui::ThematicSettingWidget)
  , thematicEditor_(new ThematicEditor(this))
  , exprEditor_(new ExprEditor(this))
{
  title_ = QObject::tr("Тематическая обработка");
  ui_->setupUi(this);

  kThemTypes.insert(conf::kThemUnk,    "Неизвестный тип");
  kThemTypes.insert(conf::kFalseColor, "Псевдоцвета");
  kThemTypes.insert(conf::kGrayScale,     "Серый");
  kThemTypes.insert(conf::kIndex8,     "Палитра");
  kThemTypes.insert(conf::kCloudMask,  "Маска облачности");
  kThemTypes.insert(conf::kCloudType,  "Тип облачности");
  kThemTypes.insert(conf::kCloudFull,  "Маска облачности (все)");
  kThemTypes.insert(conf::kSnowBorder,  "Граница снега");
  kThemTypes.insert(conf::kIceBorder,  "Граница льда");
  kThemTypes.insert(conf::kSeaTempr,   "Температура поверхности моря");
  kThemTypes.insert(conf::kSurfTempr,  "Температура поверхности суши");
  kThemTypes.insert(conf::kTotalPrecip,  "Общее влагосодержание");
  kThemTypes.insert(conf::kCloudTempr, "Температура верхней границы облачности");
  
  for(auto it : {kRed, kGreen, kBlue })
  {
    QWidget* tmp;
    switch (it) {
      case kRed:   tmp = ui_->redExprWdg;   break;
      case kGreen: tmp = ui_->greenExprWdg; break;
      case kBlue:  tmp = ui_->blueExprWdg;  break;
      default:     tmp = nullptr;           break;
    }
    if(nullptr == tmp) continue;
    QHBoxLayout *layout = new QHBoxLayout(tmp);
    ExprGuidePaintWidget* exprVis_ = new ExprGuidePaintWidget(this);
    exprVis_->setFontSize(12);
    layout->addWidget(exprVis_);
    tmp->setLayout(layout);
    exprsVis_.insert(it, exprVis_);
  }

  QObject::connect(ui_->redBtn,    &QToolButton::clicked, this, &ThematicSettingWidget::slotEditRed);
  QObject::connect(ui_->greenBtn,  &QToolButton::clicked, this, &ThematicSettingWidget::slotEditGreen);
  QObject::connect(ui_->blueBtn,   &QToolButton::clicked, this, &ThematicSettingWidget::slotEditBlue);
  QObject::connect(exprEditor_,    &ExprEditor::complite, this, &ThematicSettingWidget::slotSetExpr);

  QObject::connect(ui_->thematicsTbl, &QTableWidget::currentItemChanged, this, &ThematicSettingWidget::slotChangeThematic);
  QObject::connect(ui_->thematicsTbl, &QTableWidget::cellDoubleClicked, this, &ThematicSettingWidget::slotDoubleClickThematic);
  QObject::connect(ui_->addBtn,       &QPushButton::clicked, [&]() {
    thematic_.Clear();
    thematicEditor_->open(thematic_);
  });
  QObject::connect(ui_->removeBtn,    &QPushButton::clicked, this, &ThematicSettingWidget::slotRemoveThematic);
  QObject::connect(thematicEditor_,   &ThematicEditor::complite, this, &ThematicSettingWidget::slotSaveThematic);

  loadSettings(kSettings);
}
ThematicSettingWidget::~ThematicSettingWidget()
{
  saveSettings(kSettings);
  delete ui_;
}

void ThematicSettingWidget::loadSettings(const QString& filename)
{
  if(false == QFile::exists(filename)) return;
  QSettings settings(filename, QSettings::IniFormat);
  ui_->thematicsTbl->horizontalHeader()->restoreState(settings.value("thematic/table").toByteArray());
  thematicEditor_->restoreGeometry(settings.value("thematic/thematiceditor").toByteArray());
  exprEditor_->restoreGeometry(settings.value("thematic/expreditor").toByteArray());
  loadedSettings_ = true;
}

void ThematicSettingWidget::saveSettings(const QString& filename) const
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.setValue("thematic/table", ui_->thematicsTbl->horizontalHeader()->saveState());
  settings.setValue("thematic/thematiceditor", thematicEditor_->saveGeometry());
}

void ThematicSettingWidget::slotLoadConf()
{
  auto thems = ::mappi::inter::Settings::instance()->thematicsCalc();
  //auto thems = ::mappi::inter::Settings::instance()->thematics();
  for(const auto &them : thems.thematics()) {
    if(them.has_name()) {
      QString themName = QString::fromStdString(them.name());
      thematics_.insert(themName, them);
      if(them.has_type()) {
        switch (them.type()) {
          case conf::kIndex8:
          case conf::kGrayScale:
          case conf::kSeaTempr:
          case conf::kCloudMask:
          case conf::kCloudType:
          case conf::kCloudFull:
          case conf::kSurfTempr:
          case conf::kCloudTempr:
          case conf::kTotalPrecip:
          case conf::kCloudAlt:
          case conf::kSnowBorder:
          case conf::kIceBorder:


            grads_.insert(them.name(), ::mappi::inter::Settings::instance()->palette(them.name()));
          break;
          default: break;
        }
      }
    }
  }

  fillTable();

  if(false == thematics_.isEmpty()) {
    ui_->thematicsTbl->selectRow(0);
    thematic_ = thematics_.first();
    fillContent();
  }

  //emit changed(false);
  Q_EMIT(changed(false));
}
void ThematicSettingWidget::slotSaveConf() const
{
  conf::ThematicProcs thems;
  for(const auto &them : thematics_) {
    thems.add_thematics()->CopyFrom(them);
  }

  ::mappi::inter::Settings::instance()->saveThematics(&thems);
  //meteo::mappi::TMeteoSettings::instance()->save(MnCommon::etcPath() + kThematicsConf, &thems);

  for(auto it = grads_.begin(); it != grads_.end(); ++it) {
    ::mappi::inter::Settings::instance()->setPalette(it.key(), it.value());
  }
  //::mappi::inter::Settings::instance()->save();
  //emit changed(false);
  Q_EMIT(changed(false));
}

void ThematicSettingWidget::fillTable()
{
  int row = 0, rowCount = ui_->thematicsTbl->rowCount();
  //Заполняем таблицу
  for(const auto & them : qAsConst(thematics_))
  {
    if(row >= rowCount) {
      insertRow(ui_->thematicsTbl, them);
      ++rowCount;
    }
    else {
      fillRow(ui_->thematicsTbl, row, them);
    }
    ++row;
  }
  //Удаляем лишние строки
  while (row < rowCount) {
    ui_->thematicsTbl->removeRow(--rowCount);
  }
  //Подбираем ширину столбцов при первом запуске
  if(false == loadedSettings_) {
    ui_->thematicsTbl->resizeColumnsToContents();
    loadedSettings_ = true;
  }
}
void ThematicSettingWidget::fillContent()
{
  exprsVis_.value(kRed  )->setExpr(thematic_.has_red()   ? QString::fromStdString(thematic_.red())   : "");
  exprsVis_.value(kGreen)->setExpr(thematic_.has_green() ? QString::fromStdString(thematic_.green()) : "");
  exprsVis_.value(kBlue )->setExpr(thematic_.has_blue()  ? QString::fromStdString(thematic_.blue())  : "");
  if(thematic_.has_type())
  {
    if(conf::kIndex8 != thematic_.type()&&conf::kGrayScale != thematic_.type()) {
      ui_->greenWdg->setVisible(true);
      ui_->blueWdg->setVisible(true);
      ui_->redLbl->setText(QObject::tr("Красный:"));
    }
    else {
      ui_->greenWdg->setVisible(false);
      ui_->blueWdg->setVisible(false);
      ui_->redLbl->setText(QObject::tr("Индекс:"));
    }
  }
  QSet<QString> satellites;
  for(const auto & it : thematic_.vars()) {
    for(const auto & ch : it.channel()) {
      satellites.insert(QString::fromStdString(ch.satellite()));
    }
  }
  ui_->satelliteLst->clear();
  ui_->satelliteLst->insertItems(0, satellites.values());
}

void ThematicSettingWidget::slotSaveThematic(const Thematic& them)
{
  const QString themName = QString::fromStdString(them.name());
  if(QString::fromStdString(thematic_.name()) != themName)
  {
    if(thematics_.contains(themName)) {
      QMessageBox::critical(this, QObject::tr("Ошибка"), QObject::tr("Тематическая обработка %1 уже существует").arg(themName));
      return;
    }
    thematics_.insert(themName, them);
    int row = ui_->thematicsTbl->rowCount();
    insertRow(ui_->thematicsTbl, them);
    ui_->thematicsTbl->setCurrentCell(row, kName);
  }
  else {
    thematics_[themName].CopyFrom(them);
    for(int row = 0, sz = ui_->thematicsTbl->rowCount(); row < sz; ++row) {
      if(cellText(row, kName) == themName) {
        fillRow(ui_->thematicsTbl, row, them);
        break;
      }
    }
  }
  thematic_.CopyFrom(thematics_.value(themName));
  fillContent();
  thematicEditor_->close();
}

/*!
 * \brief ThematicSettings::slotRemoveThematic - Удалить тематическую выбранную в таблице обработку
 */
void ThematicSettingWidget::slotRemoveThematic()
{
  int row = ui_->thematicsTbl->currentRow();
  auto response = QMessageBox::question(
                    this, QObject::tr("Удаление"),
                    QObject::tr("Вы уверены что хотите удалить тематическую обработку %1?").arg(cellText(row, kName)));
  if(QMessageBox::Yes != response) return;

  QString themName = cellText(row, kName);
  thematics_.remove(themName);
  grads_.remove(themName.toStdString());
  ui_->thematicsTbl->removeRow(row);
  //emit changed(true);
  Q_EMIT(changed(true));
}

/*!
 * \brief ThematicSettings::slotChangedThematic - Сменить тематическую обработку
 * \param current - ячейка связанная с новой тематической обработкой
 */
void ThematicSettingWidget::slotChangeThematic(QTableWidgetItem *current)
{
  if(nullptr == current) return;
  int row = current->row();
  auto them = thematics_.find(cellText(row, kName));
  if(thematics_.end() == them) return;
  thematic_.CopyFrom(them.value());
  fillContent();
}

void ThematicSettingWidget::slotDoubleClickThematic(int row, int col)
{
  if(row >= ui_->thematicsTbl->rowCount()) return;
  if(kEnable == col) {
    thematic_.set_enabled(!thematic_.enabled());
    fillCell(ui_->thematicsTbl->item(row, col), static_cast<Column>(col), thematic_);
  }
  else if(kPalette == col && thematic_.has_type()) {
    switch (thematic_.type()) {
      case conf::kIndex8:
      case conf::kSeaTempr:
      case conf::kCloudMask:
      case conf::kCloudType:
      case conf::kCloudFull:
      case conf::kSurfTempr:
      case conf::kCloudTempr:
      case conf::kGrayScale:
      case conf::kCloudAlt:
      case conf::kSnowBorder:
      case conf::kIceBorder:
      {
        QString themName = QString::fromStdString(thematic_.name());
        meteo::map::GradientEditor* editor =
            new meteo::map::GradientEditor(grads_.value(thematic_.name()), this);
        QObject::connect(editor, &meteo::map::GradientEditor::accepted, [&, themName, editor]() {
          grads_[thematic_.name()] = editor->gradient();
           Q_EMIT(changed(true));
          //emit changed(true);
        });
        editor->show();
        break;
      }
      default:
      break;
    }
  }
  else {
    thematicEditor_->open(thematic_);
  }
}

/* Открыть редактор формул для Красного/Зеленого/Синего канала */
void ThematicSettingWidget::slotEditRed()
{
  exprType_ = kRed;
  openExprEditor(exprsVis_.value(exprType_)->expr());
}
void ThematicSettingWidget::slotEditGreen()
{
  exprType_ = kGreen;
  openExprEditor(exprsVis_.value(exprType_)->expr());
}
void ThematicSettingWidget::slotEditBlue()
{
  exprType_ = kBlue;
  openExprEditor(exprsVis_.value(exprType_)->expr());
}
void ThematicSettingWidget::openExprEditor(const QString& expr)
{
  QVector<conf::ThematicVariable> vars;
  for(const auto &it : thematic_.vars()) {
    vars.append(it);
  }
  exprEditor_->open(expr, vars);
}

/*!
 * \brief ThematicSettings::slotSetExpr - Обновить измененное выражение(формулу)
 * \param expr - Выражение(формула)
 * \param vars - Список аргументов
 */
void ThematicSettingWidget::slotSetExpr(const QString &expr, const QMap<QString, conf::ThematicVariable> &vars)
{
  thematic_.clear_vars();
  for(const auto & it : vars) {
    thematic_.add_vars()->CopyFrom(it);
  }
  switch (exprType_) {
    case kRed:   thematic_.set_red(expr.toStdString());   break;
    case kGreen: thematic_.set_green(expr.toStdString()); break;
    case kBlue:  thematic_.set_blue(expr.toStdString());  break;
    default:                                              break;
  }
  thematics_[QString::fromStdString(thematic_.name())].CopyFrom(thematic_);
  exprsVis_.value(exprType_)->setExpr(expr);
 // emit changed(true);
   Q_EMIT(changed(true));
}

QString ThematicSettingWidget::cellText(int row, Column col)
{
  return ui_->thematicsTbl->item(row, col)->text();
}

/*!
 * \brief ThematicSettings::insertRow - Вставить в таблицу строку с тематической обработкой
 * \param table - Таблица
 * \param them  - Тематическая обработка
 */
void ThematicSettingWidget::insertRow(QTableWidget *table, const Thematic &them)
{
  if(nullptr == table) return;
  int row = table->rowCount();
  table->insertRow(row);
  for(auto cel : { kEnable, kName, kType, kPalette }) {
    table->setItem(row, cel, createCell(cel, them));
  }
}
/*!
 * \brief ThematicSettings::fillRow - Заполнить строку таблицы тематической обработкой
 * \param table - Таблица
 * \param row   - Номер строки
 * \param them  - Тематическая обработка
 */
void ThematicSettingWidget::fillRow(QTableWidget *table, int row, const Thematic &them)
{
  if(nullptr == table) return;
  for(auto cel : { kEnable, kName, kType, kPalette }) {
    table->setItem(row, cel, createCell(cel, them));
  }
}

/*!
 * \brief ThematicSettings::createCell - Создание ячейки
 * \param col   - Тип данных (колонка)
 * \param them  - Тематическая обработка
 * \return созданная ячейка
 */
QTableWidgetItem* ThematicSettingWidget::createCell(Column col, const Thematic& them)
{
  QTableWidgetItem* item = new QTableWidgetItem();
  fillCell(item, col, them);
  return item;
}
/*!
 * \brief ThematicSettings::fillCell - Заполнение ячейки таблицы Тематической обработкой
 * \param item  - Ячейка
 * \param col   - Тип данных (колонка)
 * \param them  - Тематическая обработка
 */
void ThematicSettingWidget::fillCell(QTableWidgetItem *item, Column col, const Thematic &them)
{
  static const QPixmap kEnableThem    = QPixmap(":/mappi/icons/restart_green.png");
  static const QPixmap kDisableThem   = QPixmap(":/mappi/icons/restart_red.png");

  switch (col) {
    case kEnable:
      if(them.has_enabled()) {
        item->setIcon(them.enabled() ? kEnableThem : kDisableThem);
        item->setToolTip(them.enabled() ? QObject::tr("Выполнять") : QObject::tr("Пропускать"));
      }
      else {
        item->setIcon(kDisableThem);
        item->setToolTip(QObject::tr("Пропускать"));
      }
    break;
    case kName:
      if(them.has_name()) {
        item->setText(QString::fromStdString(them.name()));
      }
      if(them.has_description()) {
        item->setToolTip(QString::fromStdString(them.description()));
      }
    break;
    case kType:
      if(them.has_type()) {
        item->setText(kThemTypes.value(them.type()));
        item->setToolTip(item->text());
      }
    break;
    case kPalette:
      if(them.has_type()) {
        switch (them.type()) {
          case conf::kIndex8:
          case conf::kSeaTempr:
          case conf::kCloudMask:
          case conf::kCloudType:
          case conf::kCloudFull:
          case conf::kSurfTempr:
          case conf::kCloudTempr:
          case conf::kGrayScale:
          case conf::kCloudAlt:
          case conf::kSnowBorder:
          case conf::kIceBorder:
            // case conf::kIndex24:
            // case conf::kIndex32:
            item->setIcon(QPixmap(":/mappi/icons/index_palette.xpm"));
          break;
          default: break;
        }
      }
  }
}

} // mappi
