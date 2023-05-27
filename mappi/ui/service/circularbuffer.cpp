#include "circularbuffer.h"
#include <math.h>

CircularBuffer::CircularBuffer( double interval, size_t numPoints )
{
  d_interval = interval;
  d_values.resize( numPoints+1 );
  d_values.fill( 0.0 );
  d_step = d_interval / ( numPoints);

}

void CircularBuffer::addPoint( double y  )
{
  cury_ = y;
}


void CircularBuffer::setReferenceTime( double  )
{
  d_values.pop_front();
  d_values.append(cury_);
}


size_t CircularBuffer::size() const
{
    return d_values.size();
}

QPointF CircularBuffer::sample( size_t i ) const
{
    const int size = d_values.size();
   // int index = d_startIndex + i;
    int index = i;
    if ( index >= size )
        index -= size;
    const double x = i * d_step -  d_interval;
    const double y = d_values.data()[index];
    return QPointF( x, y );
}

QRectF CircularBuffer::boundingRect() const
{
    return QRectF( -1.0, -d_interval, 2.0, d_interval );
}
