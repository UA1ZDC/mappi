#ifndef TCOLORGRAD_H
#define TCOLORGRAD_H

#include <qpair.h>
#include <qcolor.h>
#include <qlist.h>
#include <commons/mathtools/mnmath.h>

class TColorGrad
{
public:
    TColorGrad( float beg, float end, const QColor& begc, const QColor& endc );
    TColorGrad( float beg, float end, const QColor& color );
    TColorGrad( float val, const QColor& color );
    TColorGrad();
    ~TColorGrad();

    static QRgb interpolColor(QRgb c1, QRgb c2, float offs);
    
    bool oneval() const { return MnMath::isEqual(begend_.first, begend_.second); }
    bool onecolor() const { return becolors_.first == becolors_.second; }
    QColor color( float val, bool* ok ) const ;
    QColor color( float val, const QColor& defaultValue  ) const;
    QRgb color( float val) const;
    
    const QColor& begcolor() const { return becolors_.first; }
    const QColor& endcolor() const { return becolors_.second; }
    float begval() const { return begend_.first; }
    float endval() const { return begend_.second; }
    const QString& title() const { return title_; }
    bool contains( float val ) const ;

    void setColors( const QColor& beg, const QColor& end );
    void setColor( const QColor& color );
    void setValue( float val );
    void setValues( float beg, float end );
    void setTitle( const QString & val );

private:
    QPair< float, float > begend_;
    QPair< QColor, QColor > becolors_;
    QString title_;
    void setKoef();
    float koef_;
    
    QRgb begrgba_;
    QRgb endrgba_;
};

class TColorGradList : public QList<TColorGrad>
{
  public:
    TColorGradList() : QList<TColorGrad>() {}
    TColorGradList( const QList<TColorGrad>& other ) : QList<TColorGrad>(other) {}
    ~TColorGradList() {}
    void set(const TColorGrad& val) { clear(); append(val); }
    QColor color( float val, bool* ok ) const ;
    QColor color( float val, const QColor& defaultValue = QColor(255,255,255,0) ) const;
    bool contains( float val ) const ;

    QColor begcolor() const ;
    QColor endcolor() const ;

    float begval() const ;
    float endval() const ;
    QString title() const ;
};

#endif
