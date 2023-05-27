#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_NUMFORMAT_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_NUMFORMAT_H

#include <qdialog.h>

#include <meteo/commons/proto/puanson.pb.h>

namespace Ui {
  class NumFormat;
}

namespace meteo {
namespace puanson {

class NumFormat : public QDialog
{
  Q_OBJECT
  public:
    NumFormat( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~NumFormat();

    void setNumbeRule( proto::CellRule::NumbeRule* rule, int align, double val );
    void setSwitchButtonText( const QString& txt, const QFont& fnt );

    const proto::CellRule::NumbeRule& numbeRule() const { return rule_; }
    int align() const ;
    double testvalue() const { return testval_; }
    proto::CellType type() const { return type_; }

  private:
    Ui::NumFormat* ui_;
    proto::CellRule::NumbeRule rule_;
    double testval_;
    proto::CellType type_;

    void connectSlots();
    void disconnectSlots();

    void setUiFromProto();
    void setProtoFromUi();

    void setExampleText();

  private slots:
    void slotSwitch2Signview();
    void slotCmbIntCount( int indx );
    void slotCmbIntMode( int indx );
    void slotCmbFractCount( int indx );
    void slotCmbSepMode( int indx );
    void slotCmbSignMode( int indx );
    void slotLblTestDoubleClicked();
    void slotFontParamChanged( const QFont& font, int align );
    void slotPlusColorChanged( QRgb clr );
    void slotMinusColorChanged( QRgb clr );
    void slotSpinFactorChanged( double val );
    void slotPostfixChanged();
    void slotZeroHideToggled();

    void slotBtnOk();
    void slotBtnNo();
};

}
}

#endif
