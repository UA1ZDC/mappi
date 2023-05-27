#ifndef METEO_COMMONS_UI_PLUGINS_PUANSONEDITOR_H
#define METEO_COMMONS_UI_PLUGINS_PUANSONEDITOR_H

#include <qdialog.h>


#include <meteo/commons/proto/puanson.pb.h>

class QTreeWidget;

namespace Ui {
class PunchSelect;
}

namespace meteo {
namespace puanson {

class PunchSelect : public QDialog
{
  Q_OBJECT
  public:
    PunchSelect( QWidget* parent = 0 );
    ~PunchSelect();

    void setSelectedPunch( const meteo::puanson::proto::Puanson& punch );
    const meteo::puanson::proto::Puanson& selectedPunch() const { return selectedpunch_; }

  private:
    Ui::PunchSelect* ui_;
    meteo::puanson::proto::Puanson selectedpunch_;

    void loadPunchLibrary();
    void loadPunchesInTree( QTreeWidget* tree, const QMap<QString, meteo::puanson::proto::Puanson>& library );

    meteo::puanson::proto::Puanson currentPunch( bool* ok ) const ;

  private slots:
    void slotCurrentTabChanged( int indx );
    void slotAddPunch();
    void slotRmPunch();
    void slotEditPunch();

    void accept();
    void reject();
};

}
}

#endif
