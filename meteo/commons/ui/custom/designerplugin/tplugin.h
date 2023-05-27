#ifndef TPLUGIN_H
#define TPLUGIN_H

#include <qstring.h>
#include <qicon.h>

class TPlugin {

  public:
    TPlugin( const QString& include, const QString& name );

    bool isContainer() const ;
    QIcon icon() const ;
    QString domXml() const ;
    QString group() const ;
    QString includeFile() const ;
    QString name() const ;
    QString toolTip() const ;
    QString whatsThis() const ;

  protected:
    bool iscontainer_;
    QIcon icon_;
    QString group_;
    QString includefile_;
    QString name_;
    QString tooltip_;
    QString whatsthis_;
};

#define DESIGNER_IMPLEMENT( ClassName, INCLUDEFILE )                    \
  class ClassName##Plugin : public QObject, public QDesignerCustomWidgetInterface       \
{                                                                                       \
  Q_INTERFACES(QDesignerCustomWidgetInterface)                                          \
  public:                                                                               \
    ClassName##Plugin( QObject* p )                                                     \
      : QObject(p), plugface_( INCLUDEFILE, #ClassName ) {}                             \
    bool isContainer() const { return plugface_.isContainer(); }                        \
    QIcon icon() const { return plugface_.icon(); }                                     \
    QString domXml() const { return plugface_.domXml(); }                               \
    QString group() const { return plugface_.group(); }                                 \
    QString includeFile() const { return plugface_.includeFile(); }                     \
    QString name() const { return plugface_.name(); }                                   \
    QString toolTip() const { return plugface_.toolTip(); }                             \
    QString whatsThis() const { return plugface_.whatsThis(); }                         \
    QWidget* createWidget( QWidget* p ) { return new ClassName(p); }                    \
  private:                                                                              \
    TPlugin plugface_;                                                                  \
};

#define APPEND_NEW_PLUGIN( LIST, ClassName ) \
  LIST.append( new ClassName##Plugin(this) )
  

#endif
