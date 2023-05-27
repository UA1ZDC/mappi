#include "pinningstorage.h"

namespace meteo {
namespace documentviewer {

static const QString pinningsPath = meteo::global::kDocumentViewerConfigPath + QObject::tr("pinning.bin");

PinningStorage::PinningStorage()
{
  this->deserialize();
}


PinningStorage::~PinningStorage()
{
  this->serialize();  
  while( false == savedPinnings.isEmpty() ) {

    auto documentClass = savedPinnings.keys().first();
    while ( false == savedPinnings[documentClass].isEmpty() ){      
      auto documentName = savedPinnings[documentClass].keys().first();
      auto pinning = savedPinnings[documentClass][documentName];      
      delete pinning;
      savedPinnings[documentClass].remove(documentName);
    }
    savedPinnings.remove(documentClass);
  }
}

const QHash<QString, PinningStorage::Pin*>& PinningStorage::getStorage( DocumentHeader::DocumentClass docClass )
{
  return this->savedPinnings[docClass];
}

void PinningStorage::addPinning( const DocumentHeader::DocumentClass docClass, Pin* pin )
{
  this->savedPinnings[docClass][pin->name] = pin;
}

bool PinningStorage::contains( const DocumentHeader::DocumentClass docClass, const QString& name )
{
  return this->savedPinnings[docClass].contains(name);
}

const PinningStorage::Pin& PinningStorage::getPin( DocumentHeader::DocumentClass docClass, const QString& name )
{
  return *this->savedPinnings[docClass][name];
}



void PinningStorage::serialize()
{
  QTime ttt;
  ttt.start();  
  meteo::proto::AllSavedPinnings allSavedPins;

  for ( auto datatype : this->savedPinnings.keys() ){

    for ( auto pinname: this->savedPinnings[datatype].keys() ){
      auto newPinning = allSavedPins.add_pinnings();
      newPinning->set_name(pinname.toStdString());
      auto pin = this->savedPinnings[datatype][pinname];
      newPinning->set_type(datatype);
      for ( int i = 0; i < meteo::map::Faxaction::markersCount; ++i ){
        *newPinning->add_geo() = geopoint2pbgeopoint(pin->mapPoints[i] );
        *newPinning->add_image() = qpoint2point(pin->imagePoints[i]);
      }
      for ( auto column : pin->conditions.keys() ){
        auto condition = pin->conditions[column];
        if ( false == condition.isNull() ) {
          newPinning->add_columns(column);
          auto conditions = newPinning->add_conditions();
          conditions->CopyFrom( condition.serialize() );
        }
      }
    }
  }  
  auto data = allSavedPins.SerializeAsString();  
  QFile ofile(pinningsPath);
  if ( true == ofile.open( QIODevice::WriteOnly ) ){
    ofile.write( data.c_str(), data.size() );
  }
}

void PinningStorage::deserialize()
{
  QFile ifile(pinningsPath);
  if ( true == ifile.open(QIODevice::ReadOnly)){
    QByteArray data = ifile.readAll();
    meteo::proto::AllSavedPinnings allSavedPins;
    allSavedPins.ParseFromString(data.toStdString());    
    for ( int i = 0; i < allSavedPins.pinnings_size(); ++i ){
      auto proto = allSavedPins.pinnings(i);
      auto pin = new Pin();
      pin->name = QString::fromStdString(proto.name());
      auto datatype = static_cast<DocumentHeader::DocumentClass>(proto.type());
      for ( int i = 0; i < meteo::map::Faxaction::markersCount; ++i ){
        pin->mapPoints[i] = pbgeopoint2geopoint(proto.geo(i));
        pin->imagePoints[i] = point2qpoint(proto.image(i));
      }
      for ( int i = 0; i < proto.conditions_size(); ++i ){
        auto col = static_cast<WgtDocViewPanel::TreeWidgetsColumns>(proto.columns(i));
        pin->conditions.insert( col, Condition::deserialize(proto.conditions(i)));
      }
      this->savedPinnings[datatype][pin->name] = pin;
    }    
  }
}

void PinningStorage::removePinning( const DocumentHeader::DocumentClass docClass, const QString& name )
{  
  auto pin = this->savedPinnings[docClass].value(name, nullptr);  
  if ( nullptr != pin ){
    this->savedPinnings[docClass].remove(name);
    while( false == pin->conditions.isEmpty() ) {
      auto key = pin->conditions.keys().first();
      auto conditon = pin->conditions[key];
      pin->conditions.remove(key);      
    }
    delete pin;
  }  
}

}
}
