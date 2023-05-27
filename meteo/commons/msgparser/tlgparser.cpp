#include "tlgparser.h"

#include <google/protobuf/descriptor.h>

#include <cross-commons/debug/tlog.h>
#include <meteo/commons/xmlproto/txmlproto.h>
#include <meteo/commons/global/global.h>

#include <meteo/commons/proto/msgparser.pb.h>

#include "tlgmsg.h"

namespace meteo
{

TlgParser::TlgParser( const QByteArray& arr, bool splitstuckedmessages_ )
  : tlgmsg_( new TlgMsg )
{
  tlgmsg_->setStuckedSplit(splitstuckedmessages_);
  setData( arr );
}

TlgParser::TlgParser( const QByteArray& arr, const QString& rulename, bool splitstuckedmessages_ )
  : tlgmsg_( new TlgMsg(rulename) ),
  rulename_(rulename)
{
  tlgmsg_->setStuckedSplit(splitstuckedmessages_);
  setData( arr );
}

TlgParser::~TlgParser()
{
  delete tlgmsg_;
  tlgmsg_ = 0;
}

void TlgParser::setData( const QByteArray& arr )
{
  arr_ = arr;
  tlgmsg_->reset();
}

bool TlgParser::parseNextMessage( tlg::MessageNew* newmsg )
{
  newmsg->Clear();
  int st = tlgmsg_->parseArray( &arr_ );
  while ( ( TlgMsg::SEARCH_BINARYSIZE  == st
         || TlgMsg::SEARCH_BINARYPART  == st
         || TlgMsg::SEARCH_BINARYCHUNK == st
         || TlgMsg::SEARCH_BINARYEND  == st ) 
         && false == tlgmsg_->toolittlearray() ) {
    if ( true == tlgmsg_->stuckedSplit() && TlgMsg::SEARCH_BINARYCHUNK == st ) {
      break;
    }
    st = tlgmsg_->parseArray( &arr_ );
  }
  if ( TlgMsg::UNKNOWN_TLGSTATE == st ) {
    TlgMsg::saveUnparsed( arr_.left(1) );
    brokenTlg_.Emit();
    arr_.remove( 1, 1 );
    return parseNextMessage(newmsg);
  }
  if ( TlgMsg::MESSAGE_PARSED == st || ( TlgMsg::SEARCH_BINARYCHUNK == st && true == tlgmsg_->stuckedSplit() ) ) {
    fillMessageNew(newmsg);
    fillBbb(newmsg);
    return true;
  }
   return false;
}

bool fillProto( const msgformat::TMsgFormat::Section& section, const QString& str, google::protobuf::Message* msg )
{
  QRegExp ex = section.exp;
  QMap<int,QString> parts = section.headerparts;
  if ( 0 != str.indexOf(ex) ) {
    error_log << QObject::tr("Неизвестная ошибка!");
    return false;
  }
  for ( int i = 1, sz = ex.captureCount(); i <= sz; ++i ) {
    if ( false == parts.contains(i) ) {
      continue;
    }
    const QString& part = parts[i];
    QStringList partsplit = part.split('.', QString::SkipEmptyParts );
    QString capstring = ex.cap(i);
    if ( true == capstring.isEmpty() ) {
      continue;
    }
    if ( 1 == partsplit.size() ) {
      TXmlProto::setValue( msg, parts[i], QStringList( capstring ) );
    }
    else if ( 2 == partsplit.size() ) {
      const google::protobuf::Reflection* msgRefl = msg->GetReflection();
      const google::protobuf::Descriptor* msgDesc = msg->GetDescriptor();
      const google::protobuf::FieldDescriptor* fieldDesc = msgDesc->FindFieldByLowercaseName( partsplit[0].toLower().toStdString() );
      bool isMessage  = fieldDesc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE;
      bool isRepeated = fieldDesc->is_repeated();
      if ( isMessage && !isRepeated ){
        google::protobuf::Message* m = msgRefl->MutableMessage( msg, fieldDesc );
        TXmlProto::setValue( m, partsplit[1], QStringList( capstring ) );
      }
      else if ( isMessage && isRepeated ) {
        google::protobuf::Message* m = 0;
        if ( 0 == msgRefl->FieldSize( *msg, fieldDesc ) ) {
          m = msgRefl->AddMessage( msg, fieldDesc );
        }
        else {
          m = msgRefl->MutableRepeatedMessage( msg, fieldDesc, 0 );
        }
        TXmlProto::setValue( m, partsplit[1], QStringList( capstring ) );
      }
    }
  }
  return true;
}

void TlgParser::fillMessageNew( tlg::MessageNew* msg ) const
{
  fillProto( tlgmsg_->startRule(), tlgmsg_->startline(), msg->mutable_startline() );
  fillProto( tlgmsg_->headerRule(), tlgmsg_->header(), msg->mutable_header() );
  QString headername = tlgmsg_->headerRule().name;
  if ( "circular" == headername ) {
    msg->set_addrmode( tlg::kCircular );
  }
  else if ( "address" == headername ) {
    msg->set_addrmode( tlg::kAddress );
  }
  else if ( "multiaddress" == headername ) {
    msg->set_addrmode( tlg::kMultiAddres );
  }
  else if ( "convert" == headername ) {
    msg->set_addrmode( tlg::kConvert );
  }
  if ( "wmo" == tlgmsg_->msgrule().name ) {
    msg->set_format( tlg::kWMO );
  }
  else if ( "gms" == tlgmsg_->msgrule().name ) {
    msg->set_format( tlg::kGMS );
  }
  /*else {
    msg->set_format( tlg::kOther );
  }*/
  msg->mutable_startline()->set_data( std::string( tlgmsg_->startline().data(), tlgmsg_->startline().size() ) );
  msg->mutable_header()->set_data( std::string( tlgmsg_->header().data(), tlgmsg_->header().size() ) );
  msg->set_msg( std::string( tlgmsg_->message().data(), tlgmsg_->message().size() ) );
  msg->set_end( std::string( tlgmsg_->end().data(), tlgmsg_->end().size() ) );
  if ( TlgMsg::BINTLG == tlgmsg_->type() ) {
    msg->set_isbinary(true);
  }
  else {
    msg->set_isbinary(false);
  }
  msg->set_msgtype( tlgmsg_->msgtype().toStdString() );
}

void TlgParser::fillBbb( tlg::MessageNew* msg ) const
{
  if ( false == msg->header().has_bbb() ) {
    msg->clear_cor_number();
    return;
  }
  int cornumber = global::bbb2cornumber( QString::fromStdString( msg->header().bbb() ) );
  if ( 0 < cornumber ) {
    msg->set_cor_number(cornumber);
  }
}

QByteArray TlgParser::tlg() const
{
  return tlgmsg_->tlg();
}

const QByteArray& TlgParser::startline() const
{
  return tlgmsg_->startline();
}

const QByteArray& TlgParser::header() const
{
  return tlgmsg_->header();
}

const QByteArray& TlgParser::message() const
{
  return tlgmsg_->message();
}

const QByteArray& TlgParser::end() const
{
  return tlgmsg_->end();
}

}
