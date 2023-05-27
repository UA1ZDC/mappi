#ifndef COMMONS_QCOMPRESS_LIB_H
#define COMMONS_QCOMPRESS_LIB_H

#include <qbuffer.h>

#ifndef  IBUFSIZ
#  define  IBUFSIZ  BUFSIZ  /* Defailt input buffer size              */
#endif
#ifndef  OBUFSIZ
#  define  OBUFSIZ  BUFSIZ  /* Default output buffer size              */
#endif

#define  SIZE_INNER_LOOP    256  /* Size of the inter (fast) compress loop      */

              /* Defines for third byte of header           */
#define  MAGIC_1    (char_type)'\037'/* First byte of compressed file        */
#define  MAGIC_2    (char_type)'\235'/* Second byte of compressed file        */
#define BIT_MASK  0x1f      /* Mask for 'number of compresssion bits'    */
                  /* Masks 0x20 and 0x40 are free.          */
                  /* I think 0x20 should mean that there is    */
                  /* a fourth header byte (for expansion).      */
#define BLOCK_MODE  0x80      /* Block compresssion if table is full and    */
                  /* compression rate is dropping flush tables  */

      /* the next two codes should not be changed lightly, as they must not  */
      /* lie within the contiguous general code space.            */
#define FIRST  257          /* first free entry               */
#define  CLEAR  256          /* table clear output code             */

#define INIT_BITS 9      /* initial number of bits/code */

#ifndef SACREDMEM
  /*
    * SACREDMEM is the amount of physical memory saved for others; compress
    * will hog the rest.
    */
#  define SACREDMEM  0
#endif

#ifndef USERMEM
  /*
    * Set USERMEM to the maximum amount of physical user memory available
    * in bytes.  USERMEM is used to determine the maximum BITS that can be used
    * for compression.
   */
#  define USERMEM   450000  /* default user memory */
#endif

#ifndef  BYTEORDER
#  define  BYTEORDER  0000
#endif

#ifndef  NOALLIGN
#  define  NOALLIGN  0
#endif

/*
 * machine variants which require cc -Dmachine:  pdp11, z8000, DOS
 */

#ifdef interdata  /* Perkin-Elmer                          */
#  define SIGNED_COMPARE_SLOW  /* signed compare is slower than unsigned       */
#endif

#ifdef pdp11     /* PDP11: don't forget to compile with -i             */
#  define  BITS     12  /* max bits/code for 16-bit machine           */
#  define  NO_UCHAR    /* also if "unsigned char" functions as signed char   */
#endif /* pdp11 */

#ifdef z8000    /* Z8000:                             */
#  define  BITS   12  /* 16-bits processor max 12 bits              */
#  undef  vax      /* weird preprocessor                     */
#endif /* z8000 */

#ifdef  DOS      /* PC/XT/AT (8088) processor                  */
#  define  BITS   16  /* 16-bits processor max 12 bits              */
#  if BITS == 16
#    define  MAXSEG_64K
#  endif
#  undef  BYTEORDER
#  define  BYTEORDER   4321
#  undef  NOALLIGN
#  define  NOALLIGN  1
#endif /* DOS */

#ifndef  O_BINARY
#  define  O_BINARY  0  /* System has no binary mode              */
#endif

#ifdef M_XENIX      /* Stupid compiler can't handle arrays with */
#  if BITS == 16     /* more than 65535 bytes - so we fake it */
#     define MAXSEG_64K
#  else
#  if BITS > 13      /* Code only handles BITS = 12, 13, or 16 */
#    define BITS  13
#  endif
#  endif
#endif

#ifndef BITS    /* General processor calculate BITS                */
#  if USERMEM >= (800000+SACREDMEM)
#    define FAST
#  else
#  if USERMEM >= (433484+SACREDMEM)
#    define BITS  16
#  else
#  if USERMEM >= (229600+SACREDMEM)
#    define BITS  15
#  else
#  if USERMEM >= (127536+SACREDMEM)
#    define BITS  14
#   else
#  if USERMEM >= (73464+SACREDMEM)
#    define BITS  13
#  else
#    define BITS  12
#  endif
#  endif
#   endif
#  endif
#  endif
#endif /* BITS */

#ifdef FAST
#  define  HBITS    17      /* 50% occupancy */
#  define  HSIZE     (1<<HBITS)
#  define  HMASK     (HSIZE-1)
#  define  HPRIME     9941
#  define  BITS       16
#  undef  MAXSEG_64K
#else
#  if BITS == 16
#    define HSIZE  69001    /* 95% occupancy */
#  endif
#  if BITS == 15
#    define HSIZE  35023    /* 94% occupancy */
#  endif
#  if BITS == 14
#    define HSIZE  18013    /* 91% occupancy */
#  endif
#  if BITS == 13
#    define HSIZE  9001    /* 91% occupancy */
#  endif
#  if BITS <= 12
#    define HSIZE  5003    /* 80% occupancy */
#  endif
#endif

#define CHECK_GAP 10000

typedef long int      code_int;

#ifdef SIGNED_COMPARE_SLOW
  typedef unsigned long int  count_int;
  typedef unsigned short int  count_short;
  typedef unsigned long int  cmp_code_int;  /* Cast to make compare faster  */
#else
  typedef long int       count_int;
  typedef long int      cmp_code_int;
#endif

typedef  unsigned char  char_type;

#define MAXCODE(n)  (1L << (n))

#ifndef  REGISTERS
#  define  REGISTERS  2
#endif
#define  REG1  
#define  REG2  
#define  REG3  
#define  REG4  
#define  REG5  
#define  REG6  
#define  REG7  
#define  REG8  
#define  REG9  
#define  REG10
#define  REG11  
#define  REG12  
#define  REG13
#define  REG14
#define  REG15
#define  REG16
#if REGISTERS >= 1
#  undef  REG1
#  define  REG1  register
#endif
#if REGISTERS >= 2
#  undef  REG2
#  define  REG2  register
#endif
#if REGISTERS >= 3
#  undef  REG3
#  define  REG3  register
#endif
#if REGISTERS >= 4
#  undef  REG4
#  define  REG4  register
#endif
#if REGISTERS >= 5
#  undef  REG5
#  define  REG5  register
#endif
#if REGISTERS >= 6
#  undef  REG6
#  define  REG6  register
#endif
#if REGISTERS >= 7
#  undef  REG7
#  define  REG7  register
#endif
#if REGISTERS >= 8
#  undef  REG8
#  define  REG8  register
#endif
#if REGISTERS >= 9
#  undef  REG9
#  define  REG9  register
#endif
#if REGISTERS >= 10
#  undef  REG10
#  define  REG10  register
#endif
#if REGISTERS >= 11
#  undef  REG11
#  define  REG11  register
#endif
#if REGISTERS >= 12
#  undef  REG12
#  define  REG12  register
#endif
#if REGISTERS >= 13
#  undef  REG13
#  define  REG13  register
#endif
#if REGISTERS >= 14
#  undef  REG14
#  define  REG14  register
#endif
#if REGISTERS >= 15
#  undef  REG15
#  define  REG15  register
#endif
#if REGISTERS >= 16
#  undef  REG16
#  define  REG16  register
#endif


union  bytes
{
  long  word;
  struct
  {
#if BYTEORDER == 4321
    char_type  b1;
    char_type  b2;
    char_type  b3;
    char_type  b4;
#else
#if BYTEORDER == 1234
    char_type  b4;
    char_type  b3;
    char_type  b2;
    char_type  b1;
#else
#  undef  BYTEORDER
    int        dummy;
#endif
#endif
  } bytes;
} ;
#ifdef BYTEORDER
#  if BYTEORDER == 4321 && NOALLIGN == 1
#    define  output(b,o,c,n)  {                          \
              *(long *)&((b)[(o)>>3]) |= ((long)(c))<<((o)&0x7);\
              (o) += (n);                    \
            }
#  else
#    define  output(b,o,c,n)  {  REG1 char_type  *p = &(b)[(o)>>3];        \
              union bytes i;                  \
              i.word = ((long)(c))<<((o)&0x7);        \
              p[0] |= i.bytes.b1;                \
              p[1] |= i.bytes.b2;                \
              p[2] |= i.bytes.b3;                \
              (o) += (n);                    \
            }
#  endif
#else
#  define  output(b,o,c,n)  {  REG1 char_type  *p = &(b)[(o)>>3];        \
              REG2 long     i = ((long)(c))<<((o)&0x7);  \
              p[0] |= (char_type)(i);              \
              p[1] |= (char_type)(i>>8);            \
              p[2] |= (char_type)(i>>16);            \
              (o) += (n);                    \
            }
#endif

#ifdef BYTEORDER
#  if BYTEORDER == 4321 && NOALLIGN == 1
#    define  input(b,o,c,n,m){                          \
              (c) = (*(long *)(&(b)[(o)>>3])>>((o)&0x7))&(m);  \
              (o) += (n);                    \
            }
#  else
#    define  input(b,o,c,n,m){  REG1 char_type     *p = &(b)[(o)>>3];      \
              (c) = ((((long)(p[0]))|((long)(p[1])<<8)|    \
                   ((long)(p[2])<<16))>>((o)&0x7))&(m);  \
              (o) += (n);                    \
            }
#  endif
#else
#define  input(b,o,c,n,m){  REG1 char_type     *p = &(b)[(o)>>3];      \
              (c) = ((((long)(p[0]))|((long)(p[1])<<8)|    \
                   ((long)(p[2])<<16))>>((o)&0x7))&(m);  \
              (o) += (n);                    \
            }
#endif

#define  htabof(i)        htab[i]
#define  codetabof(i)      codetab[i]
#define  tab_prefixof(i)      codetabof(i)
#define  tab_suffixof(i)      ((char_type *)(htab))[i]
#define  de_stack        ((char_type *)&(htab[HSIZE-1]))
#define  clear_htab()      memset(htab, -1, sizeof(htab))
#define  clear_tab_prefixof()  memset(codetab, 0, 256);

namespace compress {

class Compress
{
  public:

    void  setInBuffer(const QByteArray &inbuf);
    void  compress();
    void  decompress(bool *ok=nullptr);

    QByteArray compress(const QByteArray& inbuf );
    QByteArray decompress(const QByteArray &inbuf, bool *ok=nullptr);

  private:
    void init();
    bool getcharFromInBuffer( char_type* c );
    int getcharsFromInBuffer(char_type* ch, int sz);
    void addCharToOutBuffer(uint8_t ch);
    int addCharsToOutBuffer(char_type* ch, int sz);

    QByteArray in_buffer_;
    QByteArray out_buffer_;
    bool nomagic_ = true;
    int  block_mode = BLOCK_MODE;/* Block compress mode -C compatible with 2.0*/
    int  maxbits = BITS;    /* user settable max # bits/code         */

    char_type    inbuf[IBUFSIZ+64];  /* Input buffer                  */
    char_type    outbuf[OBUFSIZ+2048];/* Output buffer                */

    long       bytes_in;      /* Total number of byte from input        */
    long       bytes_out;     /* Total number of byte to output         */
    count_int    htab[HSIZE];
    unsigned short  codetab[HSIZE];
    long cur_char_ = 0;
    long in_buffer_size_ = -1;

#ifdef FAST
  int primetab[256] =    /* Special secudary hash table.    */
  {
       1013, -1061, 1109, -1181, 1231, -1291, 1361, -1429,
       1481, -1531, 1583, -1627, 1699, -1759, 1831, -1889,
       1973, -2017, 2083, -2137, 2213, -2273, 2339, -2383,
       2441, -2531, 2593, -2663, 2707, -2753, 2819, -2887,
       2957, -3023, 3089, -3181, 3251, -3313, 3361, -3449,
       3511, -3557, 3617, -3677, 3739, -3821, 3881, -3931,
       4013, -4079, 4139, -4219, 4271, -4349, 4423, -4493,
       4561, -4639, 4691, -4783, 4831, -4931, 4973, -5023,
       5101, -5179, 5261, -5333, 5413, -5471, 5521, -5591,
       5659, -5737, 5807, -5857, 5923, -6029, 6089, -6151,
       6221, -6287, 6343, -6397, 6491, -6571, 6659, -6709,
       6791, -6857, 6917, -6983, 7043, -7129, 7213, -7297,
       7369, -7477, 7529, -7577, 7643, -7703, 7789, -7873,
       7933, -8017, 8093, -8171, 8237, -8297, 8387, -8461,
       8543, -8627, 8689, -8741, 8819, -8867, 8963, -9029,
       9109, -9181, 9241, -9323, 9397, -9439, 9511, -9613,
       9677, -9743, 9811, -9871, 9941,-10061,10111,-10177,
       10259,-10321,10399,-10477,10567,-10639,10711,-10789,
       10867,-10949,11047,-11113,11173,-11261,11329,-11423,
       11491,-11587,11681,-11777,11827,-11903,11959,-12041,
       12109,-12197,12263,-12343,12413,-12487,12541,-12611,
       12671,-12757,12829,-12917,12979,-13043,13127,-13187,
       13291,-13367,13451,-13523,13619,-13691,13751,-13829,
       13901,-13967,14057,-14153,14249,-14341,14419,-14489,
       14557,-14633,14717,-14767,14831,-14897,14983,-15083,
       15149,-15233,15289,-15359,15427,-15497,15583,-15649,
       15733,-15791,15881,-15937,16057,-16097,16189,-16267,
       16363,-16447,16529,-16619,16691,-16763,16879,-16937,
       17021,-17093,17183,-17257,17341,-17401,17477,-17551,
       17623,-17713,17791,-17891,17957,-18041,18097,-18169,
       18233,-18307,18379,-18451,18523,-18637,18731,-18803,
       18919,-19031,19121,-19211,19273,-19381,19429,-19477
  } ;
#endif

};

}
#endif

