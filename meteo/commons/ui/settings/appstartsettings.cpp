#include "appstartsettings.h"
#include "ui_appstartsettings.h"

#include <cross-commons/app/paths.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/appconf.pb.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/ui/settings/appsettings/appstarteditor.h>
#include <meteo/commons/ui/settings/appsettings/editapp.h>
#include <meteo/commons/settings/settings.h>

#include <commons/textproto/tprototext.h>

#include <QAction>
#include <QFile>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmap>
#include <QString>
#include <QTableWidgetItem>

namespace {
  QString settingsHumanName() { return QString::fromUtf8("Запуск приложений"); }
  QString redledIconPath() { return ":/meteo/icons/ledred.png"; }
  QString greenledIconPath() { return ":/meteo/icons/ledgreen.png"; }
  QString yesStatusIconPath() { return ":/meteo/icons/ok.png"; }
  QString noStatusIconPath() { return ":/meteo/icons/process_error.png"; }
}

namespace meteo {

static const QString& kErrorIncorrectColumnData = QObject::tr("Ошибка: полученные из ячейки таблицы данные сформированы некорректно");
static const QString& kErrorNoRowSelected = QObject::tr("Ошибка: не выбрана ни одна запись в таблице");

AppStartSettingsWidget::AppStartSettingsWidget(QWidget* parent) :
  SettingsWidget(parent),
  ui_(new Ui::AppStartSettings())
{
  setObjectName(::settingsHumanName());
  ui_->setupUi(this);

  ui_->appTableWidget->setIconSize(QSize(36,36));
  ui_->appTableWidget->setDragEnabled(true);
  ui_->appTableWidget->setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);
  ui_->appTableWidget->header()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
  ui_->appTableWidget->header()->setSectionResizeMode(TITLE_ICON, QHeaderView::ResizeMode::Stretch);
  ui_->appTableWidget->header()->setSectionResizeMode(SPO_NAME, QHeaderView::ResizeMode::ResizeToContents);
  ui_->appTableWidget->header()->setStretchLastSection(false);
  ui_->appTableWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
  connect(ui_->appTableWidget->selectionModel(),SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), SLOT(slotEnableActions()));
  QObject::connect(ui_->appTableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotTableContextMenuRequested(QPoint)));
}

AppStartSettingsWidget::~AppStartSettingsWidget()
{
  delete ui_;
  ui_ = nullptr;
}

void AppStartSettingsWidget::load()
{  
  QDir confDir(MnCommon::etcPath() + "/app.conf.d");
  QStringList confFiles = confDir.entryList(QDir::Files);
  app::AppConfig tmpConf;

  for( const QString& name : confFiles ) {
    QString fname = confDir.path() +  "/" + name;
    if( QFile::exists(fname) ) {
      QFile file(fname);
      if( false == file.open(QIODevice::ReadOnly) ) {
        error_log << QObject::tr("Не удалось открыть %1 для чтения").arg(fname);
        continue;
      }
      QString strconf = QString::fromUtf8(file.readAll());
      file.close();
      TProtoText::fillProto(strconf, &tmpConf);
      for (const auto& app : tmpConf.app()) {
        QSharedPointer<app::Application> appcopy(new app::Application(app));
        this->allApps_.insert(QString::fromStdString(app.id()), appcopy);
      }
    }    
  }
  if (ui_->appTableWidget->topLevelItemCount() > 0) {
    auto first = ui_->appTableWidget->topLevelItem(0);
    ui_->appTableWidget->setItemSelected(first, true);
  }
  else {
    ui_->appTableWidget->selectionModel()->clearSelection();
  }  
  connect(ui_->appTableWidget, &QTreeWidget::itemDoubleClicked,
          this, &AppStartSettingsWidget::slotItemEdit);

  connect(ui_->addButton, SIGNAL(clicked()), SLOT(slotAdd()));
  connect(ui_->editButton, SIGNAL(clicked()), SLOT(slotEdit()));
  connect(ui_->removeButton, SIGNAL(clicked()), SLOT(slotRemove()));
  connect(ui_->cloneBtn, SIGNAL(clicked()), SLOT(slotClone()));


  this->ui_->cbSpoName->addItem(QObject::tr("Все"));
  for ( auto data : this->spoNames() ){
    this->ui_->cbSpoName->addItem(data, data);
  }
  QObject::connect(this->ui_->cbSpoName, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCbSpoNameCurrentIndexChanged(int)) );  

  updateAppTable();
}

void AppStartSettingsWidget::keyPressEvent(QKeyEvent* event)
{
  switch (event->key()) {
    case Qt::Key_Delete:
        slotRemove();
      break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
       slotEdit();
      break;
    default:
      break;
  }
}


bool AppStartSettingsWidget::isApptableNeedFullUpdate()
{
  if ( ui_->appTableWidget->topLevelItemCount() != this->allApps_.size() ){
    return true;
  }

  for ( int i  =0; i < ui_->appTableWidget->topLevelItemCount(); ++i ){
    auto row = ui_->appTableWidget->topLevelItem(i);
    auto configId = row->data(TITLE_ICON, Qt::UserRole).toString();
    if ( false == this->allApps_.contains(configId) )    {
      return true;
    }
  }
  return false;
}


void AppStartSettingsWidget::updateAppTable()
{
  bool needFullUpdate = this->isApptableNeedFullUpdate();

  if ( true == needFullUpdate ) {
    ui_->appTableWidget->clear();
    for (auto config: this->allApps_){
      auto item = new QTreeWidgetItem(ui_->appTableWidget);;
//      updateAppTableRow(*config.get(), item);
      updateAppTableRow(*config.data(), item);
    }
  }
  else {
    for ( int i  =0; i < ui_->appTableWidget->topLevelItemCount(); ++i ){
      auto row = ui_->appTableWidget->topLevelItem(i);
      auto configId = row->data(TITLE_ICON, Qt::UserRole).toString();      
      auto config = this->allApps_[configId];
//      updateAppTableRow(*config.get(), row);
      updateAppTableRow(*config.data(), row);
    }
  }

  if ( 0 != ui_->appTableWidget->selectedItems().count() &&
       nullptr == ui_->appTableWidget->currentItem() &&
       0 != ui_->appTableWidget->topLevelItemCount() ){
    auto first = ui_->appTableWidget->topLevelItem(0);
    ui_->appTableWidget->setItemSelected(first, true);
  }  
}

void AppStartSettingsWidget::updateAppTableRow(const app::Application& config, QTreeWidgetItem* item)
{
  QIcon icon(QString::fromStdString(config.icon()));

  if ( false == icon.isNull() ){
    item->setIcon(TITLE_ICON, icon);
  }
  item->setText(TITLE_ICON, QString::fromStdString(config.title()));
  item->setToolTip(TITLE_ICON, appProcessString(config));
  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable );

  bool isValid = isValidConfig(config);  
  item->setData(STATUS, Qt::DisplayRole, isValid? QObject::tr(" "): QObject::tr("\t"));
  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  this->setCellPixmap(stateIcon(config), item, STATUS);

  this->setCellPixmap(autostartIcon(config), item, AUTOSTART);
  item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  item->setData(AUTOSTART, Qt::DisplayRole, config.autostart()? QObject::tr(" "): QObject::tr("\t"));

  item->setText(SPO_NAME, QString::fromStdString(config.sponame()));

  for ( auto col: { TITLE_ICON, STATUS, AUTOSTART, SPO_NAME }){
    item->setData(col, Qt::UserRole, QString::fromStdString(config.id()));
  }

  if ( false == this->spoFilterData_.isNull() ) {
    auto filterString = this->spoFilterData_.toString();
    item->setHidden(filterString != QString::fromStdString(config.sponame()));
  }
  else {
    item->setHidden(false);
  }
  item->setData(TITLE_ICON, Qt::ItemDataRole::SizeHintRole, QSize(-1, 32));
}

void AppStartSettingsWidget::setCellPixmap(const QPixmap& pixmap, QTreeWidgetItem* row, int column)
{
  Q_UNUSED(pixmap);
  Q_UNUSED(row);
  Q_UNUSED(column);

  QPixmap resPix = pixmap.scaled(17, 17, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  QLabel* lb = new QLabel(ui_->appTableWidget);
  lb->setPixmap(resPix);
  lb->setAlignment(Qt::AlignCenter);
  ui_->appTableWidget->setItemWidget(row, column, lb);//row, column, lb);
}

QString AppStartSettingsWidget::appProcessString(const app::Application& config) const
{
  QString result = QString::fromStdString(config.path());
  for (int i = 0, sz = config.arg_size(); i < sz; ++i) {
    result += " ";
    result += QString::fromStdString(config.arg(i));
  }
  return result;
}

QPixmap AppStartSettingsWidget::stateIcon(const app::Application& config) const
{
  QPixmap result;  
  if (isValidConfig(config) == true) {
    result.load(::greenledIconPath());
  }
  else {
    result.load(::redledIconPath());
  }
  return result;
}

bool AppStartSettingsWidget::isValidConfig(const app::Application& config, QStringList* errors) const
{
  QStringList resultErrors;
  QString id = QString::fromStdString( config.id() );
  if ( 0 == id.size() ) {    
    resultErrors << QObject::tr("Отсутствует уникальный идентификатор");
  }
  if ( false == allApps_.contains(id) ) {
    resultErrors << QObject::tr("Идентификатора %1 нет среди файлов настрооек").arg(id);
  }
  QString path = QString::fromStdString( config.path() );
  if ( 0 == path.size() ) {
    resultErrors << QObject::tr("Не указан путь для настроек = %1").arg(id);
  }
  if ( false == QFile::exists(path) ) {
    resultErrors << QObject::tr("Исполняемый файл настроек не существует = %1").arg(path);
  }
  else {
    auto permissions = QFile::permissions(path);
    if ( false == permissions.testFlag(QFile::Permission::ExeUser) ){
      resultErrors << QObject::tr("Файл %1 существует, но не является исполняемым").arg(path);
    }
  }

  if (nullptr != errors ){
    *errors = resultErrors;
  }

  if ( false == resultErrors.isEmpty() ) {
    error_log << resultErrors.join(",");
    return false;
  }
  return true;
}

QPixmap AppStartSettingsWidget::autostartIcon(const app::Application& config) const
{
  QPixmap result;
  if (config.autostart() == true) {
    result.load(::yesStatusIconPath());
  }
  else {
    result.load(::noStatusIconPath());
  }
  return result;
}

void AppStartSettingsWidget::save()
{
  if (changed_ == false) {
    return;
  }

  for ( auto app: allApps_){
    QStringList errors;
    if ( false == isValidConfig(*app, &errors)){
      QMessageBox::critical(nullptr, "Ошибка", QObject::tr("Параметры запуска приложения \"%1\" заданы некорректно\n - %2")
                            .arg(QString::fromStdString(app->title()))
                            .arg(errors.join("\n - ")));
      //error_log.msgBox() << ;
      return;
    }    
  }

  QDir confDir(MnCommon::etcPath() + "/app.conf.d");
  QStringList confFiles = confDir.entryList(QDir::Files);

  QHash<QString, QString> appDestFiles_;
  for ( auto confFile: confFiles ){
    QString fname = QObject::tr("%1/%2").arg(confDir.path()).arg(confFile);

    app::AppConfig tmpConf;
    if ( false == TProtoText::fromFile(fname, &tmpConf) ) {
      error_log.msgBox() << meteo::msglog::kFileReadFailed.arg(fname);
      return;
    }

    for ( const app::Application& app : tmpConf.app() ) {
      const QString& appId = QString::fromStdString(app.id());
      appDestFiles_.insert(appId, confFile);
    }
  }

  //Создаем резервную копию файлов и очищаем их
  for ( auto confFile: confFiles ) {
    const QString& fullFName = QObject::tr("%1/%2").arg(confDir.path()).arg(confFile);
    const QString& fnameBak = QObject::tr("%1.bak").arg(fullFName);
    if ( false == QFile::copy(fullFName, fnameBak) ) {
      error_log << meteo::msglog::kFileWriteFailed.arg(fnameBak);
      return;
    }
    if ( false == QFile::resize(fullFName, 0) ) {
      error_log << meteo::msglog::kFileWriteFailed.arg(fullFName);
      return;
    }
  }

  //Собираем конфиги, которые будут записаны в соответствующие файлы
  QHash<QString,meteo::app::AppConfig*> fileConfigs_;

  for ( auto app: this->allApps_ ){
    const QString& appId = QString::fromStdString(app->id());
    static const QString& defaultSettingsFileName = QObject::tr("user.services.conf");
    const QString& outputFileName = appDestFiles_.value(appId, defaultSettingsFileName);;

    meteo::app::AppConfig* config_ = fileConfigs_.value(outputFileName, nullptr);
    if ( nullptr == config_ ){
      config_ = new meteo::app::AppConfig();
      fileConfigs_.insert(outputFileName, config_);
    }

    config_->add_app()->CopyFrom(*app);
  }

  //Записываем данные на диск.
  auto writeDataToDisk = [this, fileConfigs_, confDir]() {
    for ( const QString& fileName: fileConfigs_.keys() ){
      const meteo::app::AppConfig* config = fileConfigs_[fileName];
      const QString& fullFName = QObject::tr("%1/%2").arg(confDir.path()).arg(fileName);

      if ( false == TProtoText::toFile(*config, fullFName) ){
        error_log.msgBox() << meteo::msglog::kFileWriteFailed.arg(fullFName);
        return false;
      }
    }
    return true;
  };

  //что бы всегда почистить память
  bool writeOk = writeDataToDisk();

  for ( meteo::app::AppConfig* conf: fileConfigs_ ){
    delete conf;
  }
  fileConfigs_.clear();

  if ( false == writeOk ){
    return;
  }

  //Удаляем сохраненные копии файлов
  for ( auto confFile: confFiles ){
    const QString& fnameBak = QObject::tr("%1/%2.bak").arg(confDir.path()).arg(confFile);
    if ( false == QFile::remove(fnameBak) ) {
      warning_log << meteo::msglog::kFileRemoveFailed.arg(fnameBak);
    }
  }

  this->changed_ = false;

  if ( false == this->changed_ ){
    info_log.msgBox() << QObject::tr("Настройки успешно сохранены");
  }
}

QStringList AppStartSettingsWidget::spoNames()
{
  QStringList output;
  for ( auto appConf: this->allApps_ ) {
    auto spo = QString::fromStdString(appConf->sponame());
    if ( false == output.contains(spo)){
      output << spo;
    }
  }
  return output;
}

void AppStartSettingsWidget::slotAdd()
{
  EditAppDialog dlg(this->spoNames());
  if (dlg.exec() == QDialog::Accepted) {
    QSharedPointer<app::Application> newapp(new app::Application());
    newapp->set_title(dlg.appTitle().toStdString());
    newapp->set_icon(dlg.iconPath().toStdString());
    newapp->set_sponame(dlg.spoName().toStdString());
    newapp->set_autostart(false);

    allApps_.insert(QString::fromStdString(newapp->id()), newapp);    
    setChange();
  }
}

bool AppStartSettingsWidget::slotAdd(const QString& appid)
{
  if (allApps_.contains(appid) == false) {
    return false;
  }

  app::Application& target = *(allApps_[appid]);
  EditAppDialog dlg(this->spoNames());
  dlg.init(QString::fromStdString(target.title()),
           QString::fromStdString(target.icon()));

  if (dlg.exec() != QDialog::Accepted) {
    return false;
  }
  target.set_title(dlg.appTitle().toStdString());
  target.set_icon(dlg.iconPath().toStdString());
  target.set_sponame(dlg.spoName().toStdString());
  setChange();
  return true;
}

void AppStartSettingsWidget::slotRemove()
{
  if (ui_->appTableWidget->selectionModel()->hasSelection() == false) {
    return;
  }

  auto selectedItems = ui_->appTableWidget->selectedItems();
  if ( true == selectedItems.isEmpty() ){
    error_log.msgBox() << kErrorNoRowSelected;
    return ;
  }

  auto data = selectedItems.first()->data(TITLE_ICON, Qt::UserRole);
  if ( false == data.isValid() || true == data.isNull() ){
    error_log << kErrorIncorrectColumnData;
  }

  slotRemove(data.toString());
}

void AppStartSettingsWidget::slotRemove(const QString& appId)
{
  if (allApps_.contains(appId) == true) {
    const app::Application& original = *(allApps_[appId]);
    if( original.preset() == true ){
      warning_log.msgBox() << QObject::tr("Предустановленное приложение не может быть удалено.");
      return;
    }
    if (QMessageBox::information(this, QString::fromUtf8("Подтверждение удаления"),
                                 QString::fromUtf8("Удалить приложение '%1'?").arg(QString::fromStdString(original.title())),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No) == QMessageBox::Yes) {
      allApps_.remove(appId);
      setChange();
    }
  }
  else {
    error_log << QObject::tr("Ошибка: невозможно удалить приложение %1, такого приложения нет").arg(appId);
  }
}

void AppStartSettingsWidget::slotEdit()
{
  if (ui_->appTableWidget->selectionModel()->hasSelection() == false) {
    return;
  }

  auto selectedItems = ui_->appTableWidget->selectedItems();
  if ( true == selectedItems.isEmpty() ){
    error_log.msgBox() << kErrorNoRowSelected;
    return;
  }

  auto data = selectedItems.first()->data(TITLE_ICON, Qt::UserRole);
  if ( false == data.isValid() || true == data.isNull() ) {
    error_log << kErrorIncorrectColumnData;
  }

  slotEdit(data.toString());
}

void AppStartSettingsWidget::slotEdit(const QString& row)
{
  Q_UNUSED(row);
  if (false == allApps_.contains(row) ) {
    return;
  }
  auto keys = allApps_.keys();
  AppStartEditorWidget dlg(keys);
  dlg.init(allApps_[row].data());
  if (dlg.exec() != QDialog::Accepted) {
    return;
  }
  app::Application edited = dlg.config();
  auto original = allApps_[row];

  bool changed = false;
  if (original->id() != edited.id()) {
    auto newId = QString::fromStdString(edited.id());
    this->allApps_.remove(row);
    original->set_id(edited.id());
    allApps_.insert(newId, original);
    changed = true;
  }
  if (original->path() != edited.path()) {
    original->set_path(edited.path());
    changed = true;
  }
  QStringList originalArgs;
  for ( auto arg: original->arg() ){
    auto argQString = QString::fromStdString(arg);
    originalArgs << argQString;
  }


  bool needChangeArgs = ( edited.arg_size() != original->arg_size() );
  for ( auto arg: edited.arg() ){
    auto argQString = QString::fromStdString(arg);
    if ( false == originalArgs.contains(argQString) ){
      needChangeArgs = true;
      break;
    }
  }
  if ( true == needChangeArgs ){
    original->mutable_arg()->CopyFrom(edited.arg());
    changed = true;
  }

  if ( false == changed ){
    return;
  }

  setChange();
}

void AppStartSettingsWidget::slotItemEdit(QTreeWidgetItem* current, int column){
  if ( current == nullptr ) {
    return;
  }
  auto data = current->data(TITLE_ICON, Qt::UserRole);
  auto appId = data.toString();
  if ( false == data.isValid() || true == data.isNull() ){
    return;
  }
  if (column != AUTOSTART) {
    slotEdit(appId);
  }
  else {
    if (allApps_.contains(appId) == true) {
      auto application = this->allApps_[appId];
      application->set_autostart(!application->autostart());
      setChange();
    }
  }
}

void AppStartSettingsWidget::setChange()
{
  updateAppTable();
  changed_ = true;
  emit changed();
}


void AppStartSettingsWidget::slotEnableActions()
{
  bool enable = ui_->appTableWidget->selectionModel()->hasSelection();
  ui_->editButton->setEnabled(enable);
  ui_->removeButton->setEnabled(enable);
  auto current = ui_->appTableWidget->currentItem();
  if ( nullptr == current ){
    return;
  }
  auto data = current->data(TITLE_ICON, Qt::UserRole);
  if ( false == data.isValid() ){
    return;
  }
  if (allApps_.contains(data.toString()) == false) {
    return;
  }
  app::Application& target = *(allApps_[data.toString()]);
  ui_->removeButton->setDisabled( target.preset() == true );
}

void AppStartSettingsWidget::slotCbSpoNameCurrentIndexChanged(int index)
{
  this->spoFilterData_ = this->ui_->cbSpoName->itemData(index);
  this->updateAppTable();
}

void AppStartSettingsWidget::slotTableContextMenuRequested(const QPoint&)
{
  QMenu menu;
  QAction add(QString::fromUtf8("Добавить"), &menu);
  QAction edit(QString::fromUtf8("Редактировать"), &menu);
  QAction remove(QString::fromUtf8("Удалить"), &menu);
  QAction autostart(QString::fromUtf8("Автозапуск"), &menu);
  autostart.setCheckable(true);
  QAction rename(QString::fromUtf8("Переименовать"), &menu);
  QAction clone(QString::fromUtf8("Создать копию"), &menu);
  QAction changeico(QString::fromUtf8("Сменить значок"), &menu);
  QList<QAction*> actions;
  actions.append(&add);
  auto current = this->ui_->appTableWidget->currentItem();
  if ( nullptr == current ){
    return;
  }
  auto data = current->data(TITLE_ICON, Qt::UserRole);
  if ( false == data.isValid() || true == data.isNull() ){
    return ;
  }
  QSharedPointer<app::Application> a;
  if ( true == allApps_.contains(data.toString()) ) {
    a = allApps_[data.toString()];
    actions.append(&edit);
    actions.append(&remove);
    actions.append(&rename);
    actions.append(&clone);
    actions.append(&changeico);
    autostart.setChecked(a->autostart());
    actions.append(&autostart);
    remove.setDisabled(a->preset());
  }

  QAction* answer = menu.exec(actions, QCursor::pos());  

  if (answer == &add) {
    slotAdd();
  }
  else if (answer == &clone){
    slotClone(data.toString());
  }
  else if (answer == &edit) {
    slotEdit(data.toString());
  }
  else if (answer == &remove) {
    slotRemove(data.toString());
  }
  else if (answer == &rename || answer == &changeico) {
    slotAdd(data.toString());
  }
  else if (answer == &autostart) {
    a->set_autostart(!a->autostart());    
    setChange();
  }
}


void AppStartSettingsWidget::slotClone()
{
  auto selectedItems = this->ui_->appTableWidget->selectedItems();
  if ( true == selectedItems.isEmpty() ){
    warning_log.msgBox() << kErrorNoRowSelected;
    return;
  }

  auto data = selectedItems.first()->data(TITLE_ICON, Qt::UserRole);
  if ( false == data.isValid() || true == data.isNull() ) {
    error_log << kErrorIncorrectColumnData;
    return;
  }
  slotClone(data.toString());
}

void AppStartSettingsWidget::slotClone(const QString& appid)
{
  auto original = this->allApps_[appid];
  if ( nullptr == original ){
    error_log << QObject::tr("Невозможно склонировать выбранный процесс: нет данных о таком процессе");
    return;
  }
//  auto clone = QSharedPointer<app::Application>(new app::Application(*original.get()));
  auto clone = QSharedPointer<app::Application>(new app::Application(*original.data()));
  QString cloneId;
  QString cloneTitle = QObject::tr("(Копия) %1").arg(QString::fromStdString(original->title()));
  for ( int i = 1; ; ++i){
    cloneId = QObject::tr("%1.clone.%2")
                      .arg(QString::fromStdString(original->id()))
                      .arg(i);
    if ( false == this->allApps_.contains(cloneId) ){
      break;
    }
  }
  clone->set_id(cloneId.toStdString());
  clone->set_title(cloneTitle.toStdString());
  clone->set_autostart(false);
  clone->set_preset(false);
  this->allApps_[cloneId] = clone;
  this->setChange();
  slotAdd(cloneId);
  slotEdit(cloneId);
}

void AppStartSettingsWidget::slotAppListSelectionChanged()
{
}

} //meteo
