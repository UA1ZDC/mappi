#ifndef METEO_COMMONS_UI_MAP_VIEW_PUANSON_SYMBOLFORMAT_H
#define METEO_COMMONS_UI_MAP_VIEW_PUANSON_SYMBOLFORMAT_H

#include <qdialog.h>

#include <meteo/commons/proto/puanson.pb.h>

namespace Ui {
  class SymbolFormat;
}

class QTreeWidgetItem;

namespace meteo {
namespace puanson {

class SymbolFormat : public QDialog
{
  Q_OBJECT
  public:
    SymbolFormat( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~SymbolFormat();

    void setSymbolRule( proto::CellRule::SymbolRule* rule, int align, double val );
    void setSwitchButtonText( const QString& txt, const QFont& fnt );

    const proto::CellRule::SymbolRule& symbolRule() const { return rule_; }
    double testvalue() const { return testval_; }
    proto::CellType type() const { return type_; }

  private:
    Ui::SymbolFormat* ui_;
    proto::CellRule::SymbolRule rule_;
    double testval_;
    proto::CellType type_;

    void connectSlots();
    void disconnectSlots();

    void setUiFromProto();
    void setProtoFromUi();

    void setExampleText();

    void setupTreeWidgetItem( QTreeWidgetItem* item, const proto::CellRule::Symbol& symbol );

  private slots:
    void slotSwitch2Signview();

    void slotBtnAddRange();
    void slotBtnRmRange();
    void slotItemDoubleClicked( QTreeWidgetItem* item, int clmn );

    void slotLblTestDoubleClicked();
    void slotFontParamChanged( const QFont& font, int align );

    void slotCodeDigitStateChanged( int state );

    void slotBtnOk();
    void slotBtnNo();

    void slotRangeReady( bool* valid, double* confl_min, double* confl_max );
};

}
}

#endif
