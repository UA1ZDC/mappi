#include "thematiceditor.h"
#include "ui_thematiceditor.h"
#include "exprguidepaintwidget.h"
#include "expreditor.h"

#include <qsettings.h>
#include <qmessagebox.h>
#include <qdir.h>

#include <cross-commons/debug/tlog.h>

namespace mappi {

static const QString kSettings      = QDir::homePath() + "/.meteo/mappi/settings.ini";

static QMap<mappi::conf::ThemType, QString>  kThemTypes;

ThematicEditor::ThematicEditor(QWidget *parent)
  : QDialog(parent)
  , ui_(new Ui::ThematicEditor)
  , exprEditor_(new ExprEditor(this))
{
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


  for(auto it = kThemTypes.begin(); it != kThemTypes.end(); ++it) {
    if(it.key() > 0) {
      ui_->typeCbox->addItem(it.value());
    }
  }

  for(auto it : { kRed, kGreen, kBlue }) {
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
    exprVis_->setFontSize(14);
    layout->addWidget(exprVis_);
    tmp->setLayout(layout);
    exprsVis_.insert(it, exprVis_);
  }

  QObject::connect(ui_->typeCbox,  &QComboBox::currentTextChanged, this, &ThematicEditor::slotChangeType);

  QObject::connect(ui_->redBtn,    &QToolButton::clicked, this, &ThematicEditor::slotEditRed);
  QObject::connect(ui_->greenBtn,  &QToolButton::clicked, this, &ThematicEditor::slotEditGreen);
  QObject::connect(ui_->blueBtn,   &QToolButton::clicked, this, &ThematicEditor::slotEditBlue);
  QObject::connect(exprEditor_,    &ExprEditor::complite, this, &ThematicEditor::slotSetExpr);
  QObject::connect(ui_->cancleBtn, &QPushButton::clicked, this, &ThematicEditor::close);
  QObject::connect(ui_->saveBtn,   &QPushButton::clicked, this, &ThematicEditor::slotSave);

  loadSettings(kSettings);
}

ThematicEditor::~ThematicEditor()
{
  saveSettings(kSettings);
  delete ui_;
}

void ThematicEditor::loadSettings(const QString& filename)
{
  if(false == QFile::exists(filename)) return;
  QSettings settings(filename, QSettings::IniFormat);
  exprEditor_->restoreGeometry(settings.value("thematic/expreditor").toByteArray());
  loadedSettings_ = true;
}
void ThematicEditor::saveSettings(const QString& filename) const
{
  QSettings settings(filename, QSettings::IniFormat);
  settings.setValue("thematic/expreditor", exprEditor_->saveGeometry());
}


void ThematicEditor::open(const Thematic &thematic)
{
  thematic_.CopyFrom(thematic);
  fillEditor();
  show();
}

void ThematicEditor::changeType(::mappi::conf::ThemType type)
{
  ui_->iRedWdg->setVisible(true);
  switch (type) {
    case conf::kIndex8:
    case conf::kGrayScale:
    case conf::kNdvi:

      ui_->greenWdg->setVisible(false);
      ui_->blueWdg->setVisible(false);
      ui_->redLbl->setText(QObject::tr("Индекс:"));
   break;
    case conf::kFalseColor:
      ui_->greenWdg->setVisible(true);
      ui_->blueWdg->setVisible(true);
      ui_->redLbl->setText(QObject::tr("Красный:"));
   break;
    default:
      ui_->greenWdg->setVisible(false);
      ui_->blueWdg->setVisible(false);
      ui_->iRedWdg->setVisible(false);
    break;
  }

}

void ThematicEditor::fillEditor()
{
  ui_->nameEdt->setText(thematic_.has_name() ? QString::fromStdString(thematic_.name()) : "");
  ui_->descriptionEdt->setText(thematic_.has_description() ? QString::fromStdString(thematic_.description()) : "");

  exprsVis_.value(kRed  )->setExpr(thematic_.has_red()   ? QString::fromStdString(thematic_.red())   : "");
  exprsVis_.value(kGreen)->setExpr(thematic_.has_green() ? QString::fromStdString(thematic_.green()) : "");
  exprsVis_.value(kBlue )->setExpr(thematic_.has_blue()  ? QString::fromStdString(thematic_.blue())  : "");

  if(thematic_.has_type()) {
    ui_->typeCbox->setCurrentText(kThemTypes.value(thematic_.type()));
    changeType(thematic_.type());
  }
}

void ThematicEditor::slotSave()
{
  if(ui_->nameEdt->text().isEmpty()) {
    QMessageBox::question(this, QObject::tr("Редактор тематичекой обработки")
                        , QObject::tr("У тематической обработке должно быть имя.")
                        , QMessageBox::Ok);
    return;
  }
  thematic_.set_name(ui_->nameEdt->text().toStdString());
  thematic_.set_type(kThemTypes.key(ui_->typeCbox->currentText()));
  thematic_.set_enabled(true);
  if(conf::kIndex8 == thematic_.type()|| conf::kGrayScale == thematic_.type()) {
    thematic_.release_green();
    thematic_.release_blue();
  }
  thematic_.set_description(ui_->descriptionEdt->text().toStdString());
  Q_EMIT(complite(thematic_));
}

void ThematicEditor::slotChangeType(const QString &type)
{
  auto themType = kThemTypes.key(type);

  changeType(themType);
  thematic_.set_type(themType);
}

/* Открыть редактор формул для Красного/Зеленого/Синего канала */
void ThematicEditor::slotEditRed()
{
  exprType_ = kRed;
  openExprEditor(exprsVis_.value(exprType_)->expr());
}
void ThematicEditor::slotEditGreen()
{
  exprType_ = kGreen;
  openExprEditor(exprsVis_.value(exprType_)->expr());
}
void ThematicEditor::slotEditBlue()
{
  exprType_ = kBlue;
  openExprEditor(exprsVis_.value(exprType_)->expr());
}
void ThematicEditor::openExprEditor(const QString &expr)
{
  QVector<Variable> vars;
  for(const auto& it : thematic_.vars()) {
    vars.append(it);
  }
  exprEditor_->open(expr, vars);
}

/*!
 * \brief ThematicSettings::slotSetExpr - Обновить измененное выражение(формулу)
 * \param expr - Выражение(формула)
 * \param vars - Список аргументов
 */
void ThematicEditor::slotSetExpr(const QString &expr, const QMap<QString, Variable> &vars)
{
  thematic_.clear_vars();
  for(const auto &it : vars) {
    thematic_.add_vars()->CopyFrom(it);
  }
  switch (exprType_) {
  case kRed:   thematic_.set_red(expr.toStdString());   break;
  case kGreen: thematic_.set_green(expr.toStdString()); break;
  case kBlue:  thematic_.set_blue(expr.toStdString());  break;
  default:                                              break;
  }
  exprsVis_.value(exprType_)->setExpr(expr);
}

} // mappi
