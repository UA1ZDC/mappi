#ifndef METEO_COMMONS_UI_CUSTOM_MULTICHOOSEBTN_H
#define METEO_COMMONS_UI_CUSTOM_MULTICHOOSEBTN_H

#include <qtoolbutton.h>
#include <qchar.h>
#include <qfont.h>
#include <qpair.h>

const int kCols = 5;

class MultiChooseBtn: public QToolButton
{
  Q_OBJECT
  public:
    MultiChooseBtn( QWidget* parent);
    ~MultiChooseBtn();

    void setIcons( const QList< QPair< QString, QString> >& icons );
    void setIndexes(const QList<int> &list);

    void setCols(int cols);

    const QList< QPair< QString, QString > >& icons() const { return icons_; }
    QList<int> indexes() const { return indexes_; }

    QStringList indexesToStringList() const;

    MultiChooseBtn* addIco( const QPair< QString, QString >& pair );

  private slots:
    void slotClicked();

  signals:
    void changed(QList<int>);

  private:
    void setDefault();

    QList< QPair<QString, QString> > icons_;
    int cols_;
    QList<int> indexes_;



};

#endif
