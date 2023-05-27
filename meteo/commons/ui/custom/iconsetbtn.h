#ifndef METEO_COMMONS_UI_CUSTOM_ICONSETBTN_H
#define METEO_COMMONS_UI_CUSTOM_ICONSETBTN_H

#include <qtoolbutton.h>
#include <qchar.h>
#include <qfont.h>
#include <qpair.h>

class IconsetButton: public QToolButton
{
  Q_OBJECT
  public:
    IconsetButton( QWidget* parent );
    ~IconsetButton();

    void setIcons( const QList< QPair< QIcon, QString> >& icons );
    void setIcons( const QList<QIcon>& icons );
    void loadIconsFromPath( const QString& path );

    void setCurrentIndex( int indx );

    const QList< QPair< QIcon, QString > >& icons() const { return icons_; }
    int currentIndex() const { return index_; }
    QIcon currentIcon() const ;
    QString currentText() const ;
    void setPoint(QPoint point);
    void clearPoint() { setPoint_ = false; }
    IconsetButton* addIco( const QPair< QIcon, QString >& pair );


  private:
    bool setPoint_ = false;
    QPoint point_;
    int index_;
    QList< QPair<QIcon, QString> > icons_;

    void setDefault();

  private slots:
    void slotClicked();

  signals:
    void indexChanged( int indx );

};

#endif
