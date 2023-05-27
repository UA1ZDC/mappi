#ifndef METEO_COMMONS_ASTRODATA_WINDOW_ASTRODATAWIDGET_H
#define METEO_COMMONS_ASTRODATA_WINDOW_ASTRODATAWIDGET_H

#include <meteo/commons/astrodata/astrodata.h>
#include <meteo/commons/astrodata/astrodatamodel.h>

#include <qmainwindow.h>
#include <qwidget.h>

class DateIntervalEdit;
class StationWidget;

namespace Ui
{
    class AstroDataWidget;
}


namespace astro
{
/** @brief */
class AstroDataWidget : public QMainWindow
{
    Q_OBJECT
public :
    explicit AstroDataWidget(QWidget *parent = nullptr);
    virtual ~AstroDataWidget();

    QString calendarInfo() const;

public slots :
    void openCalendar(const QString& filePath);
    void saveCalendar(const QString& filePath);
    void printCalendar();

private slots :
    void makeUp();
    void slotCmbTimeSpecChanged();
    void onOpenCalendar();
    void onSaveCalendar();
    void onPrintCalendar();

private :
  bool initStationWidget();



    Ui::AstroDataWidget* m_ui;
    AstroDataModel* m_model;
    AstroDataCalendar m_calendar;

};

}

#endif // _ASTRO_DATA_WIDGET_H_
