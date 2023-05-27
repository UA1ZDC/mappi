#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_ADDSYMBOL_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_ADDSYMBOL_H

#include <qdialog.h>
#include <qfont.h>

#include <meteo/commons/proto/puanson.pb.h>

namespace Ui {
  class AddSymbol;
}

namespace meteo {
namespace puanson {

class AddSymbol : public QDialog
{
  Q_OBJECT
  public:
    AddSymbol( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~AddSymbol();

    void setProtoFromUi();
    void setUiFromProto();

    void accept();

    void setMinMax( double min, double max );
    void setMin( double min );
    void setMax( double max );

    double min() const { return min_; }
    double max() const { return max_; }

    void setSymbol( const proto::CellRule::Symbol& smb ) { symbol_.CopyFrom(smb); setUiFromProto(); }
    const proto::CellRule::Symbol& symbol() const { return symbol_; }


  protected:
    void keyReleaseEvent( QKeyEvent* e );

  private:
    Ui::AddSymbol* ui_;
    double min_;
    double max_;
    QString codeval_;
    int mincurpos_;
    int maxcurpos_;
    proto::CellRule::Symbol symbol_;
    proto::CellRule::NumbeRule number_;
    bool has_number_ = false;
 
    void connectSlots();
    void disconnectSlots();

  private slots:
    void slotFontChanged( const QFont& f );
    void slotColorChanged( QRgb rgb );
    void slotMinTextChanged( const QString& str );
    void slotMaxTextChanged( const QString& str );
    void slotTextChanged( const QString& str );
    void slotLabelChanged( const QString& str );
    void slotSymbolChanged( const QChar& smb );

    void slotLeminTextEdited( const QString& str );
    void slotLemaxTextEdited( const QString& str );

    void slotSetNumbeRule();
    void slotClearNumbeRule();

  signals:
    void rangeReady( bool* flag, double* min, double* max );

};

}
}

#endif
