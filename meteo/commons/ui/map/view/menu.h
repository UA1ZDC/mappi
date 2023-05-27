#ifndef METEO_COMMONS_UI_MAP_VIEW_MAPMENU_H
#define METEO_COMMONS_UI_MAP_VIEW_MAPMENU_H

#include <qmap.h>
#include <qlist.h>
#include <qaction.h>
#include <qmenu.h>

namespace meteo {
namespace map {

const QString kStrDocument      = QObject::tr("Документ");
const QString kStrLayer         = QObject::tr("Слой");
const QString kStrObject        = QObject::tr("Объект");
const QString kAnalyse        = QObject::tr("Анализ");
const QString kStrOther         = QObject::tr("Прочее");

class MapView;
class ContextMenuEventAction;

class Menu : public QMenu
{
  Q_OBJECT
  public:
    Menu( MapView* view );
    ~Menu();

    void addAction( const QString& section, QAction* action );
    void addDocumentAction( QAction* action );
    void addLayerAction( QAction* action );
    void addObjectAction( QAction* action );
    void addAnalyseAction( QAction* );
    
    QMenu *addMenu(const QString&);
    
    QAction* addAction( const QString& a ) { return QMenu::addAction(a); }
    QAction* addAction( const QIcon& ico, const QString& a ) { return QMenu::addAction( ico, a ); }
    QAction* addAction( const QString& t, const QObject* r, const char* m, const QKeySequence& sh = 0 ) { return QMenu::addAction( t, r, m, sh ); }
    QAction* addAction( const QIcon& i, const QString& t, const QObject* r, const char* m, const QKeySequence& sh = 0 ) { return QMenu::addAction( i, t, r, m, sh ); }
    void addAction( QAction* a ) { QMenu::addAction(a); }

  private:
    MapView* view_;

    QMap<QString, QList<QAction*> > actions_;

    void buildMenu();

  friend class ContextMenuEventAction;

};

}
}

#endif // MAPMENU_H
