#ifndef TDBCONNECTION_H
#define TDBCONNECTION_H

#include <qwidget.h>
#include <qmap.h>

class QString;
class QEvent;
class QLineEdit;
class QMenu;

namespace Ui{
   class DbConnection;
}

class TDbConnection : public QWidget {
  Q_OBJECT
  public:
    TDbConnection( QWidget * parent = 0);
    ~TDbConnection();
    bool setHost(const QString& host);
    bool setDbName(const QString& name);
    bool setLogin(const QString& login);
    bool setPass(const QString& pass);

    QString getHost();
    QString getDbName();
    QString getLogin();
    QString getPass();

    bool setDbList(QMap<QString, QString> list);

    QLineEdit* getHostEdit();
  signals:
    void menuClicked(QString);
    void save();
  private slots:
    void updateParams(QAction* act);
    void saveSlot();
  private:
    Ui::DbConnection* _ui;
    QMenu* _menu;
};


#endif
