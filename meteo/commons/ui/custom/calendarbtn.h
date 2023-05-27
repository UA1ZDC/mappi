#ifndef METEO_COMMONS_UI_CUSTOM_CALENDARBTN_H
#define METEO_COMMONS_UI_CUSTOM_CALENDARBTN_H

#include <qtoolbutton.h>
#include <qdatetime.h>
#include <meteo/commons/ui/custom/calendardlg.h>

class CalendarButton: public QToolButton
{
  Q_OBJECT
  public:
    CalendarButton( QWidget* parent );
    ~CalendarButton();

    const QDateTime& datetime() const { return dt_; }
    void setDatetime( const QDateTime& dt );

  private:
    QDateTime dt_;
    CalendarDlg* dlg_ = nullptr;

  private slots:
    void slotClicked();

  signals:
    void dtChanged( const QDateTime& dt );

};

#endif
