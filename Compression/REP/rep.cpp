/*
    REP is an LZ77-family algorithm, i.e. it founds matches and outputs them as
    (len,offset) pairs. It is oriented toward very fast compression and small
    memory overhead (1/4 of buffer size), but limited to rather large values of
    mimimal match length (say, 32), and don't search for optimum match. It's
    intended to preprocess data before using full-fledged compressors. and in
    this area it beats RZIP and, to some degree, LZP preprocessors. Small
    memory overhead means that RZIP/LZP/REP are capable to find matches at very
    long distances and this algorithm does it much better than RZIP and LZP.
    The algorithm implemented in functions REPEncode() and REPDecode().

    Main differences comparing to RZIP:
    1) Sliding window which slides at 1/16 of buffer size each time
    2) Almost ideal hash function (see update_hash)
    3) Direct hashing without hash chains which 1.5x cuts memory requirements
    4) Tags are not saved in hashtable, which again halves memory requirements.
         Instead, a few lower bits of hash table entry are used to save a few
         bits of tag (see chksum)
    5) Hash size is proportional to buffer size (which is equal to the maximum
         search distance) and by default limited to 1/4 of buffer size
    6) In order to find strings of length >=MinLen, blocks of length L=MinLen/2
         are indexed via hash. Of all those possible blocks, only 1/sqrt(L) are
         indexed and only 1/sqrt(L) are searched. It is alternative to solution
         described in RZIP paper where 1/L of blocks are indexed and each block
         searched. This means that logb(sqrt(L)) lower bits of hash entry are
         zeroes which allows to use trick 4.


References for RZIP algorithm explanation and implementations:
http://samba.org/~tridge/phd_thesis.pdf
http://rzip.samba.org/ftp/rzip/rzip-2.1.tar.gz
http://ck.kolivas.org/apps/lrzip/lrzip-0.18.tar.bz2
http://www.edcassa-ict.nl/lrzip.zip
http://www.edcassa-ict.nl/rzip21.zip

TAYLOR, R., JANA, R., AND GRIGG, M. 1997. Checksum testing of remote
synchronisation tool. Technical Report 0627 (November), Defence Science and
Technology Organisation, Canberra, Australia. (p.72)


References for LZP algorithm implementations:
http://magicssoft.ru/content/download/GRZipII/GRZipIISRC.zip
http://www.compression.ru/ds/lzp.rar


** Detailed algorithm description in Russian **********************************************

    ���� �������� �������� �������������� LZ77, �.�. �� ������� �������������
    ������ �� ������� ������, � �������� �� ��� (len,offset). ��� ������������
    �������� ���������� �� ����� ���������� ���������� ������� ����� �� �������
    ����������. ������� �� ������ ���������� ���������� ������ - ��� �������,
    ��� �������� ������ ��������� �� ����� 25% �� ������� ���� ������. ��� ����
    �� ������� ����������� ��� ���������� ���� ����������� ����� (MinLen)
    ������� ����� - 512 ����, � ������� 98% - � ����� ��� ������������ ��
    ����� ���������� � ������ �� 32 ����. �� �������� ���� �������� ������� ��
    ������������� � �������� �������������, ������������ ������������ �����
    �/��� ���������� ��������� �� ����� ����������, ������� ����������
    ��������� ��������� ��������, � � ���� �������� �� ����������� � ������
    �����������, ��� LZP by Ilya Grebnev � RZIP. ��� ����, ��� ����������
    ������������, ��� ������������� ����������� �������� ����������� �������
    ������ ��������� ������ � ���� �������� - 32-512 ����. ���� ��������
    ������� ���� ������ ����������, ��� LZP/RZIP, � ����� ����, ���
    �������� ������ ������������� ��� ���������� MinLen.

    �������� ����������� ��������� REPEncode() � REPDecode(), � ����������
    ��������� ���� �� LZP, RZIP � ���� �����������. ����� ���������� ������ �
    ���������� ���� - ������� ������ ����������� ������� �� 1/16 �� �������
    ������, � ��� �������� ��� � ����� ������ ������� ��� ������� 15/16 ������
    �������� ���������� ������, ������� ����������� � ������� ����������. ���
    ��������� ��������� �� ������� �����, �� ���������� �� ����� ����������
    ������� ������.

    ��� ������, ��� ������ ����� � ������ �� MinLen � ������� ����� �����
    ������ MinLen ����������� ����� ����������� ����� (��), ������� ��������� �
    ���-�������. ��������� �������� ������������ �� ������� �������� MinLen,
    ������� ���������� �� �� ������ ����� ����� �������� ���������. ���
    �������� �������� �������������� "���������� ��", �� ���� �����, �������
    ����� ������ ����������� ��� ���������� ������ ����� � ����� ����� �
    �������� ������ ����� � ������ (��. update_hash).

    ������ ��������� ������� ��� ����������� ����������� ��� ���������
    ������������. � ����� ������ ������� ������� hash = p[-1] + PRIME*p[-2] +
    PRIME*PRIME*p[-3] + ..., ��� PRIME - ������� �����, ��������� ����� �������
    � ������ ������ ����������� �������������. ����������, ��� ���������� ����
    �� ������ 1<<32, ������� ���������������� ��� ����������� :)

    �����, ���� ������������ �������������� ���� ��� ���������� ���������� �
    ������ � ���������� ��������. ���������� � ������� ������ ��������� ���
    MinLen=512. ��������� ����� 512-������� ���� �������� � ���� 256-�������
    ����, ������������ � �������, ������� 256, �� ��� ���������� ��������� �
    ���-������� ������ ������ �� ��� ����� � ������ ���������� ������ � ����.
    ����������, ��� �������� ���������� �� �� �������������� � �������� 256
    �������, � �������� ���������� ��� ��� ����� ������ � ��� �������. ������
    ��� � ��������� ����������� ��������� ������ ������ ��� ���������������
    ���������� ����� ���� ���������� - �� ������ ������, ����� MinLen
    ���������� ������.

    ������ ����� ����� ��� ������ - ������ ����, ����� ��������� � ���-�������
    ������ 256-� ����, �� ������ ������-������, �� ����� �������� ���������
    ������ 32-�, � ������ ������ 8-�, ��� ��������� ������ 2-�, � ������ ������
    128-�. ����������, ��������� ����� ��������� � ������ ������ 16-� ����.
    ������ ������, ����� ��������� ���� ���� ����� ������ 16 ����, � ������
    ������ 16 ������ �� ������ 256, �� ���� ��������� �����, ������������ �
    ������� 0, 16, 32..., � ���� �����, ������������ � ������� 0, 1, 2..., 15,
    256. 257... ����� �������, ��� MinLen=512 ����������� 8-������� ���������
    ������ (�� ���� 8-�������� ���������� ���������� ��������� � ������) ��
    ��������� � ������������� ����������� - ������, �� ���� ����������
    ���������� � ������ (� 1/64 ������� ������ �� 1/4, ��� �� ��� ������ ������
    ���������).

    �������, ��������� ������ �������� ������������� ������� ����� ������ �
    ���-������� ��� �������� ���������� ��� �� �������� ���-������� (chksum) -
    ����������, ���, ������� �� �������� ������ ������� � ���-�������. ���
    ��������� ������� ������� ����� ������ ����������, �� ��������� ����������
    ������, � ��� ����� ��������� ���������� ��������� � ������ � ��� ������
    �������� ������ ���������.

    � ��������� ������������ ����������� � ������ ����������, ��� ����������
    �����������, ��� ������ ���������� ����� �������. �������� ���-�������
    �� 256-�������� ����� (� ����� ������ ������ ����� ����� - L=MinLen/2)
    ������������ ��� ������ � ���-������� (hasharr[hash&HashMask]), ���
    ��������� ����� ���� ������ �������� ����� ������. �� �������� ���
    (�����������) �� ���� � ���������� ����������. ��� ��� ���������, ���
    ���� ��������, � ������� �� ����������� LZ77 ����������, ���� ��
    ����������� (����� �������) ����������, � ��������� ���� ���� ������ - ��
    ��������� ����, ������� ����� ���� ���-����, � ��� ��, �������������,
    ���������������� ��������� � �� �������� �����.

    ������ ���� (HashSize): ��� ���������� ��������� � �����������, ��� ��
    ������ ���� � 2-4 ���� ������ ���������� ���������, ������� � ���� �������
    ���������. ������ �� �������� ���������, ��� ������ ���������� ����� �� ��
    ����� ���-�� ������, � ��� MinLen=32 - ���� �������� (!) �������. �� ����,
    ��������, ��� 32 �� ����� ��� MinLen=512 � ��� ����������� ������ 16-�
    256-������� ���� � ����� ���������� ����������� ��������� - 32���/16=2���,
    �.�. 8 ��, � ��� �������� ������ ������ �������. ��� MinLen=32 �����
    ���������� ��������� 32���/4=8���, �� �� ������ ���-������� ��������
    ������, �� ���� ���������� �� �� ����� 8 ��. ����� �������, �����������
    ���������� ������������� ������ ���-������� ������� �� ����������� 1/4
    ������� �������� ������. ���� �� ������ ���������� ������ �������� - ��
    ����������� �������� HashBits (����� -h). ���������� HashSize ��� ���������
    MinLen �������� ������� ��������� ������� ������.

    Amplifier: ��� ���� ������� ����, ��� ������ ����������� ������ �����
    ������, ������� �� � ��������� ������� ��� ���������� ���� ����� � ������
    >=MinLen - ���� � ��� ��������� �����������. ������ ���� �����������
    ����������, � ����� ������������� ����� ��-�� ����� ��������. ��������
    Amplifier (����� -a) ��������� ����������� ������������ �������� �����
    ������ (� ��� ����� Amplifier ���). ����� �������, ��� �����������
    ����������� ������ ����� ������ ���������� Amplifier � ���������� �������
    ��������, ������ 99. ����������, ��� ��������� �������� � ���� ���������
    ����������� ������.

    Barrier � SmallestLen: ��������� ���������, � ��������� ppmd, ����������,
    ���� ������������ ���������� ������� �������� MinLen ��� �������
    ���������. ��� ��� ��������� ��������� ���������� ��������������� �������
    ������ ����������, �������� "� ������ ��������� - MinLen=128, �����
    MinLen=32" ������� ����� MinLen=128, Barrier=1<<20, SmallestLen=32
    (����� -l128 -d1048576 -s32). ��� ���� ����� ����� �������������, ��-��,
    �� ���������� ����� � ������ �� SmallestLen ������ MinLen.


** Benchmarks using 1GHz processor ****************************************************************

Test results for 26mb:
        Compression time   Compressed size
-l8192  0.5 seconds
 -l512  1.1
 -l128  1.4
  -l32  2.5                12.7 mb
lrzip   2.6                14.1
lzp:h20 6.5                13.1
lzp:h13 3.0                20.6

Compression speed on incompressible data:
-l8192  52 mb/sec
 -l512  25 mb/sec
 -l128  17 mb/sec
  -l32   8 mb/sec
lrzip    8 mb/sec


** REP �� ������� ������ ������� ���**************************************************************

������, ��� �� �������� �������� ������ rep � �������� ���������. �� ���������
� ��� ������������ ������ 1�� ��� ����� ������ � �������� ������ - ��� �������.
������� ������� ������ ���������� ������ ������, ��� ����������� 4-������� ���,
������� ����������� �� ���� 512 ���� - �������� � ������ ��������. ������
����������� ����, ��� �� ������ ������ ���� 16-������� cryptographically strong
hash - ���� md5. ����� ������� ����� ���������� �����. ����� ����, �� ���
���-������� �������� �������� �� ������ ��������������� ������ - ���
�������������. ���� ������� ����� ��� ��� ������� 256-�������� ����� ������, ��
��� ����������� ��� ���������� ���� ������ ����� �� 511 (��������� ����� �����
���� �������� ��� ������� ���� ������ 256-������� ����, ������������ �
256-������� �������). �.�. ��� ������ ����� ����� 511+ c N-�� ��������
���������� ������ � N/16 ��

�������� ��������� ������ ��� ���������� :D  ���� ��� �������� ������ ������
��� � �� ����� - ���������� ����� 100% ����������� � �� ����������, �� ���
���������� ��� ���-����� ���� �� ���������� �� ������� ����� :D  ���� �������,
��� �� ��� ��� ������ ����� ������� �� ����� ������ ���, �� ����������� ������
������ ��������� �������� ������ � �����, ��������� ������� �� �������
����������� ����� disk seek time - �.�. 10 �� ��� ����� � 1 �� ��� �����
������� ������

���������� ����, ��� �� ����� ���������� �������� ���������� ������ 1 ��/�. ���
��������, ��� ������ 10 �� �� ������ ������������� ��� ������� 10 ��, ��� �
���� ������� ������������� ������ ���� � ��� rep �������� ������ ����������
����� 10��+

������, ���� ������������ �� ����������� ����� ����� 4��+, �� ��� ������
����������� N/128 �� ������ (�.�. ���� 18 ��� ����� �����������, ���������
����� 160 ��� ���) � �������� ���������� ����� ���������� 400 ��/�. ��� ������
���� ����� :D

Ghost, �������� ��� �������� - ��� �������� ������ ����� ������ ��� �������� ��
rep:512 (�� ���������) � rep:4096? � ���������� ���������� lzma � ��� ����.
�������, ��� ���� ��������, ��������� ������� rep ������ ��������� �������
��������� �� ����� :(  �����, ��� ��� ���� �������� �� ������� ����, ���
����������� ����������...

*/


// �������� ************************************************************************
#include "../Compression.h"


#ifdef REP_LIBRARY
#define stat1(nextmsg,size)
#else
void stat1 (char *nextmsg, int Size);
#endif


// ����� ��������� ������ **********************************************************************
#ifndef REP_LIBRARY
// ����� ����������, ���������� �� stdout
//   0   ������ ������
//   1   ����� ����������
//   2   ��������� ���������� � ��������
static int verbose = 0;

#endif


// ��������������� ������� *********************************************************************

// ���������� � �������
inline static unsigned power (unsigned base, unsigned n)
{
    int result;
    for (result=1; n != 0; result *= base, n--);
    return result;
}

// ���������� ������� base, �� ������������� sqrt(n),
// �������� sqrtb(36,2) = 4
inline static unsigned sqrtb (unsigned n, unsigned base = 2)
{
    int result;
    for (result=1; (n/=base*base) != 0; result *= base);
    return result;
}

// ������� ����� ������ ����������, ��� ����� �� *p � *q
static inline byte* find_match_start (byte* p, byte* q, byte* start)
{
    while (q>start)   if (*--p != *--q)  return q+1;
    return q;
}

// ������� ����� ������� �������������� �����, ��� ����� �� *p � *q
static inline byte* find_match_end (byte* p, byte* q, byte* end)
{
    while (q<end && *p==*q) p++,q++;
    return q;
}

// �������� ������ �� ������ � �����, ��� � ������� ����������� �������
// (��� �����, ��������� ������ ����� ������������ � � ���� ������ �����
// ���������� ������������ ������)
static inline void memcpy_lz_match (byte* p, byte* q, unsigned len)
{
    if (len)
    do *p++ = *q++;
    while (--len);
}


// �����, ������������ ��� ����������� ���������� ����������� ������� ������
// � ���������. ����� ����� ���������� � ���� 32-��������� �����. �������
// ���������� ������ ������������ � �������� �����.
// ������������� ����� ������������ ������ ����� ���������� � ���� ������.
// ����� ���������� ����� ������ - ��� max(p,end), ��� p - ������� ���������,
// � end - ������������ ������� ����� ���������� ������.
// �������� ������������ �� ������������, ��������� �������� �����������,
// ��� ������������ �� ���������.
struct Buffer
{
    byte*  buf;                 // ����� ����������� ������
    byte*  p;                   // ������� ��������� ������/������ ������ ����� ������
    byte*  end;                 // ����� ����� ����� �����������/���������� ������
    byte*  bufend;              // ����� ����������� ������
    byte   smallbuf[16];        // ��������� �����, ������������ ��� ������ ��������� ��������
    int    len()                { return mymax(p,end)-buf; }
    Buffer (int size)           { buf=p=end= size<sizeof(smallbuf)? smallbuf : (byte*) BigAlloc(size);  bufend=buf+size;}
    void   free ()              { if (bufend>buf+sizeof(smallbuf))  BigFree(buf);  buf=p=end=NULL; }
    void   put32(int x)         { *(int32*)p = x; p+= sizeof(int32); }  // only for FREEARC_INTEL_BYTE_ORDER!
    void   put(void *b, int n)  { memcpy(p,b,n); p+= n; }
// ��� ������ ������
    void   rewind()             { end=mymax(p,end); p=buf; }
    int    get32()              { int x = *(int32*)p; p+= sizeof(int32); return x; }
    bool   eof()                { return p>=end; }
// ��� FWRITE
    int    remainingSpace()     { return bufend-p; }
    void   empty()              { p=end=buf; }
};

// �������� 32-������ ����� � �������� �����
#define Put32(x)                                           \
{                                                          \
    Buffer header(sizeof(int32));                          \
    header.put32 (x);                                      \
    FWRITE (header.buf, header.len());                     \
    header.free();                                         \
}


// �������� �������� *********************************************************************

/*
    ��� ���������� ���������� ������ �� MinLen ���� ����� �������� � ��� ��������
    ����������� ������� �� ������ ������ L = MinLen/2 ���� � �������� k = sqrt(L) ����.
    ������ � ���� ���-������� ���������� ��� ������, ������������ � ������ test=k ������
    �� ������� ����� ������ L ����.
*/

#define update_hash(sub,add)                        \
{                                                   \
    hash = hash*PRIME + add - sub*cPOWER_PRIME_L;   \
}

#define chksum         ((hash>>28)&k1)
#define PRIME          153191           /* or any other prime number */
#define POWER_PRIME_L  power(PRIME,L)

const int MAX_READ = 8*mb;  // ����. ����� ������� ������, �������� �� ���


// ��������� ���������� ��������� ����
MemSize CalcHashSize (MemSize HashBits, MemSize BlockSize, MemSize k)
{
    // ������ ���� ������ ��������������� ���������� ��������. ������� �� ����� � ���� �������, �� �� ��������� �������� �� ������� ������ / ������ ������� ������ (Size/16*sizeof(int)==Size/4)
    return HashBits>0? (1<<HashBits) : roundup_to_power_of(BlockSize/3*2,2) / mymax(k,16);
}

#ifndef FREEARC_DECOMPRESS_ONLY
int rep_compress (unsigned BlockSize, int MinCompression, int MinMatchLen, int Barrier, int SmallestLen, int HashBits, int Amplifier, CALLBACK_FUNC *callback, void *auxdata)
{
    // ��������� ���������� ���������  (����� � REP_METHOD::GetCompressionMem!)
    if (SmallestLen>MinMatchLen)  SmallestLen=MinMatchLen;
    int L = roundup_to_power_of (SmallestLen/2, 2);  // ������ ������, �� ������� ��������� � ���
    int k = sqrtb(L*2), k1=k-1, test=mymin(k*Amplifier,L), cPOWER_PRIME_L = POWER_PRIME_L;
    int HashSize, HashMask=0, *hasharr=NULL, hash=0;  int errcode=FREEARC_OK;
    int Base=0, last_i=0, last_match=0;    // last_match points to the end of last match written, we shouldn't start new match before it
#ifdef DEBUG
    int matches=0, total=0, lit=0;
#endif
    byte *buf = (byte*) BigAlloc(BlockSize);   // �����, ���� ����� ���������� ������� ������
    if (buf==NULL)  return FREEARC_ERRCODE_NOT_ENOUGH_MEMORY;    // Error: not enough memory
    FOPEN();

    int bsize = (mymin(BlockSize,MAX_READ)/SmallestLen+1) * sizeof(int32);    // ����. ����� ������, ������� ����� ���� ������� � ����� ���� ��� ��������
    Buffer lens(bsize), offsets(bsize), datalens(bsize), dataOffsets(bsize);  // ������ ��� ���������� �������� ����, �������� ����������, ���� �������� ������ � ����� ���� ������. ��� ����������� ��������� ��������� �������� ������� ������

    // ������ �������� ����� ����� ������, ������������ � ���������� ���� ���� ������
    // �������� � min(1/8 ������,8��). ��� ������������ ��������� ���� sliding window,
    // �� ���� ����������� ������ ���������� � ����������� ������� ����� �� ��� ����� ������
    for (int FirstTime=1; ; FirstTime=0) {

        // ������ ������� ������
        int Size = callback ("read", buf+Base, mymin (BlockSize-Base, FirstTime? MAX_READ : mymin (BlockSize/8, MAX_READ)), auxdata);
        if (Size < 0)  {errcode=Size; goto finished;}   // Error: can't read input data
        if (FirstTime) {
            HashSize = CalcHashSize (HashBits, BlockSize, k);
            HashMask = HashSize-1;
            hasharr  = (int *) BigAlloc (HashSize * sizeof(int));
            if (HashSize && hasharr==NULL)  {errcode=FREEARC_ERRCODE_NOT_ENOUGH_MEMORY; goto finished;}   // Error: not enough memory
            memset (hasharr, 0, HashSize * sizeof(int));
            debug (verbose>0 && MinMatchLen==SmallestLen && printf(" Buf %d mb, MinLen %d, Hash %d mb, Amplifier %d\n", ((Size-1)>>20)+1, MinMatchLen, (HashSize*sizeof(int))>>20, test/k));
            debug (verbose>0 && MinMatchLen!=SmallestLen && printf(" Buf %d mb, MinLen %d, Barrier %d, Smallest Len %d, Hash %d mb, Amplifier %d\n", ((Size-1)>>20)+1, MinMatchLen, Barrier, SmallestLen, (HashSize*sizeof(int))>>20, test/k));
            Put32 (BlockSize);   // ������� ������ ������� � �������� �����
        }
        if (Size == 0) break;  // No more input data
        debug (verbose>0 && printf(" Bytes read: %u\n", Size));
        if (Base==0)  {   // � ������ ��� ��� ����� �������� ����� ������� ������
            hash=0;  for (int i=0; i < mymin(L,Size); i++)  update_hash (0, buf[i]);  // ��������� �������� hash - �� �� ������ L ���� ������
        }
        int literals=0; lens.empty(), offsets.empty(), datalens.empty(), dataOffsets.empty();  // �������� ������

        // �������� ����, ��������� ������������� ������ �� ������� ������
        for (int i=last_i; i+L*2 < Base+Size; last_i=i) {   // ������������ �� L ���� �� ���� �������� ����� + ���� ����� L ���� lookahead

            // ���� ���������� � ������ test ������ ����� ����� L
            for (int j=0; j<test; j++, i++) {
                if (i>=last_match) {   // ��������� ���������� ������ ���� ���������� ��������� ���������� ��� ���������
                    int match = hasharr[hash&HashMask];
                    if (match && chksum==(match&k1)) {  // ������� ���� �������� match ������ ����������� ����� chksum. Ÿ �������� ��������� ���������� ����������� ��������� ������ � ������ ���-�������� (������������� ������ �������� hasharray ��� ������ ��������� hash)
                        match &= ~k1;   // ����� �� �� match. ������ i � match - ������ ���������������� ����������� ������ ����� L
                        if (match>=i && match<Base+Size)  goto no_match;  // match �������� �� ��� �� ������������ ������, �� ���� �� �������� �������
                        // ����������/���������� ��������, ������� ����� ��������� ��� ������
                        // ������ ������������ �� i, ����� ������ ������������ �� match,
                        // �� ����� �� ������� ������ � �� �������� � ������� ������
                        int LowBound  = match<i? i-match : match-(Base+Size)>i? 0 : i - (match-(Base+Size));
                        int HighBound = BlockSize - match + i;
                        // ����� �������� ������ � ����� ����������, ��������� ����� � ����� �� buf[i] <=> buf[match]
                        // i ���������� ����� � ������ ���������� last_match � Base+Size, ��������������
                        int start = find_match_start (buf+match, buf+i, buf+mymax(last_match,LowBound)) - buf;
                        int end   = find_match_end   (buf+match, buf+i, buf+mymin(Base+Size,HighBound)) - buf;
                        // start � end - ������� ���������� ������ i. ��������, ��� ��������� ���������� ����� ����� >=MinMatchLen (��� SmallestLen, ���� ��������� >Barrier)
                        if (end-start >= (i-match<Barrier? MinMatchLen : SmallestLen) ) {
                            int offset = i-match;  if (offset<0)  offset+=BlockSize;
                            // ���������� �������! ������� ���������� � ��� � �������� ������
                            dataOffsets.put32 (last_match);         // ����� ����������� ������
                               datalens.put32 (start-last_match);   // ����� ����������� ������
                                offsets.put32 (offset);             // �������� match'�
                                   lens.put32 (end-start);          // ����� match'�
                            // ��������� ������� ����� ���������� ���������� � ������� ���������� ����������
                            debug ((matches++, total += end-start, lit += start-last_match));
                            debug (verbose>1 && printf ("Match %d %d %d  (lit %d)\n", -offset, start, end-start, start-last_match));
                            literals += start-last_match;  last_match=end;
                        }
                    }
                }
      no_match: // ������� � ������� ����� ����� ����� ������ k ����. ���� Amplifier=1, �� ��� ������� ����������� ������ ��� j=0, � ��������� ����� ������������� � ��������� �����
                if ((i&k1) == 0)  hasharr[hash&HashMask] = i + chksum;
                update_hash (buf[i], buf[i+L]);  // ������� sliding hash, ����� � ���� buf[i+L] � ������ buf[i]
            }
            // NB! ����������� �� ������� k �������!

            // ������� � ������� ����� ����� ����� ������ k ���� �� ����� �������� ����� ����� L
            while ((i&(L-1)) != 0) {
                hasharr[hash&HashMask] = i + chksum;
                for (int j=0; j<k; j++, i++)   update_hash (buf[i], buf[i+L]);
            }
        }

        // ����� ������ ������ � �������� ����� � ���������� � ��������� ��������� ������ ������
        Base += Size;
        if (Base==BlockSize)  last_i=Base;       // ������������ ��� ������ �� ����� ������
        if (last_match > last_i) {               // ���� ��������� ���� ��������� � ��� �� ������������������ �������
          datalens.put32 (0);                    //   ������ ���������� �� ����, �� datalens ������ �� ����� ���� ����� �� ���� ������ ������� lens/offsets
        } else {
          // �������� � �������� ������ ������� ������ �� ���������� ���������� ���������� �� ��������� ������������������ ������
          dataOffsets.put32 (last_match);          // ����� ������� ������
             datalens.put32 (last_i-last_match);   // ����� ������� ������
          literals  += last_i-last_match;
          last_match = last_i;
        }
        if (Base==BlockSize) {       // ���� ���������� ������� ����� ������� ������
          Base=last_match=last_i=0;  //   ��! ������ ��������� ����� � ������!
        }
        // �������� ������ ������ ������ � ���������� ��������� ���������� � �����
        int outsize = sizeof(int32)*2+lens.len()+offsets.len()+datalens.len()+literals;
        QUASIWRITE (outsize);
        Put32 (outsize-sizeof(int32));
        Put32 (lens.len()/sizeof(int32));
        // ������� ���������� ������� � �������� ������ � �������� �����
        FWRITE (    lens.buf,     lens.len());
        FWRITE ( offsets.buf,  offsets.len());
        FWRITE (datalens.buf, datalens.len());
        dataOffsets.rewind(); datalens.rewind();
        while (!dataOffsets.eof()) {
            FWRITE (buf + dataOffsets.get32(), datalens.get32());
        }
        FFLUSH();
        // ���������� ����������
        debug (verbose>0 && printf(" Total %d bytes in %d matches (%d + %d = %d)\n", total, matches, sizeof(int32)*2+lens.len()+offsets.len()+datalens.len(), lit, sizeof(int32)*2+lens.len()+offsets.len()+datalens.len()+lit));
    }

    // �������� ��������� ����, ���������� ����������� ������� ������, � 0 - ������� ����� ������
   {int datalen = Base-last_match;
    Put32 (sizeof(int32)*2 + datalen);  // ����� ������� �����
    Put32 (0);                          //   0 matches in this block
    Put32 (datalen);                    //   ����� ������� ������
    FWRITE (buf+last_match, datalen);   //   ���� ��� ������
    Put32 (0);}                         //   EOF flag (see below)
finished:
    FCLOSE();
    BigFree(hasharr);
    BigFree(buf);
    lens.free(); offsets.free(); datalens.free(); dataOffsets.free();
    return errcode>=0? 0 : errcode;
}
#endif // FREEARC_DECOMPRESS_ONLY


// Classical LZ77 decoder with sliding window
int rep_decompress (unsigned BlockSize, int MinCompression, int MinMatchLen, int Barrier, int SmallestLen, int HashBits, int Amplifier, CALLBACK_FUNC *callback, void *auxdata)
{
    int errcode;
    byte *buf0=NULL, *data0=NULL, *data1=NULL, *end0, *end1;
    MemSize bufsize, data0_size, ComprSize;
    bool block0 = TRUE;

    // ����������� ������ ������� �������� �� ������� ������
    READ4(BlockSize);
    data0_size = BlockSize;
    data0 = (byte*) BigAlloc (BlockSize);
    // ���� �������� ������ ����� ������ �� ������� - �������� ����� LZ-������� �� ��� �����
    if (data0==NULL)
    {
      while (data0==NULL && data0_size>1*mb)
        data0 = (byte*) BigAlloc (data0_size -= 1*mb);
      data1   = (byte*) BigAlloc (BlockSize - data0_size);
    }
    end0 = data0 + data0_size;
    end1 = data1 + BlockSize - data0_size;
    if (data0==NULL || (data0_size<BlockSize && data1==NULL))  ReturnErrorCode (FREEARC_ERRCODE_NOT_ENOUGH_MEMORY);

    // �����, ���� ����� ���������� ������� ������
    bufsize = mymin(BlockSize,MAX_READ)+1024;
    buf0 = (byte*) BigAlloc (bufsize);
    if (buf0==NULL)  ReturnErrorCode (FREEARC_ERRCODE_NOT_ENOUGH_MEMORY);

    // ����, ������ �������� �������� ������������ ���� ���� ������ ������
    for (byte *last_data=data0, *start=data0, *data=data0, *end=end0; ; last_data=data) {

        // ��������� ���� ���� ������ ������
        READ4(ComprSize);
        if (ComprSize == 0)  break;    // EOF flag (see above)

        if (ComprSize > bufsize)
        {
            BigFree(buf0); bufsize=ComprSize; buf0 = (byte*) BigAlloc(bufsize);
            if (buf0==NULL)  ReturnErrorCode (FREEARC_ERRCODE_NOT_ENOUGH_MEMORY);
        }
        byte *buf = buf0;

        READ(buf, ComprSize);

        // ��������� ����� �������� ������ ������ lens/offsets/datalens; ����� ���� ���� ��� ������� � ������� ����������� ������
        int         num = *(int32*)buf;  buf += sizeof(int32);           // ���������� ���������� (= ���������� ������� � �������� lens/offsets/datalens)
        int32*     lens =  (int32*)buf;  buf += num*sizeof(int32);
        int32*  offsets =  (int32*)buf;  buf += num*sizeof(int32);
        int32* datalens =  (int32*)buf;  buf += (num+1)*sizeof(int32);   // ������, datalens �������� num+1 �������

        // ������ �������� ����� ����� �������� ���� ���� �������� ������ � ���� match, ������� interleaved � ����� ���������� �������� ��������
        for (int i=0; ; i++) {
            // ���� ���������� ������ ���������� ������� ������
            while (end-data < datalens[i])
            { //printf("  %d %d %d %d\n", BlockSize, data-data0, end-data, datalens[i]);

              // �������� ��������� �������, ���������� ����������� �����, � ������������� �� ������
              int len = end-data;
              memcpy (data, buf, len);  buf += len;  data += len;  datalens[i] -= len;

              WRITE(last_data, data-last_data);

              if (data0_size<BlockSize)  block0 = !block0;
              last_data = start = data = block0? data0 : data1;
              end = block0? end0 : end1;
            }
            // ����������� ��� ����������� ������� ������
            memcpy (data, buf, datalens[i]);  buf += datalens[i];  data += datalens[i];


            if (i==num)  break;   // � ����� ����� � ��� ��� ���� ���� ����������� ������ (��������, ������� �����) ��� ������� ��� lz-�����


            debug (verbose>1 && printf ("Match %d %d %d\n", -offsets[i], block0? data-data0 : data-data1+data0_size, lens[i]));
            int offset = offsets[i];
            // ���� ���� �� ���������� ����� ���������� ������� ������
            while (offset > data-start && lens[i]  ||  end-data < lens[i])
            {
              MemSize dataPos = block0? data-data0 : data-data1+data0_size;                     // Absolute position of LZ dest
              MemSize fromPos = offset<=dataPos? dataPos-offset : dataPos-offset+BlockSize;     // Absolute position of LZ src
              byte *from = fromPos<data0_size? data0+fromPos : data1+(fromPos-data0_size);      // Memory address of LZ src
              byte *fromEnd = fromPos<data0_size? end0 : end1;                                  // End of membuf containing LZ src
              int len = mymin(end-data, fromEnd-from);  // How much bytes we can copy without overrunning src or dest buffers
                  len = mymin(len, lens[i]);
              //printf("? %d-%d=%d %d(%d %d)\n", dataPos, offset, fromPos, len, end-data, fromEnd-from);

              // �������� ��������� �������
              memcpy_lz_match (data, from, len);  data += len;  lens[i] -= len;

              // ���� dest ����� �������� - ���������� ����������� �����, � ������������� �� ������
              if (data==end)
              {
                WRITE(last_data, data-last_data);

                if (data0_size<BlockSize)  block0 = !block0;
                last_data = start = data = block0? data0 : data1;
                end = block0? end0 : end1;
              }
            }
            // ����������� ��� ����������� ������� ������
            memcpy_lz_match (data, data-offset, lens[i]);  data += lens[i];
        }

        // ����� ������������� ������, ������ ���������� ���������� � ���������� � ��������� �������� �����
        WRITE(last_data, data-last_data);
        debug (verbose>0 && printf( " Decompressed: %u => %u bytes\n", ComprSize+sizeof(int32), data-last_data) );
        // NB! check that buf==buf0+Size, data==data0+UncomprSize, and add buffer overflowing checks inside cycle
    }
    errcode = FREEARC_OK;
finished:
    BigFree(buf0);  BigFree(data1);  BigFree(data0);  return errcode;
}


/* to do:
+1. sliding window, In() function to read data
+2. ������������ ������, ���. ��� �������
+3. save pointers to unmatched blocks instead of copying data
4. ���������, ��� ���� ����� ����������, � �������� ��� ����� ���������.
     ������, ���������� ������� �������� last_match � �������� ��� �������� ������
5. last_small_match - ���� ��������� match ������ �� ��������� ���������� (<Barrier),
     �� ������������ ��������� ����� �� ������� ���������� (>Barrier) ���� ���� �� ��������.
     ��� �������� ��� ��������� �������� ���� � ������� ����� :)
6. -l8192 -s512
7. buffer data for Out() in 256k blocks

Fixed bugs:
1. �������� ������ �� ������� ������: offset<data-data0 ������ <=
2. last_match �� ��������� ��� ������ �� ����� ��� Base=0 � Size=0
*/
