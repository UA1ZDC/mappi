#ifndef METEO_COMMONS_PUNCHRULES_H
#define METEO_COMMONS_PUNCHRULES_H

#include <commons/meteo_data/meteo_data.h>
#include <commons/meteo_data/tmeteodescr.h>
#include <meteo/commons/proto/puanson.pb.h>
#include <meteo/commons/proto/surface.pb.h>

namespace meteo {
namespace map {

QString stringFromRuleValue( float val, const QString& id, const puanson::proto::Puanson& punch, const QString& code = QString() );
QString stringFromRuleValue( float val, const puanson::proto::Id& id, const puanson::proto::Puanson& punch, const QString& code = QString() );
QString stringFromRuleValue( float val, const puanson::proto::CellRule& rule, const QString& code = QString() );
QString stringFromRuleValue( float val, const puanson::proto::CellRule::NumbeRule& rule );
QString stringFromRuleValue( float val, const puanson::proto::CellRule::SymbolRule& rule, const QString& code = QString() );

QFont fontFromRuleValue( float val, const puanson::proto::Id& id, const puanson::proto::Puanson& punch );
QFont fontFromRuleValue( float val, const puanson::proto::CellRule& rule );
QFont fontFromRuleValue( float val, const puanson::proto::CellRule::NumbeRule& rule );
QFont fontFromRuleValue( float val, const puanson::proto::CellRule::SymbolRule& rule );

QColor colorFromRuleValue( float val, const puanson::proto::Id& id, const puanson::proto::Puanson& punch );
QColor colorFromRuleValue( float val, const puanson::proto::CellRule& rule );
QColor colorFromRuleValue( float val, const puanson::proto::CellRule::NumbeRule& rule );
QColor colorFromRuleValue( float val, const puanson::proto::CellRule::SymbolRule& rule );

QMap<QString, QString> stringForCloud(const meteo::surf::MeteoDataProto* data, const puanson::proto::Puanson& punch );

class Param
{
  public:
    void setUuid(const std::string& uuid) { uuid_ = uuid; }
    std::string uuid() { return uuid_; }
    float operator[](const QString& param) const { return params_[param]; };
    bool operator==( const Param& param ) const;
    void appendParam( const QString& param, const float& value );
    bool contains(const QString& param) const { return params_.contains(param); }
    friend TLog& operator<<(TLog& out, const Param& p);
  private:
    std::string uuid_;
    QMap<QString,float> params_;
};

class ParamList : public QList<Param>
{
  public:
    void init( const puanson::proto::Puanson& punch );
    void append( const Param& param );
    QString getH();
    QString getC();
    QString getCH();
    QString getCM();
    QString getCL();
    QString getN();
    QString getRawC();
    QString getRawN();
    QString getRawH();
    QMap<QString, QString> getMap();

  private:
    puanson::proto::Puanson punch_;
    QHash<QString,::meteo::puanson::proto::CellRule>  paramsRules_; //словарь правил по имени параметра
};

}
}

#endif
