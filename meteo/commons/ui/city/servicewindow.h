#ifndef SERVICEWINDOW_H
#define SERVICEWINDOW_H

#include <QMainWindow>

struct Aux{
    QByteArray titleBA;
    QString title;
    double lat;
    double lon;
    int genLevel;
    int priority;
    int citymode;
    bool visible;
};

namespace Ui {
class ServiceWindow;
}

class ServiceWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit ServiceWindow(QWidget *parent = 0);
    ~ServiceWindow();
    
private:
    Ui::ServiceWindow *ui;
    QStringList geoList;
    QStringList confList;


private slots:
    void engage3();
    void engage();
    void engage2();
    void engage4();
    void engage5();
    void engage6();
};

#endif // SERVICEWINDOW_H
