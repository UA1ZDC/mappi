#include "displaysettingseditdialog.h"
#include "ui_displaysettingseditdialog.h"

#include <meteo/commons/ui/map/commongrid.h>
#include <meteo/commons/ui/map/view/mapscene.h>
#include <meteo/commons/ui/map/view/mapview.h>
#include <meteo/commons/ui/wrb/map/coordrequestaction.h>

Q_DECLARE_METATYPE( ::meteo::GeoPoint )

namespace meteo {

DisplaySettingsEditDialog::DisplaySettingsEditDialog(QWidget *parent) :
  QDialog(parent),
  ui_(new Ui::DisplaySettingsEditDialog)
{
  ui_->setupUi(this);

  coordUpdateOn_ = false;

  ui_->cityMarkComboBox->addItem( QObject::tr("Без знака"), QVariant(0) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kCircleCityMark)), "", QVariant(1) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kCircleFilledCityMark)), "", QVariant(2) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kSquareCityMark)), "", QVariant(3) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kSquareFilledCityMark)), "", QVariant(4) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kStarCityMark)), "", QVariant(5) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kStarFilledCityMark)), "", QVariant(6) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kAirCityMark)), "", QVariant(7) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kFlagCityMark)), "", QVariant(8) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kFlagRedCityMark)), "", QVariant(9) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kFlagRectRedCityMark)), "", QVariant(10) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kFlagBlueCityMark)), "", QVariant(11) );
  ui_->cityMarkComboBox->addItem( QIcon(map::cityMarkIconByIndex(map::proto::kFlagRectBlueCityMark)), "", QVariant(12) );
  

  QMap <Position, bool> aligns;
  aligns.insert(kTopCenter, true);
  aligns.insert(kTopLeft, true);
  aligns.insert(kTopRight, true);
  ui_->fontWidget->setAllowedAligns(aligns);

  setWindowTitle(QObject::tr("Настройки населенного пункта"));

  ui_->cityNameLineEdit->hide();//по умолчанию все, что относится к городу - спрятано и показывется функцией setCitySettings()
  ui_->cityNameLabel->hide();
  ui_->geoPointEditor->hide();
  ui_->addPushButton->hide();

  ui_->fontWidget->setSpecialVisible(false);
  ui_->fontWidget->setSpecFontEnabled(false);

  connect(ui_->acceptPushButton, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui_->cancelPushButton, SIGNAL(clicked()), this, SLOT(reject()));

  connect(ui_->visibilityComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotToggleWidgetsVisibility()));

  // connect(ui_->geoPointEditor, SIGNAL(requestCoord(bool)), SLOT(slotRequestCoordChnged(bool)) ); NOTE: такого сигнала не существует и вроде как работает и без этого слота
  connect( qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(slotFocusWidgetChanged(QWidget*,QWidget*)), Qt::UniqueConnection );
}

DisplaySettingsEditDialog::~DisplaySettingsEditDialog()
{
  delete ui_;
}

void DisplaySettingsEditDialog::setCitySettings(map::proto::CitySetting citySetting)
{
  cs_.CopyFrom(citySetting);

  if (citySetting.has_geopoint()){
    GeoPointPb pbgp = citySetting.geopoint();
    GeoPoint gp;
    gp = pbgeopoint2geopoint(pbgp);
    ui_->geoPointEditor->setCoord(gp);
  }
  else return;

  if (citySetting.has_title()){
    ui_->cityNameLineEdit->setText(QString::fromStdString(citySetting.title()));
  }
  else {
    return;//если нет координат или названия, то это какой-то неправильный город
  }

  ui_->cityNameLineEdit->show();
  ui_->cityNameLabel->show();
  ui_->geoPointEditor->show();
}

void DisplaySettingsEditDialog::setDisplaySetting(map::proto::DisplaySetting displaySetting)
{
  ds_.CopyFrom(displaySetting);

  if ( displaySetting.has_city_mode() ){
    QString titleStr = QObject::tr("Настройки отображения для группы \"");

    switch (displaySetting.city_mode()) {
      case map::proto::Capital:
        titleStr += QObject::tr("Столицы\"");
        break;
      case map::proto::RussianCapital:
        titleStr += QObject::tr("Столица РФ\"");
        break;
      case map::proto::Regional:
        titleStr += QObject::tr("Областные центры\"");
        break;
      case map::proto::City:
        titleStr += QObject::tr("Города\"");
        break;
      case map::proto::Village:
        titleStr += QObject::tr("Села\"");
        break;
      case map::proto::Location:
        titleStr += QObject::tr("Свои пункты\"");
        break;
      default:
        break;
    }
  }

  if ( displaySetting.has_is_default() )
    if ( displaySetting.is_default() == true )
      setWindowTitle(QObject::tr("Настройки отображения по умолчанию"));

  //тут не написаны ветки else в расчете на то, что передаваемый извне displaySetting учтет все наследуемые и умолчальные параметры и будет полностью (!) укомплектован
  if ( displaySetting.has_generalization_level() )
    ui_->generalizationLevelSpinBox->setValue(displaySetting.generalization_level());

  if ( displaySetting.has_priority() )
    ui_->prioritySpinBox->setValue(displaySetting.priority());

  if ( displaySetting.has_visible() ) {
    int vIndex;
    if ( displaySetting.visible() == true )
      vIndex = 0;
    else
      vIndex = 1;
    ui_->visibilityComboBox->setCurrentIndex(vIndex);
  }

  if ( displaySetting.has_city_mark() == true ){
    int cityMarkIndex = static_cast <int> (displaySetting.city_mark());
    ui_->cityMarkComboBox->setCurrentIndex(cityMarkIndex);
  }

  if ( displaySetting.has_city_scale() == true ){
    ui_->cityScaleSpinBox->setValue(displaySetting.city_scale());
  }   

  if ( displaySetting.has_always_visible() == true &&
       displaySetting.always_visible() == true &&
       displaySetting.has_visible() == true &&
       displaySetting.visible() == true )
    ui_->alwaysVisibleCheckBox->setChecked(true);
  else
    ui_->alwaysVisibleCheckBox->setChecked(false);

  if ( (displaySetting.has_font()) &&
       (displaySetting.has_pen()) &&
       (displaySetting.has_position()) ){
    Font font = displaySetting.font();
    QFont qFont = font2qfont(font);

    Position pos = displaySetting.position();

    Pen pen = displaySetting.pen();
    QPen qPen = pen2qpen(pen);
    QRgb colorRgb = qPen.color().rgb();

    ui_->fontWidget->setFont(qFont, pos, colorRgb);
  }
}

QString DisplaySettingsEditDialog::getCityName()
{
  return ui_->cityNameLineEdit->text();
}

map::proto::CitySetting DisplaySettingsEditDialog::getCitySetting()
{
  cs_.set_title(ui_->cityNameLineEdit->text().toStdString().c_str());

  GeoPoint gp = ui_->geoPointEditor->coord();
  GeoPointPb pbgp = geopoint2pbgeopoint(gp);
  cs_.mutable_geopoint()->CopyFrom(pbgp);

  return cs_;
}

map::proto::DisplaySetting DisplaySettingsEditDialog::getDisplaySetting()
{
  //if ( ui->cityNameLineEdit->isVisible() ) //&& //если мы возвращаем настройки для города
  //ds.has_city_title() )
  //ds.set_city_index(ui->cityNameLineEdit->text().toStdString().c_str());

  ds_.set_generalization_level(ui_->generalizationLevelSpinBox->value());
  ds_.set_priority(ui_->prioritySpinBox->value());

  if ( ui_->visibilityComboBox->currentIndex() == 0 )
    ds_.set_visible(true);
  else ds_.set_visible(false);

  if ( ui_->alwaysVisibleCheckBox->isChecked() )
    ds_.set_always_visible(true);
  else
    ds_.clear_always_visible();

  QFont qFont = ui_->fontWidget->font();
  QPen qPen(QColor(ui_->fontWidget->color()));
  Position pos = static_cast <Position> (ui_->fontWidget->align());

  Font *fontToSet = ds_.mutable_font();
  fontToSet->CopyFrom(qfont2font(qFont));

  Pen pen = qpen2pen(qPen);
  ds_.mutable_pen()->set_color(pen.color());

  ds_.set_position(pos);

  map::proto::CityMark cityMark = static_cast <map::proto::CityMark> (ui_->cityMarkComboBox->itemData(ui_->cityMarkComboBox->currentIndex(), Qt::UserRole).toInt());
  ds_.set_city_mark(cityMark);

  ds_.set_city_scale(ui_->cityScaleSpinBox->value());
  
  return ds_;
}

void DisplaySettingsEditDialog::slotToggleWidgetsVisibility()
{
  if ( ui_->visibilityComboBox->currentIndex() == 0 ) {
    ui_->label->setVisible(true);
    ui_->label_2->setVisible(true);
    ui_->generalizationLevelSpinBox->setVisible(true);
    ui_->prioritySpinBox->setVisible(true);
    ui_->citymarkLabel->setVisible(true);
    ui_->cityMarkComboBox->setVisible(true);
    ui_->cityScaleLabel->setVisible(true);
    ui_->cityScaleSpinBox->setVisible(true);
    ui_->alwaysVisibleCheckBox->setVisible(true);
  }
  else {
    ui_->label->setVisible(false);
    ui_->label_2->setVisible(false);
    ui_->generalizationLevelSpinBox->setVisible(false);
    ui_->prioritySpinBox->setVisible(false);
    ui_->citymarkLabel->setVisible(false);
    ui_->cityMarkComboBox->setVisible(false);
    ui_->cityScaleLabel->setVisible(false);
    ui_->cityScaleSpinBox->setVisible(false);
    ui_->alwaysVisibleCheckBox->setVisible(false);
  }
}

void DisplaySettingsEditDialog::slotRequestCoordChnged(bool b)
{
  coordUpdateOn_ = b;
}

void DisplaySettingsEditDialog::slotFocusWidgetChanged(QWidget* old, QWidget* now)
{
  Q_UNUSED( old );

  map::MapView* view = qobject_cast<map::MapView*>(now);
  if ( 0 == view ) {
    return;
  }

  map::MapScene* scene = view->mapscene();
  if ( 0 == scene ) {
    return;
  }

  map::Action* act = scene->getAction(map::CoordRequestAction::kName);
  map::CoordRequestAction* a = qobject_cast<map::CoordRequestAction*>(act);
  if ( 0 == a ) {
    map::CoordRequestAction* a = new map::CoordRequestAction(scene);
    scene->addAction(a);
  }

  connect( a, SIGNAL(coordinate(GeoPoint)), SLOT(slotUpdateCoord(GeoPoint)) );
}

void DisplaySettingsEditDialog::slotUpdateCoord(const GeoPoint& coord)
{
  if ( coordUpdateOn_ ) {
    ui_->geoPointEditor->setCoord(coord);
  }
}

} // meteo
