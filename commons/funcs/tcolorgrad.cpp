#include "tcolorgrad.h"

#include <commons/mathtools/mnmath.h>

TColorGrad::TColorGrad( float beg, float end, const QColor& begc, const QColor& endc )
  : begend_( beg, end ),
    becolors_( begc, endc )
{
//  koef_ = 1.f / ( begend_.second - begend_.first );
  
//  begrgba_ = becolors_.first.rgba();
//  endrgba_ = becolors_.second.rgba( );

  setKoef();


}

TColorGrad::TColorGrad( float beg, float end, const QColor& color )
  : begend_( beg, end ),
    becolors_( color, color )
{

  setKoef();
  
}

TColorGrad::TColorGrad( float val, const QColor& color )
  : begend_( val, val ),
    becolors_( color, color )
{
  setKoef();
}

TColorGrad::TColorGrad()
  : begend_( 0.0, 0.0 ),
  becolors_( QColor( 255,255,255,0  ), QColor( 255,255,255,0 ) )
{
  setKoef();
}

TColorGrad::~TColorGrad()
{
}


QRgb TColorGrad::interpolColor(QRgb color1, QRgb color2, float offs){
  int r = qRed(color1) + (qRed(color2) - qRed(color1)) * offs;
  int g = qGreen(color1) + (qGreen(color2) - qGreen(color1)) * offs;
  int b = qBlue(color1) + (qBlue(color2) - qBlue(color1)) * offs;
  int a = qAlpha(color1) + (qAlpha(color2) - qAlpha(color1)) * offs;
  return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
}

void TColorGrad::setColors( const QColor& beg, const QColor& end ) {
  becolors_.first = beg; becolors_.second = end;
  setKoef();
  
}
void TColorGrad::setColor( const QColor& color ) {
  becolors_.first = color; becolors_.second = color; 
  setKoef();
  
}
void TColorGrad::setValue( float val ) {
  begend_.first = val; begend_.second = val;
  setKoef();
}

void TColorGrad::setTitle( const QString & val )
{
  title_ = val;
}

void TColorGrad::setValues( float beg, float end ) {
  begend_.first = beg; begend_.second = end; 
  setKoef();
}

void TColorGrad::setKoef(){
  if( false == MnMath::isZero( begend_.second - begend_.first ) ) {
    koef_ = 1.f / ( begend_.second - begend_.first );
  }
  else {
    koef_ = 1.f;
  }
  begrgba_ = becolors_.first.rgba( );
  endrgba_ = becolors_.second.rgba( );
  
}

QColor TColorGrad::color( float val, bool* ok ) const
{

  if ( false == contains(val) ) {
    if ( nullptr != ok ) {
      *ok = false;
    }
    return QColor( 255,255,255,0  );
  }
  else if ( true == onecolor() ) {
    if ( nullptr != ok ) {
      *ok = true;
    }
    return becolors_.first;
  }
  else if ( true == oneval() ) {
    if ( nullptr != ok ) {
      *ok = true;
    }
    return becolors_.first;
  }
   float k = ( val - begend_.first ) * koef_;
  if ( nullptr != ok ) {
    *ok = true;
  }  
  QRgb rgba = interpolColor(begrgba_, endrgba_, k);
  return QColor(qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba));
}

QRgb TColorGrad::color( float val) const
{
  
  if ( false == contains(val) ) {
    return RGB_MASK;
  }
  float k = ( val - begend_.first ) * koef_;
  return interpolColor(begrgba_, endrgba_,  k);
 
}



QColor TColorGrad::color(float val, const QColor& defaultValue) const
{
  bool ok = false;
  QColor c = color( val, &ok );

  return (ok) ? c : defaultValue;
}

bool TColorGrad::contains( float val ) const
{
  if (  begend_.first <= val && begend_.second >= val ) {
    return true;
  }
  return false;
}

QColor TColorGradList::color( float val, bool* ok ) const
{
  if ( nullptr != ok ) {
    *ok = false;
  }

  for ( int i = 0, sz = size(); i < sz; ++i ) {
    const TColorGrad& grad = TColorGradList::operator[](i);
    if ( true == grad.contains(val) ) {
      if ( nullptr != ok ) {
        *ok = true;
      }
      QRgb rgba = grad.color(val);
      return QColor(qRed(rgba), qGreen(rgba), qBlue(rgba), qAlpha(rgba));
    }
  }
  return QColor( 255,255,255,0 );
}

QColor TColorGradList::color(float val, const QColor& defaultValue) const
{
  bool ok = false;
  QColor c = color( val, &ok );

  return (ok) ? c : defaultValue;
}



bool TColorGradList::contains( float val ) const
{
  for ( int i = 0, sz = size(); i < sz; ++i ) {
    TColorGrad grad = TColorGradList::operator[](i);
    if ( true == grad.contains(val) ) {
      return true;
    }
  }
  return false;
}

QColor TColorGradList::begcolor() const
{
  if ( 0 == QList::size() ) {
    return Qt::black;
  }
  return QList::at(0).begcolor();
}

QColor TColorGradList::endcolor() const
{
  if ( 0 == QList::size() ) {
    return Qt::black;
  }
  return QList::at( QList::size() - 1 ).endcolor();
}

float TColorGradList::begval() const
{
  if ( 0 == QList::size() ) {
    return 0.0;
  }
  return QList::at(0).begval();
}

QString TColorGradList::title() const
{
  if ( 0 == QList::size() ) {
    return QString();
  }
  return QList::at(0).title();
}


float TColorGradList::endval() const
{
  if ( 0 == QList::size() ) {
    return 0.0;
  }
  return QList::at( QList::size() - 1 ).endval();
}
