#ifndef METEO_COMMONS_UI_CUSTOM_SYMBOLBTN_H
#define METEO_COMMONS_UI_CUSTOM_SYMBOLBTN_H

#include <qtoolbutton.h>
#include <qchar.h>
#include <qfont.h>

class SymbolButton: public QToolButton
{
  Q_OBJECT
  public:
    SymbolButton( QWidget* parent );
    ~SymbolButton();

    const QChar& symbol() const { return symbol_; }
    void setSymbol( const QChar& smb );

  private:
    QChar symbol_;

  private slots:
    void slotClicked();

  signals:
    void symbolChanged( const QChar& smb );

};

#endif
