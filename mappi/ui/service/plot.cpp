#include <qglobal.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_symbol.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

#include "plot.h"
#include "circularbuffer.h"

Plot::Plot(QWidget *parent , float min_y, float max_y, double period, int numpoint ):
  QwtPlot( parent ),
  d_interval(numpoint* period ), // seconds
  d_period(period),
  d_timerId( -1 )
{
  // Assign a title
  setTitle( "" );

  QwtPlotCanvas *canvas = new QwtPlotCanvas();
  canvas->setFrameStyle( QFrame::Box | QFrame::Plain );
  canvas->setLineWidth( 1 );
  canvas->setPalette( Qt::white );

  setCanvas( canvas );
  // Axis
  setAxisTitle( QwtPlot::xBottom, "Seconds" );
  setAxisScale( QwtPlot::xBottom, -d_interval, 0.0 );
  setAxisTitle( QwtPlot::yLeft, "Values" );
  setAxisScale( QwtPlot::yLeft, min_y, max_y );
  alignScales();
  setSettings( );
}

Plot::~Plot()
{
  delete d_grid;
  delete d_curve;

}


//  Set a plain canvas frame and align the scales to it
//
void Plot::alignScales()
{
  // The code below shows how to align the scales to
  // the canvas frame, but is also a good example demonstrating
  // why the spreaded API needs polishing.

  for ( int i = 0; i < QwtPlot::axisCnt; i++ )
  {
    QwtScaleWidget *scaleWidget = axisWidget( i );
    if ( scaleWidget )
      scaleWidget->setMargin( 0 );

    QwtScaleDraw *scaleDraw = axisScaleDraw( i );
    if ( scaleDraw )
      scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
  }

  plotLayout()->setAlignCanvasToScales( true );
}

void Plot::setSettings(  )
{
  // Insert grid
  d_grid = new QwtPlotGrid();
  d_grid->attach( this );

  int numpoint = d_interval/d_period;
  // Insert curve
  d_curve = new QwtPlotCurve( QObject::tr("линия 1 ") );
  d_curve->setPen( Qt::blue );// цвет и толщина кривой
  d_curve->setRenderHint ( QwtPlotItem::RenderAntialiased, true ); // сглаживание
  d_curve->setData( new CircularBuffer( d_interval, numpoint ) );

  //d_curve->setStyle(QwtPlotCurve::Lines);
  d_curve->setStyle(QwtPlotCurve::Dots);
/*  QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
                                     QBrush( Qt::yellow ), QPen( Qt::red, 1 ), QSize( 2, 2 ) );
  // Маркеры кривой
  d_curve->setSymbol( symbol );
*/
  d_curve->attach( this );

  // Включить возможность приближения/удаления графика

  QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas());
  // клавиша, активирующая приближение/удаление
  magnifier->setMouseButton(Qt::MidButton);
  // Включить возможность перемещения по графику
  QwtPlotPanner *d_panner = new QwtPlotPanner( canvas() );
  // клавиша, активирующая перемещение
  d_panner->setMouseButton( Qt::RightButton );
  QwtPlotPicker *d_picker =
      new QwtPlotPicker(
        QwtPlot::xBottom, QwtPlot::yLeft, // ассоциация с осями
        QwtPlotPicker::CrossRubberBand, // стиль перпендикулярных линий
        QwtPicker::ActiveOnly, // включение/выключение
        canvas() ); // ассоциация с полем

  // Цвет перпендикулярных линий
  d_picker->setRubberBandPen( QColor( Qt::red ) );

  // цвет координат положения указателя
  d_picker->setTrackerPen( QColor( Qt::black ) );

  // непосредственное включение вышеописанных функций
  d_picker->setStateMachine( new QwtPickerDragPointMachine() );


  if ( d_timerId >= 0 )
    killTimer( d_timerId );

  d_timerId = startTimer( d_period);

  d_grid->setPen( QPen(Qt::SolidLine ));
  d_grid->setVisible( true );

  d_curve->setPen( Qt::blue, 2 ); // цвет и толщина кривой
  d_curve->setBrush( QBrush(Qt::NoBrush) );

  d_curve->setPaintAttribute( QwtPlotCurve::ClipPolygons);
  d_curve->setPaintAttribute( QwtPlotCurve::FilterPoints);
  d_curve->setRenderHint( QwtPlotItem::RenderAntialiased);
  QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>( canvas() );
  if ( plotCanvas == NULL )
  {
      plotCanvas = new QwtPlotCanvas();
      plotCanvas->setFrameStyle( QFrame::Box | QFrame::Plain );
      plotCanvas->setLineWidth( 1 );
      plotCanvas->setPalette( Qt::white );

      setCanvas( plotCanvas );
  }
  plotCanvas->setPaintAttribute( QwtPlotCanvas::BackingStore );
  plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint );

  QwtPainter::setPolylineSplitting(true );
}

void Plot::addPoint(double y)
{
  CircularBuffer *buffer = static_cast<CircularBuffer *>( d_curve->data() );
  buffer->addPoint(y);
}

void Plot::timerEvent( QTimerEvent * )
{
  CircularBuffer *buffer = static_cast<CircularBuffer *>( d_curve->data() );
  buffer->setReferenceTime( 0 );

  if (  true )
  {
    // the axes in this example doesn't change. So all we need to do
    // is to repaint the canvas.
    QMetaObject::invokeMethod( canvas(), "replot", Qt::DirectConnection );
  }
  else
  {
    replot();
  }
}
