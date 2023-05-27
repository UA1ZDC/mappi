#include "psql.h"

#include <pg_config.h>
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <iostream>

#include <qtimer.h>
#include <qsocketnotifier.h>
#include <qdatetime.h>
#include <qtextcodec.h>

#include <cross-commons/debug/tlog.h>

#include "psqlquery.h"

namespace meteo {

const QString DATETIME_FORMATSTRING = "yyyy-MM-dd hh:mm:ss";

const QString& Psql::dtFormat() {
  return DATETIME_FORMATSTRING;
}

Dbi* Psql::createDatabase( const ConnectProp& params )
{
  Dbi* client = new Psql(params);
  if ( false == client->connect() ) {
    error_log << QObject::tr("Ошибка подключения к БД");
    delete client;
    return nullptr;
  }
  return client;
}

QString Psql::dt2string( const QDateTime& dt, bool* ok ) {
  if ( false == dt.isValid() ) {
    if ( 0 != ok ) {
      *ok = false;
    }
    return QString();
  }
  if ( 0 != ok ) {
    *ok = true;
  }
  QString str = dt.toString( Qt::ISODate );
  str.remove("Z");
  return str;
}

QDateTime Psql::string2dt( const QString& string, bool* ok ) {
  if ( true == string.isEmpty() ) {
    if ( 0 != ok ) {
      *ok = false;
    }
    return QDateTime();
  }

  QDateTime dt = QDateTime::fromString( string, Qt::ISODate );

  if ( 0 != ok ) {
    *ok = dt.isValid();
  }
  dt.setTimeSpec(Qt::LocalTime);
  return dt;
}



bool PsqlResult::setData(const char * a_Data)
{
  if (a_Data == NULL)
  {
    Empty = true;
    Data = NULL;
    return false;
  }
  Empty = false;
  Data = a_Data;
  return true;
}

int PsqlResult::asInt()
{
  if (Empty) return 0;
  int Value;
  Value = atoi(Data);
  return Value;
}

double PsqlResult::asDouble()
{
  if (Empty) return 0;
  double Value;
  char *outstr;

  QString oldnumlocale = ::getenv("LC_NUMERIC");
  ::setlocale( LC_NUMERIC, "C" );
  Value = strtod(Data,&outstr);
  ::setlocale( LC_NUMERIC, ASCII_FROM_QSTRING(oldnumlocale) );

  return Value;
}

bool PsqlResult::asBool(){
  if (Empty) return false;
  return Data[0] == 't';
}

const char* PsqlResult::asString(){
  if (Empty) return "";
  return Data;
}

bool PsqlResult::asDateTime( QDateTime* dt ) {
  if ( 0 == dt ) {
    return false;
  }

  if ( Empty || 0 == Data ) {
    return false;
  }

  *dt = TCommon::datetimeFromString( Data, DATETIME_FORMATSTRING );

  return dt->isValid();
}

Psql::Psql()
  : Dbi(),
    mutex( QMutex::Recursive ),
    connection_(nullptr),
    pgResult_(nullptr),
    sockNotif_(nullptr),
    timer_(nullptr),
    busy_timer_(nullptr),
    connStatus_(CONNECTION_BAD),
    conn_busy_(false)
{
  setHostname("localhost");
  setPort(5432);
  timer_ = new TTimer(this);
  timer_->setInterval(10);
  QObject::connect( timer_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  busy_timer_ = new TTimer(this);
  QObject::connect(busy_timer_, SIGNAL(timeout()), this, SLOT(slotGetResult()));
}

Psql::Psql( const ConnectProp& prop )
  : Dbi(prop),
    mutex( QMutex::Recursive ),
    connection_(nullptr),
    pgResult_(nullptr),
    sockNotif_(nullptr),
    timer_(nullptr),
    busy_timer_(nullptr),
    connStatus_(CONNECTION_BAD),
    conn_busy_(false)
{
  if ( true == hostname().isEmpty() ) {
    setHostname("localhost");
  }
  if ( 0 == port() ) {
    setPort(5432);
  }
  timer_ = new TTimer(this);
  timer_->setInterval(10);
  QObject::connect( timer_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  busy_timer_ = new TTimer(this);
  QObject:: connect(busy_timer_, SIGNAL(timeout()), this, SLOT(slotGetResult()));
}

Psql::Psql(const QString& host, uint16_t port, const QString& db, const QString& login, const QString& pass )
  : Dbi( host, port, db, login, pass ),
    mutex( QMutex::Recursive ),
    connection_(nullptr),
    pgResult_(nullptr),
    sockNotif_(nullptr),
    timer_(nullptr),
    busy_timer_(nullptr),
    connStatus_(CONNECTION_BAD),
    conn_busy_(false)
{
  timer_ = new TTimer(this);
  timer_->setInterval(10);
  QObject::connect( timer_, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
  busy_timer_ = new TTimer(this);
  QObject:: connect(busy_timer_, SIGNAL(timeout()), this, SLOT(slotGetResult()));
}

Psql::~Psql()
{
  if ( nullptr != pgResult_ ) {
    PQclear(pgResult_);
    pgResult_ = nullptr;
  }
  disconnect();
  delete sockNotif_;
  sockNotif_ = nullptr;
}

void Psql::disconnect()
{
  mutex.lock();

  if ( nullptr != connection_ ) {
    PQfinish(connection_);
    connection_ = 0;
  }

  mutex.unlock();
}

void Psql::reset()
{
  mutex.lock();

  if ( nullptr != pgResult_ ) {
    PQclear(pgResult_);
    pgResult_ = nullptr;
  }
  delete sockNotif_;
  sockNotif_ = nullptr;
  if ( 0 != connection_ ) {
    PQfinish(connection_);
    connection_ = 0;
  }

  mutex.unlock();
}

bool Psql::connect()
{
  TMutexLocker l( &mutex );

  if ( PQstatus(connection_) == CONNECTION_OK ) {
    return true;
  }
  if ( nullptr != connection_ ) {
    disconnect();
  }
  connection_ = PQsetdbLogin( hostname().toUtf8(),
                              QString("%1").arg( port() ).toUtf8(),
                              NULL,
                              NULL,
                              dbname().toUtf8() ,
                              login().toUtf8(),
                              pass().toUtf8() );
  if ( PQstatus(connection_) == CONNECTION_BAD ) {
    error_log << QObject::tr("Ошибка подключения к БД.") << params().print();
    debug_log << PQerrorMessage(connection_);
    return false;
  }

  if ( 0 != PQsetClientEncoding(connection_, "UTF8") ) {
    error_log << tr("Ошибка: не удалось установить кодировку клиента.");
  }

  l.unlock();

  reconnectNotify();

  return true;
}

void Psql::reconnectNotify()
{
  QStringList::const_iterator cur = _notifyList.constBegin();
  QStringList::const_iterator end = _notifyList.constEnd();
  for(; cur!=end; ++cur ){
    addNotify(*cur);
  }
}

bool Psql::connectNonBlock()
{
  TMutexLocker l( &mutex );

  if(
      true == hostname().isEmpty() ||
      true == dbname().isEmpty() ||
      true == login().isEmpty() ||
      0 == port()
      )
  {
    return false;
  }

  QString conninfo = "dbname = " + dbname() + " host = " + hostname() + " user = " + login() + " port = " + port();
  if ( false == pass().isEmpty() ) {
    conninfo += " password = " + pass();
  }

  if ( timer_->isActive() ) {
    timer_->stop();
  }

  connection_ = PQconnectStart( conninfo.toUtf8() );
  if ( 0 == connection_ ) {
    emit connectionStatusChanged(CONNECTION_BAD);
    emit connectionNonBlockFailed();
    return false;
  }
  else if ( connStatus_ != PQstatus( connection_ ) ) {
    connStatus_ = PQstatus( connection_ );
    emit connectionStatusChanged(connStatus_);
  }

  PQsetnonblocking( connection_, 1 );

  if ( CONNECTION_OK != connStatus_ ) {
    timer_->start();
  }
  else {
    emit connectionNonBlockSucces();
  }

  if ( 0 != PQsetClientEncoding(connection_, "UTF8") ) {
    error_log << tr("Ошибка: не удалось установить кодировку клиента.");
  }

  return true;
}

void Psql::slotTimeout()
{
  if ( 0 == connection_ ) {
    if ( true == timer_->isActive() ) {
      timer_->stop();
    }
    if ( CONNECTION_BAD != connStatus_ ) {
      connStatus_ = CONNECTION_BAD;
      emit connectionStatusChanged(CONNECTION_BAD);
      emit connectionNonBlockFailed();
    }
    return;
  }

  PostgresPollingStatusType pollstatus = PQconnectPoll(connection_);
  ConnStatusType st = PQstatus(connection_);
  if ( st != connStatus_ ) {
    connStatus_ = st;
    emit connectionStatusChanged(connStatus_);
  }
  switch  (pollstatus) {
    case PGRES_POLLING_FAILED:
      debug_log << "PGRES_POLLING_FAILED";
      PQsetnonblocking( connection_, 0 );
      timer_->stop();
      emit connectionNonBlockFailed();
      break;
    case PGRES_POLLING_READING:
      debug_log << "PGRES_POLLING_READING";
      break;
    case PGRES_POLLING_WRITING:
      debug_log << "PGRES_POLLING_WRITING";
      break;
    case PGRES_POLLING_OK:
      PQsetnonblocking( connection_, 0 );
      timer_->stop();
      emit connectionNonBlockSucces();
      debug_log << "PGRES_POLLING_OK";
      break;
    case PGRES_POLLING_ACTIVE:
      debug_log << "PGRES_POLLING_ACTIVE";
      break;
  }
}

bool Psql::execQuery( const QString& sql )
{
  if( true == conn_busy_ ){
    error_log << QString("Ошибка выполнения запроса %1").arg(sql);
    error_log << tr("Другая команда выполняется в асинхронном режиме");
    return false;
  }
  if ( false == connected() && false == connect() ) {
    error_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return false;
  }
  if (sql.isEmpty() ) {
    error_log << QObject::tr("Пустой запрос");
    return false;
  }

  mutex.lock();
  if ( 0 != pgResult_ ) {
    PQclear(pgResult_);
    pgResult_ = 0;
  }

  QByteArray arr = sql.toUtf8();
//  char* escap = PQescapeLiteral( connection_, arr.data(), arr.size() );
  pgResult_ = PQexec( connection_, arr.constData() );
//  PQfreemem(escap);
  mutex.unlock();

  if ( PQresultStatus(pgResult_) != PGRES_COMMAND_OK && PQresultStatus(pgResult_) != PGRES_TUPLES_OK ) {
    error_log << tr("Ошибка выполнения запроса.");
    error_log << QString::fromUtf8(PQerrorMessage(connection_));
    debug_log << tr("SQL:") << sql;
    return false;
  }

  return true;
}

bool Psql::execQuery( DbiQuery* sql )
{
  if( true == conn_busy_ ){
    error_log << QString("Ошибка выполнения запроса %1").arg(sql->query());
    error_log << tr("Другая команда выполняется в асинхронном режиме");
    return false;
  }
  if ( false == connected() && false == connect() ) {
   debug_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return false;
  }
  if ( true == sql->query().isEmpty() ) {
    error_log << tr("Пустой запрос");
    return false;
  }

  mutex.lock();
  PsqlQuery* psql = static_cast<PsqlQuery*>(sql);
  QByteArray arr = psql->query().toUtf8();
//  char* escap = PQescapeLiteral( connection_, arr.data(), arr.size() );
  psql->res_ = PQexec( connection_, arr.constData() );
//  PQfreemem(escap);
  mutex.unlock();
  if ( PGRES_COMMAND_OK != PQresultStatus( psql->res_ ) && PGRES_TUPLES_OK != PQresultStatus( psql->res_ ) ) {
    error_log << tr("Ошибка выполнения запроса. Запрос = %1").arg( psql->query() );
    debug_log << QString::fromUtf8(PQerrorMessage(connection_));
    debug_log << "----- SQL -----\n" << psql->query() << "\n";
    return false;
  }

  return true;
}

DbiQuery* Psql::query( const QString& q )
{
  if ( true == q.isEmpty() ) {
    warning_log << QObject::tr("Пустой запрос!");
    return new PsqlQuery(this);
  }
  return new PsqlQuery( q, this );
}

std::unique_ptr<DbiQuery> Psql::queryptr( const QString& q )
{
  return std::unique_ptr<DbiQuery>( query(q) );
}

DbiQuery* Psql::queryByName( const QString& q )
{
  if ( false == global::kSqlQueries.contains(q) ) {
    warning_log << QObject::tr("Не найден запрос = %1").arg(q);
    return nullptr; 
  }
  return query( global::kSqlQueries[q] );
}

std::unique_ptr<DbiQuery> Psql::queryptrByName( const QString& q )
{
  return std::unique_ptr<DbiQuery>( queryByName(q) );
}

bool Psql::execSQL( const QString& sql )
{
  if ( false == connected() && false == connect() ) {
    error_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return false;
  }
  if (sql.isEmpty() ) {
    error_log << QObject::tr("Пустой запрос");
    return false;
  }

  if (!startTransaction()){
    return false;
  }

  mutex.lock();
  if ( 0 != pgResult_ ) {
    PQclear(pgResult_);
    pgResult_ = 0;
  }
  QByteArray arr = sql.toUtf8();
//  char* escap = PQescapeLiteral( connection_, arr.data(), arr.size() );
  pgResult_ = PQexec( connection_, arr.constData() );
  mutex.unlock();
  if ( PQresultStatus(pgResult_) != PGRES_COMMAND_OK && PQresultStatus(pgResult_) != PGRES_TUPLES_OK ) {
    error_log << tr("Ошибка выполнения запроса.");
    error_log << QString::fromUtf8(PQerrorMessage(connection_));
    debug_log << tr("SQL:") << sql;
    rollback();
    return false;
  }

  return commit();
}

PsqlResult* Psql::getCell( int row, int column )
{
  if ( row >= PQntuples(pgResult_) ) {
    res_.clear();
    return &res_;
  }
  if (PQgetisnull(pgResult_,row,column)) {
    res_.clear();
  }
  else {
    res_.setData(PQgetvalue(pgResult_, row ,column));
  }
  return &res_;
}

PsqlResult* Psql::getCell( int row, const char* column ) {
  int num = getColumnNum(column);
  if ( -1 == num ) {
    res_.clear();
    return &res_;
  }
  return getCell( row, num );
}

QString Psql::getValue( int row, int column, bool* ok ) const
{
  if ( 0 != ok ) {
    *ok = false;
  }
  if ( row >= PQntuples(pgResult_) ) {
    return QString();
  }
  if (PQgetisnull(pgResult_,row,column)) {
    return QString();
  }
  if ( 0 != ok ) {
    *ok = true;
  }
  const char* res = PQgetvalue(pgResult_, row ,column);
  return QString::fromUtf8(res);
}

QString Psql::getValue( int row, const QString& column, bool* ok ) const
{
  if ( 0 != ok ) {
    *ok = false;
  }
  if ( true == column.isEmpty() ) {
    return QString();
  }
  int num = getColumnNum( ASCII_FROM_QSTRING(column) );
  if ( -1 == num ) {
    return QString();
  }
  return getValue( row, num, ok );
}

PsqlResult* Psql::getNameCell(int x)
{
  if ( 0 == PQntuples(pgResult_) ) {
    res_.clear();
    return &res_;
  }
  res_.setData(PQfname(pgResult_, x));
  return &res_;
}

int Psql::getColumnNum( const char* column ) const
{
  if ( 0 == PQntuples(pgResult_) ) {
    return -1;
  }
  int num = PQfnumber( pgResult_, column );
  return num;
}

Psql::FieldFormat Psql::getColumnFormat( int column ) const
{
  switch( PQfformat( pgResult_, column ) ) {
    case 1 :
      return BINARY;
    case 0 :
    default :
      return TEXT;
  }
}

bool Psql::startTransaction()
{
  if ( false == connected() && false == connect() ) {
    error_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return false;
  }

  mutex.lock();
  PGresult *res = PQexec(connection_,"begin");
  mutex.unlock();

  if (PQresultStatus(res)!=PGRES_COMMAND_OK) {
    PQclear(res);
    //	    as_log->add(ASL_DEBUG,"Error starting transaction!");
    error_log << "Error starting transaction!";
    return false;
  }
  PQclear(res);

  return true;
}

bool Psql::rollback()
{
  if ( false == connected() && false == connect() ) {
    error_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return false;
  }

  mutex.lock();
  PGresult *res = PQexec(connection_,"rollback");
  mutex.unlock();

  if (PQresultStatus(res)!=PGRES_COMMAND_OK)
  {
    PQclear(res);
    error_log << "Psql:" << QString::fromLocal8Bit(PQerrorMessage(connection_));
    return false;
  }
  PQclear(res);
  error_log << "Rollback transaction";
  return true;
}

bool Psql::commit()
{
  if ( false == connected() && false == connect() ) {
    error_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return false;
  }

  mutex.lock();
  PGresult *res = PQexec(connection_,"commit");
  mutex.unlock();

  if (PQresultStatus(res)!=PGRES_COMMAND_OK)
  {
    PQclear(res);
    error_log << "Error commit transaction!";
    return false;
  }

  PQclear(res);
  return true;
}

// abort function = message + rollback
void Psql::abort(const char *message)
{
  error_log << "Psql:" << QString::fromLocal8Bit(message);
  rollback();
}

//insert data into large object from void* size int
int Psql::writeDataIntoLO(void *data, int data_size, Oid tmp_oid)
{
  int fd_oid;
  int w_num;

  startTransaction();

  TMutexLocker l(&mutex);
  if ((fd_oid = lo_open(connection_, tmp_oid, INV_WRITE))<0)
  {
    abort("Error while opening large object.");
    return 0;
  }

  if ((w_num = lo_write(connection_, fd_oid, (char*)data, data_size))<0)
  {
    abort("Error while writing data into large object.");
    return 0;
  }
  if (lo_close(connection_,fd_oid)<0)
  {
    abort("Error while closing large object.");
    return 0;
  }
  l.unlock();

  commit();
  return w_num;
}

//insert data into large object from void* size int
Oid Psql::insertDataIntoLO(void* data, int data_size)
{
  if ( false == connected() && false == connect() ) {
    error_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return 0;
  }

  Oid tmp_oid;

  int fd_oid;
  int w_num;

  startTransaction();

  TMutexLocker l(&mutex);
  tmp_oid = lo_creat(connection_, INV_READ|INV_WRITE);
  if (tmp_oid==0)
  {
    abort("Error while creating large object.");
    return 0;
  }
  if ((fd_oid = lo_open(connection_, tmp_oid, INV_WRITE))<0)
  {
    abort("Error while opening large object.");
    return 0;
  }
  if ((w_num = lo_write(connection_, fd_oid, (char*)data, data_size))<0)
  {
    abort("Error while writing data into large object.");
    return 0;
  }
  //as_log->add(ASL_DEBUG,"Write %d bytes into large object",w_num);
  //  fprintf(stderr,"Write %d bytes into large object\n",w_num);
  if (lo_close(connection_,fd_oid)<0)
  {
    abort("Error while closing large object.");
    return 0;
  }
  l.unlock();

  commit();

  return tmp_oid;
}

//insert data into large object from void* size int
Oid Psql::insertDataIntoLO(const char* data, int data_size)
{
  if ( false == connected() && false == connect() ) {
    error_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return false;
  }

  Oid tmp_oid;

  int fd_oid;
  int w_num;

  startTransaction();

  TMutexLocker l(&mutex);
  tmp_oid = lo_creat(connection_, INV_READ|INV_WRITE);
  if (tmp_oid==0)
  {
    abort("Error while creating large object.");
    return 0;
  }
  if ((fd_oid = lo_open(connection_, tmp_oid, INV_WRITE))<0)
  {
    abort("Error while opening large object.");
    return 0;
  }
  if ((w_num = lo_write(connection_, fd_oid, const_cast<char*>(data), data_size))<0) // в старой версии lo_write требует char*
  {
    abort("Error while writing data into large object.");
    return 0;
  }
  //as_log->add(ASL_DEBUG,"Write %d bytes into large object",w_num);
  //  fprintf(stderr,"Write %d bytes into large object\n",w_num);
  if (lo_close(connection_,fd_oid)<0)
  {
    abort("Error while closing large object.");
    return 0;
  }
  l.unlock();

  commit();

  return tmp_oid;
}

// read data from large object
int Psql::readDataFromLO( void* data, int data_size, Oid tmp_oid )
{
  int w_num;
  int fd_oid;

  startTransaction();

  TMutexLocker l(&mutex);
  if ((fd_oid = lo_open(connection_, tmp_oid, INV_READ))<0)
  {
    abort("Error while opening large object.");
    data = NULL;
    return -1;
  }

  if ((w_num = lo_read(connection_, fd_oid, (char*)data, data_size))<0)
  {
    abort("Error while reading data from large object.");
    data = NULL;
    return -1;
  }
  if (lo_close(connection_,fd_oid)<0)
  {
    abort("Error while closing large object.");
    data = NULL;
    return -1;
  }
  l.unlock();

  commit();

  return w_num;

}

// remove large object from database
int Psql::removeLO(Oid tmp_oid)
{
  int w_num;
  startTransaction();

  TMutexLocker l(&mutex);
  if ((w_num = lo_unlink(connection_, tmp_oid))<0)
  {
    abort("\nError while removing large object from database.");
    return -1;
  }
  l.unlock();

  commit();

  return w_num;
}

const char * Psql::errorMessage()
{
  TMutexLocker l(&mutex);
  return PQerrorMessage(connection_);
}

bool Psql::connected() const
{
  TMutexLocker l(&mutex);

  if ( 0 == connection_ ) {
    return false;
  }

  return PQstatus(connection_) == CONNECTION_OK;
}

int Psql::recordCount() const
{
  return PQntuples(pgResult_);
}

int Psql::columnCount() const
{
  return PQnfields(pgResult_);
}

bool Psql::addNotify(const QString& noti_name )
{
  if ( false == connected() ) {
    error_log
      << QObject::tr("Нет соединения")
      << QObject::tr("Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    return false;
  }

  TMutexLocker l( &mutex );

  QString sql = "LISTEN "+noti_name;
  pgResult_ = PQexec(connection_, ASCII_FROM_QSTRING(sql) );
  if (PQresultStatus(pgResult_) != PGRES_COMMAND_OK ){
    error_log << "LISTEN command failed:" << QString::fromLocal8Bit(PQerrorMessage(connection_));
    PQclear(pgResult_);
    pgResult_ = 0;
    return false;
  }

  PQclear(pgResult_);
  pgResult_ = 0;

  if ( 0 == sockNotif_ ) {
    int sock = PQsocket( connection_ );
    QSocketNotifier::Type atype = QSocketNotifier::Read ;
    sockNotif_ = new QSocketNotifier(sock, atype/*,this*/);
    QObject::connect( sockNotif_, SIGNAL( activated (int) ), this, SLOT( sqlNotify(int) ) );
  }

  if( !_notifyList.contains(noti_name) ){
    _notifyList << noti_name;
  }

  return true;
}

void Psql::sqlNotify(int )
{
  TMutexLocker l(&mutex);

  QStringList singleParamNotofies;
  QList<QPair<QString, QString> > doubleParamNotifies;

  PQconsumeInput(connection_);
  PGnotify *notify;
  while ((notify = PQnotifies(connection_)) != NULL){
    singleParamNotofies.append(QString(notify->relname));
    if ( 0 != notify->extra ) {
      doubleParamNotifies.append(qMakePair(singleParamNotofies.last(), QString(notify->extra)));
    }
    PQfreemem(notify);
  }

  l.unlock();

  for (QStringList::const_iterator it = singleParamNotofies.constBegin(), end = singleParamNotofies.constEnd(); it != end; ++it) {
    emit notifyActivated(it->toUtf8().data());
  }

#ifdef PG_VERSION_NUM
#if PG_VERSION_NUM > 89999
  typedef QList<QPair<QString, QString> >::const_iterator Iter;
  for (Iter it = doubleParamNotifies.constBegin(), end = doubleParamNotifies.constEnd(); it != end; ++it) {
    emit notifyActivated(it->first.toUtf8().data(), it->second.toUtf8().data());
  }
#endif
#endif
}

void Psql::sendQuery(const QString& sql)
{
  QString error;
  if ( false == connected() && false == connect() ) {
    error = QObject::tr("Нет соединения. Параметры соединения: Хост = %1 БД = %2 Пользователь = %3 Порт = %4")
      .arg( hostname() )
      .arg( dbname() )
      .arg( login() )
      .arg( port() );
    error_log << error;
    emit queryError(error);
    return;
  }
  if (sql.isEmpty() ) {
    error = QObject::tr("Пустой запрос");
    error_log << error;
    emit queryError(error);
    return;
  }
  query_ = sql;
  mutex.lock();
  if ( 0 != pgResult_ ) {
    PQclear(pgResult_);
    pgResult_ = 0;
  }
  if ( 0 == PQsendQuery(connection_, sql.toUtf8().constData()) ) {
    error_log << QString("Ошибка выполнения запроса %1").arg(sql);
    error_log << QString::fromUtf8(PQerrorMessage(connection_));
    emit queryError(QString(PQerrorMessage(connection_)));
    mutex.unlock();
    return;
  }
  conn_busy_ = true;
  busy_timer_->start(1);
  mutex.unlock();
}

void Psql::cancelQuery()
{
  if( 0 ==  PQrequestCancel(connection_) ){
    error_log << QObject::tr("Ошибка. Не удалость отменить команду") << PQerrorMessage(connection_);
  }
}

void Psql::slotGetResult()
{
  if( PQconsumeInput(connection_) == 1 && PQisBusy(connection_) == 0){
    busy_timer_->stop();
    while( (pgResult_ = PQgetResult(connection_)) ){
      conn_busy_ = false;
      if ( PQresultStatus(pgResult_) != PGRES_COMMAND_OK && PQresultStatus(pgResult_) != PGRES_TUPLES_OK ){
        error_log << "Psql: Error request to DataBase!";
        error_log << "Psql:" << query_;
        error_log << "Psql:" << QString::fromLocal8Bit(PQerrorMessage(connection_));
        emit queryError(QString(PQerrorMessage(connection_)));
      }
      else{
        emit queryDone();
      }
    }
  }
}

}
