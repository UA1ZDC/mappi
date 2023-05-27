#include "servicewindow.h"
#include "ui_servicewindow.h"
#include "oldgeo.h"
#include <QFile>
#include <cross-commons/debug/tlog.h>
#include <meteo/commons/proto/map_isoline.pb.h>
#include <commons/textproto/tprototext.h>

#include <meteo/commons/proto/map_city.pb.h>
#include <fstream>
#include <displaysettingseditdialog.h>

ServiceWindow::ServiceWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ServiceWindow)
{
    ui->setupUi(this);

    //engage();
    //engage2();
    //connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(engage4()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(engage5()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(engage6()));
}

ServiceWindow::~ServiceWindow()
{
    delete ui;
}

class GeoRecord
{
  public:
    GeoRecord();

    int16_t index;
    int16_t map;
    int16_t bot;
    int16_t top;
    int32_t type;
    int16_t count;
    int16_t tlength;
    bool readFromFile( QFile* file );
    //bool writeToFile( QFile* file ) const ;

  private:
    int size() const;
};

GeoRecord::GeoRecord() {
  index = 0;
  map = 0;
  bot = 0;
  top = 0;
  type = 0;
  count = 0;
  tlength = 0;
}



int GeoRecord::size() const {
  int s = 0;
  s += sizeof( index );
  s += sizeof( map );
  s += sizeof( bot );
  s += sizeof( top );
  s += sizeof( type );
  s += sizeof( count );
  s += sizeof( tlength );
  return s;
}


bool GeoRecord::readFromFile( QFile* file )
{
  int rsize = size();
  QByteArray arr( rsize, 0 );
  if ( rsize != file->read( arr.data(), rsize ) ) {
    error_log << QObject::tr("Не удалось прочитать геоданные из файла %1").arg(file->fileName());
    return false;
  }
  int shift = 0;
  memcpy( &index, &(arr.data()[shift]), sizeof(index) );
  shift += sizeof(index);
  memcpy( &map, &(arr.data()[shift]), sizeof(map) );
  shift += sizeof(map);
  memcpy( &bot, &(arr.data()[shift]), sizeof(bot) );
  shift += sizeof(bot);
  memcpy( &top, &(arr.data()[shift]), sizeof(top) );
  shift += sizeof(top);
  memcpy( &type, &(arr.data()[shift]), sizeof(type) );
  shift += sizeof(type);
  memcpy( &count, &(arr.data()[shift]), sizeof(count) );
  shift += sizeof(count);
  memcpy( &tlength, &(arr.data()[shift]), sizeof(tlength) );

  return true;
}

void ServiceWindow::engage()//пишет протобуфер!!!!
{
    QString filename("/home/hal/src.git/meteo/commons/ui/map/loader/old.geo/files/vcity.geo");

    QTextCodec* cdc = QTextCodec::codecForName("koi8r");
    QFile file(filename);

    if ( false == file.exists() ) {
      error_log << QObject::tr("Файл с геоданными %1 не найден")
        .arg(filename);
      return;
    }

    if ( false == file.open( QIODevice::ReadOnly ) ) {
      error_log << QObject::tr("Не удалось открыть файл %1")
        .arg(filename);
      return;
    }



    QList <Aux> auxList;

    //2
    meteo::map::proto::MapSettings mapSettings;
    meteo::map::proto::MapSettingsConfig mapSettingsConfig;

    QString fileName("/home/hal/src.git/meteo/novost/etc/map.settings.conf");
    QFile file2(fileName);
    if ( false == file2.open( QIODevice::ReadOnly ) ) {
        error_log << QObject::tr("Не могу открыть файл %1 для чтения")
                     .arg(fileName);
    }

    QString str2 = QString::fromLocal8Bit( file2.readAll() );

    if ( false == TProtoText::fillProto( str2, &mapSettingsConfig) ) {
        error_log << QObject::tr("Не удалось загрузить настройки из файла %1")
                     .arg(fileName);
    }
    mapSettings = mapSettingsConfig.settings();
    //mapSettings.CopyFrom(mapSettingsConfig);
    QStringList nameConfFileList;
    //2



    QString str("");
    QStringList nameGeoList;
    GeoRecord rcrd;
    while ( false == file.atEnd() && true == rcrd.readFromFile(&file) )
    {
        Aux aux_record;

        QString substr("");
        substr += "index: " + QVariant(rcrd.index).toString() + "\n" +
                "map: " + QVariant(rcrd.map).toString() + "\n" +
                "bot: " + QVariant(rcrd.bot).toString() + "\n" +
                "top: " + QVariant(rcrd.top).toString() + "\n" +
                "type: " + QVariant(rcrd.type).toString() + "\n" +
                "count: " + QVariant(rcrd.count).toString() + "\n" +
                "tlength: " + QVariant(rcrd.tlength).toString() + "\n" +
                "----------\n";

        QByteArray arrname( rcrd.tlength+1, '\0' );
        if ( rcrd.tlength+1 != file.read( arrname.data(), rcrd.tlength+1 ) ) {
            warning_log << QObject::tr("Ошибка чтения населенного пункта");
            return;
        }
        arrname = arrname.left( arrname.size() - 1 );
        QString name;
        if ( 0 == cdc ) {
            name = QObject::tr( arrname.data());
        }
        else {
            name = cdc->toUnicode(arrname);
        }
        /*substr += "name: " + name + "\n";
        nameGeoList.append(name);*/
        aux_record.title = name;
        aux_record.titleBA = arrname;
        aux_record.title = aux_record.title.toLower();
        aux_record.title[0] = aux_record.title[0].toUpper();
        if (aux_record.title.contains("-")){
            int qty = aux_record.title.count("-");
            int ind = 0;
            for (int n = 0; n < qty; n++){
                ind = aux_record.title.indexOf("-", ind);
                ind++;
                aux_record.title[ind] = aux_record.title[ind].toUpper();
            }

        }
        aux_record.priority = 1;

        struct GP
        {
          double lat;
          double lon;
        };

        GP gp;
        int pntsize = sizeof( gp.lat ) + sizeof( gp.lon );
        if ( pntsize != file.read( reinterpret_cast<char*>(&gp), pntsize ) ) {
            error_log << QObject::tr("Не удалось прочитать данные для GeoPoint из файла %1")
                         .arg( file.fileName() );
            return;
        }
        aux_record.lat = gp.lat;
        aux_record.lon = gp.lon;

        /*substr += "lat: " + QVariant(gp.lat).toString() + "\n";
        substr += "lon: " + QVariant(gp.lon).toString() + "\n";

        substr += "\n====================\n";

        str += substr;*/

        //2
        for (int i = 0; i < mapSettings.city_settings_size(); i++){
            QString title = QString::fromStdString(mapSettings.city_settings(i).title());

            if (name == title){
                if (mapSettings.city_settings(i).has_priority())
                    aux_record.genLevel = mapSettings.city_settings(i).priority();
                else
                    aux_record.genLevel = mapSettings.city_settings(i).priority();

                if (mapSettings.city_settings(i).has_visible())
                    aux_record.visible = mapSettings.city_settings(i).visible();
                //else
                    aux_record.visible = true;
            }
            else {
                aux_record.genLevel = 20;
                aux_record.visible = true;
            }
        }

        //2
        aux_record.citymode = -1;

        auxList.append(aux_record);
    }

    str = "";
    meteo::map::proto::Cities cities;

    for (int i = 0; i < auxList.size(); i++){
        str += "title: " + auxList.at(i).title + "\n" +
                "lat: " + QVariant(auxList.at(i).lat).toString() + "\n" +
                "lon: " + QVariant(auxList.at(i).lon).toString() + "\n" +
                "generalization level: " + QVariant(auxList.at(i).genLevel).toString() + "\n" +
                "priority: " + QVariant(auxList.at(i).priority).toString() + "\n" +
                "citymode: " + QVariant(auxList.at(i).citymode).toString() + "\n" +
                "visible: " + QVariant(auxList.at(i).visible).toString() + "\n" +
                "\n=========================\n";

        meteo::map::proto::CitySettings_new * citySettings = cities.add_city_settings();
        citySettings->set_title(auxList.at(i).title.toStdString());
        //citySettings->set_title(auxList.at(i).titleBA);

        //cdc->fromUnicode(auxList.at(i).title).t;

        meteo::map::proto::GeoPoint geopoint;
        geopoint.set_lat_deg(auxList.at(i).lat);
        geopoint.set_lon_deg(auxList.at(i).lon);
        citySettings->mutable_geopoint()->CopyFrom(geopoint); // = geopoint;

        //citySettings->set_generalization_level(auxList.at(i).genLevel);
        //citySettings->set_priority(auxList.at(i).priority);
        //citySettings->set_city_mode(meteo::map::proto::Capital);
        //NO MODE; DEFAULT = LOCATION
        //citySettings->set_visible(auxList.at(i).visible);
    }

    //str = QString::fromStdString(cities.DebugString());
    str = QString::fromStdString(cities.Utf8DebugString());
    //str = QString::fromStdString(cities.city_settings(2).title());
    /*std::string sstr;
    cities.SerializeToString(&sstr);
    str = QString::fromStdString(sstr);*/


     //СЕРИАЛИЗАЦИЯ!!!
     std::fstream outFileStream("cityout.pbf", std::ios::out | std::ios::binary | std::ios::trunc);
    if (!cities.SerializeToOstream(&outFileStream)){
        error_log << "Не удалось записать протобуфер ((";
    }

    //cities.SerializeToOstream();
    //QFile file;


    nameGeoList.sort();
    //str = nameGeoList.join("\n");

    geoList = nameGeoList;

    ui->textBrowser->setText(str);
}

void ServiceWindow::engage2()
{

     //ВЫВОД vcity.geo
    QString filename("/home/hal/src.git/meteo/commons/ui/map/loader/old.geo/files/vcity.geo");

    QTextCodec* cdc = QTextCodec::codecForName("koi8r");
    QFile file(filename);

    if ( false == file.exists() ) {
      error_log << QObject::tr("Файл с геоданными %1 не найден")
        .arg(filename);
      return;
    }

    if ( false == file.open( QIODevice::ReadOnly ) ) {
      error_log << QObject::tr("Не удалось открыть файл %1")
        .arg(filename);
      return;
    }

    QString str("");
    GeoRecord rcrd;
    while ( false == file.atEnd() && true == rcrd.readFromFile(&file) )
    {

        QString substr("");
        substr += "index: " + QVariant(rcrd.index).toString() + "\n" +
                "map: " + QVariant(rcrd.map).toString() + "\n" +
                "bot: " + QVariant(rcrd.bot).toString() + "\n" +
                "top: " + QVariant(rcrd.top).toString() + "\n" +
                "type: " + QVariant(rcrd.type).toString() + "\n" +
                "count: " + QVariant(rcrd.count).toString() + "\n" +
                "tlength: " + QVariant(rcrd.tlength).toString() + "\n" +
                "----------\n";

        QByteArray arrname( rcrd.tlength+1, '\0' );
        if ( rcrd.tlength+1 != file.read( arrname.data(), rcrd.tlength+1 ) ) {
            warning_log << QObject::tr("Ошибка чтения населенного пункта");
            return;
        }
        arrname = arrname.left( arrname.size() - 1 );
        QString name;
        if ( 0 == cdc ) {
            name = QObject::tr( arrname.data());
        }
        else {
            name = cdc->toUnicode(arrname);
        }
        substr += "name: " + name + "\n";

        struct GP
        {
          double lat;
          double lon;
        };

        GP gp;
        int pntsize = sizeof( gp.lat ) + sizeof( gp.lon );
        if ( pntsize != file.read( reinterpret_cast<char*>(&gp), pntsize ) ) {
            error_log << QObject::tr("Не удалось прочитать данные для GeoPoint из файла %1")
                         .arg( file.fileName() );
            return;
        }


        substr += "lat: " + QVariant(gp.lat).toString() + "\n";
        substr += "lon: " + QVariant(gp.lon).toString() + "\n";

        substr += "\n====================\n";

        str += substr;
    }





    ui->textBrowser_2->setText(str);
//ВЫВОД vcity.geo

    /*nameConfFileList.sort();
    str = nameConfFileList.join("\n");

    str = QString::fromStdString(mapSettings.DebugString());

    ui->textBrowser_2->setText(str);
    confList = nameConfFileList;*/
}

void ServiceWindow::engage4()
{
    //ВЫВОД cityout.pbf
    std::fstream file("cityout.pbf", std::ios::in | std::ios::binary);
    meteo::map::proto::Cities cities;
    if (!cities.ParseFromIstream(&file)){
        ui->textBrowser_2->setText("что-то не то с твоим протобуфером ((");
        return;
    }

    QString str("");
    str = QString::fromStdString(cities.Utf8DebugString());
    ui->textBrowser_2->setText(str);
}

void ServiceWindow::engage5()
{//вызвать диалог, задать настройки отображения по умолчанию и сериализовать их в файл
    std::fstream file("displaysettings.pbf", std::ios::out | std::ios::binary | std::ios::trunc);

    DisplaySettingsEditDialog dialog;
    if (dialog.exec() != QDialog::Accepted)
        return;

    meteo::map::proto::DisplaySetting ds = dialog.getDisplaySetting();
    ds.set_is_default(true);

    ui->textBrowser->setText(QString::fromStdString(ds.Utf8DebugString()));

    meteo::map::proto::DisplaySettings dss;
    meteo::map::proto::DisplaySetting *dsPointer = dss.add_display_setting();
    dsPointer->CopyFrom(ds);

    ui->textBrowser_2->setText(QString::fromStdString(dss.Utf8DebugString()));

    //СЕРИАЛИЗАЦИЯ ОТКЛЮЧЕНА
    //dss.SerializeToOstream(&file);
}

void ServiceWindow::engage6()
{//вывод displaysettings.pbf
    std::fstream file("displaysettings.pbf", std::ios::in | std::ios::binary);

    meteo::map::proto::DisplaySettings dss;
    dss.ParseFromIstream(&file);

    ui->textBrowser_2->setText(QString::fromStdString(dss.Utf8DebugString()));
}

void ServiceWindow::engage3()
{
    //СВЕРЯЕТ vcity.geo и map.settings.conf, НЕДОРАБОТАНА (проверить списки строк и вывод, если надо будет запустить)
    QStringList::iterator iter = geoList.begin();
    while (iter != geoList.end()){
        if (confList.contains(*iter, Qt::CaseInsensitive)){
            //confList.removeOne(*iter);
            int i = confList.indexOf(*iter);
            if (i != -1) confList.replace(i, "THERE");//[i] = confList[i] + "_____";
        }
        iter++;
    }
    QString str = confList.join("\n");
    ui->textBrowser_2->setText(str);


    //meteo::map::proto::ci
}


