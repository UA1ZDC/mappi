#ifndef _PLOT_H_
#define _PLOT_H_ 1

#include <qwt_plot.h>
#include <qwt_system_clock.h>
//#include "settings.h"

class QwtPlotGrid;
class QwtPlotCurve;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget* , float min_y, float max_y , double period, int numpoint);
    ~Plot();

public Q_SLOTS:
    void setSettings( );
    void addPoint(double y);

protected:
    virtual void timerEvent( QTimerEvent *e );

private:
    void alignScales();

    QwtPlotGrid *d_grid = nullptr;
    QwtPlotCurve *d_curve = nullptr;

    double d_interval = 1;
    double d_period = 100;

    int d_timerId = -1;
};

#endif
