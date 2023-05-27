#ifndef MYTIMER_H
#define MYTIMER_H

#include <QTimer>

class myTimer : public QTimer
{
    Q_OBJECT

signals:

public:
explicit myTimer(QObject *parent = nullptr);

public slots:
void timerAction();
void timerStart();

protected:

private:

};
#endif // MYTIMER_H
