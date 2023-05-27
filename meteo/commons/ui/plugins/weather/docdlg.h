#ifndef METEO_COMMONS_UI_MAP_VIEW
#define METEO_COMMONS_UI_MAP_VIEW

#include <qdialog.h>

namespace meteo {
namespace map {
class DocDlg : public QDialog
{
  Q_OBJECT
  public:
    DocDlg( QWidget* parenti = nullptr );
    ~DocDlg();

  private:
    void keyReleaseEvent( QKeyEvent* event );
};
}
}

#endif
