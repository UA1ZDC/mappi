#ifndef METEO_COMMONS_UI_CUSTOM_FONTWIDGET_H
#define METEO_COMMONS_UI_CUSTOM_FONTWIDGET_H

#include <qwidget.h>
#include <qfont.h>
#include <qabstractitemdelegate.h>

#include <meteo/commons/global/common.h>

namespace Ui {
  class FontWidget;
}

class FontWidget: public QWidget
{
  Q_OBJECT

  Q_PROPERTY(bool specialVisible READ specialVisible WRITE setSpecialVisible );
  Q_PROPERTY(bool symbolVisible READ symbolVisible WRITE setSymbolVisible );
  Q_PROPERTY(bool colorVisible READ colorVisible WRITE setColorVisible );
  Q_PROPERTY(bool alignVisible READ alignVisible WRITE setAlignVisible );
  Q_PROPERTY(bool boldVisible READ boldVisible WRITE setBoldVisible );
  Q_PROPERTY(bool italicVisible READ italicVisible WRITE setItalicVisible );
  Q_PROPERTY(bool underlineVisible READ underlineVisible WRITE setUnderlineVisible );
  Q_PROPERTY(bool strikeVisible READ strikeVisible WRITE setStrikeVisible );

  public:
    FontWidget( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~FontWidget();

    void hideAlign();
    void showAlign();
    void hideSymbol();
    void showSymbol();
    void hideColor();
    void showColor();

    void setSpecFontEnabled( bool fl );

    bool specialVisible() const { return specvisible_; }
    bool symbolVisible() const { return symbolvisible_; }
    bool colorVisible() const { return colorvisible_; }
    bool alignVisible() const { return alignvisible_; }
    bool boldVisible() const { return boldvisible_; }
    bool italicVisible() const { return italicvisible_; }
    bool underlineVisible() const { return underlinevisible_; }
    bool strikeVisible() const { return strikevisible_; }
    QMap <meteo::Position, bool> allowedAligns();

    void setSpecialVisible( bool fl );
    void setSymbolVisible( bool fl );
    void setColorVisible( bool fl );
    void setAlignVisible( bool fl );
    void setBoldVisible( bool fl );
    void setItalicVisible( bool fl );
    void setUnderlineVisible( bool fl );
    void setStrikeVisible( bool fl );
    void setAllowedAligns(QMap <meteo::Position, bool> aligns);



    const QFont& font() const { return font_; }
    void setFont( const QFont& font, int align = meteo::kTopRight, QRgb clr = 0xFF000000 ) ;

    int align() const { return align_; }
    QRgb color() const { return rgb_; }

    void reloadSpecialFonts();

  private:
    Ui::FontWidget* ui_;

    QFont font_;
    int align_;
    QRgb rgb_;

    bool specvisible_;
    bool symbolvisible_;
    bool colorvisible_;
    bool alignvisible_;
    bool boldvisible_;
    bool italicvisible_;
    bool underlinevisible_;
    bool strikevisible_;


    void setFontFromUi();
    void setUiFromFont();

    void connectSlots();
    void disconnectSlots();

  private slots:
    void slotCmbFamilyChanged( int indx );
    void slotBtnSpecialToggled( bool checked );
    void slotBtnSizeClicked();
    void slotBtnBoldToggled( bool checked );
    void slotBtnItalicToggled( bool checked );
    void slotBtnUnderlineToggled( bool checked );
    void slotBtnStrikeoutToggled( bool checked );
    void slotBtnAlignPosChanged( int pos );
    void slotBtnSymbolChanged( const QChar& smb );
    void slotBtnColorChanged( QRgb clr );

  signals:
    void paramChanged( const QFont& font, int align, QRgb rgb );
    void fontChanged( const QFont& font );
    void alignChanged( int align );
    void symbolChanged( const QChar& smb );
    void colorChanged( QRgb rgb );


};

#endif
