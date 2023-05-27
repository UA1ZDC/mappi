#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <qwt_series_data.h>
#include <qvector.h>

class CircularBuffer: public QwtSeriesData<QPointF>
{
public:
    CircularBuffer( double interval = 10.0, size_t numPoints = 1000 );
    void addPoint( double y  );

    void setReferenceTime( double );

    virtual size_t size() const;
    virtual QPointF sample( size_t i ) const;

    virtual QRectF boundingRect() const;

    void setFunction( double( *y )( double ) );

private:

    double d_interval;
    QVector<double> d_values;

    double d_step=1;

    double interval = 10;

    double cury_=0;
};

#endif
