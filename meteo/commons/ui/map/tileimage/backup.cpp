
  if ( true == property_.cache() ) {
    BasisCache bcache;
    if ( false == bcache.renderBasis( this, p ) ) {
      if ( false == this->hasGeoBasis() ) {
        bcache.loadGeoBasis(this);
      }
      bcache.flushBasis(this);
      bcache.renderBasis( this, p );
    }
  }
