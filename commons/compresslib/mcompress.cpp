#include "mcompress.h"


#include <unistd.h>
#include <stdio.h>
#include <cross-commons/debug/tlog.h>


namespace compress {

void Compress::init()
{
  out_buffer_.clear();
  cur_char_ = 0;
}


bool Compress::getcharFromInBuffer( char_type* c ) {
  if ( cur_char_ >= in_buffer_size_ ) {
    return false;
  }
//  debug_log << "curchar =" << cur_char_ << in_buffer_size_ ;
  *c = in_buffer_.at(cur_char_++);
  return true;
//  return (uint8_t)in_buffer_.at(cur_char_++);
}

int Compress::getcharsFromInBuffer( char_type* ch, int sz )
{
  if ( cur_char_ >= in_buffer_size_ ) {
    return 0;
  }
  int i = 0;
  for ( i = 0; i < sz; ++i ) {
    char_type c;
    bool res = getcharFromInBuffer(&c);
    if ( false == res ) {
      return i;
    }
    ch[i] = c;
  }
  return i;
}

int  Compress::addCharsToOutBuffer(uint8_t* ch, int sz)
{
  out_buffer_.append( (char*)ch, sz );
  return sz;
}

void  Compress::addCharToOutBuffer( uint8_t ch )
{
  out_buffer_.append(ch);
}

void  Compress::setInBuffer(const QByteArray &inbuf) {
  in_buffer_ = inbuf;
  in_buffer_size_ = in_buffer_.size();
}

QByteArray  Compress::compress(const QByteArray &inbuf) {
  setInBuffer(inbuf);
  compress();
//  debug_log << "OUTBUF =" << out_buffer_.toHex();
  return out_buffer_;
}

void  Compress::compress() {
  init();
  REG2  long    hp;
  REG3  int      rpos;
#if REGISTERS >= 5
  REG5  long    fc;
#endif
  REG6  int      outbits;
  REG7    int      rlop;
  REG8  int      rsize;
  REG9  int      stcode;
  REG10  code_int  free_ent;
  REG11  int      boff;
  REG12  int      n_bits;
  REG13  int      ratio;
  REG14  long    checkpoint;
  REG15  code_int  extcode;
  union
  {
    long      code;
    struct
    {
      char_type    c;
      unsigned short  ent;
    } e;
  } fcode;

  ratio = 0;
  checkpoint = CHECK_GAP;
  extcode = MAXCODE(n_bits = INIT_BITS)+1;
  stcode = 1;
  free_ent = FIRST;

  memset(outbuf, 0, sizeof(outbuf));
  bytes_out = 0; bytes_in = 0;
  if ( false == nomagic_ ) {
    outbuf[0] = MAGIC_1;
    outbuf[1] = MAGIC_2;
    outbuf[2] = (char)(maxbits | block_mode);
    boff = outbits = (3<<3);
  }
  else {
    outbuf[0] = (char)(maxbits | block_mode);
    boff = outbits = (1<<3);
  }
  fcode.code = 0;

  clear_htab();

  while ((rsize = getcharsFromInBuffer(inbuf, IBUFSIZ)) > 0)
  {
    if (bytes_in == 0)
    {
      fcode.e.ent = inbuf[0];
      rpos = 1;
    }
    else
      rpos = 0;

    rlop = 0;

    do
    {
      if (free_ent >= extcode && fcode.e.ent < FIRST)
      {
        if (n_bits < maxbits)
        {
          boff = outbits = (outbits-1)+((n_bits<<3)-
                  ((outbits-boff-1+(n_bits<<3))%(n_bits<<3)));
          if (++n_bits < maxbits)
            extcode = MAXCODE(n_bits)+1;
          else
            extcode = MAXCODE(n_bits);
        }
        else
        {
          extcode = MAXCODE(16)+OBUFSIZ;
          stcode = 0;
        }
      }

      if (!stcode && bytes_in >= checkpoint && fcode.e.ent < FIRST)
      {
        REG1 long int rat;

        checkpoint = bytes_in + CHECK_GAP;

        if (bytes_in > 0x007fffff)
        {              /* shift will overflow */
          rat = (bytes_out+(outbits>>3)) >> 8;

          if (rat == 0)        /* Don't divide by zero */
            rat = 0x7fffffff;
          else
            rat = bytes_in / rat;
        }
        else
          rat = (bytes_in << 8) / (bytes_out+(outbits>>3));  /* 8 fractional bits */
        if (rat >= ratio)
          ratio = (int)rat;
        else
        {
          ratio = 0;
          clear_htab();
          output(outbuf,outbits,CLEAR,n_bits);
          boff = outbits = (outbits-1)+((n_bits<<3)-
                  ((outbits-boff-1+(n_bits<<3))%(n_bits<<3)));
          extcode = MAXCODE(n_bits = INIT_BITS)+1;
          free_ent = FIRST;
          stcode = 1;
        }
      }

      if (outbits >= (OBUFSIZ<<3))
      {
        addCharsToOutBuffer( outbuf, OBUFSIZ );
//        if (write(fdout, outbuf, OBUFSIZ) != OBUFSIZ)
//          write_error();

        outbits -= (OBUFSIZ<<3);
        boff = -(((OBUFSIZ<<3)-boff)%(n_bits<<3));
        bytes_out += OBUFSIZ;

        memcpy(outbuf, outbuf+OBUFSIZ, (outbits>>3)+1);
        memset(outbuf+(outbits>>3)+1, '\0', OBUFSIZ);
      }

      {
        REG1  int    i;

        i = rsize-rlop;

        if ((code_int)i > extcode-free_ent)  i = (int)(extcode-free_ent);
        if (i > ((sizeof(outbuf) - 32)*8 - outbits)/n_bits)
          i = ((sizeof(outbuf) - 32)*8 - outbits)/n_bits;
        
        if (!stcode && (long)i > checkpoint-bytes_in)
          i = (int)(checkpoint-bytes_in);

        rlop += i;
        bytes_in += i;
      }

      goto next;
hfound:      fcode.e.ent = codetabof(hp);
next:        if (rpos >= rlop)
             goto endlop;
next2:       fcode.e.c = inbuf[rpos++];
#ifndef FAST
      {
        REG1   code_int  i;
#if REGISTERS >= 5
        fc = fcode.code;
#else
#  define      fc fcode.code
#endif
        hp = (((long)(fcode.e.c)) << (BITS-8)) ^ (long)(fcode.e.ent);

        if ((i = htabof(hp)) == fc)
          goto hfound;

        if (i != -1)
        {
          REG4 long    disp;

          disp = (HSIZE - hp)-1;  /* secondary hash (after G. Knott) */

          do
          {
            if ((hp -= disp) < 0)  hp += HSIZE;

            if ((i = htabof(hp)) == fc)
              goto hfound;
          }
          while (i != -1);
        }
      }
#else
      {
        REG1 long  i;
        REG4 long  p;
#if REGISTERS >= 5
          fc = fcode.code;
#else
#  define      fc fcode.code
#endif
        hp = ((((long)(fcode.e.c)) << (HBITS-8)) ^ (long)(fcode.e.ent));

        if ((i = htabof(hp)) == fc)  goto hfound;
        if (i == -1)        goto out;

        p = primetab[fcode.e.c];
lookup:        hp = (hp+p)&HMASK;
        if ((i = htabof(hp)) == fc)  goto hfound;
        if (i == -1)        goto out;
        hp = (hp+p)&HMASK;
        if ((i = htabof(hp)) == fc)  goto hfound;
        if (i == -1)        goto out;
        hp = (hp+p)&HMASK;
        if ((i = htabof(hp)) == fc)  goto hfound;
        if (i == -1)        goto out;
        goto lookup;
      }
out:      ;
#endif
      output(outbuf,outbits,fcode.e.ent,n_bits);

      {
#if REGISTERS < 5
#  undef  fc
        REG1 long  fc;
        fc = fcode.code;
#endif
        fcode.e.ent = fcode.e.c;


        if (stcode)
        {
          codetabof(hp) = (unsigned short)free_ent++;
          htabof(hp) = fc;
        }
      } 

      goto next;

endlop:      if (fcode.e.ent >= FIRST && rpos < rsize)
        goto next2;

      if (rpos > rlop)
      {
        bytes_in += rpos-rlop;
        rlop = rpos;
      }
    }
    while (rlop < rsize);
  }

  if (rsize < 0)
    debug_log << QObject::tr("Не прочитано!");

  if (bytes_in > 0)
    output(outbuf,outbits,fcode.e.ent,n_bits);

  addCharsToOutBuffer( outbuf,(outbits+7)>>3 );

  bytes_out += (outbits+7)>>3;

  return;
}

QByteArray  Compress::decompress( const QByteArray& inbuf, bool* ok )
{
  setInBuffer(inbuf);
  decompress();
  return out_buffer_;
}

void  Compress::decompress(bool *ok) {
  init();
  REG2   char_type* stackp;
  REG3  code_int code;
  REG4  int finchar;
  REG5  code_int oldcode;
  REG6  code_int incode;
  REG7  int inbits;
  REG8  int posbits;
  REG9  int outpos;
  REG10 int insize;
  REG11 int bitmask;
  REG12 code_int free_ent;
  REG13 code_int maxcode;
  REG14 code_int maxmaxcode;
  REG15 int n_bits;
  REG16 int rsize;

  bytes_in = 0;
  bytes_out = 0;
  insize = 0;

  int HEADERSIZE = 1;
  if ( false == nomagic_ ) {
    HEADERSIZE = 3;
  }

  while ( insize < HEADERSIZE && (rsize = getcharsFromInBuffer( inbuf+insize, IBUFSIZ)) > 0)
    insize += rsize;

  if (insize < HEADERSIZE || ( false == nomagic_ && (inbuf[0] != MAGIC_1 || inbuf[1] != MAGIC_2) ) ) {
    if ( rsize < 0)
      error_log << QObject::tr("Не смог прочитать");

    if ( insize > 0 ) {
      error_log << QObject::tr("Error: not in compressed format");
    }

    return ;
  }

  maxbits = inbuf[HEADERSIZE-1] & BIT_MASK;
  block_mode = inbuf[HEADERSIZE-1] & BLOCK_MODE;

  if (maxbits > BITS) {
      error_log << QObject::tr("Сжато с ")<<maxbits
                << QObject::tr(" бит. Поддерживается только ")<<BITS;
    return;
  }

  maxmaxcode = MAXCODE(maxbits);

  bytes_in = insize;
    maxcode = MAXCODE(n_bits = INIT_BITS)-1;
  bitmask = (1<<n_bits)-1;
  oldcode = -1;
  finchar = 0;
  outpos = 0;
  posbits = HEADERSIZE<<3;

    free_ent = ((block_mode) ? FIRST : 256);

  clear_tab_prefixof();  /* As above, initialize the first
                 256 entries in the table. */

    for (code = 255 ; code >= 0 ; --code)
    tab_suffixof(code) = (char_type)code;

  do
  {
resetbuf:  ;
    {
      REG1   int  i;
      int        e;
      int        o;

      o = posbits >> 3;
      e = o <= insize ? insize - o : 0;

      for (i = 0 ; i < e ; ++i)
        inbuf[i] = inbuf[i+o];

      insize = e;
      posbits = 0;
    }

    if (insize < sizeof(inbuf)-IBUFSIZ)
    {
      if ((rsize = getcharsFromInBuffer( inbuf+insize, IBUFSIZ)) < 0 )
        error_log << QObject::tr("Не прочитано!");

      insize += rsize;
    }

    inbits = ((rsize > 0) ? (insize - insize%n_bits)<<3 : 
                (insize<<3)-(n_bits-1));

    while (inbits > posbits)
    {
      if (free_ent > maxcode)
      {
        posbits = ((posbits-1) + ((n_bits<<3) -
                 (posbits-1+(n_bits<<3))%(n_bits<<3)));

        ++n_bits;
        if (n_bits == maxbits)
          maxcode = maxmaxcode;
        else
            maxcode = MAXCODE(n_bits)-1;

        bitmask = (1<<n_bits)-1;
        goto resetbuf;
      }

      input(inbuf,posbits,code,n_bits,bitmask);

      if (oldcode == -1)
      {
        if (code >= 256) {
          error_log << QObject::tr( "oldcode:-1 code:") << (int)(code) << QObject::tr("uncompress: corrupt input");
          return;
        }
        outbuf[outpos++] = (char_type)(finchar = (int)(oldcode = code));
        continue;
      }

      if (code == CLEAR && block_mode)
      {
        clear_tab_prefixof();
          free_ent = FIRST - 1;
        posbits = ((posbits-1) + ((n_bits<<3) -
              (posbits-1+(n_bits<<3))%(n_bits<<3)));
          maxcode = MAXCODE(n_bits = INIT_BITS)-1;
        bitmask = (1<<n_bits)-1;
        goto resetbuf;
      }

      incode = code;
        stackp = de_stack;

      if (code >= free_ent)  /* Special case for KwKwK string.  */
      {
        if (code > free_ent)
        {
          REG1 char_type     *p;

          posbits -= n_bits;
          p = &inbuf[posbits>>3];

          error_log << QObject::tr("insize:%1 posbits:%2 inbuf:%3 %4 %5 %6 %7 (%8)")
           .arg(insize)
           .arg(posbits)
           .arg(p[-1])
           .arg(p[0])
           .arg(p[1])
           .arg(p[2])
           .arg(p[3])
           .arg((posbits&07));
          error_log << QObject::tr("uncompress: corrupt input");
          return;
        }

              *--stackp = (char_type)finchar;
          code = oldcode;
      }

      while ((cmp_code_int)code >= (cmp_code_int)256)
      { /* Generate output characters in reverse order */
          *--stackp = tab_suffixof(code);
          code = tab_prefixof(code);
      }

      *--stackp =  (char_type)(finchar = tab_suffixof(code));

    /* And put them out in forward order */

      {
        REG1 int  i;

        if (outpos+(i = (de_stack-stackp)) >= OBUFSIZ)
        {
          do
          {
            if (i > OBUFSIZ-outpos) i = OBUFSIZ-outpos;

            if (i > 0)
            {
              memcpy(outbuf+outpos, stackp, i);
              outpos += i;
            }

            if (outpos >= OBUFSIZ)
            {
              if (addCharsToOutBuffer( outbuf, outpos ) != outpos )
                error_log << QObject::tr("Не записал!");

              outpos = 0;
            }
            stackp+= i;
          }
          while ((i = (de_stack-stackp)) > 0);
        }
        else
        {
          memcpy(outbuf+outpos, stackp, i);
          outpos += i;
        }
      }

      if ((code = free_ent) < maxmaxcode) /* Generate the new entry. */
      {
          tab_prefixof(code) = (unsigned short)oldcode;
          tab_suffixof(code) = (char_type)finchar;
          free_ent = code+1;
      } 

      oldcode = incode;  /* Remember previous code.  */
    }

    bytes_in += rsize;
    }
  while (rsize > 0);

  if (outpos > 0 && addCharsToOutBuffer( outbuf, outpos) != outpos)
    error_log << QObject::tr("Не записал!");
}

}
