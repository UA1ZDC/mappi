#include "puansonview.h"

#include <qevent.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qtransform.h>
#include <qpainter.h>
#include <qmap.h>

#include <sql/psql/psqlquery.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/proto/weather.pb.h>
#include <meteo/commons/proto/msgcenter.pb.h>
#include <meteo/commons/proto/sprinf.pb.h>
#include <meteo/commons/proto/surface_service.pb.h>
#include <meteo/commons/global/global.h>
#include <meteo/commons/ui/map/layer.h>
#include <meteo/commons/ui/map/document.h>

#include <meteo/commons/ui/map/view/actions/hidebuttonsaction.h>
#include <meteo/commons/ui/mainwindow/widgethandler.h>
#include <meteo/commons/ui/meteogram/meteogramwindow.h>


#include "ui_puansonview.h"

#include "dlgvalue.h"

namespace {

static const int kClmnVisible   = 0;
static const int kClmnDescr     = 6;
static const int kClmnUnit      = 2;
static const int kClmnVal       = 3;
static const int kClmnCode      = 4;
static const int kClmnQual      = 5;
static const int kClmnParam     = 1;

}

namespace meteo {
namespace map {

PuansonView::PuansonView( MapWindow* p )
  : MapWidget(p),
  ui_(new Ui::PuansonView),
  puanson_(0),
  changed_(false)
{
  ui_->setupUi(this);
  if ( 0 != ui_->descrtree->headerItem() ) {
    ui_->descrtree->headerItem()->setText( kClmnVisible, "" );
  }
  QObject::connect(ui_->descrtree,SIGNAL( itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT( slotItemDoubleClicked( QTreeWidgetItem*, int ) ) );
  QObject::connect(ui_->descrtree,SIGNAL( itemClicked(QTreeWidgetItem*,int)), this, SLOT( slotItemClicked( QTreeWidgetItem*, int ) ) );
  QObject::connect(ui_->okbtn, SIGNAL( clicked() ), this, SLOT( slotOkClicked() ) );
  QObject::connect(ui_->closebtn, SIGNAL( clicked() ), this, SLOT( slotCloseClicked() ) );
  QObject::connect(ui_->tabWidget, SIGNAL( currentChanged(int ) ), this, SLOT( tabchangedslot(int) ) );
  if ( 0 != mapdocument() && 0 != mapdocument()->eventHandler() ) {
    mapdocument()->eventHandler()->installEventFilter(this);
  }
}




PuansonView::~PuansonView()
{
  delete ui_; ui_ = 0;
  puanson_ = 0;
}

void PuansonView::setPuanson( Puanson* p )
{
  has_new_ = true;
  changed_ = false;
  ui_->descrtree->clear();
  puanson_ = p;
  if ( nullptr == puanson_ ) {
    return;
  }

  meteodata_ = puanson_->meteodata();
  punch_.CopyFrom( puanson_->punch() );

  meteodescr::TMeteoDescriptor* md = TMeteoDescriptor::instance();

  QStringList descrlist;
  for ( auto rule : punch_.rule() ) {
    descr_t descr = -1;
    auto name = QString::fromStdString( rule.id().name() );
    if ( true == md->isAdditional(name) ) {
      auto add = md->additional(name);
      descr = add.descr;
    }
    else if ( false == md->contains(name) ) {
      continue;
    }
    else {
      descr = md->descriptor(name);
    }
    if ( -1 == descr ) {
      continue;
    }
    QString strdescr = QString::number(descr);
    if ( false == descrlist.contains(strdescr) ) {
      descrlist.append(strdescr);
    }
  }
  global::loadBufrTables(descrlist);

  punch_.clear_scale();
  if ( 0 != puanson_->document() ) {
    punch_.set_ruleview( puanson_->document()->property().ruleview() );
  }

  if ( 0 == puanson_->layer() || proto::kSurface != puanson_->layer()->info().source() ) {
      int indx = ui_->tabWidget->indexOf( ui_->telegramtab );
      if ( -1 != indx ) {
          ui_->tabWidget->removeTab(indx);
      }
      int mindx = ui_->tabWidget->indexOf( ui_->meteogramtab );
      if ( -1 != mindx ) {
          ui_->tabWidget->removeTab(mindx);
      }

  }
  else {
    int indx = ui_->tabWidget->indexOf( ui_->telegramtab );
    if ( -1 == indx ) {
        ui_->tabWidget->addTab( ui_->telegramtab, QObject::tr("Телеграмма") );
    }
    int mindx = ui_->tabWidget->indexOf( ui_->meteogramtab );
    if ( -1 == mindx ) {
        ui_->tabWidget->addTab( ui_->meteogramtab, QObject::tr("Метеограмма") );
      }

    loadTelegrams();
  }

  setWindowTitle( getTitle() );
  setDescription();

  QMap< QString, int > items;
  QMap< int, QPair< QString, int > > values;
  for ( int i = 0, sz = punch_.rule_size(); i < sz; ++i ) {
    const puanson::proto::CellRule& rule = punch_.rule(i);
    QString name = QString::fromStdString( rule.id().name() );
    descr_t descr = -1;
    QString strdescr;
    if ( true == md->isAdditional(name) ) {
      auto add = md->additional(name);
      descr = add.descr;
    }
    else if ( false == md->contains(name) ) {
    }
    else {
      descr = md->descriptor(name);
    }
    if ( -1 != descr ) {
      strdescr = QString::number(descr);
    }
    meteodescr::Property prop;
    md->property( name, &prop );
    QTreeWidgetItem* item = new QTreeWidgetItem( ui_->descrtree );
    item->setText( kClmnParam, prop.description );
    item->setText( kClmnUnit, prop.unitsRu );

    QString strval = QObject::tr("Отсутствует");
    QString strcode = strval;
    QString strqual = strval;
    if ( true == meteodata_.hasParam(name) ) {
      const TMeteoParam& param = meteodata_.meteoParam(name);
      strval = QString::number( param.value(), 'f', 1 );
      strcode = param.code();
      strqual = control::titleForQuality( param.quality() );
      values[i].first = name;
      values[i].second = param.value();
      if ( 0 != strdescr.size() ) {
        auto descrvalue = global::bufrValueDescription( strdescr, param.value() );
        if ( 0 != descrvalue ) {
          item->setToolTip( kClmnVal, descrvalue );
        }
      }
    }
    if ( QString("NULL") == strcode || ( 0 == strcode.size() && 0 != strval.size() ) ) {
      strcode = QString("BIN");
    }
    item->setText( kClmnVal, strval );
    item->setText( kClmnCode, strcode );
    item->setText( kClmnQual, strqual );
    item->setText( kClmnDescr, name );

    items.insert( name, i );
    setVisible( item, rule.visible() );
  }

  for (int i = 0, sz = ui_->descrtree->columnCount(); i < sz; ++i) {
    ui_->descrtree->resizeColumnToContents(i);
  }
  setupPreview();

}

bool PuansonView::eventFilter( QObject* o, QEvent* e )
{
  if ( o == mapdocument()->eventHandler() && LayerEvent::LayerChanged  == e->type() && 0 != puanson_ ) {
    LayerEvent* ev = reinterpret_cast<LayerEvent*>(e);
    if ( LayerEvent::Deleted == ev->changeType() ) {
      if ( 0 == puanson_->layer() ) {
        setPuanson(0);
      }
      else if ( ev->layer() == puanson_->layer()->uuid() ) {
        setPuanson(0);
      }
    }
  }
  return false;
}

void PuansonView::slotItemDoubleClicked( QTreeWidgetItem* item, int clmn )
{
  Q_UNUSED(item);
  Q_UNUSED(clmn);
  if ( kClmnVal != clmn ) {
    return;
  }

  DlgValue* dlg = new DlgValue( item->text( kClmnParam ), item->text( kClmnVal ).toDouble(), this );
  int res = dlg->exec();
  double val = dlg->value();
  control::QualityControl qual = dlg->qual();
  delete dlg; dlg = 0;
  if ( QDialog::Accepted != res ) {
    return;
  }
  QString id = item->text( kClmnDescr );
  TMeteoParam param( item->text(kClmnCode), val, qual );
  meteodata_.remove(id);
  meteodata_.add( id, param );
  item->setText( kClmnVal, QString::number( val, 'f', 2 ) );
  item->setText( kClmnQual, control::titleForQuality( param.quality() ) );
  setupPreview();
  changed_ = true;
}

void PuansonView::slotItemClicked( QTreeWidgetItem* item, int clmn )
{
  if ( 0 == item || kClmnVisible != clmn ) {
    return;
  }
  std::string str = item->text( kClmnDescr ).toStdString();
  for ( int i = 0, sz = punch_.rule_size(); i < sz; ++i ) {
    if ( punch_.rule(i).id().name() ==  str ) {
      if ( false == punch_.rule(i).visible() ) {
        setVisible( item, true );
      }
      else {
        setVisible( item, false );
      }
      setupPreview();
      return;
    }
  }
}

void PuansonView::setDescription()
{
  QString station = QObject::tr("Станция");
  QString height = QObject::tr("Высота");
  if ( 0 != puanson_ && 0 != puanson_->layer() ) {
    if ( proto::kField == puanson_->layer()->info().source() ||
         proto::kGribSource == puanson_->layer()->info().source() ) {
      station = QObject::tr("Центр");
      height = QObject::tr("Уровень");
    }
    if ( 100 == puanson_->layer()->info().type_level() ) {
      height = QObject::tr("Уровень");
    }
    else if ( 160 == puanson_->layer()->info().type_level() ) {
      height = QObject::tr("Глубина");
    }
  }
  QString str = QObject::tr(
      "<table>"
      "<tr><td>%1</td><td>%2</td></tr>"
      "<tr><td>%3</td><td>%4</td></tr>"
      "<tr><td>%5</td><td>%6</td></tr>"
      "<tr><td>%7</td><td>%8</td></tr>"
      )
    .arg( station )
    .arg( stationStr() )
    .arg( strLat() )
    .arg( strLon() )
    .arg( height )
    .arg( heightStr() )
    .arg( QObject::tr("Срок") )
    .arg( termStr() );
  QString hour = hourStr();
  if ( false == hour.isEmpty() ) {
    str += QObject::tr(
        "<tr><td>%1</td><td>%2</td></tr>"
        )
      .arg( QObject::tr("Прогноз") )
      .arg( hourStr() );
  }
  str += QString("</table>");
  ui_->infolbl->setText(str);
  ui_->infolbl->adjustSize();
}

QString PuansonView::getTitle() const
{
  return QString("%1 %2 %3 %4 %5")
    .arg( stationStr() )
    .arg( coordStr() )
    .arg( heightStr() )
    .arg( termStr() )
    .arg( hourStr() );
}

QString PuansonView::coordStr() const
{
  QString coordstr = QObject::tr("Нет координат");
  coordstr = QString("%1 %2")
    .arg( strLat() )
    .arg( strLon() );
  return coordstr;
}

QString PuansonView::strLat() const
{
  if ( 0 == puanson_ ) {
    return QString();
  }
  QString coordstr = QObject::tr("Нет координат");
  const GeoVector& gv = puanson_->skelet();
  if ( 0 != gv.size() ) {
    const GeoPoint& gp = gv[0];
    coordstr = QString("%1")
      .arg( gp.strLat() );
  }
  return coordstr;
}

QString PuansonView::strLon() const
{
  if ( 0 == puanson_ ) {
    return QString();
  }
  QString coordstr = QObject::tr("Нет координат");
  const GeoVector& gv = puanson_->skelet();
  if ( 0 != gv.size() ) {
    const GeoPoint& gp = gv[0];
    coordstr = QString("%1")
      .arg( gp.strLon() );
  }
  return coordstr;
}

QString PuansonView::heightStr() const
{
  if ( 0 == puanson_ ) {
    return QString();
  }
  QString heightstr = QObject::tr("Нет данных");
  const GeoVector& gv = puanson_->skelet();
  if ( 0 != gv.size() ) {
    const GeoPoint& gp = gv[0];
    if ( 0 != puanson_->layer() ) {
      const proto::WeatherLayer& info = puanson_->layer()->info();
      if ( 100 == info.type_level() ) {
        heightstr = QString("%1 мбар")
          .arg( info.level() );
      }
      else if ( 1 == info.type_level() ) {
        if ( proto::kSurface == info.source() ) {
          if ( false == MnMath::isEqual( gp.alt(), BAD_METEO_ELEMENT_VAL ) ) {
            heightstr = QString("%1")
              .arg( gp.strAlt() );
          }
        }
        else {
          heightstr = QString("%1")
            .arg( QObject::tr("У земли") );
        }
      }
      else if ( 160 == info.type_level() ) {
        heightstr = QObject::tr("%1 м")
          .arg( info.level() );
      }
      else if ( 9 == info.type_level() ) {
        heightstr = QObject::tr("Дно");
      }
    }
    else {
      heightstr = QString("Высота неизвестна")
        .arg( gp.strLat() )
        .arg( gp.strLon() );
    }
  }
  return heightstr;
}

QString PuansonView::stationStr() const
{
  if ( 0 == puanson_ ) {
    return QString();
  }
  QString indexstr = QObject::tr("Нет индекса");
  QString stident = TMeteoDescriptor::instance()->stationIdentificator(meteodata_);
  if ( 0 != puanson_->layer()
       &&( proto::kField == puanson_->layer()->info().source()
           || proto::kGribSource == puanson_->layer()->info().source())) {
    indexstr = QString::fromStdString( puanson_->layer()->info().center_name() );
  }
  else {
    indexstr = stident;

    meteo::rpc::Channel* ctrl = global::serviceChannel( settings::proto::kSprinf );
    if ( 0 != ctrl ) {
      sprinf::MultiStatementRequest req;
      req.add_station( stident.toStdString() );
      for(int i = 0, sz = puanson_->layer()->info().data_type_size(); i < sz; ++i) {
        req.add_data_type(puanson_->layer()->info().data_type(i));
      }

      sprinf::Stations* resp = ctrl->remoteCall( &sprinf::SprinfService::GetStations, req, 1000 );
      delete ctrl; ctrl = 0;
      if ( 0 != resp ) {
        if ( 0 != resp->station_size() ) {
          QString stname;
          if ( true == resp->station(0).name().has_rus() ) {
            stname = QString::fromStdString( resp->station(0).name().rus() );
          }
          else if ( true == resp->station(0).name().has_international() ) {
            stname = QString::fromStdString( resp->station(0).name().international() );
          }
          else if ( true == resp->station(0).name().has_short_() ) {
            stname = QString::fromStdString( resp->station(0).name().short_() );
          }
          if ( false == stname.isEmpty() ) {
            indexstr += QObject::tr(" %1").arg( QString::fromStdString( resp->station(0).name().rus() ) );
          }
        }
        delete resp; resp = 0;
      }
    }
  }
  return indexstr;
}

QString PuansonView::termStr() const
{
  if ( 0 == puanson_ ) {
    return QString();
  }
  QString termstr;
  if ( 0 != puanson_->layer() ) {
    const proto::WeatherLayer& info = puanson_->layer()->info();
    QDateTime dt = TMeteoDescriptor::instance()->dateTime( puanson_->meteodata() );
    if ( false == dt.isValid() ) {
      dt = PsqlQuery::datetimeFromString( info.datetime() );
    }
    termstr = dt.toString("dd.MM.yy hh:mm");
  }
  return termstr;
}

QString PuansonView::hourStr() const
{
  if ( 0 == puanson_ ) {
    return QString();
  }
  QString hour;
  if ( 0 != puanson_->layer() ) {
    const proto::WeatherLayer& info = puanson_->layer()->info();
    if ( 0 != info.hour() ) {
      hour += QObject::tr("на %1 часов").arg( info.hour() );
    }
  }
  return hour;
}

void PuansonView::loadTelegrams()
{
  QString text;
  meteo::rpc::Channel* ctrl = global::serviceChannel( settings::proto::kMsgCenter );
  if ( 0 == ctrl ) {
    text += QObject::tr("Не удалось получить содержимое телеграммы");
  }
  else {
    QMap< int, TMeteoParam > list = meteodata_.meteoParamList("ii_tlg");
    if ( 0 == list.size() ) {
      text += QObject::tr("Нет информации об источнике данных");
    }
    else {
      QMapIterator< int, TMeteoParam > it(list);
      meteo::msgcenter::GetTelegramRequest request;
      request.set_onlyheader(false);
      while ( true == it.hasNext() ) {
        it.next();
        int64_t ptkpp = it.value().code().toLongLong();
        request.add_ptkpp_id(ptkpp);
      }
      msgcenter::GetTelegramResponse* response = ctrl->remoteCall( &msgcenter::MsgCenterService::GetTelegram, request, 10000 );
      delete ctrl; ctrl = 0;
      if ( 0 == response ) {
        text += QObject::tr("Не удалось получить информацию об источнике данных");
      }
      else if ( response->has_error() ) {
        text += QObject::tr("Ошибка при получении информации  об источнике данных = %1")
          .arg( QString::fromStdString( response->error() ) );
        delete response; response = 0;
      }
      else {
        for ( int i = 0, sz = response->msg_size(); i < sz; ++i ) {
          const tlg::MessageNew& tlg = response->msg(i);
          const tlg::Header& header = tlg.header();
          QString strheader = QString::fromUtf8( header.data().data(), header.data().size() );
          text += QObject::tr("Идентификатор в таблице телеграмм: %1").arg( tlg.metainfo().id() ) + '\n';
          text +=   "------------------------------------------------------\n";
          text += strheader;
          if ( false == tlg.isbinary() ) {
            text += QObject::tr("%1").arg( tlg.msg().c_str() );
          }
          else {
            text += QObject::tr("***Бинарные данные***");
          }
          text += "\n------------------------------------------------------\n\n";
        }
      }
    }
  }
  ui_->telegramview->setText(text);
}

void PuansonView::setVisible( const QString& name, bool fl )
{
  std::string str = name.toStdString();
  for ( int i = 0, sz = punch_.rule_size(); i < sz; ++i ) {
    if ( punch_.rule(i).id().name() == str ) {
      if ( punch_.rule(i).visible() != fl ) {
        punch_.mutable_rule(i)->set_visible(fl);
      }
      break;
    }
  }
}

void PuansonView::setVisible( QTreeWidgetItem* item, bool fl )
{
  if ( 0 == item ) {
    return;
  }
  QIcon ico = ( true == fl ) ? QIcon(":/meteo/icons/tools/layer_visible.png") : QIcon(":/meteo/icons/tools/layer_hidden.png") ;
  item->setIcon( kClmnVisible, ico );
  setVisible( item->text( kClmnDescr ), fl );
}

bool PuansonView::visible( const QString& name ) const
{
  std::string str = name.toStdString();
  for ( int i = 0, sz = punch_.rule_size(); i < sz; ++i ) {
    if ( punch_.rule(i).id().name() == str ) {
      return punch_.rule(i).visible();
    }
  }
  return false;
}

void PuansonView::setupPreview()
{
  if ( 0 == puanson_ ) {
    return;
  }
  Puanson* copy = mapobject_cast<Puanson*>( puanson_->copy( puanson_->projection() ) );
  if ( 0 == copy ) {
    return;
  }
  QSize sz = ui_->puansonlbl->size();
  copy->setScreenPos( QPoint(0,0) );
  copy->setPunch(punch_);
  copy->setMeteodata(meteodata_);
  QPixmap pix(sz);
  pix.fill(Qt::white);
  QPainter pntr(&pix);
  QTransform tr;
  tr.translate( pix.rect().center().x(), pix.rect().center().y() );
  tr.scale(2,2);
  pntr.setTransform(tr);
  copy->render( &pntr, copy->boundingRect(), QTransform() );
  ui_->puansonlbl->setPixmap(pix);
  int indx = ui_->tabWidget->indexOf( ui_->meteogramtab );
  if(ui_->tabWidget->currentIndex() == indx){
      showMeteogram();
    }
  delete copy;
}

void PuansonView::showEvent(QShowEvent *event){
  int indx = ui_->tabWidget->indexOf( ui_->meteogramtab );
  if(ui_->tabWidget->currentIndex() == indx){
      showMeteogram();
    }
  event->accept();
}

void PuansonView::closeEvent( QCloseEvent* e )
{
  if ( true == changed_ ) {
    int res = QMessageBox::question(
        this,
        QObject::tr("Предупреждение"),
        QObject::tr("Данные изменены. Сохранить изменения?"),
        QObject::tr("Сохранить и закрыть"),
        QObject::tr("Не сохранять и закрыть"),
        QObject::tr("Отмена")
        );
    if ( 0 == res ) {
      slotOkClicked();
    }
    else if ( 2 == res ) {
      e->ignore();
      return;
    }
    changed_ = false;
  }
  ui_->tabWidget->setCurrentIndex(0);
  e->accept();
}

void PuansonView::slotOkClicked()
{
  if ( 0 == puanson_ ) {
    return;
  }
  if ( true == changed_ ) {
    puanson_->setMeteodata(meteodata_);
    changed_ = false;
    qApp->setOverrideCursor( Qt::WaitCursor );
    meteo::rpc::Channel* ctrl = global::serviceChannel( meteo::settings::proto::kSrcData );
    if ( nullptr != ctrl ) {
      QStringList allnames = meteodata_.allNames();
      QMap<int,TMeteoParam> idlist = meteodata_.meteoParamList("UNK");
      QMapIterator<int,TMeteoParam> it(idlist);
      while ( true == it.hasNext() ) {
        it.next();
        TMeteoParam id = it.value();
        QStringList plist = id.code().split("_"); //В показатель кач-ва записаны id записи и имя дескриптора
        if ( 2 != plist.size() ) {
          continue;
        }
        QString descr = plist[0];
        QString val = plist[1];
        if ( true == meteodata_.hasParam(descr) ) {
          TMeteoParam param = meteodata_.meteoParam(descr);
          surf::UpdateDataRequest request;
          request.set_id( val.toLongLong() );
          request.set_value( param.value() );
          request.set_quality( param.quality() );
          surf::Value* reply = ctrl->remoteCall( &surf::SurfaceService::UpdateMeteoData, request, 10000 );
          if ( 0 != reply ) {
            delete reply;
          }
        }
      }
    }
    delete ctrl; ctrl = nullptr;
    qApp->restoreOverrideCursor();
  }
}

void PuansonView::slotCloseClicked()
{
  if ( true == changed_ ) {
    int res = QMessageBox::question(
        this,
        QObject::tr("Предупреждение"),
        QObject::tr("Данные изменены. Сохранить изменения?"),
        QObject::tr("Сохранить и закрыть"),
        QObject::tr("Не сохранять и закрыть"),
        QObject::tr("Отмена")
        );
    if ( 0 == res ) {
      slotOkClicked();
    }
    else if ( 2 == res ) {
      return;
    }
    changed_ = false;
  }
  QWidget::close();
}

void PuansonView::reject()
{
  if ( true == changed_ ) {
    int res = QMessageBox::question(
        this,
        QObject::tr("Предупреждение"),
        QObject::tr("Данные изменены. Сохранить изменения?"),
        QObject::tr("Сохранить и закрыть"),
        QObject::tr("Не сохранять и закрыть"),
        QObject::tr("Отмена")
        );
    if ( 0 == res ) {
      slotOkClicked();
      QDialog::accept();
      return;
    }
    else if ( 2 == res ) {
      return;
    }
    changed_ = false;
  }
  QDialog::reject();
}

void PuansonView::showMeteogram()
{
  if(!has_new_){return;}

  QApplication::setOverrideCursor(Qt::WaitCursor);
  map::MapWindow* mapWindow = MeteogramWindow::createWindow(0);
  map::Document* document = MeteogramWindow::createDocument(mapWindow);
  //mapWindow->setSizePolicy();
  mapWindow->menuBar()->hide();
  //mapWindow->
  ui_->scrollArea->setWidget(mapWindow);
  MeteogramWindow * meteogramWindow = new MeteogramWindow(mapWindow, document,false);
  QDateTime dt = TMeteoDescriptor::instance()->dateTime( puanson_->meteodata() );
  meteogramWindow->setBeginDateTime(dt.addDays(-5));
  meteogramWindow->setEndDateTime(dt);
  if(nullptr != mapWindow->mapview() && nullptr != mapWindow->mapview()->mapscene())
    {
      MapScene* scene = mapWindow->mapview()->mapscene();
      HideButtonsAction* act = qobject_cast< HideButtonsAction*> (scene->getAction("hidebuttonsaction"));
      if(nullptr != act){
          act->setVisibleButtons(false);
        }
    }

  QList<TMeteoData> mdList;
  QString stn = TMeteoDescriptor::instance()->stationIdentificator(meteodata_);
  mdList = meteogramWindow->loadSrcData(false,stn);
   if (!mdList.isEmpty()) {
     meteogramWindow->generateDoc(mdList);
  }
  has_new_ = false;
  QApplication::restoreOverrideCursor();

}
void PuansonView::tabchangedslot(int ct){
  if(2==ct)   showMeteogram();
}

}
}
