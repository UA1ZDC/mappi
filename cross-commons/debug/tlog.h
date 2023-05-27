#ifndef CROSS_COMMONS_DEBUG_TLOG_H
#define CROSS_COMMONS_DEBUG_TLOG_H

#include <stdint.h>
#include <string>
#include <iostream>

//#include <cross-commons/includes/tcommoninclude.h>
#include <cross-commons/includes/tcrossdefines.h>

#include <qtextcodec.h>
#include <qtextstream.h>
#include <qiodevice.h>
#include <qstring.h>
#include <qvector.h>
#include <qsize.h>
#include <qrect.h>
#include <qline.h>

#ifdef QT_GUI_LIB
#include <qcolor.h>
#endif // QT_GUI_LIB

#if TQT_VERSION == 4
#include <qset.h>
#endif // TQT_VERSION == 4


namespace tlog
{

enum Priority { kNone, kDebug, kInfo, kWarning, kError, kCritical };
const int kPriorityMin = kNone;
const int kPriorityMax = kCritical;
const int kPriority_ARRAYSIZE = kPriorityMax + 1;

static const QString kPriorityStr[tlog::kCritical + 1] = {
  QString(),        // tlog::kNone
  QString("[D] "),  // tlog::kDebug
  QString("[I] "),  // tlog::kInfo
  QString("[W] "),  // tlog::kWarning
  QString("[E] "),  // tlog::kError
  QString("[C] ")   // tlog::kCritical
};

} // tlog

typedef void (*fMsgHandler)( tlog::Priority priority, const QString &facility, const QString& fileName, int line, const QString &msg ) ;

class TLog
{
public:
  static fMsgHandler setMsgHandler( fMsgHandler f );

  //! выводит все сообщения в stderr
  static void stdOut( tlog::Priority priority, const QString &facility, const QString& fileName, int line, const QString &msg );
  //! выводит info,warning,debug в stdout, error,critical в stderr;
  //! имя файла и строка выводится только для debug-сообщений
  static void clearStdOut( tlog::Priority priority, const QString &facility, const QString& fileName, int line, const QString &msg );
  //! логирование по средствам rsyslog
  static void syslogOut( tlog::Priority priority, const QString &facility, const QString& fileName, int line, const QString &msg );
  //! Вывод сообщений в файл, указанный с помощью setLogFileName()
  static void fileOut( tlog::Priority priority, const QString &facility, const QString& fileName, int line, const QString &msg );

  //! Изменяет файл для вывода сообщений на fileName. Если fileName является относительным путём,
  //! то добавляется префикс /var/log/
  static void setLogFileName(const QString& fileName);


  inline TLog( tlog::Priority p = tlog::kNone, const QString& f = QString(), const QString& file = QString(), int line = 0 ) :
       stream_( new Stream() ),
    priority_( p ),
    facility_( f ),
    fileName_( file ),
        line_( line )
  {
    init();
  }

  inline TLog( QIODevice* device, tlog::Priority p = tlog::kNone, const QString& f = QString(), const QString& file = QString(), int line = 0 ) :
       stream_( new Stream(device) ),
    priority_( p ),
    facility_( f ),
    fileName_( file ),
        line_( line )
  {
    init();
  }

  inline TLog( const TLog& debug ) :
    stream_( debug.stream_ ),
    priority_( debug.priority_ ),
    facility_( debug.facility_ ),
    fileName_( debug.fileName_ ),
        line_( debug.line_ )
  {
    init(debug.showMsgBox_);

    ++stream_->ref;
  }

  inline void init(bool showMsgBox = false)
  {
    showMsgBox_ = showMsgBox;
    if( 0 == TLog::_funcOut ) TLog::_funcOut = TLog::stdOut;
  }

  ~TLog();


  inline TLog& operator=(const TLog& other);

  inline TLog& msgBox()     { showMsgBox_ = true; return *this; }
  inline TLog& noMsgBox()   { showMsgBox_ = false; return *this; }

  inline TLog& space()      { stream_->space = true; stream_->ts << ' '; return *this; }
  inline TLog& nospace()    { stream_->space = false; return *this;  }
  inline TLog& maybeSpace() { if( stream_->space ) stream_->ts << ' '; return *this; }

  inline TLog& operator<<( QChar t )  { stream_->ts << '\'' << t << '\''; return maybeSpace(); }
  inline TLog& operator<<( bool t )   { stream_->ts << (t ? "true" : "false"); return maybeSpace(); }
  inline TLog& operator<<( char t )   { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( signed short t )   { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( unsigned short t ) { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( signed int t )     { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( unsigned int t )   { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( signed long t )    { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( unsigned long t )  { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( long long int t )                { stream_->ts << QString::number(t); return maybeSpace(); }
  // inline TLog& operator<<( int64_t t )                { stream->ts << QString::number(t); return maybeSpace(); }
  inline TLog& operator<<( unsigned long long int t ) { stream_->ts << QString::number(t); return maybeSpace(); }
  inline TLog& operator<<( float t )    { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( double t )   { stream_->ts << t; return maybeSpace(); }
  inline TLog& operator<<( const char* t )        { stream_->ts << QString::fromUtf8(t); return maybeSpace(); }
  inline TLog& operator<<( const std::string& t ) { stream_->ts << '\"' << QString::fromUtf8(t.data(), t.length()) << '\"'; return maybeSpace(); }
  inline TLog& operator<<( const QByteArray& t )  { stream_->ts << '\"' << QString::fromUtf8(t.data(), t.size()) << '\"'; return maybeSpace(); }
  inline TLog& operator<<( const QDateTime& t )   { stream_->ts << t.toString(Qt::ISODate); return maybeSpace(); }
  inline TLog& operator<<( const void* t )        { stream_->ts << const_cast<void*>(t); return maybeSpace(); }

  TLog& operator<<( const QString& s );
  TLog& operator<<( const QStringList& l );
  TLog& operator<<( const QList<QPoint>& l );
  inline TLog& operator<<( const QPoint& p ) { stream_->ts << "QPoint(" << p.x() << ',' << p.y() << ')'; return maybeSpace(); }
  inline TLog& operator<<( const QPointF& p ){ stream_->ts << "QPointF(" << p.x() << ',' << p.y() << ')'; return maybeSpace(); }
  inline TLog& operator<<( const QSize& s )  { stream_->ts << "(" << s.width() << "," << s.height() << ")"; return maybeSpace(); }
  inline TLog& operator<<( const QRect& r )  { stream_->ts << "(" << r.x() << "," << r.y() << ";" << r.width() << "x" << r.height() << ")";return maybeSpace(); }
  inline TLog& operator<<( const QRectF& r ) { stream_->ts << "(" << r.x() << "," << r.y() << ";" << r.width() << "x" << r.height() << ")";return maybeSpace(); }
  inline TLog& operator<<( const QLine& l )  { stream_->ts << "QLine(" << l.x1() << "," << l.y1() << ";" << l.x2() << "," << l.y2() << ")";return maybeSpace(); }

#ifdef QT_GUI_LIB

  inline TLog& operator<<( const QColor& c ) { stream_->ts << "QColor(" << c.red() << "," << c.green() << "," << c.blue() << "," << c.alpha() << ")";return maybeSpace(); }

#endif // QT_GUI_LIB


  inline TLog& operator<<( const TLog& l )
  {
    this->stream_->ts << l.stream_->buffer;
    l.stream_->message_output = false;
    return maybeSpace();
  }

  template<class K, class V> TLog& operator<<( const QMap<K,V>& m )
  {
    typename QMap<K,V>::const_iterator it = m.constBegin();
    typename QMap<K,V>::const_iterator end = m.constEnd();
    stream_->ts << "QMap {\n";
    for(; it!=end; ++it ){
      (*this) << "    " << it.key() << " : "
#if TQT_VERSION == 3
                 << it.data()
#else
                 << it.value()
#endif
                 << '\n';
    }
    stream_->ts << "}";
    return maybeSpace();
  }


#if TQT_VERSION == 4
  template<class K, class V> TLog& operator<<( const QHash<K,V>& m ){
    typename QHash<K,V>::const_iterator it = m.constBegin();
    typename QHash<K,V>::const_iterator end = m.constEnd();
    stream_->ts << "QHash {\n";
    for(; it!=end; ++it ){
      (*this) << "    " << it.key() << ":"
                        << it.value()
                        << '\n';
    }
    stream_->ts << "}";
    return maybeSpace();
  }

  template<class K> TLog& operator<<( const QList<K>& l )
  {
    stream_->ts << "QList {\n";
    for ( int i=0,isz=l.size(); i<isz; ++i ) {
      nospace() << "    " << l[i] << (i+1 < isz ? ",\n" : "\n");
    }
    stream_->ts << '}';
    return nospace();
  }

  template<class K> TLog& operator<<( const QSet<K>& set )
  {
    stream_->ts << "QSet {\n";
    int i = 0;
    int sz = set.size();
    for ( const K& item : set ) {
      nospace() << "    " << item << (i++ < sz-1 ? ",\n" : "\n");
    }
    stream_->ts << '}';
    return nospace();
  }

  template<class K> TLog& operator<<( const QVector<K>& vector )
  {
    stream_->ts << "QVector {\n";
    for ( int i=0,isz=vector.size(); i<isz; ++i ) {
      nospace() << "    " << vector[i] << (i < isz-1 ? ",\n" : "\n");
    }
    stream_->ts << '}';
    return nospace();
  }

  template<class F, class S> TLog& operator<<( const QPair<F,S>& p ){
    stream_->ts << "QPair {" << p.first << " ; " << p.second << "}";
    return maybeSpace();
  }
#endif // TQT_VERSION == 4

private:
  static fMsgHandler  _funcOut;
  static QString logFileName_;

  struct Stream {
    Stream()
      : ts(&buffer, TWRITE_ONLY), ref(1), space(true), message_output(true)  { ts.setCodec( QTextCodec::codecForLocale() ); }
    Stream( QIODevice* device )
      : ts(device), ref(1), space(true), message_output(false) {}
    QTextStream ts;
    QString     buffer;
    int  ref;
    bool space;
    bool message_output;
  } *stream_;

  tlog::Priority priority_;
  QString facility_;
  QString fileName_;
  int line_;
  bool showMsgBox_;
};


inline TLog& TLog::operator=( const TLog& other )
{
  if( this != &other ){
    TLog copy( other );
    Stream* s = stream_;
    stream_ = other.stream_;
    copy.stream_ = s;
  }
  return *this;
}


const QString log_module_name = LOG_MODULE_NAME;
#define none_log      TLog( tlog::kNone,     log_module_name, __FILE__, __LINE__ ).noMsgBox()
#define debug_log     TLog( tlog::kDebug,    log_module_name, __FILE__, __LINE__ ).noMsgBox()
#define info_log      TLog( tlog::kInfo,     log_module_name, __FILE__, __LINE__ ).noMsgBox()
#define warning_log   TLog( tlog::kWarning,  log_module_name, __FILE__, __LINE__ ).noMsgBox()
#define error_log     TLog( tlog::kError,    log_module_name, __FILE__, __LINE__ ).noMsgBox()
#define critical_log  TLog( tlog::kCritical, log_module_name, __FILE__, __LINE__ ).noMsgBox()

#define none_msg      none_log.msgBox()
#define debug_msg     debug_log.msgBox()
#define info_msg      info_log.msgBox()
#define warning_msg   warning_log.msgBox()
#define error_msg     error_log.msgBox()
#define critical_msg  critical_log.msgBox()

#define none_log_if( c )      if ( c ) TLog( tlog::kNone,     log_module_name, __FILE__, __LINE__ )
#define debug_log_if( c )     if ( c ) TLog( tlog::kDebug,    log_module_name, __FILE__, __LINE__ )
#define info_log_if( c )      if ( c ) TLog( tlog::kInfo,     log_module_name, __FILE__, __LINE__ )
#define warning_log_if( c )   if ( c ) TLog( tlog::kWarning,  log_module_name, __FILE__, __LINE__ )
#define error_log_if( c )     if ( c ) TLog( tlog::kError,    log_module_name, __FILE__, __LINE__ )
#define critical_log_if( c )  if ( c ) TLog( tlog::kCritical, log_module_name, __FILE__, __LINE__ )

#if TQT_VERSION == 3
#define QSTR_TOCHAR(X) X.ascii()
#else
#define QSTR_TOCHAR(X) X
#endif

#define time_log        none_log << QTime::currentTime().toString("hh:mm:ss.zzz")
#define trc             debug_log << __func__
#define printvar( v )   debug_log << #v":" << (v)
#define var( v )        debug_log << #v":" << (v)
#define varname( v )    #v":" << (v)
#define assert_log( c ) debug_log_if( false == (c) ) << "ASSERT:"<< #c
#define not_impl        trc << " < < < !!! NOT IMPLEMENTED !!! > > > "

#endif // CROSS_COMMONS_DEBUG_TLOG_H
