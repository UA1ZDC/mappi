#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_METAEDIT_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_METAEDIT_H

#include <qstring.h>
#include <qmap.h>
#include <qdialog.h>

namespace Ui {
class Metaedit;
}

namespace meteo {
namespace puanson {

QMap< int, QString > loadPunchTypes();
static const QMap< int, QString > kPunchTypes = loadPunchTypes();

class Metaedit : public QDialog
{
  Q_OBJECT
  public:
    Metaedit( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~Metaedit();

    void setCurrentCodeAndTitleAndType( const QString& code, const QString& title, int type );
    const QString& code() const { return code_; }
    const QString& title() const { return title_; }
    int type() const { return type_; }


  private:
    Ui::Metaedit* ui_;
    QString code_;
    QString title_;
    int type_;

  private slots:
    void accept();
};

}
}

#endif
