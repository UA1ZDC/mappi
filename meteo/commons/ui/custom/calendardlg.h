#ifndef METEO_COMMONS_UI_CUSTOM_CALENDARDLG_H
#define METEO_COMMONS_UI_CUSTOM_CALENDARDLG_H

#include <qdialog.h>
#include <qlist.h>
#include <qdatetime.h>

class QToolButton;
namespace Ui {
  class CalendarDlg;
}

class CalendarDlg : public QDialog
{
  Q_OBJECT
  public:
    CalendarDlg( QWidget* prnt );
    ~CalendarDlg();

    const QDateTime currentDt() const ;
    void setDt( const QDate& date, const QTime& time );

  protected:
    void keyReleaseEvent( QKeyEvent* event );

  private:
    Ui::CalendarDlg* ui_ = nullptr;
    QDateTime dt_;
};

#endif
