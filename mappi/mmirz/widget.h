#ifndef WIDGET_H
#define WIDGET_H
#include <QVBoxLayout>
#include <QPushButton>
#include "qcustomplot.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

public  slots:
      void plot(QVector<QStringList> list, QStringList times, QVector<QString> cities, const QString &satelite = QString::null);
      void setupBars(QVector<QString> cities);
private:
      int findMaxY();
private:
    QCustomPlot *customPlot;
    QVBoxLayout *vbox;
    QPushButton *res;
//    QCPBars *fossil;
    QVector<QCPBars*> bars;
    QCPBars* emptybar;


};

#endif // WIDGET_H
