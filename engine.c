#include<stdio.h>
#include<string.h>
#include<stdlib.h>//for atoi() function
#ifdef WIN64//for get time in ms function (cross platform service)
    #include <windows.h>
#else
    # include <sys/time.h>
#endif


#define U64 unsigned long long


#define emptyBoard "8/8/8/8/8/8/8/8 w - - "
#define startPosition "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define trickyPosition "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"
#define killerPosition "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR b KQkq e6 0 1"
#define cmkPosition "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "


#define getBitStatus(bitboard,square) ((bitboard) & (1ULL << (square))) 
#define setBitStatus(bitboard,square) ((bitboard) |= (1ULL << (square)))
#define popBitStatus(bitboard,square) ((bitboard) &= ~(1ULL << (square)))//better logic than original one 

static inline int countBits(U64 bitboard)
{
    int cnt = 0;
    while(bitboard > 0)
    {
       cnt++;
       bitboard &= bitboard - 1;
    }
    return cnt;
}
static inline int getLsbBitIndex(U64 bitboard)
{
    if(bitboard == 0) return -1;
    
    return countBits((bitboard & -bitboard) - 1);
}
//representation of a chess board ans each square has its unique index through enum
enum
{
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,noSq
};

// castling rights binary encoding

/*

    bin  dec
    
   0001    1  white king can castle to the king side
   0010    2  white king can castle to the queen side
   0100    4  black king can castle to the king side
   1000    8  black king can castle to the queen side

   examples

   1111       both sides an castle both directions
   1001       black king => queen side
              white king => king side

*/

enum//types of castling
{
    wk = 1, wq = 2, bk = 4, bq = 8
};

enum//uppercase -> white pieces 
{
   P, N, B, R, Q, K, p, n, b, r, q, k 
};
//representing 6 black peices and 6 white pieces
//pawn,rook,bishop,king,queen,rook
//each bitboard represents one of their separate table



/**********************************\
 ==================================
 
            Chess board
 
 ==================================
\**********************************/

/*
                            WHITE PIECES


        Pawns                  Knights              Bishops
        
  8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 0 0 0
  7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0
  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
  2  1 1 1 1 1 1 1 1    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0
  1  0 0 0 0 0 0 0 0    1  0 1 0 0 0 0 1 0    1  0 0 1 0 0 1 0 0

     a b c d e f g h       a b c d e f g h       a b c d e f g h


         Rooks                 Queens                 King

  8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 0 0 0    8  0 0 0 0 0 0 0 0
  7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0
  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
  2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0
  1  1 0 0 0 0 0 0 1    1  0 0 0 1 0 0 0 0    1  0 0 0 0 1 0 0 0

     a b c d e f g h       a b c d e f g h       a b c d e f g h


                            BLACK PIECES


        Pawns                  Knights              Bishops
        
  8  0 0 0 0 0 0 0 0    8  0 1 0 0 0 0 1 0    8  0 0 1 0 0 1 0 0
  7  1 1 1 1 1 1 1 1    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0
  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
  2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0
  1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 0 0 0

     a b c d e f g h       a b c d e f g h       a b c d e f g h


         Rooks                 Queens                 King

  8  1 0 0 0 0 0 0 1    8  0 0 0 1 0 0 0 0    8  0 0 0 0 1 0 0 0
  7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0    7  0 0 0 0 0 0 0 0
  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
  2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0    2  0 0 0 0 0 0 0 0
  1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 0 0 0    1  0 0 0 0 0 0 0 0

     a b c d e f g h       a b c d e f g h       a b c d e f g h



                             OCCUPANCIES


     White occupancy       Black occupancy       All occupancies

  8  0 0 0 0 0 0 0 0    8  1 1 1 1 1 1 1 1    8  1 1 1 1 1 1 1 1
  7  0 0 0 0 0 0 0 0    7  1 1 1 1 1 1 1 1    7  1 1 1 1 1 1 1 1
  6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0    6  0 0 0 0 0 0 0 0
  5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0    5  0 0 0 0 0 0 0 0
  4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0    4  0 0 0 0 0 0 0 0
  3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0    3  0 0 0 0 0 0 0 0
  2  1 1 1 1 1 1 1 1    2  0 0 0 0 0 0 0 0    2  1 1 1 1 1 1 1 1
  1  1 1 1 1 1 1 1 1    1  0 0 0 0 0 0 0 0    1  1 1 1 1 1 1 1 1



                           ALL TOGETHER

                        8  ♜ ♞ ♝ ♛ ♚ ♝ ♞ ♜
                        7  ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎ ♟︎
                        6  . . . . . . . .
                        5  . . . . . . . .
                        4  . . . . . . . .
                        3  . . . . . . . .
                        2  ♙ ♙ ♙ ♙ ♙ ♙ ♙ ♙
                        1  ♖ ♘ ♗ ♕ ♔ ♗ ♘ ♖

                           a b c d e f g h


*/
U64 bitboards[12];

//black occupancy,white occupancy and total occupancy(both)
U64 occupancies[3];

int side;//side to move

int enpassant = noSq;

int castle;


char asciiPieces[12] = "PNBRQKpnbrqk";

char *unicodePieces[12]= {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

// convert ASCII character pieces to encoded constants

int charPieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

char promotedPieces[] = {
    [Q] = 'q',
    [R] = 'r',
    [B] = 'b',
    [N] = 'n',
    [q] = 'q',
    [r] = 'r',
    [b] = 'b',
    [n] = 'n'
};

enum
{
    white,black,both
};

enum 
{
    rook,bishop
};
const char *squareToCoordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};
//not a file
const U64 notAfile = 18374403900871474942ULL;
const U64 notABfile = 18229723555195321596ULL;
const U64 notHfile = 9187201950435737471ULL;
const U64 notGHfile = 4557430888798830399ULL;

const U64 rookMagicNumbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};


const U64 bishopMagicNumbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};

const int bishopRelevantBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};


const int rookRelevantBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};


U64 pawnAttacks[2][64];
U64 knightAttacks[64];
U64 kingAttacks[64];

//slider pieces 
U64 bishopMasks[64];
U64 rookMasks[64];

U64 bishopAttacks[64][512];
U64 rookAttacks[64][4096];

U64 maskPawnAttacks(int side,int square)
{
   U64 attacks = 0ULL;
   U64 bitboard = 0ULL;
   
   //set the specific bit first
   setBitStatus(bitboard,square);
   
   if(!side)
   {
      if((bitboard >> 7) & notAfile) attacks |= (bitboard >> 7);
      if((bitboard >> 9) & notHfile) attacks |= (bitboard >> 9);
   }
   else
   {
      if((bitboard << 7) & notHfile) attacks |= (bitboard << 7);
      if((bitboard << 9) & notAfile) attacks |= (bitboard << 9);
   }
   
   return attacks;
   
  
}

U64 maskKnightAttacks(int square)
{
   U64 attacks = 0ULL;
   U64 bitboard = 0ULL;
   
   setBitStatus(bitboard,square);
   
   if((bitboard >> 15) & notAfile) attacks |= (bitboard >> 15);
   if((bitboard >> 17) & notHfile) attacks |= (bitboard >> 17);
   if((bitboard >> 6) & notABfile) attacks |= (bitboard >> 6);
   if((bitboard >> 10) & notGHfile) attacks |= (bitboard >> 10);
   
   if((bitboard << 15) & notHfile) attacks |= (bitboard << 15);
   if((bitboard << 17) & notAfile) attacks |= (bitboard << 17);
   if((bitboard << 6) & notGHfile) attacks |= (bitboard << 6);
   if((bitboard << 10) & notABfile) attacks |= (bitboard << 10);
   
   return attacks;
}
U64 maskKingAttacks(int square)
{
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    
    setBitStatus(bitboard, square);
    
    
    if (bitboard >> 8) attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & notHfile) attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & notAfile) attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & notHfile) attacks |= (bitboard >> 1);
    if (bitboard << 8) attacks |= (bitboard << 8);
    if ((bitboard << 9) & notAfile) attacks |= (bitboard << 9);
    if ((bitboard << 7) & notHfile) attacks |= (bitboard << 7);
    if ((bitboard << 1) & notAfile) attacks |= (bitboard << 1);
    
    
    return attacks;
}

U64 maskBishopAttacks(int square)
{
   U64 attacks = 0ULL;
   
   int r,f;
   //target ranks and files
   int tr = square / 8;
   int tf = square % 8;
   
   for(r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++,f++) attacks |= (1ULL << (r * 8 + f));
   for(r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--,f--) attacks |= (1ULL << (r * 8 + f));
   for(r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++,f--) attacks |= (1ULL << (r * 8 + f));
   for(r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--,f++) attacks |= (1ULL << (r * 8 + f));
   
   
   return attacks;
}
U64 maskRookAttacks(int square)
{
   U64 attacks = 0ULL;
   int r ,f;
   int tr = square / 8;
   int tf = square % 8;
   
   for(r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
   for(f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
   for(r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
   for(f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
   
   return attacks;
}
U64 bishopAttacksOnTheFly(int square,U64 block)
{
   U64 attacks = 0ULL;
   
   int r,f;
   //target ranks and files
   int tr = square / 8;
   int tf = square % 8;
   
   for(r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++,f++)
   {
   	attacks |= (1ULL << (r * 8 + f));
   	if((1ULL << (r * 8 + f)) & block) break;
   }
   for(r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--,f--)
   {
   	attacks |= (1ULL << (r * 8 + f));
   	if((1ULL << (r * 8 + f)) & block) break;
   }
   for(r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++,f--)
   {
   	attacks |= (1ULL << (r * 8 + f));
   	if((1ULL << (r * 8 + f)) & block) break;
   }
   for(r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--,f++)
   {
   	attacks |= (1ULL << (r * 8 + f));
   	if((1ULL << (r * 8 + f)) & block) break;
   }
   
   
   return attacks;
}
U64 rookAttacksOnTheFly(int square,U64 block)
{
   U64 attacks = 0ULL;
   int r ,f;
   int tr = square / 8;
   int tf = square % 8;
   
   for(r = tr + 1; r <= 7; r++) 
   {
   	attacks |= (1ULL << (r * 8 + tf));
   	if((1ULL << (r * 8 + tf)) & block) break;
   }
   for(f = tf + 1; f <= 7; f++)
   {
   	attacks |= (1ULL << (tr * 8 + f));
   	if((1ULL << (tr * 8 + f)) & block) break;
   }
   for(r = tr - 1; r >= 0; r--) 
   {
   	attacks |= (1ULL << (r * 8 + tf));
   	if((1ULL << (r * 8 + tf)) & block) break;
   }
   for(f = tf - 1; f >= 0; f--)
   {
   	attacks |= (1ULL << (tr * 8 + f));
   	if((1ULL << (tr * 8 + f)) &  block) break;
   }
   
   return attacks;
}
void printBitboard(U64 bitboard)
{
  for(int rank = 0; rank < 8; rank++)
  {
    printf(" %d ",8 - rank);//printing ranks 
    for(int file = 0; file < 8; file++)
    {
       
       int square = rank * 8 + file;
      //printing present state of the bitboard
      //we are using %d so we need to have ternary operator
      printf(" %d", getBitStatus(bitboard,square)? 1 : 0);
    }
    printf("\n");
  }
  printf("\n");
  printf("    a b c d e f g h\n\n");
  printf("     Bitboard: %llud\n\n", bitboard);
  
}
void initLeaperAttacks()
{
  for(int square = 0; square < 64; square ++)
  {
     pawnAttacks[white][square] = maskPawnAttacks(white,square);
     pawnAttacks[black][square] = maskPawnAttacks(black,square);
     
     knightAttacks[square] = maskKnightAttacks(square);
     
     kingAttacks[square] = maskKingAttacks(square);
  
  }
}
U64 setOccupancy(int index, int bitsInMask, U64 attackMask)
{
   U64 occupancy = 0ULL;
   
   for(int count = 0; count < bitsInMask; count++)
   {
      int square = getLsbBitIndex(attackMask);
      
      popBitStatus(attackMask,square);
      
      if(index & (1ULL << count))
        occupancy |= (1ULL << square);
   }
   
   return occupancy;
   
}
unsigned int randomState = 1804289383;

unsigned int getRandomU32Number()
{
   unsigned int number = randomState;
   
   number ^= number << 13;
   number ^= number >> 17;
   number ^= number << 5;
   
   randomState = number;
   
   return number;
}

U64 getRandomU64Number()
{
   U64 n1,n2,n3,n4;
   
   //initialise random numbers by slicing 16 bits from MS1B side
   n1 = (U64)(getRandomU32Number()) & 0xFFFF;
   n1 = (U64)(getRandomU32Number()) & 0xFFFF;
   n1 = (U64)(getRandomU32Number()) & 0xFFFF;
   n1 = (U64)(getRandomU32Number()) & 0xFFFF;
   
   return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}
U64 generateMagicNumber()
{
    return getRandomU64Number() & getRandomU64Number() & getRandomU64Number();

}


U64 findMagicNumber(int square, int relevantBits,int bishop)
{
    U64 occupancies[4096];
    U64 attacks[4096];
    U64 usedAttacks[4096];
    U64 attackMask = bishop ? maskBishopAttacks(square) : maskRookAttacks(square);
    
    int occupancyIndicies = 1 << relevantBits;
    
    for(int index = 0; index < occupancyIndicies; index ++)
    {
        occupancies[index] = setOccupancy(index, relevantBits,attackMask);
        
        attacks[index] = bishop ? bishopAttacksOnTheFly(square,occupancies[index]) : rookAttacksOnTheFly(square,occupancies[index]);
        
    }
    for(int randomCount = 0; randomCount < 100000000; randomCount++)
    {
        U64 magicNumber = generateMagicNumber();
        
        if(countBits((attackMask * magicNumber) & 0xFF00000000000000) < 6) continue;
        
        memset(usedAttacks,0ULL,sizeof(usedAttacks));
        int index,fail;
        
        for(index = 0,fail = 0; !fail && index < occupancyIndicies; index ++)
        {
           int magicIndex = (int)((occupancies[index] * magicNumber) >>(64 - relevantBits));
           
           if(usedAttacks[magicIndex] = 0ULL)
           usedAttacks[magicIndex] = attacks[index];
           
           else if(usedAttacks[magicIndex] != attacks[index])
           fail = 1;
        }
        if(!fail) return magicNumber;
    }
    
    printf("invalid magic number\n");
    
    return 0ULL;
}
/*
void initMagicNumbers()
{
  
    for (int square = 0; square < 64; square++)
        rookMagicNumbers[square] = findMagicNumber(square, rookRelevantBits[square], rook);


    for (int square = 0; square < 64; square++)
        bishopMagicNumbers[square] = findMagicNumber(square, bishopRelevantBits[square], bishop);
}
*/

void initSliderAttacks(int bishop)
{
    for(int square = 0; square < 64; square++)
    {
        bishopMasks[square] = maskBishopAttacks(square);
        rookMasks[square] = maskRookAttacks(square);
        
        U64 attackMask = bishop ? bishopMasks[square] : rookMasks[square];
        
        int relevantBitsCount = countBits(attackMask);
        
        int occupancyIndicies = (1 << relevantBitsCount);
        
        for(int index = 0; index < occupancyIndicies; index ++)
        {
           if(bishop) 
           {
              U64 occupancy = setOccupancy(index,relevantBitsCount,attackMask);
              
              int magicIndex = (occupancy * bishopMagicNumbers[square]) >> (64 - bishopRelevantBits[square]);
              
              bishopAttacks[square][magicIndex] = bishopAttacksOnTheFly(square,occupancy);
           }
           else
           {
               U64 occupancy = setOccupancy(index,relevantBitsCount,attackMask);
              
              int magicIndex = (occupancy * rookMagicNumbers[square]) >> (64 - rookRelevantBits[square]);
              
              rookAttacks[square][magicIndex] = rookAttacksOnTheFly(square,occupancy);
              
           }
        }
    }
}

static inline U64 getBishopAttacks(int square,U64 occupancy)
{
   occupancy &= bishopMasks[square];
   occupancy *= bishopMagicNumbers[square];
   occupancy >>= 64 - bishopRelevantBits[square];
   
   return bishopAttacks[square][occupancy];
}

static inline U64 getRookAttacks(int square,U64 occupancy)
{
   occupancy &= rookMasks[square];
   occupancy *= rookMagicNumbers[square];
   occupancy >>= 64 - rookRelevantBits[square];
   
   return rookAttacks[square][occupancy];
}

static inline U64 getQueenAttacks(int square,U64 occupancy)
{
   return (getBishopAttacks(square,occupancy) | getRookAttacks(square,occupancy));
}
void initAll()
{
    // init leaper pieces attacks
    initLeaperAttacks();
    
    initSliderAttacks(bishop);
    initSliderAttacks(rook);
    
    // init magic numbers
    //initMagicNumbers();
}

void printBoard()
{
  printf("\n");
  
  for(int rank = 0; rank < 8; rank++)
  {
    printf(" %d ",8 - rank);//printing ranks 
    for(int file = 0; file < 8; file++)
    {
       
       int square = rank * 8 + file;
       int piece = -1;
       
       for(int bbPiece = P; bbPiece <= k; bbPiece++)
       {
          if(getBitStatus(bitboards[bbPiece], square))
          piece = bbPiece;
       }
       
        // print different piece set depending on OS
            #ifdef WIN64
                printf(" %c", (piece == -1) ? '.' : asciiPieces[piece]);
            #else
                printf(" %s", (piece == -1) ? "." : unicodePieces[piece]);
            #endif
    }
    printf("\n");
  }
  printf("\n");
  printf("    a b c d e f g h\n\n");
  
  printf("    side:   %s\n", !side ?"white" : "black");
  
  printf("    Enpassant:   %s\n",(enpassant != noSq) ?squareToCoordinates[enpassant] : "no");
  
  printf("    Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
                                           (castle & wq) ? 'Q' : '-',
                                           (castle & bk) ? 'k' : '-',
                                           (castle & bq) ? 'q' : '-');
}


void parseFen(char *fen)
{
    memset(bitboards,0ULL,sizeof(bitboards));
    memset(occupancies,0ULL,sizeof(occupancies));
    
    side = 0;
    enpassant = noSq;
    castle = 0;
    
    for(int rank = 0; rank < 8; rank++)
    {
        for(int file = 0; file < 8; file++)
        {
           int square = rank * 8 + file;
           
           if((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
           {
              int piece = charPieces[*fen];
              setBitStatus(bitboards[piece],square);
              fen++;
           }
           if(*fen >= '0' && *fen <= '9')
           {
              int offset = *fen - '0';
              
              int piece = -1;
              for(int bbPiece = P; bbPiece <= k; bbPiece++)
              {
                  if(getBitStatus(bitboards[bbPiece],square))
                    piece = bbPiece;
              
              }
              
              if(piece == -1)
                file--;
                
              file += offset;
              
              fen++;
           }
           
           if(*fen == '/')
             fen++;
        }
        
    }
    fen++;
    
    (*fen == 'w') ? (side = white) : (side = black);
    
    fen += 2;
    
    while(*fen != ' ')
    {
        switch(*fen)
        {
            case 'K' : castle |= wk; break;
            case 'Q' : castle |= wq; break;
            case 'k' : castle |= bk; break;
            case 'q' : castle |= bq; break;
            case '-' : break;
        
        }
        fen++;
    }
    fen++;
    
    if(*fen != '-')
    {
       int file = fen[0] - 'a';
       int rank = 8 - (fen[1] - '0');
       
       enpassant = rank *8 + file;
    }
    else enpassant = noSq;
    
    for(int piece = P; piece <= K; piece++)
       occupancies[white] |= bitboards[piece];
       
    for(int piece = p; piece <= k; piece++)
       occupancies[black] |= bitboards[piece];
       
    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];
     
}

static inline int isSquareAttacked(int square,int side)
{
    if((side == white) && (pawnAttacks[black][square] & bitboards[P])) return 1;
    
    if((side == black) && (pawnAttacks[white][square] & bitboards[p])) return 1;
    
    if(knightAttacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;
    
    if(getBishopAttacks(square,occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;
    
    if(getRookAttacks(square,occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;
    
    if(getQueenAttacks(square,occupancies[both]) &((side == white) ? bitboards[Q] : bitboards[q])) return 1;
    
    if(kingAttacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;
    
    return 0;
}

void printAttackedSquares(int side)
{
   printf("\n");
  
  for(int rank = 0; rank < 8; rank++)
  {
    printf(" %d ",8 - rank);//printing ranks 
    for(int file = 0; file < 8; file++)
    {
       
       int square = rank * 8 + file;
       printf(" %d",isSquareAttacked(square,side) ? 1 : 0);
       
    }
    printf("\n");
  }
  printf("\n");
  printf("    a b c d e f g h\n\n");
  
       
      
}


/*
          binary move bits                               hexidecimal constants
    
    0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0001 0000 0000 0000 0000 0000    capture flag        0x100000
    0010 0000 0000 0000 0000 0000    double push flag    0x200000
    0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
    1000 0000 0000 0000 0000 0000    castling flag       0x800000
*/


#define encodeMove(source, target, piece, promoted, capture, double, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \
    
#define getMoveSource(move) (move & 0x3f)

#define getMoveTarget(move) ((move & 0xfc0) >> 6)

#define getMovePiece(move) ((move & 0xf000) >> 12)

#define getMovePromoted(move) ((move & 0xf0000) >> 16)

#define getMoveCapture(move) (move & 0x100000)

#define getMoveDouble(move) (move & 0x200000)

#define getMoveEnpassant(move) (move & 0x400000)

#define getMoveCastling(move) (move & 0x800000)

typedef struct {
   int moves[256];
   int count;

}moves;

static inline void addMove(moves * moveList, int move)
{
    moveList -> moves[moveList -> count] = move;
    
    moveList -> count++;
}

void printMove(int move)
{
   if(getMovePromoted(move))
   printf("%s%s%c\n", squareToCoordinates[getMoveSource(move)],
                     squareToCoordinates[getMoveTarget(move)],
                     promotedPieces[getMovePromoted(move)]);
   else
     printf("%s%s \n",squareToCoordinates[getMoveSource(move)],
                     squareToCoordinates[getMoveTarget(move)]);
}

void printMoveList(moves *moveList)
{
    if(!moveList -> count)
    {
       printf("\n No move in the move list!\n");
       return;
    }
    
        printf("\n    move    piece   capture   double    enpass    castling\n\n");
    for(int moveCount = 0; moveCount < moveList -> count; moveCount++)
    {
        int move = moveList->moves[moveCount];
       
        #ifdef WIN64
            // print move
            printf("    %s%s%c   %c       %d         %d         %d         %d\n", squareToCoordinates[getMoveSource(move)],
                                                                                  squareToCoordinates[getMoveTarget(move)],
                                                                                  promotedPieces[getMovePromoted(move)],
                                                                                  asciiPieces[getMovePiece(move)],
                                                                                  getMoveCapture(move) ? 1 : 0,
                                                                                  getMoveDouble(move) ? 1 : 0,
                                                                                  getMoveEnpassant(move) ? 1 : 0,
                                                                                  getMoveCastling(move) ? 1 : 0);
        #else
            // print move
            printf("    %s%s%c   %s       %d         %d         %d         %d\n", squareToCoordinates[getMoveSource(move)],
                                                                                  squareToCoordinates[getMoveTarget(move)],
                                                                                  promotedPieces[getMovePromoted(move)],
                                                                                  unicodePieces[getMovePiece(move)],
                                                                                  getMoveCapture(move) ? 1 : 0,
                                                                                  getMoveDouble(move) ? 1 : 0,
                                                                                  getMoveEnpassant(move) ? 1 : 0,
                                                                                  getMoveCastling(move) ? 1 : 0);
        #endif
        
        
    }
    // print total number of moves
        printf("\n\n    Total number of moves: %d\n\n", moveList->count);   


}

#define copyBoard()                                                 \
   U64 bitboardsCopy[12], occupanciesCopy[3];                       \
   int sideCopy, enpassantCopy, castleCopy;                         \
   memcpy(bitboardsCopy,bitboards,96);                              \
   memcpy(occupanciesCopy,occupancies,24);                          \
   sideCopy = side, enpassantCopy = enpassant, castleCopy = castle; \
   
#define takeBack()                                                  \
   memcpy(bitboards,bitboardsCopy,96);                              \
   memcpy(occupancies,occupanciesCopy,24);                          \
   side = sideCopy, enpassant = enpassantCopy, castle = castleCopy; \

enum
{
   allMoves,onlyCaptures
};

/*
                           castling   move     in      in
                              right update     binary  decimal

 king & rooks didn't move:     1111 & 1111  =  1111    15

        white king  moved:     1111 & 1100  =  1100    12
  white king's rook moved:     1111 & 1110  =  1110    14
 white queen's rook moved:     1111 & 1101  =  1101    13
     
         black king moved:     1111 & 0011  =  1011    3
  black king's rook moved:     1111 & 1011  =  1011    11
 black queen's rook moved:     1111 & 0111  =  0111    7

*/

// castling rights update constants
const int castlingRights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

static inline int makeMove(int move,int moveFlag)
{
   if(moveFlag == allMoves)
   {
     copyBoard();
     
     int sourceSquare = getMoveSource(move);
     int targetSquare = getMoveTarget(move);
     int piece = getMovePiece(move);
     int promotedPiece = getMovePromoted(move);
     int capture = getMoveCapture(move);
     int doublePush = getMoveDouble(move);
     int enpass = getMoveEnpassant(move);
     int castling = getMoveCastling(move);
     
     popBitStatus(bitboards[piece], sourceSquare);
     setBitStatus(bitboards[piece], targetSquare);
     
     if(capture)
     {
       int startPiece, endPiece;
       
       if(side == white)
       {
         startPiece = p;
         endPiece = k;
       }
       else
       {
         startPiece = P;
         endPiece = K;
       }
       
       for(int bbPiece = startPiece; bbPiece <= endPiece; bbPiece++)
       {
          if(getBitStatus(bitboards[bbPiece], targetSquare))
          {
            popBitStatus(bitboards[bbPiece],targetSquare);
            break;
          }
       }
       
     }
     if(promotedPiece)
     {
       popBitStatus(bitboards[(side == white) ? P : p],targetSquare);
       
       setBitStatus(bitboards[promotedPiece], targetSquare);
     }
     
     if(enpass)
     {
       (side == white) ? popBitStatus(bitboards[p], targetSquare + 8) : popBitStatus(bitboards[P], targetSquare - 8);
     }
     enpassant = noSq;
   
   
   if(doublePush)
   {
     (side == white) ? (enpassant = targetSquare + 8) : (enpassant = targetSquare - 8);
   }
   if(castling)
   {
     switch(targetSquare)
     {
       case (g1):
          popBitStatus(bitboards[R], h1);
          setBitStatus(bitboards[R], f1);
          break;
       
       case (c1):
          popBitStatus(bitboards[R], a1);
          setBitStatus(bitboards[R],d1);
          break;
          
       case (g8):
          popBitStatus(bitboards[r],h8);
          setBitStatus(bitboards[r],f8);
          break;
          
       case (c8):
          popBitStatus(bitboards[r],a8);
          setBitStatus(bitboards[r],d8);
          break;
     }
   }
   //updating castling rights 
    castle &= castlingRights[sourceSquare];
    castle &= castlingRights[targetSquare];
    
    //reset all the occupancies
    memset(occupancies,0ULL,24);
    
    for(int bbPiece = P; bbPiece <= K; bbPiece++)
       occupancies[white] |= bitboards[bbPiece];
    
    for(int bbPiece = p; bbPiece <= k; bbPiece++)
       occupancies[black] |= bitboards[bbPiece];
       
     occupancies[both] |= occupancies[white];
        occupancies[both] |= occupancies[black];
        
        //changing the side
        side ^= 1;
        
        //making sure that the king hasn't been exposed to a check 
        if(isSquareAttacked((side == white) ? getLsbBitIndex(bitboards[k]) : getLsbBitIndex(bitboards[K]),side))
        {
           takeBack();
           
           return 0;
        }
        else return 1;
   }
   else
   {
     if(getMoveCapture(move))
       makeMove(move, allMoves);
     else
       return 0;
   }
}
static inline void generateMoves(moves *moveList)
{
    moveList -> count = 0;
    int sourceSquare,targetSquare;
    U64 bitboard, attacks;
    
    for(int piece = P; piece <= k; piece++)
    {
        bitboard = bitboards[piece];
        
        if(side == white)
        {
           if(piece == P)
           {
              while(bitboard)
              {
                 sourceSquare = getLsbBitIndex(bitboard);
                 targetSquare = sourceSquare - 8;
                 
                 if(!(targetSquare < a8) && !getBitStatus(occupancies[both],targetSquare))
                 {
                    if(sourceSquare >= a7 && sourceSquare <= h7)
                    {
                       
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, Q, 0, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, R, 0, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, B, 0, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, N, 0, 0, 0, 0));
                    }
                    else 
                    {
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 0, 0, 0, 0));
                       
                       if((sourceSquare >= a2 && sourceSquare <= h2) && !getBitStatus(occupancies[both],targetSquare - 8))
                         addMove(moveList, encodeMove(sourceSquare,targetSquare - 8, piece, 0 , 0, 1, 0, 0));
                    }
                 }
                 
                 attacks = pawnAttacks[side][sourceSquare] & occupancies[black];
                 
                 while(attacks)
                 {
                    targetSquare = getLsbBitIndex(attacks);
                    if(sourceSquare >= a7 && sourceSquare <= h7)
                    {
                       
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, Q, 1, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, R, 1, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, B, 1, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, N, 1, 0, 0, 0));
                    }
                    else 
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 1, 0, 0, 0));
                       
                       popBitStatus(attacks,targetSquare);
                       
                 }
                 if(enpassant != noSq)
                 {
                    U64 enpassantAttacks = pawnAttacks[side][sourceSquare] & (1ULL << enpassant);
                     if (enpassantAttacks)
                        {
                            
                            int targetEnpassant = getLsbBitIndex(enpassantAttacks);
                            addMove(moveList, encodeMove(sourceSquare,targetEnpassant, piece, 0, 1, 0, 1, 0));
                        }
                 }
                 popBitStatus(bitboard,sourceSquare);
              }
           }
           if(piece == K)
           {
              if(castle & wk)
              {
                if(!getBitStatus(occupancies[both],f1) && !getBitStatus(occupancies[both],g1))
                {
                  if(!isSquareAttacked(e1,black) && !isSquareAttacked(f1,black))
                  addMove(moveList, encodeMove(e1,g1, piece, 0, 0, 0, 0, 1));
                }
              }
              
              if(castle & wq)
              {
                if(!getBitStatus(occupancies[both],d1) && !getBitStatus(occupancies[both],c1)  && !getBitStatus(occupancies[both],b1))
                {
                   if(!isSquareAttacked(e1,black) && !isSquareAttacked(d1,black))
                   addMove(moveList, encodeMove(e1,c1, piece, 0, 0, 0, 0, 1));
                }
                
              }
           }
        }
        else
        {
           if(piece == p)
           {
              while(bitboard)
              {
                 sourceSquare = getLsbBitIndex(bitboard);
                 targetSquare = sourceSquare + 8;
                 
                 if(!(targetSquare > h1) && !getBitStatus(occupancies[both],targetSquare))
                 {
                    if(sourceSquare >= a2 && sourceSquare <= h2)
                    {
                       
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, q, 0, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, r, 0, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, b, 0, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, n, 0, 0, 0, 0));
                    }
                    else 
                    {
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 0, 0, 0, 0));
                       
                       if((sourceSquare >= a7 && sourceSquare <= h7) && !getBitStatus(occupancies[both],targetSquare + 8))
                         addMove(moveList, encodeMove(sourceSquare,targetSquare + 8, piece, 0, 0, 1, 0, 0));
                    }
                 }
                 attacks = pawnAttacks[side][sourceSquare] & occupancies[white];
                 
                 while(attacks)
                 {
                    targetSquare = getLsbBitIndex(attacks);
                    if(sourceSquare >= a2 && sourceSquare <= h2)
                    {
                       
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, q, 1, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, r, 1, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, b, 1, 0, 0, 0));
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, n, 1, 0, 0, 0));
                    }
                    else 
                       addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 1, 0, 0, 0));
                       popBitStatus(attacks,targetSquare);
                 
                 }
                 if(enpassant != noSq)
                 {
                    U64 enpassantAttacks = pawnAttacks[side][sourceSquare] & (1ULL << enpassant);
                     if (enpassantAttacks)
                        {
                            
                            int targetEnpassant = getLsbBitIndex(enpassantAttacks);
                            addMove(moveList, encodeMove(sourceSquare,targetEnpassant, piece, 0, 1, 0, 1, 0));
                        }
                 }
                 popBitStatus(bitboard,sourceSquare);
                 
        }
    }
   if(piece == k)
           {
              if(castle & bk)
              {
                if(!getBitStatus(occupancies[both],f8) && !getBitStatus(occupancies[both],g8))
                {
                  if(!isSquareAttacked(e8,white) && !isSquareAttacked(f8,white))
                  addMove(moveList, encodeMove(e8,g8, piece, 0, 0, 0, 0, 1));
                }
              }
              
              if(castle & bq)
              {
                if(!getBitStatus(occupancies[both],d8) && !getBitStatus(occupancies[both],c8)  && !getBitStatus(occupancies[both],b8))
                {
                   if(!isSquareAttacked(e8,white) && !isSquareAttacked(d8,white))
                   addMove(moveList, encodeMove(e8,c8, piece, 0, 0, 0, 0, 1));
                }
                
              }
           }
           
           
}
         if((side == white) ? piece == N : piece == n)
         {
            while(bitboard)
            {
               sourceSquare = getLsbBitIndex(bitboard);
               
               attacks = knightAttacks[sourceSquare] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
               
               while(attacks)
               {
                  targetSquare = getLsbBitIndex(attacks);
                  
                  if(!getBitStatus((side == white) ? occupancies[black] : occupancies[white],targetSquare))
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 0, 0, 0, 0));
                    else 
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 1, 0, 0, 0));
                    
                    popBitStatus(attacks,targetSquare);
               }
               
               popBitStatus(bitboard,sourceSquare);
            }
         }
         
         
         if((side == white) ? piece == B : piece == b)
         {
            while(bitboard)
            {
               sourceSquare = getLsbBitIndex(bitboard);
               
               attacks = getBishopAttacks(sourceSquare,occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
               
               while(attacks)
               {
                  targetSquare = getLsbBitIndex(attacks);
                  
                  if(!getBitStatus((side == white) ? occupancies[black] : occupancies[white],targetSquare))
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 0, 0, 0, 0));
                    else 
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 1, 0, 0, 0));
                    
                    popBitStatus(attacks,targetSquare);
               }
               
               popBitStatus(bitboard,sourceSquare);
            }
         }
         
         if((side == white) ? piece == R : piece == r)
         {
            while(bitboard)
            {
               sourceSquare = getLsbBitIndex(bitboard);
               
               attacks = getRookAttacks(sourceSquare,occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
               
               while(attacks)
               {
                  targetSquare = getLsbBitIndex(attacks);
                  
                  if(!getBitStatus((side == white) ? occupancies[black] : occupancies[white],targetSquare))
                   addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 0, 0, 0, 0));
                    else 
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 1, 0, 0, 0));
                    popBitStatus(attacks,targetSquare);
               }
               
               popBitStatus(bitboard,sourceSquare);
            }
         }
         
         if((side == white) ? piece == Q : piece == q)
         {
            while(bitboard)
            {
               sourceSquare = getLsbBitIndex(bitboard);
               
               attacks = getQueenAttacks(sourceSquare,occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
               
               while(attacks)
               {
                  targetSquare = getLsbBitIndex(attacks);
                  
                  if(!getBitStatus((side == white) ? occupancies[black] : occupancies[white],targetSquare))
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 0, 0, 0, 0));
                    else 
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 1, 0, 0, 0));
                    
                    popBitStatus(attacks,targetSquare);
               }
               
               popBitStatus(bitboard,sourceSquare);
            }
         }
         
         if((side == white) ? piece == K : piece == k)
         {
            while(bitboard)
            {
               sourceSquare = getLsbBitIndex(bitboard);
               
               attacks = kingAttacks[sourceSquare] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
               
               while(attacks)
               {
                  targetSquare = getLsbBitIndex(attacks);
                  
                  if(!getBitStatus((side == white) ? occupancies[black] : occupancies[white],targetSquare))
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 0, 0, 0, 0));
                    else 
                    addMove(moveList, encodeMove(sourceSquare,targetSquare, piece, 0, 1, 0, 0, 0));
                    popBitStatus(attacks,targetSquare);
               }
               
               popBitStatus(bitboard,sourceSquare);
            }
         }
}
}
int getTimeTaken()
{
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval timeValue;
        gettimeofday(&timeValue, NULL);
        return timeValue.tv_sec * 1000 + timeValue.tv_usec / 1000;
    #endif
    //returning the seconds + microseconds
}
long nodes;//indicates the number of positions reached

// perft driver
static inline void perftDriver(int depth)
{
    
    if (depth == 0)
    {
        // increment nodes count (count reached positions)
        nodes++;
        return;
    }
    
    moves moveList[1];
    
    generateMoves(moveList);
    
    for (int moveCount = 0; moveCount < moveList->count; moveCount++)
    {   
        copyBoard();
        
        if (!makeMove(moveList->moves[moveCount], allMoves))
            continue;
        
        perftDriver(depth - 1);
        
        takeBack();
    }
}

// perft test
void perftTest(int depth)
{
    printf("\n     Performance test\n\n");
    
    moves moveList[1];
    
    generateMoves(moveList);
    
    long start = getTimeTaken();
    
    for (int moveCount = 0; moveCount < moveList->count; moveCount++)
    {   
        copyBoard();
        
        if (!makeMove(moveList->moves[moveCount], allMoves))
            continue;
        
        long cummulativeNodes = nodes;
      
        perftDriver(depth - 1);
        
        long oldNodes = nodes - cummulativeNodes;
        
        takeBack();
        
        printf("     move: %s%s%c  nodes: %ld\n", squareToCoordinates[getMoveSource(moveList->moves[moveCount])],
                                                 squareToCoordinates[getMoveTarget(moveList->moves[moveCount])],
                                                 getMovePromoted(moveList->moves[moveCount]) ? promotedPieces[getMovePromoted(moveList->moves[moveCount])] : ' ',
                                                 oldNodes);
    }
    
    // print results
    printf("\n    Depth: %d\n", depth);
    printf("    Nodes: %ld\n", nodes);
    printf("     Time: %ld\n\n", getTimeTaken() - start);
}

void searchPosition(int depth)
{
   printf("bestmove d2d4\n");
}
int parseMove(char *moveString)
{
    moves moveList[1];
   
    generateMoves(moveList);
 
    int sourceSquare = (moveString[0] - 'a') + (8 - (moveString[1] - '0')) * 8;
    
    int targetSquare = (moveString[2] - 'a') + (8 - (moveString[3] - '0')) * 8;
    
    for (int moveCount = 0; moveCount < moveList->count; moveCount++)
    {
     
        int move = moveList->moves[moveCount];
        
        if (sourceSquare == getMoveSource(move) && targetSquare == getMoveTarget(move))
        {
            int promotedPiece = getMovePromoted(move);
            
            if (promotedPiece)
            {
                
                if ((promotedPiece == Q || promotedPiece == q) && moveString[4] == 'q')
                    return move;
                
                else if ((promotedPiece == R || promotedPiece == r) && moveString[4] == 'r')
                    return move;
                
                else if ((promotedPiece == B || promotedPiece == b) && moveString[4] == 'b')
                    return move;
               
                else if ((promotedPiece == N || promotedPiece == n) && moveString[4] == 'n')
                    return move;
                
                
                continue;
            }
            
            // return legal move
            return move;
        }
    }
    
    // return illegal move
    return 0;
}
/*
    Example UCI commands to init position on chess board
    
    // init start position
    position startpos
    
    // init start position and make the moves on chess board
    position startpos moves e2e4 e7e5
    
    // init position from FEN string
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 
    
    // init position from fen string and make moves on chess board
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e8g8
*/
void parsePosition(char *command)
{
    command += 9;
   
    char *currentChar = command;
    
    if (strncmp(command, "startpos", 8) == 0)
        parseFen(startPosition);
    
    else
    {
        currentChar = strstr(command, "fen");
        
        if (currentChar == NULL)
            parseFen(startPosition);
           
        else
        {
            currentChar += 4;
            
            parseFen(currentChar);
        }
    }
    currentChar = strstr(command, "moves");
    
    if (currentChar != NULL)
    {
        currentChar += 6;
        
        while(*currentChar)
        {
            int move = parseMove(currentChar);
            
            if (move == 0)
                break;
            
            makeMove(move, allMoves);
            
            while (*currentChar && *currentChar != ' ') currentChar++;
           
            currentChar++;
        }
        
        
    }
    printBoard();
}
void parseGo(char *command)
{

    int depth = -1;
    
    char *currentDepth = NULL;
    
    if (currentDepth = strstr(command, "depth"))
        depth = atoi(currentDepth + 6);
    else
        depth = 6;
    
    searchPosition(depth);
}
void uciLoop()
{
   
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    char input[2000];
    
    // print engine info
    printf("id name mybbc\n");
    printf("id author Sumit Singh Bora\n");
    printf("uciok\n");
    
    // main loop
    while (1)
    {
        // reset user /GUI input
        memset(input, 0, sizeof(input));
        
        // make sure output reaches the GUI
        fflush(stdout);
        
        // get user / GUI input
        if (!fgets(input, 2000, stdin))
            // continue the loop
            continue;
        
        // make sure input is available
        if (input[0] == '\n')
            // continue the loop
            continue;
        
        // parse UCI "isready" command
        if (strncmp(input, "isready", 7) == 0)
        {
            printf("readyok\n");
            continue;
        }
        
        // parse UCI "position" command
        else if (strncmp(input, "position", 8) == 0)
            // call parse position function
            parsePosition(input);
        
        // parse UCI "ucinewgame" command
        else if (strncmp(input, "ucinewgame", 10) == 0)
            // call parse position function
            parsePosition("position startpos");
        
        // parse UCI "go" command
        else if (strncmp(input, "go", 2) == 0)
            // call parse go function
            parseGo(input);
        
        // parse UCI "quit" command
        else if (strncmp(input, "quit", 4) == 0)
            // quit from the chess engine program execution
            break;   
        
        // parse UCI "uci" command
        else if (strncmp(input, "uci", 3) == 0)
        {
            // print engine info
            printf("id name mybbc\n");
            printf("id author Sumit Singh Bora\n");
            printf("uciok\n");
        }
    }
}
int main()
{
  
  initAll();
  int debug = 0;
    
   pos
    if (debug)
    {
        // parse fen
        parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ");
        printBoard();
        //printf("score: %d\n", evaluate());
    }
    
    else
        // connect to the GUI
        uciLoop();

 
 return 0; 
  
}
