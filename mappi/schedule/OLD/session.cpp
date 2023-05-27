/*!
 * \class shedule::Session
 * \brief Сеанс приема спутника
 *
 * Сеанс определяется следующими параметрами:
 * - название спутника;
 * - время начала сеанса;
 * - время завершения сеанса;
 * - угол места в кульминации;
 * - направление спутника;
 * - состояние конфликта с другим сеансаом.
 *
 * \author Бурыкин Е.А.
 * \date 08.06.2010
 */
#include "session.h"
#include <commons/mathtools/mnmath.h>

#include <qstringlist.h>

#include <math.h>

// порядок полей в файле
#define _FIELDS_COUNT_ 7


using namespace mappi;
using namespace schedule;


Session::Session()
  : satellite::Schedule()
{
    defaults();
}

Session::Session( const Session& object )
  : satellite::Schedule( object )
{
  setNameSat(object.getNameSat());
  setAos(object.getAos());
  setLos(object.getLos());
  setMaxEl(object.getMaxEl());
  setRevol(object.getRevol());
  setDirection(object.getDirection());
  setState(object.getState());
}

Session::Session( const satellite::Schedule& object )
{
  setNameSat(object.name);
  setAos(object.aos);
  setLos(object.los);
  setMaxEl(object.maxEl);
  setRevol(object.revol);
  setDirection(object.direction);
  setState(object.state);
}

Session::Session( const conf::Session& object)
{
  setNameSat(QString::fromStdString(object.satellite()));
  setAos(QDateTime::fromString(QString::fromStdString(object.aos()), Qt::ISODate));
  setLos(QDateTime::fromString(QString::fromStdString(object.los()), Qt::ISODate));
  setMaxEl(object.maxel());
  setRevol(object.revol());
  setDirection(object.direction());
  setState(object.state());
}

Session& Session::operator=( const Session& object ) {
    setNameSat( object.getNameSat() );
    setAos( object.getAos() );
    setLos( object.getLos() );
    setMaxEl( object.getMaxEl() );
    setRevol( object.getRevol() );
    setDirection( object.getDirection() );
    setState( object.getState() );
    return *this;
}

Session::~Session() { }

Session& Session::operator=( const satellite::Schedule& object ) {
    setNameSat( object.name );
    setAos( object.aos );
    setLos( object.los );
    setMaxEl( object.maxEl );
    setRevol( object.revol );
    setDirection( object.direction );
    setState( object.state );
    return *this;
}

/*!
 * \brief Сравнение двух сеансов на равенство, указатели расписаний не сравниваются.
 * \return true если сеансы равны.
 */
bool Session::operator==( const Session& object ) const {
    return ( ( getNameSat() == object.getNameSat() )
        && ( getAos().toTime_t() == object.getAos().toTime_t() )
        && ( getLos().toTime_t() == object.getLos().toTime_t() )
        && ( ::fabsf( getMaxEl() - object.getMaxEl() ) < 0.001 )
        && ( getRevol() == object.getRevol() )
        && ( getDirection() == object.getDirection() )
        && ( getState() == object.getState() )
    );
}

/*! \brief Пустой сеанс (все значения по умолчанию). */
void Session::defaults() {
    name = QString::null;
    aos = los = QDateTime();
    maxEl = 89.99;
    revol = 0;
    direction = conf::kUnkDirection;
    state = conf::kUnkState;
}

/*!
 * \brief Сеанс является сеансом по умолчанию.
 * \return true является. 
 */
bool Session::isDefaults() const {
    return ( ( name.isNull() )
/*        && ( getAos().toTime_t() == QDateTime::currentDateTime( Qt::UTC ).toTime_t() )
        && ( getLos().toTime_t() == QDateTime::currentDateTime( Qt::UTC ).toTime_t() )
        && ( ::fabsf( maxEl - 89.99 ) < 0.001 )*/
        && ( getRevol() == 0 )
/*        && ( getDirection() == 0 )
        && ( getState() == satellite::UNKNOWN_STATE )*/
    );
}

/*!
 * \brief Проверка, сеанс принимается (имеет конфликты с другими сеансами)?
 * \return true если сеанс принимается.
 */
bool Session::adopted() const {
    return ( state == conf::kNormalState ) || ( state == conf::kSettlState )
        || ( state == conf::kUserEnableState );
}

/*!
 * \brief Текущая стадия сеанса.
 * \return стадия (пропущен, уже принимается или ожидается)
 */
Session::stage_t Session::currentStage() const {
  QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    if ( 0 < currentDateTime.secsTo( aos ) )
        return Session::EXPECTED;
    return ( 0 < currentDateTime.secsTo( los ) ? Session::ALREADY_TAKEN : Session::MISSING );
}

/*!
 * \brief Стадия сеанса на время dt
 * \return стадия (пропущен, уже принимается или ожидается)
 */
Session::stage_t Session::dtStage(const QDateTime& dt) const {
  QDateTime currentDateTime = dt;
    if ( 0 < currentDateTime.secsTo( aos ) )
        return Session::EXPECTED;
    return ( 0 < currentDateTime.secsTo( los ) ? Session::ALREADY_TAKEN : Session::MISSING );
}


/*!
 * \brief Разница между текущим временем и началом сеанса.
 * \return разница в секундах. Если разница положительная значит сеанс ожидается,
 * а если отрицательная пропущен или уже принимается.
 */
int Session::secsToAos() const {
    QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    return currentDateTime.secsTo( aos );
}

/*!
 * \brief Разница между текущим временем и концом сеанса.
 * \return разница в секундах. Если разница положительная значит сеанс ожидается
 * или уже принимается, а если отрицательная пропущен.
 */
int Session::secsToLos() const {
  QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    return currentDateTime.secsTo( los );
}

/*!
 * \brief Общая продолжительность сеанса.
 * \return продолжительность в секундах.
 */
int Session::length() const {
    return aos.secsTo( los );
}

/*!
 * \brief Преобразовать сеанс из строкового представления в объектное.
 * \param line - строковое представление сеанса.
 * \return true если формат строки соответсвует сеансу.
 */
bool Session::toSession( const QString& line ) {
    bool is_OK = false;
    //    QStringList fields = QStringList::split( "\t", line );
    QStringList fields = line.split("\t", QString::SkipEmptyParts);
    if ( _FIELDS_COUNT_ == fields.count() ) {

        setNameSat( fields[ 0 ] );
        setAos( QDateTime::fromString( fields[ 1 ], Qt::ISODate ) );
        setLos( QDateTime::fromString( fields[ 2 ], Qt::ISODate ) );
        setMaxEl( MnMath::deg2rad( ( fields[ 3 ] ).toDouble() ) );
        setRevol( ( fields[ 4 ] ).toUInt() );
        setDirection( ( static_cast<conf::SatDirection>(fields[ 5 ].toInt()) ) );
        setState( ( conf::ConflState )( ( fields[ 6 ] ).toInt() ) );
        is_OK = true;
    } // else
      // debug( "[ Session::toSession(1) ] - fields=[%i!=%i]\n\t%s;",
      // 	     int(fields.count()), _FIELDS_COUNT_, (const char*)line
      // 	     );

    return is_OK;
}

/*!
 * \brief Преобразовать сеанс в строку для записи в файл.
 * \return сеанс в строковом представлении.
 */
QString Session::toLine() const {
    QString result = QString( "%1\t\t%2\t%3\t")
        .arg( getNameSat() )
        .arg( getAos().toString( Qt::ISODate ) )
        .arg( getLos().toString( Qt::ISODate ) );
    result += QString( "%1\t%2\t%3\t%4" )
        .arg( MnMath::rad2deg( getMaxEl() ), 0, 'g', 4 )
        .arg( getRevol() )
        .arg( getDirection() )
        .arg( ( int )getState() );

    return result;
}
