#include "decodeviewer.h"
#include "ui_decodeviewer.h"

#include "tlgdecode.h"

#include <commons/geobasis/geopoint.h>
#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/rpc/rpc.h>
#include <meteo/commons/global/global.h>
//#include <sql/nspgbase/ns_pgbase.h>
#include <meteo/commons/proto/tgribformat.pb.h>
#include <meteo/commons/msgparser/common.h>
#include <meteo/commons/proto/msgparser.pb.h>

#include <meteo/commons/alphanum/metadata.h>

#include <qaction.h>
#include <qmenu.h>
#include <qlist.h>
#include <qstring.h>
#include <qtreewidget.h>
#include <qsettings.h>

namespace {
  const QString formatDateTime() { return QString("hh:mm:ss dd-MM-yyyy"); }

  enum Column {
    Specification,
    Code,
    Value,
    Unit,
    Quality,
    Special,
    Descriptor
  };
  int row_count = 0;
}

namespace meteo {

static const QString& basePath = MnCommon::userSettingsPath() + QObject::tr("/customviewer");

DecodeViewer::DecodeViewer(QWidget* parent) :
  QWidget(parent),
  ui_(new Ui::DecodeViewer()),
  headerItem_(nullptr),
  columnsVisibilityMenu_()
{  
  ui_->setupUi(this);
  ui_->table->setColumnCount(7);
  ui_->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
  ui_->table->horizontalHeader()->setStretchLastSection(true);

  connect(ui_->special_desc, SIGNAL(clicked(bool)), SLOT(hideRow(bool)));
  connect(ui_->codecBox, SIGNAL(currentIndexChanged(int)), SIGNAL(reload()));
  connect(ui_->columnsBtn, SIGNAL(clicked(bool)), SLOT(slotColumnsBtnClicked()));

  QList<int> types;
  types << meteo::anc::kWmoFormat << meteo::anc::kGphFormat << meteo::anc::kIonfoTlg;
  QStringList files;
  files << /*MnCommon::etcPath() + "gphalphanum.conf" <<*/ MnCommon::etcPath() + "alphanum.conf";
  meteo::anc::MetaData::instance()->loadConf(files, types);

  QDir rootFs("/");
  if ( false == rootFs.exists(basePath) ){
    rootFs.mkpath(basePath);
  }
}


void DecodeViewer::hideEvent(QHideEvent *event)
{
  this->saveTableGeometry();
  QWidget::hideEvent(event);
}

void DecodeViewer::saveTableGeometry()
{
  auto header = this->ui_->table->horizontalHeader();
  auto settings = new QSettings( basePath + "/decodeviewer.init", QSettings::IniFormat);
  settings->setValue("decodeviewer.state",header->saveState());
  settings->setValue("decodeviewer.geometry",header->saveGeometry());
}

void DecodeViewer::restoreTableGeomerty()
{
  auto settings = new QSettings( basePath + "/decodeviewer.init", QSettings::IniFormat);
  auto state = settings->value("decodeviewer.state").toByteArray();
  auto geometry = settings->value("decodeviewer.geometry").toByteArray();
  if ( false == state.isEmpty() && false == geometry.isEmpty() ){
    this->ui_->table->horizontalHeader()->restoreGeometry(geometry);
    this->ui_->table->horizontalHeader()->restoreState(state);
  }
  else {
    this->ui_->table->resizeColumnsToContents();
    ui_->table->setColumnWidth(Value, 400);
    ui_->table->setColumnWidth(Specification, 300);
  }
}

DecodeViewer::~DecodeViewer()
{
  delete ui_;
  ui_ = nullptr;
}

void DecodeViewer::init(const QByteArray& tlg, meteo::TypeTelegram type, const QString& dt)
{
  ui_->errorLabel->hide();
  ui_->errorMessageLabel->hide();
  ui_->table->clearContents();
  row_count = 0;
  ui_->table->setRowCount(0);
  ui_->stackedWidget->setCurrentIndex(2);
  ui_->table->setColumnHidden(Special, true);
  if( tlg.isEmpty() ){
    showError(QString::fromUtf8("Ошибка. Пустой запрос"));
    return;
  }

  switch( type ){
    case GRIB     : {
      ::meteo::tlg::MessageNew m = ::meteo::tlg::tlg2proto(tlg);
      foreach (const grib::TGribData& entry, getDecodedGrib(QByteArray::fromRawData(m.msg().c_str(),m.msg().size()))) {
        parseDecodedGrib(entry);
      }
      break;
    }
    case ALPHANUM : {      
      ui_->stackedWidget->setCurrentIndex(0);
      ui_->table->setColumnHidden(Code, false);
      QList<TMeteoData> md;
      bool ok = internal::getDecodedAlphanum(tlg, QDateTime::fromString(dt, Qt::ISODate), &md, ui_->codecBox->currentText());
      if (!ok) {
        showError(QString::fromUtf8("Ошибка раскодирования."));
      }
      else {
        for (int idx = 0; idx < md.size(); idx++) {
          loadReplaceDescriptors(md.at(idx));
        }
        loadCodeText();
        for (int idx = 0; idx < md.size(); idx++) {
          parseDecodedContent(md.at(idx));
        }
      }
      break;
    }
    case ALPHANUMGPH : {      
      ui_->stackedWidget->setCurrentIndex(0);
      ui_->table->setColumnHidden(Code, false);
      QList<TMeteoData> md;
      bool ok = internal::getDecodedIonex(tlg, &md, ui_->codecBox->currentText());
      if (!ok) {
        showError(QString::fromUtf8("Ошибка раскодирования."));
      }
      else {
        for (int idx = 0; idx < md.size(); idx++) {
          loadReplaceDescriptors(md.at(idx));
        }
        loadCodeText();
        for (int idx = 0; idx < md.size(); idx++) {
          parseDecodedContent(md.at(idx));
        }
      }
      break;
    }
    case BUFR     : {
      ui_->stackedWidget->setCurrentIndex(0);
      ui_->table->setColumnHidden(Code, true);
      QList<TMeteoData> md;
      bool ok = internal::getDecodedBufr(tlg, QDateTime::fromString(dt, Qt::ISODate), &md, ui_->codecBox->currentText());
      if (!ok) {
        showError(QString::fromUtf8("Ошибка раскодирования."));
      }
      else {
        for (int idx = 0; idx < md.size(); idx++) {
          loadReplaceDescriptors(md.at(idx));
        }
        loadCodeText();
        for (int idx = 0; idx < md.size(); idx++) {
          parseDecodedContent(md.at(idx));
        }
      }
      break;
    }
    default : { return; }
  }
  ui_->table->resizeColumnsToContents();


  this->restoreTableGeomerty();
}

void DecodeViewer::parseDecodedContent(const TMeteoData& content)
{
  int stationIndex = TMeteoDescriptor::instance()->station(content);
  QString dt;
  {
    QDateTime datetime = TMeteoDescriptor::instance()->dateTime(content);
    if (datetime.isValid() == true) {
      dt = datetime.toString(::formatDateTime());
    }
  }
  QString coord;
  {
    GeoPoint gp;
    if (TMeteoDescriptor::instance()->getCoord(content, &gp) == true) {
      coord = gp.toString();
      coord.remove(QObject::tr("Широта:"));
      coord.remove(QObject::tr("Долгота:"));
    }
  }
  int dataType = content.getValue(TMeteoDescriptor::instance()->descriptor("category"), BAD_METEO_ELEMENT_VAL);

  QString stationName;
  if (stationIndex != BAD_METEO_ELEMENT_VAL && dataType != BAD_METEO_ELEMENT_VAL) {
    QString errorMessage;
    rpc::Channel* ch = meteo::global::serviceChannel( meteo::settings::proto::kSprinf );
    if ( 0 == ch ) {
      error_log.msgBox() << QObject::tr("Не удалось установить соединение с сервисом справки");
      return;
    }
    stationName = internal::getStationName( ch, stationIndex, dataType, &errorMessage);
    delete ch;

    if (errorMessage.isEmpty() == false) {
      error_log << errorMessage;
    }

    if (stationName.isEmpty()) {
      stationName = QString::number(stationIndex);
    }
  }
  row_count++;
  ui_->table->setRowCount(row_count);
  headerItem_ = new QTableWidgetItem;
  ui_->table->setItem(row_count-1, 0, headerItem_);
  ui_->table->setSpan(row_count-1, 0, 1, 7);
  headerItem_->setTextAlignment(Qt::AlignJustify);
  headerItem_->setTextAlignment(Qt::AlignCenter);
  headerItem_->setBackgroundColor(Qt::yellow);
  QFont f = headerItem_->font();
  f.setBold(true);
  headerItem_->setFont(f);
  QString title;
  if( -9999 != stationIndex ){
    QString stationTitle;
    if( stationName != QString::number(stationIndex)){
      stationTitle = QString("%1 %2").arg(stationIndex).arg(stationName);
    }
    else{
      stationTitle = stationName;
    }
    title = QString( "%1  %2  %3" ).arg(stationTitle).arg(coord).arg(dt);
  }
  else{
    title = QString( "%1  %2" ).arg(coord).arg(dt);
  }
  headerItem_->setText(title);
  parseDecodedContentRecursively(content);
}

void DecodeViewer::parseDecodedContentRecursively(const TMeteoData& content) const
{
  auto list = content.getDescrList();  
  QStringList strdescrlist;
  for ( auto descriptor : list ) {
    strdescrlist.append( QString::number(descriptor) );
  }
  global::loadBufrTables(strdescrlist);
  for ( auto descriptor : list ) {
    bool spec = false;
    if( descriptor < 7000 || (descriptor > 7999 && descriptor < 10000)){
      spec = true;
    }
    QMap<int, TMeteoParam> hash = content.getParamList(descriptor);
    QString name = TMeteoDescriptor::instance()->name(descriptor);
    QString unit = TMeteoDescriptor::instance()->property(descriptor).unitsRu;
    int k = 0;
    QMap<int, TMeteoParam>::const_iterator it = hash.begin();
    QMap<int, TMeteoParam>::const_iterator end = hash.end();
    while ( it != end ) {
      int q = it.value().quality();
      if ( control::NO_OBSERVE == q ) {
        ++it;
        k++;
        continue;
      }
      row_count++;
      ui_->table->setRowCount(row_count);
      double v = it.value().value();

      name = TMeteoDescriptor::instance()->findAdditional( descriptor, k, v );

      QString codetext = it.value().code();
      QString valuetext = QString().setNum(v);
      QString spectext = ( true == spec) ? "true" : "false";
      if( "NO" == unit && false == valuetext.isEmpty() ){
        QString descstr = QString::number(descriptor);
        while ( descstr.length() < 5 ){
          descstr = "0" + descstr;
        }
        QString key = QString("%1_%2").arg(descstr).arg(valuetext);
      }
      QString unitext = unit;
      if( "NO" == unitext ){
        unitext.clear();
      }          
      if( "CCCC" == name && 0 != headerItem_ ){
        headerItem_->setText(QObject::tr("ИКАО: %1 %2").arg(codetext).arg(headerItem_->text()));
      }

      QString specification = TMeteoDescriptor::instance()->description(descriptor, k, v);
      if ( "station" == name && nullptr != headerItem_ ){
        auto text = QObject::tr("Станция: %1,%2").arg(codetext)
                                                .arg(headerItem_->text());
        headerItem_->setText(text);
      }

      QTableWidgetItem* codeitem = new QTableWidgetItem(codetext);
      codeitem->setToolTip(codetext);
      QTableWidgetItem* valueitem = new QTableWidgetItem(valuetext);
      valueitem->setToolTip( global::bufrValueDescription( QString::number(descriptor), v ) );
      QTableWidgetItem* unititem = new QTableWidgetItem(unitext);
      unititem->setToolTip(unitext);
      QTableWidgetItem* parameteritem = new QTableWidgetItem(name);
      QTableWidgetItem* specificationitem = new QTableWidgetItem(specification);
      specificationitem->setToolTip(specification);
      QTableWidgetItem* qualityitem = new QTableWidgetItem(control::titleForQuality(q));
      qualityitem->setToolTip(qualityitem->text());
      QTableWidgetItem* specitem = new QTableWidgetItem(spectext);

      ui_->table->setItem(row_count-1, Specification, specificationitem);
      ui_->table->setItem(row_count-1, Code, codeitem);
      ui_->table->setItem(row_count-1, Value, valueitem);
      ui_->table->setItem(row_count-1, Unit, unititem);
      ui_->table->setItem(row_count-1, Descriptor, parameteritem);
      ui_->table->setItem(row_count-1, Quality, qualityitem);
      ui_->table->setItem(row_count-1, Special, specitem);

      ui_->table->setRowHidden(row_count-1, false == ui_->special_desc->isChecked() && true == spec);

      QColor c;
      if( q == control::MISTAKEN || q == control::DOUBTFUL ){
        c.setRgb(255, 102, 102);
      }
      else if( q == control::NO_CONTROL || q == control::NO_OBSERVE ){
        c.setRgb(255, 255, 255);
      }
      else if( q == control::RIGHT ){
        c.setRgb(152, 255, 152);
      }
      else if( q == control::SPECIAL_VALUE ){
        c.setRgb(255, 204, 153);
      }
      for (int i = 0, sz = ui_->table->columnCount(); i < sz; ++i) {
        QTableWidgetItem* item =  ui_->table->item(row_count-1, i);
        if( 0 != item && i == Quality ){
          item->setBackground(c);
        }
      }
      if( true == spec ){
        for (int i = 0, sz = ui_->table->columnCount(); i < sz; ++i) {
          if( Quality != i ){
            QTableWidgetItem* item =  ui_->table->item(row_count-1, i);
            item->setBackground(Qt::lightGray);
          }
        }
      }
      
      ++it;
      k++;
    }
  }
  for ( int i = 0, sz = content.childsCount(); i < sz; ++i ) {
    parseDecodedContentRecursively( *(content.child(i)) );
  }
}

void DecodeViewer::parseDecodedGrib(const grib::TGribData& content) const
{
  ui_->textEdit->setText(QString::fromStdString(content.DebugString()));
}

void DecodeViewer::loadReplaceDescriptors(const TMeteoData& content)
{
  QList<descr_t> list = content.getDescrList();
  foreach (descr_t descriptor, list) {
    if( false == ui_->special_desc->isChecked() && descriptor < 10000 ){
      if( descriptor < 7000 || descriptor > 7999 ){
        continue;
      }
    }
    double value = content.getValueCur(descriptor, BAD_METEO_ELEMENT_VAL);
    QMap<int, TMeteoParam> hash = content.getParamList(descriptor);
    QString unit = TMeteoDescriptor::instance()->property(descriptor).unitsRu;
    QString valuetext = QString().setNum(value);
    if( "NO" == unit && false == valuetext.isEmpty() ){
      QString descstr = QString::number(descriptor);
      while( descstr.length() < 6){
        descstr.insert(0, '0');
      }
    }
    if( hash.count() > 1){
      QMap<int, TMeteoParam>::const_iterator it = hash.begin();
      QMap<int, TMeteoParam>::const_iterator end = hash.end();
      while( it != end ){
        double v = it.value().value();
        QString valuetext = QString().setNum(v);
        if( "NO" == unit && false == valuetext.isEmpty() ){
          QString descstr = QString::number(descriptor);
          while( descstr.length() < 6){
            descstr.insert(0, '0');
          }
        }
        ++it;
      }
    }
  }
  for ( int i = 0, sz = content.childsCount(); i < sz; ++i ) {
    loadReplaceDescriptors( *(content.child(i)) );
  }
}

void DecodeViewer::loadCodeText()
{
//  if( 0 == desc_value_.count() ){
//    return;
//  }
//  QString arr1, arr2;
//  foreach( const QString& key, desc_value_.keys() ){
//    arr1 += ("'" + key + "'" + ",");
//  }
//  foreach( int value, desc_value_){
//    arr2 += (QString::number(value) + ",");
//  }
//  arr1.chop(1);
//  arr2.chop(1);
//  db_->ExecQuery(QString("select * from sprinf.get_bufr_code_arr(ARRAY[%1], ARRAY[%2])")
//                 .arg(arr1).arg(arr2));
//
//   for( int i = 0, sz = db_->RecordCount(); i < sz; ++i ){
//     desc_code_.insert(QString("%1_%2").arg(db_->GetCell(i, "code")->AsString()).arg(db_->GetCell(i, "value")->AsString()),
//                 db_->GetCell(i, "codetext")->AsString());
//   }
}

void DecodeViewer::showError(const QString& errorMessage)
{
  ui_->errorMessageLabel->setText(errorMessage);
  ui_->errorLabel->show();
  ui_->errorMessageLabel->show();
}

QSize DecodeViewer::tableSize() const
{
  int w = ui_->table->verticalHeader()->width() + 4;
  for (int i = 0; i < ui_->table->columnCount(); i++)
    w += ui_->table->columnWidth(i);
  int h = ui_->table->horizontalHeader()->height() + 4;
  for (int i = 0; i < ui_->table->rowCount(); i++)
    h += ui_->table->rowHeight(i);
  return QSize(w, h);
}

void DecodeViewer::hideRow(bool on)
{
  for(int i = 0; i < ui_->table->rowCount(); ++i){
    QTableWidgetItem* item = ui_->table->item(i, Special);
    if( nullptr != item ){
      if(item->text() == "true")
      {
        ui_->table->setRowHidden(i, !on);
      }
    }
  }
}

const QList<grib::TGribData> DecodeViewer::getDecodedGrib(const QByteArray& tlg)
{
  ui_->stackedWidget->setCurrentIndex(1);
  QList<grib::TGribData> result;
  result = internal::getDecodedGrib(tlg);
  return result;
}

void DecodeViewer::slotColumnsBtnClicked()
{
  if ( true == columnsVisibilityMenu_.isEmpty()  ){
    auto table = this->ui_->table;
    for ( auto i = 0; i < table->columnCount(); ++i ){
      if ( i == Special ) {
        continue;
      }
      auto headerItem = this->ui_->table->horizontalHeaderItem(i);
      QAction *action = new QAction(headerItem->text(),nullptr);
      action->setCheckable(true);
      action->setData(i);
      action->setChecked(table->horizontalHeader()->sectionSize(i) != 0);
      QObject::connect(action, SIGNAL(triggered()), this, SLOT(slotMenuActionTriggered()));

      columnsVisibilityMenu_.addAction(action);
    }
  }
  columnsVisibilityMenu_.exec(QCursor::pos());
}

void DecodeViewer::slotMenuActionTriggered()
{
  auto sender = qobject_cast<QAction*>(QObject::sender());
  if ( nullptr == sender ){
    return;
  }
  auto table = this->ui_->table;
  if (nullptr == table ){
    return;
  }
  auto header = table->horizontalHeader();
  if ( nullptr == header ){
    return;
  }
  bool isVisible = sender->isChecked();
  auto column = sender->data().toInt();
  header->setSectionHidden(column, !isVisible);

}



} // meteo
