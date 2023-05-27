#include "widget.h"
#include "qcustomplot.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent), emptybar(0)
{
    resize(1200,600);
    setWindowTitle(QString::fromUtf8("Гистограмма"));
    customPlot = new QCustomPlot(this);
    vbox = new QVBoxLayout(this);
    vbox->addWidget(customPlot);
    //res = new QPushButton(QString::fromUtf8("Показать или обновить гистограмму"),this);
    //vbox->addWidget(res);
    setLayout(vbox);

    // Сигналы
    //connect(res,SIGNAL(clicked()),this,SLOT(rndres()));
}

Widget::~Widget()
{
  if (emptybar != 0)
  {
    delete emptybar;
    emptybar = 0;
  }
}

//void Widget::rndres(const QVector<QString> &labels,const QVector<double> &cnt, const QString& str)
//{
//    fossil = new QCPBars(customPlot->xAxis, customPlot->yAxis);
//    customPlot->addPlottable(fossil);
//    // Установки цвета:
//    QPen pen;
//    pen.setWidthF(1.5);//Толщина контура столбца
//    fossil->setName(str); // Легенда
//    pen.setColor(QColor(50, 50, 100));// Цвет контура столбца
//    fossil->setPen(pen);
//    // Цвет самого столбца, четвертый параметр - прозрачность
//    fossil->setBrush(QColor(50, 50, 250, 70));

//    // Установки значений оси X:
//    QVector<double> ticks;
//    for (int k=0,ksz=labels.count();k<ksz;++k)
//    {
//      ticks << k+1;
//    }

////    QVector<QString> labels;
//    //ticks << 1 << 2 << 3 << 4 << 5 << 6 << 7;
//    //labels << "1" << "2" << "3" << "4" << "5" << "6" << "7";
//    customPlot->xAxis->setAutoTicks(false);
//    customPlot->xAxis->setAutoTickLabels(false);
//    customPlot->xAxis->setTickVector(ticks);
//    customPlot->xAxis->setTickVectorLabels(labels);
//    customPlot->xAxis->setSubTickCount(0);
//    //customPlot->xAxis->setTickLength(0, 4);
//    customPlot->xAxis->grid()->setVisible(true);
//    customPlot->xAxis->setRange(0, labels.count()+1);

//    int max = 0;
//    QVector<QString> labelsY;
//    QVector<double> ticksY;
//    for (int i=0,isz=cnt.count(); i<isz; ++i)
//    {
//      if (cnt.at(i)>max) { max = cnt.at(i); }
//      labelsY << QString::number((int)cnt.at(i));
//      ticksY << cnt.at(i);
//      qDebug() << labelsY;
//    }
//    qDebug() << max;
//    // Установки значений оси Y:
//    customPlot->yAxis->setAutoTickStep(false);
////    customPlot->yAxis->setAutoTickLabels(false);
//    int stepY = 1;
//    if (max/2*2 != max) { stepY = (int)(max/5); }
//    else  { stepY = (int)(max/4); }
//    customPlot->yAxis->setTickStep(stepY);

//    customPlot->yAxis->setRange(0, max);
////    customPlot->yAxis->setPadding(5);
////    customPlot->xAxis->setAutoTickStep(false);
////    customPlot->xAxis->setAutoTickLabels(false);
//    customPlot->yAxis->setSubTickCount(0);
//    customPlot->yAxis->setLabel(QString::fromUtf8("Количество попаданий в полосу КА"));
//    customPlot->yAxis->grid()->setSubGridVisible(true);
//    QPen gridPen;
//    gridPen.setStyle(Qt::SolidLine);
//    gridPen.setColor(QColor(0, 0, 0, 25));
////    customPlot->yAxis->grid()->setPen(gridPen);
//    gridPen.setStyle(Qt::DotLine);
////    customPlot->yAxis->grid()->setSubGridPen(gridPen);


//    // Данные:
//    //QVector<double> fossilData = cnt;
//    qsrand (time(NULL));

////    fossilData  << qrand() % 10 + 2.5
////                << qrand() % 10 + 2.5
////                << qrand() % 10 + 2.5
////                << qrand() % 10 + 2.5
////                << qrand() % 10 + 2.5
////                << qrand() % 10 + 2.5
////                << qrand() % 10 + 2.5;
//    fossil->setData(ticks, cnt);

//    // Легенда:
//    customPlot->legend->setVisible(true);
//    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
//    customPlot->legend->setBrush(QColor(255, 255, 255, 200));
//    QPen legendPen;
//    legendPen.setColor(QColor(130, 130, 130, 200));
//    customPlot->legend->setBorderPen(legendPen);
//    QFont legendFont = font();
//    legendFont.setPointSize(10);
//    customPlot->legend->setFont(legendFont);
//    //customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
//    customPlot->replot();

//    // Сброс всех установок графика:
//    customPlot->removePlottable(fossil);
//}


void Widget::setupBars(QVector<QString> cities)
{
  for (int i=0;i<cities.count();++i) //kol-vo periodov
  {
    QCPBars *bar = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    bar->addToLegend();
    //bar->setName("Period "+QString::number(i));
    bar->setName(cities.at(i));
    int randomR = qrand()%256;
    int randomG = qrand()%256;
    int randomB = qrand()%256;
    QColor* color = new QColor(randomR, randomG, randomB);
    bar->setPen(QPen(*color));
    bar->setBrush(*color);
    //bar->setAntialiased(false);
    bar->setWidth(0.8);
    bars.append(bar);
//    if (i!=0)
//    {
//      QCPBars *barAbove = bars.at(i-1);
//      if (0 != barAbove)
//      {
//        bar->moveAbove(barAbove);
//      }
//    }
  }
}

int Widget::findMaxY()
{
    double max;
    for (int m=0; m<bars.count();++m) //перебираем bars
    {
      for (int k=0,ksz=bars.at(m)->data()->count();k<ksz;++k)
      {
        double val =bars.at(m)->data()->values().at(k).value;
        if (val>max) {max=val;}
      }
    }
//    qDebug() << "MAXY=" << max;
    return (int) max;
}

void Widget::plot(QVector<QStringList> data, QStringList times, QVector<QString> cities, const QString& satelite)
{
  qDebug() << satelite;
  qDebug() << times;
  setupBars(cities);
  double maxY = 0;
  QVector<QString> labelsX;
  QVector<double> ticksX;
  int num = 0;
  int point = 1;
  if (cities.count()/2*2 != cities.count())
  {
    point = cities.count()/2;
  }
  else
  {
    point = cities.count()/2-1;
  }
//  qDebug() << "point" << point << cities.count() << cities.count()/2;
  for (int m=0,msz=times.count();m<msz;++m)//perebor periodov
  {
    QString str = times.at(m);
    for (int k=0,ksz=data.count();k<ksz;++k) //perebor gorodov
    {
      if (data.at(k).count()>m)
      {
        double value = data.at(k).at(m).toDouble();
        if (value == 0) { value = 0.05; }
        if (bars.count()>k)
        {
          //qDebug() << "period: " << m << "punkt" << k << "bar" << k << "num" << num;
          bars.at(k)->data()->insert(num,QCPBarData(num,value));
          ticksX << num;
          if (k == point)
          {
            labelsX << str;
          }
          else
          {
            labelsX << "";
          }
        }
        num++;
      }
    }
    num++;
    ticksX << num;
    num++;
    ticksX << num;
    labelsX << "" << "";
  }

  customPlot->xAxis->setAutoTicks(false);
  customPlot->xAxis->setAutoTickLabels(false);
  customPlot->xAxis->setTickVector(ticksX);
  customPlot->xAxis->setTickVectorLabels(labelsX);
  customPlot->xAxis->setSubTickCount(0);
  customPlot->xAxis->setTickLength(0);
  customPlot->xAxis->setRange(-1,num-1);
  customPlot->xAxis->grid()->setVisible(true);

  maxY = findMaxY();

  int stepY = 1;
  if (maxY < 20)
  {
    stepY = 1;
  }
  else if (maxY < 200)
  {
    stepY = 10;
  }
  else
  {
    stepY = 100;
  }

  //if (maxY/2*2 != maxY) { stepY = (int)(maxY/5); }
  //else  { stepY = (int)(maxY/4); }
  customPlot->yAxis->setTickStep(stepY);
  customPlot->yAxis->setRange(0,  maxY+stepY);
  customPlot->yAxis->setAutoTickStep(false);
  customPlot->yAxis->setSubTickLength(0);
  customPlot->yAxis->grid()->setVisible(true);

  //QLinearGradient gradient(0, 0, 0, 400);
  //customPlot->setBackground(QBrush(gradient));

  // Легенда:
  customPlot->legend->setVisible(true);
  customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);
  customPlot->legend->setBrush(QColor(255, 255, 255, 200));
  QPen legendPen;
  legendPen.setColor(QColor(130, 130, 130, 200));
  customPlot->legend->setBorderPen(legendPen);
  QFont legendFont = font();
  legendFont.setPointSize(10);
  customPlot->legend->setFont(legendFont);
  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
}
