#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <sstream>

const int BOARD_SQUARES_NUMBER = 120;
const int MAX_GAME_MOVES = 2048;
const int MAX_POSITION_MOVES = 256;
const int MAX_DEPTH = 32;
const int INFINITE = 30000;
const int MATE = 29000;
const int NO_MOVE = 0;
const int MAX_PV_TABLE_ENTRIES = 65536; // 2^16

enum PIECES {
    EMPTY = 0,
    WHITE_PAWN = 1, WHITE_KNIGHT = 2, WHITE_BISHOP = 3, WHITE_ROOK = 4, WHITE_QUEEN = 5, WHITE_KING = 6,
    BLACK_PAWN = 7, BLACK_KNIGHT = 8, BLACK_BISHOP = 9, BLACK_ROOK = 10, BLACK_QUEEN = 11, BLACK_KING = 12
};

enum FILES {
    FILE_A = 0, FILE_B = 1, FILE_C = 2, FILE_D = 3,
    FILE_E = 4, FILE_F = 5, FILE_G = 6, FILE_H = 7, FILE_NONE = 8
};

enum RANKS {
    RANK_1 = 0, RANK_2 = 1, RANK_3 = 2, RANK_4 = 3,
    RANK_5 = 4, RANK_6 = 5, RANK_7 = 6, RANK_8 = 7, RANK_NONE = 8
};

enum COLOURS {
    WHITE = 0, BLACK = 1, BOTH = 2
};

enum SQUARES {
    A1 = 21, B1 = 22, C1 = 23, D1 = 24, E1 = 25, F1 = 26, G1 = 27, H1 = 28,
    A8 = 91, B8 = 92, C8 = 93, D8 = 94, E8 = 95, F8 = 96, G8 = 97, H8 = 98,
    NO_SQ = 99, OFFBOARD = 100
};

enum CASTLEBIT {
    WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8
};

const int MOVE_FLAG_EN_PASSANT = 0x40000;
const int MOVE_FLAG_PAWN_START = 0x80000;
const int MOVE_FLAG_CASTLE = 0x1000000;
const int MOVE_FLAG_CAPTURE_MASK = 0x7C000;
const int MOVE_FLAG_PROMOTION_MASK = 0xF00000;

int BoardFiles[BOARD_SQUARES_NUMBER];
int BoardRanks[BOARD_SQUARES_NUMBER];
int index120To64[BOARD_SQUARES_NUMBER];
int index64To120[64];
int PieceKeys[14 * 120];
int SideKey;
int CastleKeys[16];

const char PieceChar[] = ".PNBRQKpnbrqk";
const char SideChar[] = "wb-";
const char RankChar[] = "12345678";
const char FileChar[] = "abcdefgh";

const int PieceBig[]        = {false, false, true,  true,  true,  true,  true,  false, true,  true,  true,  true,  true};
const int PieceMaj[]        = {false, false, false, false, true,  true,  true,  false, false, false, true,  true,  true};
const int PieceMin[]        = {false, false, true,  true,  false, false, false, false, true,  true,  false, false, false};
const int PieceVal[]        = {0,     100,   325,   325,   550,   1000,  50000, 100,   325,   325,   550,   1000,  50000};
const int PieceCol[]        = {BOTH,  WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK};
const int PiecePawn[]       = {false, true,  false, false, false, false, false, true,  false, false, false, false, false};
const int PieceKnight[]     = {false, false, true,  false, false, false, false, false, true,  false, false, false, false};
const int PieceKing[]       = {false, false, false, false, false, false, true,  false, false, false, false, false, true};
const int PieceRookQueen[]  = {false, false, false, false, true,  true,  false, false, false, false, true,  true,  false};
const int PieceBishopQueen[]= {false, false, false, true,  false, true,  false, false, false, true,  false, true,  false};

const int KNIGHT_DIRECTIONS[] = {-8, -19, -21, -12, 8, 19, 21, 12};
const int ROOK_DIRECTIONS[] = {-1, -10, 1, 10};
const int BISHOP_DIRECTIONS[] = {-9, -11, 11, 9};
const int KING_DIRECTIONS[] = {-1, -10, 1, 10, -9, -11, 11, 9};

const int DirNum[] = {0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8};
const int* PieceDir[] = {nullptr, nullptr, KNIGHT_DIRECTIONS, BISHOP_DIRECTIONS, ROOK_DIRECTIONS, KING_DIRECTIONS, KING_DIRECTIONS, nullptr, KNIGHT_DIRECTIONS, BISHOP_DIRECTIONS, ROOK_DIRECTIONS, KING_DIRECTIONS, KING_DIRECTIONS};
const int LoopSlidePiece[] = {WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, 0, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, 0};
const int LoopNonSlidePiece[] = {WHITE_KNIGHT, WHITE_KING, 0, BLACK_KNIGHT, BLACK_KING, 0};
const int LoopSlideIndex[] = {0, 4};
const int LoopNonSlideIndex[] = {0, 3};
const int KINGS[] = {WHITE_KING, BLACK_KING};

const int Mirror64[] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
    8,  9,  10, 11, 12, 13, 14, 15,
    0,  1,  2,  3,  4,  5,  6,  7
};

const int CastlePerm[] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 7,  15, 15, 15, 3,  15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

int PawnRanksWhite[10];
int PawnRanksBlack[10];

const int PawnIsolated = -10;
const int PawnPassed[] = {0, 5, 10, 20, 35, 60, 100, 200};

const int PawnTable[] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    10, 10,  0,-10,-10,  0, 10, 10,
    5,  0,  0,  5,  5,  0,  0,  5,
    0,  0, 10, 20, 20, 10,  0,  0,
    5,  5,  5, 10, 10,  5,  5,  5,
    10, 10, 10, 20, 20, 10, 10, 10,
    20, 20, 20, 30, 30, 20, 20, 20,
    0,  0,  0,  0,  0,  0,  0,  0
};

const int KnightTable[] = {
    0,-10,  0,  0,  0,  0,-10,  0,
    0,  0,  0,  5,  5,  0,  0,  0,
    0,  0, 10, 10, 10, 10,  0,  0,
    0,  0, 10, 20, 20, 10,  5,  0,
    5, 10, 15, 20, 20, 15, 10,  5,
    5, 10, 10, 20, 20, 10, 10,  5,
    0,  0,  5, 10, 10,  5,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0
};

const int BishopTable[] = {
    0,  0,-10,  0,  0,-10,  0,  0,
    0,  0,  0, 10, 10,  0,  0,  0,
    0,  0, 10, 15, 15, 10,  0,  0,
    0, 10, 15, 20, 20, 15, 10,  0,
    0, 10, 15, 20, 20, 15, 10,  0,
    0,  0, 10, 15, 15, 10,  0,  0,
    0,  0,  0, 10, 10,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0
};

const int RookTable[] = {
    0,  0,  5, 10, 10,  5,  0,  0,
    0,  0,  5, 10, 10,  5,  0,  0,
    0,  0,  5, 10, 10,  5,  0,  0,
    0,  0,  5, 10, 10,  5,  0,  0,
    0,  0,  5, 10, 10,  5,  0,  0,
    0,  0,  5, 10, 10,  5,  0,  0,
    25, 25, 25, 25, 25, 25, 25, 25,
    0,  0,  5, 10, 10,  5,  0,  0
};

const int KingInEndgame[] = {
   -50,-10,  0,  0,  0,  0,-10,-50,
   -10,  0, 10, 10, 10, 10,  0,-10,
    0, 10, 20, 20, 20, 20, 10,  0,
    0, 10, 20, 40, 40, 20, 10,  0,
    0, 10, 20, 40, 40, 20, 10,  0,
    0, 10, 20, 20, 20, 20, 10,  0,
   -10,  0, 10, 10, 10, 10,  0,-10,
   -50,-10,  0,  0,  0,  0,-10,-50
};

const int KingInOpening[] = {
    0,  5,  5,-10,-10,  0, 10,  5,
   -30,-30,-30,-30,-30,-30,-30,-30,
   -50,-50,-50,-50,-50,-50,-50,-50,
   -70,-70,-70,-70,-70,-70,-70,-70,
   -70,-70,-70,-70,-70,-70,-70,-70,
   -70,-70,-70,-70,-70,-70,-70,-70,
   -70,-70,-70,-70,-70,-70,-70,-70,
   -70,-70,-70,-70,-70,-70,-70,-70
};

const int ROOK_OPEN_FILE = 10;
const int ROOK_SEMI_OPEN_FILE = 5;
const int QUEEN_OPEN_FILE = 5;
const int QUEEN_SEMI_OPEN_FILE = 3;
const int BISHOP_PAIR = 30;
const int ENDGAME_MAT = 1 * PieceVal[WHITE_ROOK] + 2 * PieceVal[WHITE_KNIGHT] + 2 * PieceVal[WHITE_PAWN] + PieceVal[WHITE_KING];

int VictimScore[] = {0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};
int MostValubleVictimLeastValuableAttackerScores[14 * 14]; // Longest variable name ever

struct Board {
    int side;
    int pieces[BOARD_SQUARES_NUMBER];
    int enPas;
    int fiftyMove;
    int ply;
    int hisPly;
    int castlePerm;
    int posKey;
    int pieceNum[13];
    int material[2];
    int pList[14 * 10];
    int fullMoveCount;
    
    int moveList[MAX_DEPTH * MAX_POSITION_MOVES];
    int moveScores[MAX_DEPTH * MAX_POSITION_MOVES];
    int moveListStart[MAX_DEPTH];
    
    int searchHistory[14 * BOARD_SQUARES_NUMBER];
    int searchKillers[3 * MAX_DEPTH];

    struct PvEntry {
        int move;
        int posKey;
    };
    PvEntry PvTable[MAX_PV_TABLE_ENTRIES];
    int PvArray[MAX_DEPTH];
    
    struct HistoryEntry {
        int move;
        int castlePerm;
        int enPas;
        int fiftyMove;
        int posKey;
        int fullMoveCount;
    };
    HistoryEntry history[MAX_GAME_MOVES];
};

struct Search {
    long long nodes;
    int fh;
    int fhf;
    int depth;
    long long time;
    long long start;
    int stop;
    int best;
    int thinking;
} search;

struct GameController {
    int EngineSide;
    int PlayerSide;
    int BoardFlipped;
    int GameOver;
    int GameSaved;
} gameController;

Board board;

inline int RAND_32() {
    return (rand() % 256 << 23) | (rand() % 256 << 16) | (rand() % 256 << 8) | (rand() % 256);
}

inline int FROMSQ(int m) { return (m & 0x7F); }
inline int TOSQ(int m) { return ((m >> 7) & 0x7F); }
inline int CAPTURED(int m) { return ((m >> 14) & 0xF); }
inline int PROMOTED(int m) { return ((m >> 20) & 0xF); }

inline int PCEINDEX(int piece, int pieceNum) {
    return (piece * 10 + pieceNum);
}

inline int FR2SQ(int f, int r) {
    return ((21 + (f)) + ((r) * 10));
}

inline int SQ64(int sq120) {
    return index120To64[sq120];
}

inline int SQ120(int sq64) {
    return index64To120[sq64];
}

inline int MIRROR64(int sq) {
    return Mirror64[sq];
}

inline int SQOFFBOARD(int sq) {
    if (BoardFiles[sq] == OFFBOARD) return true;
    return false;
}

inline void HASH_PCE(int piece, int sq) {
    board.posKey ^= PieceKeys[(piece * 120) + sq];
}

inline void HASH_CA() { 
    board.posKey ^= CastleKeys[board.castlePerm]; 
}

inline void HASH_SIDE() { 
    board.posKey ^= SideKey; 
}

inline void HASH_EP() { 
    board.posKey ^= PieceKeys[board.enPas]; 
}

int SqFromAlg(const std::string& moveAlg) {
    if (moveAlg.length() != 2) return NO_SQ;
    if (moveAlg[0] > 'h' || moveAlg[0] < 'a') return NO_SQ;
    if (moveAlg[1] > '8' || moveAlg[1] < '1') return NO_SQ;
    
    int file = moveAlg[0] - 'a';
    int rank = moveAlg[1] - '1';
    
    return FR2SQ(file, rank);
}

std::string PrMove(int move) {
    std::string MvStr;
    
    int ff = BoardFiles[FROMSQ(move)];
    int rf = BoardRanks[FROMSQ(move)];
    int ft = BoardFiles[TOSQ(move)];
    int rt = BoardRanks[TOSQ(move)];
    
    MvStr += ('a' + ff);
    MvStr += ('1' + rf);
    MvStr += ('a' + ft);
    MvStr += ('1' + rt);
    
    int promoted = PROMOTED(move);
    
    if (promoted != EMPTY) {
        char pchar = 'q';
        if (PieceKnight[promoted] == true) {
            pchar = 'n';
        } else if (PieceRookQueen[promoted] == true && PieceBishopQueen[promoted] == false) {
            pchar = 'r';
        } else if (PieceRookQueen[promoted] == false && PieceBishopQueen[promoted] == true) {
            pchar = 'b';
        }
        MvStr += pchar;
    }
    return MvStr;
}

void InitFilesRanksBrd() {
    int file = FILE_A;
    int rank = RANK_1;
    int sq = A1;
    
    for (int index = 0; index < BOARD_SQUARES_NUMBER; ++index) {
        BoardFiles[index] = OFFBOARD;
        BoardRanks[index] = OFFBOARD;
    }
    
    for (rank = RANK_1; rank <= RANK_8; ++rank) {
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file, rank);
            BoardFiles[sq] = file;
            BoardRanks[sq] = rank;
        }
    }
}

void InitSq120To64() {
    int file = FILE_A;
    int rank = RANK_1;
    int sq = A1;
    int sq64 = 0;
    
    for (int index = 0; index < BOARD_SQUARES_NUMBER; ++index) {
        index120To64[index] = 65;
    }
    
    for (int index = 0; index < 64; ++index) {
        index64To120[index] = 120;
    }
    
    for (rank = RANK_1; rank <= RANK_8; ++rank) {
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file, rank);
            index64To120[sq64] = sq;
            index120To64[sq] = sq64;
            sq64++;
        }
    }
}

void InitHashKeys() {
    for (int index = 0; index < 13 * 120; ++index) {
        PieceKeys[index] = RAND_32();
    }
    
    SideKey = RAND_32();
    
    for (int index = 0; index < 16; ++index) {
        CastleKeys[index] = RAND_32();
    }
}

void InitBoardVars() {
    for (int index = 0; index < MAX_GAME_MOVES; index++) {
        board.history[index] = {
            NO_MOVE, 0, 0, 0, 0, 0
        };
    }
    
    for (int index = 0; index < MAX_PV_TABLE_ENTRIES; index++) {
        board.PvTable[index] = {
            NO_MOVE, 0
        };
    }
}

void InitMvvLva() {
    for (int Attacker = WHITE_PAWN; Attacker <= BLACK_KING; ++Attacker) {
        for (int Victim = WHITE_PAWN; Victim <= BLACK_KING; ++Victim) {
            MostValubleVictimLeastValuableAttackerScores[Victim * 14 + Attacker] = VictimScore[Victim] + 6 - (VictimScore[Attacker] / 100);
        }
    }
}

void EvalInit() {
    for (int index = 0; index < 10; ++index) {
        PawnRanksWhite[index] = 0;
        PawnRanksBlack[index] = 0;
    }
}

void init() {
    InitFilesRanksBrd();
    InitSq120To64();
    InitHashKeys();
    InitBoardVars();
    InitMvvLva();
    EvalInit();
    search.thinking = false;
}

void UpdateListsMaterial() {
    for (int index = 0; index < BOARD_SQUARES_NUMBER; ++index) {
        int sq = index;
        int piece = board.pieces[index];
        if (piece != OFFBOARD && piece != EMPTY) {
            int colour = PieceCol[piece];
            
            board.material[colour] += PieceVal[piece];
            
            board.pList[PCEINDEX(piece, board.pieceNum[piece])] = sq;
            board.pieceNum[piece]++;
        }
    }
}

void ResetBoard() {
    for (int index = 0; index < BOARD_SQUARES_NUMBER; ++index) {
        board.pieces[index] = OFFBOARD;
    }
    
    for (int index = 0; index < 64; ++index) {
        board.pieces[SQ120(index)] = EMPTY;
    }
    
    for (int index = 0; index < 14 * 10; ++index) {
        board.pList[index] = EMPTY;
    }
    
    for (int index = 0; index < 2; ++index) {
        board.material[index] = 0;
    }
    
    for (int index = 0; index < 13; ++index) {
        board.pieceNum[index] = 0;
    }
    
    board.side = BOTH;
    board.enPas = NO_SQ;
    board.fiftyMove = 0;
    board.ply = 0;
    board.hisPly = 0;
    board.castlePerm = 0;
    board.posKey = 0;
    board.moveListStart[board.ply] = 0;
}

int GeneratePosKey() {
    int finalKey = 0;
    
    for (int sq = 0; sq < BOARD_SQUARES_NUMBER; ++sq) {
        int piece = board.pieces[sq];
        if (piece != EMPTY && piece != OFFBOARD) {
            finalKey ^= PieceKeys[(piece * 120) + sq];
        }
    }
    
    if (board.side == WHITE) {
        finalKey ^= SideKey;
    }
    
    if (board.enPas != NO_SQ) {
        finalKey ^= PieceKeys[board.enPas];
    }
    
    finalKey ^= CastleKeys[board.castlePerm];
    
    return finalKey;
}

void ParseFen(const std::string& fen) {
    int rank = RANK_8;
    int file = FILE_A;
    int piece = 0;
    int count = 0;
    int i = 0;
    int sq64 = 0;
    int sq120 = 0;
    int fenCnt = 0;
    
    ResetBoard();
    
    while ((rank >= RANK_1) && fenCnt < fen.length()) {
        count = 1;
        switch (fen[fenCnt]) {
            case 'p': piece = BLACK_PAWN; break;
            case 'r': piece = BLACK_ROOK; break;
            case 'n': piece = BLACK_KNIGHT; break;
            case 'b': piece = BLACK_BISHOP; break;
            case 'k': piece = BLACK_KING; break;
            case 'q': piece = BLACK_QUEEN; break;
            case 'P': piece = WHITE_PAWN; break;
            case 'R': piece = WHITE_ROOK; break;
            case 'N': piece = WHITE_KNIGHT; break;
            case 'B': piece = WHITE_BISHOP; break;
            case 'K': piece = WHITE_KING; break;
            case 'Q': piece = WHITE_QUEEN; break;
            case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
                piece = EMPTY;
                count = fen[fenCnt] - '0';
                break;
            case '/': case ' ':
                rank--;
                file = FILE_A;
                fenCnt++;
                continue;
            default:
                std::cerr << "Error: Invalid character in FEN string: " << fen[fenCnt] << std::endl;
                return;
        }
        
        for (i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
            sq120 = SQ120(sq64);
            if (piece != EMPTY) {
                board.pieces[sq120] = piece;
            }
            file++;
        }
        fenCnt++;
    }
    
    board.side = (fen[fenCnt] == 'w') ? WHITE : BLACK;
    fenCnt += 2;
    
    for (i = 0; i < 4; i++) {
        if (fen[fenCnt] == ' ') {
            break;
        }
        switch (fen[fenCnt]) {
            case 'K': board.castlePerm |= WKCA; break;
            case 'Q': board.castlePerm |= WQCA; break;
            case 'k': board.castlePerm |= BKCA; break;
            case 'q': board.castlePerm |= BQCA; break;
            default: break;
        }
        fenCnt++;
    }
    fenCnt++;
    
    if (fen[fenCnt] != '-') {
        file = fen[fenCnt] - 'a';
        rank = fen[fenCnt + 1] - '1';
        board.enPas = FR2SQ(file, rank);
    }
    
    // Parse full move count
    size_t lastSpace = fen.find_last_of(' ');
    if (lastSpace != std::string::npos) {
        board.fullMoveCount = std::stoi(fen.substr(lastSpace + 1));
    }
    
    board.posKey = GeneratePosKey();
    UpdateListsMaterial();
}

int SqAttacked(int sq, int side) {
    int piece;
    int t_sq;
    int index;
    
    if (side == WHITE) {
        if (board.pieces[sq - 11] == WHITE_PAWN || board.pieces[sq - 9] == WHITE_PAWN) {
            return true;
        }
    } else {
        if (board.pieces[sq + 11] == BLACK_PAWN || board.pieces[sq + 9] == BLACK_PAWN) {
            return true;
        }
    }
    
    for (index = 0; index < 8; ++index) {
        piece = board.pieces[sq + KNIGHT_DIRECTIONS[index]];
        if (piece != OFFBOARD && PieceKnight[piece] == true && PieceCol[piece] == side) {
            return true;
        }
    }
    
    for (index = 0; index < 4; ++index) {
        int dir = ROOK_DIRECTIONS[index];
        t_sq = sq + dir;
        piece = board.pieces[t_sq];
        while (piece != OFFBOARD) {
            if (piece != EMPTY) {
                if (PieceRookQueen[piece] == true && PieceCol[piece] == side) {
                    return true;
                }
                break;
            }
            t_sq += dir;
            piece = board.pieces[t_sq];
        }
    }
    
    for (index = 0; index < 4; ++index) {
        int dir = BISHOP_DIRECTIONS[index];
        t_sq = sq + dir;
        piece = board.pieces[t_sq];
        while (piece != OFFBOARD) {
            if (piece != EMPTY) {
                if (PieceBishopQueen[piece] == true && PieceCol[piece] == side) {
                    return true;
                }
                break;
            }
            t_sq += dir;
            piece = board.pieces[t_sq];
        }
    }
    
    for (index = 0; index < 8; ++index) {
        piece = board.pieces[sq + KING_DIRECTIONS[index]];
        if (piece != OFFBOARD && PieceKing[piece] == true && PieceCol[piece] == side) {
            return true;
        }
    }
    
    return false;
}

inline int MOVE(int from, int to, int captured, int promoted, int flag) {
    return (from | (to << 7) | (captured << 14) | (promoted << 20) | flag);
}

void AddCaptureMove(int move) {
    board.moveList[board.moveListStart[board.ply + 1]] = move;
    board.moveScores[board.moveListStart[board.ply + 1]++] = MostValubleVictimLeastValuableAttackerScores[CAPTURED(move) * 14 + board.pieces[FROMSQ(move)]] + 1000000;
}

void AddQuietMove(int move) {
    board.moveList[board.moveListStart[board.ply + 1]] = move;
    
    if (board.searchKillers[board.ply] == move) {
        board.moveScores[board.moveListStart[board.ply + 1]] = 900000;
    } else if (board.searchKillers[MAX_DEPTH + board.ply] == move) {
        board.moveScores[board.moveListStart[board.ply + 1]] = 800000;
    } else {
        board.moveScores[board.moveListStart[board.ply + 1]] = board.searchHistory[board.pieces[FROMSQ(move)] * BOARD_SQUARES_NUMBER + TOSQ(move)];
    }
    board.moveListStart[board.ply + 1]++;
}

void AddEnPassantMove(int move) {
    board.moveList[board.moveListStart[board.ply + 1]] = move;
    board.moveScores[board.moveListStart[board.ply + 1]++] = 105 + 1000000;
}

void AddWhitePawnCaptureMove(int from, int to, int cap) {
    if (BoardRanks[from] == RANK_7) {
        AddCaptureMove(MOVE(from, to, cap, WHITE_QUEEN, 0));
        AddCaptureMove(MOVE(from, to, cap, WHITE_ROOK, 0));
        AddCaptureMove(MOVE(from, to, cap, WHITE_BISHOP, 0));
        AddCaptureMove(MOVE(from, to, cap, WHITE_KNIGHT, 0));
    } else {
        AddCaptureMove(MOVE(from, to, cap, EMPTY, 0));
    }
}

void AddWhitePawnQuietMove(int from, int to) {
    if (BoardRanks[from] == RANK_7) {
        AddQuietMove(MOVE(from, to, EMPTY, WHITE_QUEEN, 0));
        AddQuietMove(MOVE(from, to, EMPTY, WHITE_ROOK, 0));
        AddQuietMove(MOVE(from, to, EMPTY, WHITE_BISHOP, 0));
        AddQuietMove(MOVE(from, to, EMPTY, WHITE_KNIGHT, 0));
    } else {
        AddQuietMove(MOVE(from, to, EMPTY, EMPTY, 0));
    }
}

void AddBlackPawnCaptureMove(int from, int to, int cap) {
    if (BoardRanks[from] == RANK_2) {
        AddCaptureMove(MOVE(from, to, cap, BLACK_QUEEN, 0));
        AddCaptureMove(MOVE(from, to, cap, BLACK_ROOK, 0));
        AddCaptureMove(MOVE(from, to, cap, BLACK_BISHOP, 0));
        AddCaptureMove(MOVE(from, to, cap, BLACK_KNIGHT, 0));
    } else {
        AddCaptureMove(MOVE(from, to, cap, EMPTY, 0));
    }
}

void AddBlackPawnQuietMove(int from, int to) {
    if (BoardRanks[from] == RANK_2) {
        AddQuietMove(MOVE(from, to, EMPTY, BLACK_QUEEN, 0));
        AddQuietMove(MOVE(from, to, EMPTY, BLACK_ROOK, 0));
        AddQuietMove(MOVE(from, to, EMPTY, BLACK_BISHOP, 0));
        AddQuietMove(MOVE(from, to, EMPTY, BLACK_KNIGHT, 0));
    } else {
        AddQuietMove(MOVE(from, to, EMPTY, EMPTY, 0));
    }
}

void ClearPiece(int sq) {
    int piece = board.pieces[sq];
    int col = PieceCol[piece];
    int index = 0;
    int t_pieceNum = -1;
    
    HASH_PCE(piece, sq);
    
    board.pieces[sq] = EMPTY;
    board.material[col] -= PieceVal[piece];
    
    for (index = 0; index < board.pieceNum[piece]; ++index) {
        if (board.pList[PCEINDEX(piece, index)] == sq) {
            t_pieceNum = index;
            break;
        }
    }
    
    board.pieceNum[piece]--;
    board.pList[PCEINDEX(piece, t_pieceNum)] = board.pList[PCEINDEX(piece, board.pieceNum[piece])];
}

void AddPiece(int sq, int piece) {
    int col = PieceCol[piece];
    
    HASH_PCE(piece, sq);
    
    board.pieces[sq] = piece;
    board.material[col] += PieceVal[piece];
    board.pList[PCEINDEX(piece, board.pieceNum[piece])] = sq;
    board.pieceNum[piece]++;
}

void MovePiece(int from, int to) {
    int index = 0;
    int piece = board.pieces[from];
    int col = PieceCol[piece];
    
    HASH_PCE(piece, from);
    board.pieces[from] = EMPTY;
    
    HASH_PCE(piece, to);
    board.pieces[to] = piece;
    
    for (index = 0; index < board.pieceNum[piece]; ++index) {
        if (board.pList[PCEINDEX(piece, index)] == from) {
            board.pList[PCEINDEX(piece, index)] = to;
            break;
        }
    }
}

void GenerateMoves() {
    board.moveListStart[board.ply + 1] = board.moveListStart[board.ply];
    int pieceType;
    int pieceNum;
    int pieceIndex;
    int piece;
    int sq;
    int tsq;
    int index;
    int dir;
    
    if (board.side == WHITE) {
        pieceType = WHITE_PAWN;
        for (pieceNum = 0; pieceNum < board.pieceNum[pieceType]; ++pieceNum) {
            sq = board.pList[PCEINDEX(pieceType, pieceNum)];
            if (board.pieces[sq + 10] == EMPTY) {
                AddWhitePawnQuietMove(sq, sq + 10);
                if (BoardRanks[sq] == RANK_2 && board.pieces[sq + 20] == EMPTY) {
                    AddQuietMove(MOVE(sq, (sq + 20), EMPTY, EMPTY, MOVE_FLAG_PAWN_START));
                }
            }
            
            if (SQOFFBOARD(sq + 9) == false && PieceCol[board.pieces[sq + 9]] == BLACK) {
                AddWhitePawnCaptureMove(sq, sq + 9, board.pieces[sq + 9]);
            }
            if (SQOFFBOARD(sq + 11) == false && PieceCol[board.pieces[sq + 11]] == BLACK) {
                AddWhitePawnCaptureMove(sq, sq + 11, board.pieces[sq + 11]);
            }
            
            if (board.enPas != NO_SQ) {
                if (sq + 9 == board.enPas) {
                    AddEnPassantMove(MOVE(sq, sq + 9, EMPTY, EMPTY, MOVE_FLAG_EN_PASSANT));
                }
                if (sq + 11 == board.enPas) {
                    AddEnPassantMove(MOVE(sq, sq + 11, EMPTY, EMPTY, MOVE_FLAG_EN_PASSANT));
                }
            }
        }
        
        if (board.castlePerm & WKCA) {
            if (board.pieces[F1] == EMPTY && board.pieces[G1] == EMPTY) {
                if (SqAttacked(E1, BLACK) == false && SqAttacked(F1, BLACK) == false) {
                    AddQuietMove(MOVE(E1, G1, EMPTY, EMPTY, MOVE_FLAG_CASTLE));
                }
            }
        }
        
        if (board.castlePerm & WQCA) {
            if (board.pieces[D1] == EMPTY && board.pieces[C1] == EMPTY && board.pieces[B1] == EMPTY) {
                if (SqAttacked(E1, BLACK) == false && SqAttacked(D1, BLACK) == false) {
                    AddQuietMove(MOVE(E1, C1, EMPTY, EMPTY, MOVE_FLAG_CASTLE));
                }
            }
        }
        
        pieceType = WHITE_KNIGHT;
        
    } else {
        pieceType = BLACK_PAWN;
        for (pieceNum = 0; pieceNum < board.pieceNum[pieceType]; ++pieceNum) {
            sq = board.pList[PCEINDEX(pieceType, pieceNum)];
            
            if (board.pieces[sq - 10] == EMPTY) {
                AddBlackPawnQuietMove(sq, sq - 10);
                if (BoardRanks[sq] == RANK_7 && board.pieces[sq - 20] == EMPTY) {
                    AddQuietMove(MOVE(sq, (sq - 20), EMPTY, EMPTY, MOVE_FLAG_PAWN_START));
                }
            }
            
            if (SQOFFBOARD(sq - 9) == false && PieceCol[board.pieces[sq - 9]] == WHITE) {
                AddBlackPawnCaptureMove(sq, sq - 9, board.pieces[sq - 9]);
            }
            
            if (SQOFFBOARD(sq - 11) == false && PieceCol[board.pieces[sq - 11]] == WHITE) {
                AddBlackPawnCaptureMove(sq, sq - 11, board.pieces[sq - 11]);
            }
            
            if (board.enPas != NO_SQ) {
                if (sq - 9 == board.enPas) {
                    AddEnPassantMove(MOVE(sq, sq - 9, EMPTY, EMPTY, MOVE_FLAG_EN_PASSANT));
                }
                if (sq - 11 == board.enPas) {
                    AddEnPassantMove(MOVE(sq, sq - 11, EMPTY, EMPTY, MOVE_FLAG_EN_PASSANT));
                }
            }
        }
        
        if (board.castlePerm & BKCA) {
            if (board.pieces[F8] == EMPTY && board.pieces[G8] == EMPTY) {
                if (SqAttacked(E8, WHITE) == false && SqAttacked(F8, WHITE) == false) {
                    AddQuietMove(MOVE(E8, G8, EMPTY, EMPTY, MOVE_FLAG_CASTLE));
                }
            }
        }
        
        if (board.castlePerm & BQCA) {
            if (board.pieces[D8] == EMPTY && board.pieces[C8] == EMPTY && board.pieces[B8] == EMPTY) {
                if (SqAttacked(E8, WHITE) == false && SqAttacked(D8, WHITE) == false) {
                    AddQuietMove(MOVE(E8, C8, EMPTY, EMPTY, MOVE_FLAG_CASTLE));
                }
            }
        }
        
        pieceType = BLACK_KNIGHT;
    }
    
    pieceIndex = LoopSlideIndex[board.side];
    piece = LoopSlidePiece[pieceIndex++];
    while (piece != 0) {
        for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
            sq = board.pList[PCEINDEX(piece, pieceNum)];
            
            for (index = 0; index < DirNum[piece]; ++index) {
                dir = PieceDir[piece][index];
                tsq = sq + dir;
                
                while (SQOFFBOARD(tsq) == false) {
                    if (board.pieces[tsq] != EMPTY) {
                        if (PieceCol[board.pieces[tsq]] == board.side ^ 1) {
                            AddCaptureMove(MOVE(sq, tsq, board.pieces[tsq], EMPTY, 0));
                        }
                        break;
                    }
                    AddQuietMove(MOVE(sq, tsq, EMPTY, EMPTY, 0));
                    tsq += dir;
                }
            }
        }
        piece = LoopSlidePiece[pieceIndex++];
    }
    
    pieceIndex = LoopNonSlideIndex[board.side];
    piece = LoopNonSlidePiece[pieceIndex++];
    
    while (piece != 0) {
        for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
            sq = board.pList[PCEINDEX(piece, pieceNum)];
            
            for (index = 0; index < DirNum[piece]; ++index) {
                dir = PieceDir[piece][index];
                tsq = sq + dir;
                
                if (SQOFFBOARD(tsq) == true) {
                    continue;
                }
                
                if (board.pieces[tsq] != EMPTY) {
                    if (PieceCol[board.pieces[tsq]] == board.side ^ 1) {
                        AddCaptureMove(MOVE(sq, tsq, board.pieces[tsq], EMPTY, 0));
                    }
                    continue;
                }
                AddQuietMove(MOVE(sq, tsq, EMPTY, EMPTY, 0));
            }
        }
        piece = LoopNonSlidePiece[pieceIndex++];
    }
}

void GenerateCaptures() {
    board.moveListStart[board.ply + 1] = board.moveListStart[board.ply];
    int pieceType;
    int pieceNum;
    int pieceIndex;
    int piece;
    int sq;
    int tsq;
    int index;
    int dir;
    
    if (board.side == WHITE) {
        pieceType = WHITE_PAWN;
        for (pieceNum = 0; pieceNum < board.pieceNum[pieceType]; ++pieceNum) {
            sq = board.pList[PCEINDEX(pieceType, pieceNum)];
            
            if (SQOFFBOARD(sq + 9) == false && PieceCol[board.pieces[sq + 9]] == BLACK) {
                AddWhitePawnCaptureMove(sq, sq + 9, board.pieces[sq + 9]);
            }
            if (SQOFFBOARD(sq + 11) == false && PieceCol[board.pieces[sq + 11]] == BLACK) {
                AddWhitePawnCaptureMove(sq, sq + 11, board.pieces[sq + 11]);
            }
            
            if (board.enPas != NO_SQ) {
                if (sq + 9 == board.enPas) {
                    AddEnPassantMove(MOVE(sq, sq + 9, EMPTY, EMPTY, MOVE_FLAG_EN_PASSANT));
                }
                if (sq + 11 == board.enPas) {
                    AddEnPassantMove(MOVE(sq, sq + 11, EMPTY, EMPTY, MOVE_FLAG_EN_PASSANT));
                }
            }
        }
        
        pieceType = WHITE_KNIGHT;
        
    } else {
        pieceType = BLACK_PAWN;
        for (pieceNum = 0; pieceNum < board.pieceNum[pieceType]; ++pieceNum) {
            sq = board.pList[PCEINDEX(pieceType, pieceNum)];
            
            if (SQOFFBOARD(sq - 9) == false && PieceCol[board.pieces[sq - 9]] == WHITE) {
                AddBlackPawnCaptureMove(sq, sq - 9, board.pieces[sq - 9]);
            }
            
            if (SQOFFBOARD(sq - 11) == false && PieceCol[board.pieces[sq - 11]] == WHITE) {
                AddBlackPawnCaptureMove(sq, sq - 11, board.pieces[sq - 11]);
            }
            
            if (board.enPas != NO_SQ) {
                if (sq - 9 == board.enPas) {
                    AddEnPassantMove(MOVE(sq, sq - 9, EMPTY, EMPTY, MOVE_FLAG_EN_PASSANT));
                }
                if (sq - 11 == board.enPas) {
                    AddEnPassantMove(MOVE(sq, sq - 11, EMPTY, EMPTY, MOVE_FLAG_EN_PASSANT));
                }
            }
        }
        
        pieceType = BLACK_KNIGHT;
    }
    
    pieceIndex = LoopSlideIndex[board.side];
    piece = LoopSlidePiece[pieceIndex++];
    while (piece != 0) {
        for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
            sq = board.pList[PCEINDEX(piece, pieceNum)];
            
            for (index = 0; index < DirNum[piece]; ++index) {
                dir = PieceDir[piece][index];
                tsq = sq + dir;
                
                while (SQOFFBOARD(tsq) == false) {
                    if (board.pieces[tsq] != EMPTY) {
                        if (PieceCol[board.pieces[tsq]] == board.side ^ 1) {
                            AddCaptureMove(MOVE(sq, tsq, board.pieces[tsq], EMPTY, 0));
                        }
                        break;
                    }
                    tsq += dir;
                }
            }
        }
        piece = LoopSlidePiece[pieceIndex++];
    }
    
    pieceIndex = LoopNonSlideIndex[board.side];
    piece = LoopNonSlidePiece[pieceIndex++];
    
    while (piece != 0) {
        for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
            sq = board.pList[PCEINDEX(piece, pieceNum)];
            
            for (index = 0; index < DirNum[piece]; ++index) {
                dir = PieceDir[piece][index];
                tsq = sq + dir;
                
                if (SQOFFBOARD(tsq) == true) {
                    continue;
                }
                
                if (board.pieces[tsq] != EMPTY) {
                    if (PieceCol[board.pieces[tsq]] == board.side ^ 1) {
                        AddCaptureMove(MOVE(sq, tsq, board.pieces[tsq], EMPTY, 0));
                    }
                    continue;
                }
            }
        }
        piece = LoopNonSlidePiece[pieceIndex++];
    }
}

void TakeMove() {
    board.hisPly--;
    board.ply--;
    
    int move = board.history[board.hisPly].move;
    int from = FROMSQ(move);
    int to = TOSQ(move);
    
    if (board.enPas != NO_SQ) HASH_EP();
    HASH_CA();
    
    board.castlePerm = board.history[board.hisPly].castlePerm;
    board.fiftyMove = board.history[board.hisPly].fiftyMove;
    board.enPas = board.history[board.hisPly].enPas;
    board.fullMoveCount = board.history[board.hisPly].fullMoveCount;
    
    if (board.enPas != NO_SQ) HASH_EP();
    HASH_CA();
    
    board.side ^= 1;
    HASH_SIDE();
    
    if ((MOVE_FLAG_EN_PASSANT & move) != 0) {
        if (board.side == WHITE) {
            AddPiece(to - 10, BLACK_PAWN);
        } else {
            AddPiece(to + 10, WHITE_PAWN);
        }
    } else if ((MOVE_FLAG_CASTLE & move) != 0) {
        switch (to) {
            case C1: MovePiece(D1, A1); break;
            case C8: MovePiece(D8, A8); break;
            case G1: MovePiece(F1, H1); break;
            case G8: MovePiece(F8, H8); break;
            default: break;
        }
    }
    
    MovePiece(to, from);
    
    int captured = CAPTURED(move);
    if (captured != EMPTY) {
        AddPiece(to, captured);
    }
    
    if (PROMOTED(move) != EMPTY) {
        ClearPiece(from);
        AddPiece(from, (PieceCol[PROMOTED(move)] == WHITE ? WHITE_PAWN : BLACK_PAWN));
    }
}

int MakeMove(int move) {
    int from = FROMSQ(move);
    int to = TOSQ(move);
    int side = board.side;
    
    board.history[board.hisPly].posKey = board.posKey;
    
    if ((move & MOVE_FLAG_EN_PASSANT) != 0) {
        if (side == WHITE) {
            ClearPiece(to - 10);
        } else {
            ClearPiece(to + 10);
        }
    } else if ((move & MOVE_FLAG_CASTLE) != 0) {
        switch (to) {
            case C1:
                MovePiece(A1, D1);
                break;
            case C8:
                MovePiece(A8, D8);
                break;
            case G1:
                MovePiece(H1, F1);
                break;
            case G8:
                MovePiece(H8, F8);
                break;
            default: break;
        }
    }
    
    if (board.enPas != NO_SQ) HASH_EP();
    HASH_CA();
    
    board.history[board.hisPly].move = move;
    board.history[board.hisPly].fiftyMove = board.fiftyMove;
    board.history[board.hisPly].fullMoveCount = board.fullMoveCount;
    board.history[board.hisPly].enPas = board.enPas;
    board.history[board.hisPly].castlePerm = board.castlePerm;
    
    board.castlePerm &= CastlePerm[from];
    board.castlePerm &= CastlePerm[to];
    board.enPas = NO_SQ;
    
    HASH_CA();
    
    int captured = CAPTURED(move);
    board.fiftyMove++;
    board.fullMoveCount++;
    
    if (captured != EMPTY) {
        ClearPiece(to);
        board.fiftyMove = 0;
    }
    
    board.hisPly++;
    board.ply++;
    
    if (PiecePawn[board.pieces[from]] == true) {
        board.fiftyMove = 0;
        if ((move & MOVE_FLAG_PAWN_START) != 0) {
            if (side == WHITE) {
                board.enPas = from + 10;
            } else {
                board.enPas = from - 10;
            }
            HASH_EP();
        }
    }
    
    MovePiece(from, to);
    
    int prPiece = PROMOTED(move);
    if (prPiece != EMPTY) {
        ClearPiece(to);
        AddPiece(to, prPiece);
    }
    
    board.side ^= 1;
    HASH_SIDE();
    
    if (SqAttacked(board.pList[PCEINDEX(KINGS[side], 0)], board.side)) {
        TakeMove();
        return false;
    }
    
    return true;
}

int MaterialDraw() {
    if (0 == board.pieceNum[WHITE_ROOK] && 0 == board.pieceNum[BLACK_ROOK] && 0 == board.pieceNum[WHITE_QUEEN] && 0 == board.pieceNum[BLACK_QUEEN]) {
        if (0 == board.pieceNum[BLACK_BISHOP] && 0 == board.pieceNum[WHITE_BISHOP]) {
            if (board.pieceNum[WHITE_KNIGHT] < 3 && board.pieceNum[BLACK_KNIGHT] < 3) { return true; }
        } else if (0 == board.pieceNum[WHITE_KNIGHT] && 0 == board.pieceNum[BLACK_KNIGHT]) {
            if (abs(board.pieceNum[WHITE_BISHOP] - board.pieceNum[BLACK_BISHOP]) < 2) { return true; }
        } else if ((board.pieceNum[WHITE_KNIGHT] < 3 && 0 == board.pieceNum[WHITE_BISHOP]) || (board.pieceNum[WHITE_BISHOP] == 1 && 0 == board.pieceNum[WHITE_KNIGHT])) {
            if ((board.pieceNum[BLACK_KNIGHT] < 3 && 0 == board.pieceNum[BLACK_BISHOP]) || (board.pieceNum[BLACK_BISHOP] == 1 && 0 == board.pieceNum[BLACK_KNIGHT])) { return true; }
        }
    } else if (0 == board.pieceNum[WHITE_QUEEN] && 0 == board.pieceNum[BLACK_QUEEN]) {
        if (board.pieceNum[WHITE_ROOK] == 1 && board.pieceNum[BLACK_ROOK] == 1) {
            if ((board.pieceNum[WHITE_KNIGHT] + board.pieceNum[WHITE_BISHOP]) < 2 && (board.pieceNum[BLACK_KNIGHT] + board.pieceNum[BLACK_BISHOP]) < 2) { return true; }
        } else if (board.pieceNum[WHITE_ROOK] == 1 && 0 == board.pieceNum[BLACK_ROOK]) {
            if ((board.pieceNum[WHITE_KNIGHT] + board.pieceNum[WHITE_BISHOP] == 0) && (((board.pieceNum[BLACK_KNIGHT] + board.pieceNum[BLACK_BISHOP]) == 1) || ((board.pieceNum[BLACK_KNIGHT] + board.pieceNum[BLACK_BISHOP]) == 2))) { return true; }
        } else if (board.pieceNum[BLACK_ROOK] == 1 && 0 == board.pieceNum[WHITE_ROOK]) {
            if ((board.pieceNum[BLACK_KNIGHT] + board.pieceNum[BLACK_BISHOP] == 0) && (((board.pieceNum[WHITE_KNIGHT] + board.pieceNum[WHITE_BISHOP]) == 1) || ((board.pieceNum[WHITE_KNIGHT] + board.pieceNum[WHITE_BISHOP]) == 2))) { return true; }
        }
    }
    return false;
}

void PawnsInit() {
    for (int index = 0; index < 10; ++index) {
        PawnRanksWhite[index] = RANK_8;
        PawnRanksBlack[index] = RANK_1;
    }
    
    int piece = WHITE_PAWN;
    for (int pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        int sq = board.pList[PCEINDEX(piece, pieceNum)];
        if (BoardRanks[sq] < PawnRanksWhite[BoardFiles[sq] + 1]) {
            PawnRanksWhite[BoardFiles[sq] + 1] = BoardRanks[sq];
        }
    }
    
    piece = BLACK_PAWN;
    for (int pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        int sq = board.pList[PCEINDEX(piece, pieceNum)];
        if (BoardRanks[sq] > PawnRanksBlack[BoardFiles[sq] + 1]) {
            PawnRanksBlack[BoardFiles[sq] + 1] = BoardRanks[sq];
        }
    }
}

int ParseMove(int from, int to) {
    GenerateMoves();
    
    int Move = NO_MOVE;
    int PromPiece = EMPTY;
    int found = false;
    
    for (int index = board.moveListStart[board.ply]; index < board.moveListStart[board.ply + 1]; ++index) {
        Move = board.moveList[index];
        if (FROMSQ(Move) == from && TOSQ(Move) == to) {
            PromPiece = PROMOTED(Move);
            if (PromPiece != EMPTY) {
                if ((PromPiece == WHITE_QUEEN && board.side == WHITE) || (PromPiece == BLACK_QUEEN && board.side == BLACK)) {
                    found = true;
                    break;
                }
                continue;
            }
            found = true;
            break;
        }
    }
    
    if (found != false) {
        if (MakeMove(Move) == false) {
            return NO_MOVE;
        }
        TakeMove();
        return Move;
    }
    
    return NO_MOVE;
}

int MoveExists(int move) {
    GenerateMoves();
    
    int moveFound = NO_MOVE;
    for (int index = board.moveListStart[board.ply]; index < board.moveListStart[board.ply + 1]; ++index) {
        moveFound = board.moveList[index];
        if (MakeMove(moveFound) == false) {
            continue;
        }
        TakeMove();
        if (move == moveFound) {
            return true;
        }
    }
    return false;
}

int EvalPosition() {
    int piece;
    int pieceNum;
    int sq;
    int score = board.material[WHITE] - board.material[BLACK];
    int file;
    int rank;
    
    if (0 == board.pieceNum[WHITE_PAWN] && 0 == board.pieceNum[BLACK_PAWN] && MaterialDraw() == true) {
        return 0;
    }
    
    PawnsInit();
    
    piece = WHITE_PAWN;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score += PawnTable[SQ64(sq)];
        file = BoardFiles[sq] + 1;
        rank = BoardRanks[sq];
        if (PawnRanksWhite[file - 1] == RANK_8 && PawnRanksWhite[file + 1] == RANK_8) {
            score += PawnIsolated;
        }
        
        if (PawnRanksBlack[file - 1] <= rank && PawnRanksBlack[file] <= rank && PawnRanksBlack[file + 1] <= rank) {
            score += PawnPassed[rank];
        }
    }
    
    piece = BLACK_PAWN;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score -= PawnTable[MIRROR64(SQ64(sq))];
        file = BoardFiles[sq] + 1;
        rank = BoardRanks[sq];
        if (PawnRanksBlack[file - 1] == RANK_1 && PawnRanksBlack[file + 1] == RANK_1) {
            score -= PawnIsolated;
        }
        
        if (PawnRanksWhite[file - 1] >= rank && PawnRanksWhite[file] >= rank && PawnRanksWhite[file + 1] >= rank) {
            score -= PawnPassed[7 - rank];
        }
    }
    
    piece = WHITE_KNIGHT;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score += KnightTable[SQ64(sq)];
    }
    
    piece = BLACK_KNIGHT;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score -= KnightTable[MIRROR64(SQ64(sq))];
    }
    
    piece = WHITE_BISHOP;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score += BishopTable[SQ64(sq)];
    }
    
    piece = BLACK_BISHOP;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score -= BishopTable[MIRROR64(SQ64(sq))];
    }
    
    piece = WHITE_ROOK;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score += RookTable[SQ64(sq)];
        file = BoardFiles[sq] + 1;
        if (PawnRanksWhite[file] == RANK_8) {
            if (PawnRanksBlack[file] == RANK_1) {
                score += ROOK_OPEN_FILE;
            } else {
                score += ROOK_SEMI_OPEN_FILE;
            }
        }
    }
    
    piece = BLACK_ROOK;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score -= RookTable[MIRROR64(SQ64(sq))];
        file = BoardFiles[sq] + 1;
        if (PawnRanksBlack[file] == RANK_1) {
            if (PawnRanksWhite[file] == RANK_8) {
                score -= ROOK_OPEN_FILE;
            } else {
                score -= ROOK_SEMI_OPEN_FILE;
            }
        }
    }
    
    piece = WHITE_QUEEN;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score += RookTable[SQ64(sq)];
        file = BoardFiles[sq] + 1;
        if (PawnRanksWhite[file] == RANK_8) {
            if (PawnRanksBlack[file] == RANK_1) {
                score += QUEEN_OPEN_FILE;
            } else {
                score += QUEEN_SEMI_OPEN_FILE;
            }
        }
    }
    
    piece = BLACK_QUEEN;
    for (pieceNum = 0; pieceNum < board.pieceNum[piece]; ++pieceNum) {
        sq = board.pList[PCEINDEX(piece, pieceNum)];
        score -= RookTable[MIRROR64(SQ64(sq))];
        file = BoardFiles[sq] + 1;
        if (PawnRanksBlack[file] == RANK_1) {
            if (PawnRanksWhite[file] == RANK_8) {
                score -= QUEEN_OPEN_FILE;
            } else {
                score -= QUEEN_SEMI_OPEN_FILE;
            }
        }
    }
    
    piece = WHITE_KING;
    sq = board.pList[PCEINDEX(piece, 0)];
    
    if ((board.material[BLACK] <= ENDGAME_MAT)) {
        score += KingInEndgame[SQ64(sq)];
    } else {
        score += KingInOpening[SQ64(sq)];
    }
    
    piece = BLACK_KING;
    sq = board.pList[PCEINDEX(piece, 0)];
    
    if ((board.material[WHITE] <= ENDGAME_MAT)) {
        score -= KingInEndgame[MIRROR64(SQ64(sq))];
    } else {
        score -= KingInOpening[MIRROR64(SQ64(sq))];
    }
    
    if (board.pieceNum[WHITE_BISHOP] >= 2) score += BISHOP_PAIR;
    if (board.pieceNum[BLACK_BISHOP] >= 2) score -= BISHOP_PAIR;
    
    if (board.side == WHITE) {
        return score;
    } else {
        return -score;
    }
}

inline int ProbePvTable() {
    int index = board.posKey & (MAX_PV_TABLE_ENTRIES - 1);
    
    if (board.PvTable[index].posKey == board.posKey) {
        return board.PvTable[index].move;
    }
    
    return NO_MOVE;
}

int GetPvLine(int depth) {
    int move = ProbePvTable();
    int count = 0;
    
    while (move != NO_MOVE && count < depth) {
        if (MoveExists(move)) {
            MakeMove(move);
            board.PvArray[count++] = move;
        } else {
            break;
        }
        move = ProbePvTable();
    }
    
    while (board.ply > 0) {
        TakeMove();
    }
    return count;
}

inline void StorePvMove(int move) {
    int index = board.posKey & (MAX_PV_TABLE_ENTRIES - 1);
    
    board.PvTable[index].move = move;
    board.PvTable[index].posKey = board.posKey;
}

void CheckUp() {
    if ((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - search.start) > search.time) {
        search.stop = true;
    }
}

void PickNextMove(int moveNum) {
    int index = 0;
    int bestScore = 0;
    int bestNum = moveNum;
    
    for (index = moveNum; index < board.moveListStart[board.ply + 1]; ++index) {
        if (board.moveScores[index] > bestScore) {
            bestScore = board.moveScores[index];
            bestNum = index;
        }
    }
    
    int temp = board.moveList[moveNum];
    board.moveList[moveNum] = board.moveList[bestNum];
    board.moveList[bestNum] = temp;
    
    temp = board.moveScores[moveNum];
    board.moveScores[moveNum] = board.moveScores[bestNum];
    board.moveScores[bestNum] = temp;
}

int IsRepetition() {
    for (int index = board.hisPly - board.fiftyMove; index < board.hisPly - 1; ++index) {
        if (board.posKey == board.history[index].posKey) {
            return true;
        }
    }
    return false;
}

inline void ClearPvTable() {
    for (int index = 0; index < MAX_PV_TABLE_ENTRIES; index++) {
        board.PvTable[index].move = NO_MOVE;
        board.PvTable[index].posKey = 0;
    }
}

int Quiescence(int alpha, int beta) {
    if ((search.time != -1) && (search.nodes & 0xFFFF) == 0) CheckUp(); // Only check timing every 65536 nodes
    search.nodes++;
    
    if (IsRepetition() || board.fiftyMove >= 100) {
        return 0;
    }
    
    if (board.ply > MAX_DEPTH - 1) {
        return EvalPosition();
    }
    
    int Score = EvalPosition();
    
    if (Score >= beta) {
        return beta;
    }
    
    if (Score > alpha) {
        alpha = Score;
    }
    
    GenerateCaptures();
    
    int MoveNum = 0;
    int Legal = 0;
    int OldAlpha = alpha;
    int BestMove = NO_MOVE;
    Score = -INFINITE;
    int PvMove = ProbePvTable();
    
    if (PvMove != NO_MOVE) {
        for (MoveNum = board.moveListStart[board.ply]; MoveNum < board.moveListStart[board.ply + 1]; ++MoveNum) {
            if (board.moveList[MoveNum] == PvMove) {
                board.moveScores[MoveNum] = 2000000;
                break;
            }
        }
    }
    
    for (MoveNum = board.moveListStart[board.ply]; MoveNum < board.moveListStart[board.ply + 1]; ++MoveNum) {
        PickNextMove(MoveNum);
        
        if (MakeMove(board.moveList[MoveNum]) == false) {
            continue;
        }
        
        Legal++;
        Score = -Quiescence(-beta, -alpha);
        TakeMove();
        if (search.stop == true) return 0;
        if (Score > alpha) {
            if (Score >= beta) {
                if (Legal == 1) {
                    search.fhf++;
                }
                search.fh++;
                
                return beta;
            }
            alpha = Score;
            BestMove = board.moveList[MoveNum];
        }
    }
    
    if (alpha != OldAlpha) {
        StorePvMove(BestMove);
    }
    
    return alpha;
}

int AlphaBeta(int alpha, int beta, int depth, int DoNull) {
    if (depth <= 0) {
        return Quiescence(alpha, beta);
    }
    if ((search.time != -1) && (search.nodes & 0xFFFF) == 0) CheckUp(); // Only check timing every 65536 nodes
    
    search.nodes++;
    
    if ((IsRepetition() || board.fiftyMove >= 100) && board.ply != 0) {
        return 0;
    }
    
    if (board.ply > MAX_DEPTH - 1) {
        return EvalPosition();
    }
    
    int InCheck = SqAttacked(board.pList[PCEINDEX(KINGS[board.side], 0)], board.side ^ 1);
    
    if (InCheck == true) {
        depth++;
    }
    
    int Score = -INFINITE;
    
    if (DoNull == true && false == InCheck &&
        board.ply != 0 && (board.material[board.side] > 50200) && depth >= 4) {
        
        int ePStore = board.enPas;
        if (board.enPas != NO_SQ) HASH_EP();
        board.side ^= 1;
        HASH_SIDE();
        board.enPas = NO_SQ;
        
        Score = -AlphaBeta(-beta, -beta + 1, depth - 4, false);
        
        board.side ^= 1;
        HASH_SIDE();
        board.enPas = ePStore;
        if (board.enPas != NO_SQ) HASH_EP();
        
        if (search.stop == true) return 0;
        if (Score >= beta) {
            return beta;
        }
    }
    
    GenerateMoves();
    
    int MoveNum = 0;
    int Legal = 0;
    int OldAlpha = alpha;
    int BestMove = NO_MOVE;
    Score = -INFINITE;
    int PvMove = ProbePvTable();
    
    if (PvMove != NO_MOVE) {
        for (MoveNum = board.moveListStart[board.ply]; MoveNum < board.moveListStart[board.ply + 1]; ++MoveNum) {
            if (board.moveList[MoveNum] == PvMove) {
                board.moveScores[MoveNum] = 2000000;
                break;
            }
        }
    }
    
    for (MoveNum = board.moveListStart[board.ply]; MoveNum < board.moveListStart[board.ply + 1]; ++MoveNum) {
        PickNextMove(MoveNum);
        
        if (MakeMove(board.moveList[MoveNum]) == false) {
            continue;
        }
        
        Legal++;
        Score = -AlphaBeta(-beta, -alpha, depth - 1, true);
        TakeMove();
        if (search.stop == true) return 0;
        
        if (Score > alpha) {
            if (Score >= beta) {
                if (Legal == 1) {
                    search.fhf++;
                }
                search.fh++;
                
                if ((board.moveList[MoveNum] & MOVE_FLAG_CAPTURE_MASK) == 0) {
                    board.searchKillers[MAX_DEPTH + board.ply] = board.searchKillers[board.ply];
                    board.searchKillers[board.ply] = board.moveList[MoveNum];
                }
                return beta;
            }
            alpha = Score;
            BestMove = board.moveList[MoveNum];
            if ((BestMove & MOVE_FLAG_CAPTURE_MASK) == 0) {
                board.searchHistory[board.pieces[FROMSQ(BestMove)] * BOARD_SQUARES_NUMBER + TOSQ(BestMove)] += depth;
            }
        }
    }
    
    if (Legal == 0) {
        if (InCheck) {
            return -MATE + board.ply;
        } else {
            return 0;
        }
    }
    
    if (alpha != OldAlpha) {
        StorePvMove(BestMove);
    }
    
    return alpha;
}

int ThreeFoldRep() {
    int r = 0;
    for (int i = 0; i < board.hisPly; ++i) {
        if (board.history[i].posKey == board.posKey) {
            r++;
        }
    }
    return r;
}

int DrawMaterial() {
    if (board.pieceNum[WHITE_PAWN] != 0 || board.pieceNum[BLACK_PAWN] != 0) return false;
    if (board.pieceNum[WHITE_QUEEN] != 0 || board.pieceNum[BLACK_QUEEN] != 0 || board.pieceNum[WHITE_ROOK] != 0 || board.pieceNum[BLACK_ROOK] != 0) return false;
    if (board.pieceNum[WHITE_BISHOP] > 1 || board.pieceNum[BLACK_BISHOP] > 1) { return false; }
    if (board.pieceNum[WHITE_KNIGHT] > 1 || board.pieceNum[BLACK_KNIGHT] > 1) { return false; }
    if (board.pieceNum[WHITE_KNIGHT] != 0 && board.pieceNum[WHITE_BISHOP] != 0) { return false; }
    if (board.pieceNum[BLACK_KNIGHT] != 0 && board.pieceNum[BLACK_BISHOP] != 0) { return false; }
    
    return true;
}

void SearchPosition() {
    std::fill(board.searchHistory, board.searchHistory + 14 * BOARD_SQUARES_NUMBER, 0);
    std::fill(board.searchKillers, board.searchKillers + 3 * MAX_DEPTH, 0);
    
    ClearPvTable();
    board.ply = 0;

    search.nodes = 0;
    search.fh = 0;
    search.fhf = 0;
    search.start = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    search.stop = false;
    
    int bestMove = NO_MOVE;
    int bestScore = -INFINITE;
    
    // Iterative deepening
    for (int currentDepth = 1; currentDepth <= search.depth && !search.stop; ++currentDepth) {
        bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, true);
        if (search.stop) break;
        
        int pvNum = GetPvLine(currentDepth); 
        bestMove = board.PvArray[0];
        
        long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count() - search.start;
        long long nps = (currentTime > 0) ? (search.nodes * 1000LL / currentTime) : 0;
        
        std::string info = "info depth " + std::to_string(currentDepth);
        
        if (std::abs(bestScore) > MATE - MAX_DEPTH) {
            int mateIn = (MATE - std::abs(bestScore) + 1) / 2;
            if (bestScore < 0) mateIn = -mateIn;
            info += " score mate " + std::to_string(mateIn);
        } else {
            info += " score cp " + std::to_string(bestScore);
        }
        
        info += " nodes " + std::to_string(search.nodes);
        info += " nps " + std::to_string(nps);
        info += " time " + std::to_string(currentTime);
        
        info += " pv";
        for (int i = 0; i < pvNum; i++) {
            info += " " + PrMove(board.PvArray[i]);
        }
        
        std::cout << info << std::endl;
    }
    
    std::cout << "bestmove " << PrMove(bestMove) << std::endl;
    search.best = bestMove;
    search.thinking = false;
}

void StartSearch(long long time) {
    search.thinking = true;
    search.depth = MAX_DEPTH;
    search.time = time;

    SearchPosition();
    MakeMove(search.best);
}

std::string getGameState(const std::string& fen) {
    ParseFen(fen);
    
    if (board.fiftyMove > 100 || ThreeFoldRep() >= 2 || DrawMaterial() == true) {
        return "draw";
    }
    
    GenerateMoves();
    
    int MoveNum = 0;
    int found = 0;
    for (MoveNum = board.moveListStart[board.ply]; MoveNum < board.moveListStart[board.ply + 1]; ++MoveNum) {
        if (MakeMove(board.moveList[MoveNum]) == false) {
            continue;
        }
        found++;
        TakeMove();
        break;
    }
    
    if (found != 0) return "ongoing";

    int InCheck = SqAttacked(board.pList[PCEINDEX(KINGS[board.side], 0)], board.side ^ 1);
    if (InCheck == true) {
        if (board.side == WHITE) {
            return "win";
        } else {
            return "loss";
        }
    } else {
        return "draw";
    }
}

std::string indexToChessNotation(int index) {
    if (index < 0 || index > 63) {
        throw std::runtime_error("Invalid index. Index must be between 0 and 63.");
    }
    
    int row = 8 - (index / 8);
    char column = 'a' + (index % 8);
    
    std::string result = "";
    result += column;
    result += std::to_string(row);
    return result;
}

int chessNotationToindex(const std::string& notation) {
    if (notation.length() != 2 || notation[0] < 'a' || notation[0] > 'h' || notation[1] < '1' || notation[1] > '8') {
        throw std::runtime_error("Invalid chess notation. The notation should be in the format of a letter (a-h) followed by a number (1-8).");
    }
    
    int column = notation[0] - 'a';
    int row = 8 - (notation[1] - '0');
    
    return row * 8 + column;
}

void bestMove(const std::string& fen, long long time) {
    ParseFen(fen);  
    StartSearch(time);
}

void HandleUci() {
    std::cout << "id name slowfish" << std::endl;
    std::cout << "uciok" << std::endl;
}

int ParseUciMove(const std::string& moveStr) {
    if (moveStr.length() < 4) return NO_MOVE;
    
    if (moveStr == "e1g1" && board.pieces[E1] == WHITE_KING) {
        return MOVE(E1, G1, EMPTY, EMPTY, MOVE_FLAG_CASTLE);
    }
    if (moveStr == "e1c1" && board.pieces[E1] == WHITE_KING) {
        return MOVE(E1, C1, EMPTY, EMPTY, MOVE_FLAG_CASTLE);
    }
    if (moveStr == "e8g8" && board.pieces[E8] == BLACK_KING) {
        return MOVE(E8, G8, EMPTY, EMPTY, MOVE_FLAG_CASTLE);
    }
    if (moveStr == "e8c8" && board.pieces[E8] == BLACK_KING) {
        return MOVE(E8, C8, EMPTY, EMPTY, MOVE_FLAG_CASTLE);
    }
    
    int fromFile = moveStr[0] - 'a';
    int fromRank = moveStr[1] - '1';
    int toFile = moveStr[2] - 'a';
    int toRank = moveStr[3] - '1';
    
    if (fromFile < 0 || fromFile > 7 || fromRank < 0 || fromRank > 7 ||
        toFile < 0 || toFile > 7 || toRank < 0 || toRank > 7) {
        return NO_MOVE;
    }
    
    int from = FR2SQ(fromFile, fromRank);
    int to = FR2SQ(toFile, toRank);

    int promoted = EMPTY;
    if (moveStr.length() == 5) {
        char promChar = moveStr[4];
        if (board.side == WHITE) {
            switch (promChar) {
                case 'q': promoted = WHITE_QUEEN; break;
                case 'r': promoted = WHITE_ROOK; break;
                case 'b': promoted = WHITE_BISHOP; break;
                case 'n': promoted = WHITE_KNIGHT; break;
            }
        } else {
            switch (promChar) {
                case 'q': promoted = BLACK_QUEEN; break;
                case 'r': promoted = BLACK_ROOK; break;
                case 'b': promoted = BLACK_BISHOP; break;
                case 'n': promoted = BLACK_KNIGHT; break;
            }
        }
    }
    
    return MOVE(from, to, EMPTY, promoted, 0);
}

void HandleIsReady() {
    std::cout << "readyok" << std::endl;
}

void HandleUciNewGame() {
    ClearPvTable();
    std::fill(board.searchHistory, board.searchHistory + 14 * BOARD_SQUARES_NUMBER, 0);
    std::fill(board.searchKillers, board.searchKillers + 3 * MAX_DEPTH, 0);
    ParseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void HandlePosition(const std::string& command) {
    std::istringstream iss(command);
    std::string token;
    iss >> token;
    
    iss >> token;
    if (token == "startpos") {
        ParseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        iss >> token;
        if (token == "moves") {
            while (iss >> token) {
                int move = ParseUciMove(token);
                if (move != NO_MOVE) {
                    MakeMove(move);
                }
            }
        }
    } else if (token == "fen") {
        std::string fen;
        std::getline(iss, fen);

        size_t movesPos = fen.find(" moves ");
        if (movesPos != std::string::npos) {
            fen = fen.substr(0, movesPos);
        }

        if (!fen.empty() && fen[0] == ' ') {
            fen = fen.substr(1);
        }
        
        ParseFen(fen);

        if (movesPos != std::string::npos) {
            std::string movesPart = command.substr(command.find(" moves ") + 7);
            std::istringstream movesStream(movesPart);
            while (movesStream >> token) {
                int move = ParseUciMove(token);
                if (move != NO_MOVE) {
                    MakeMove(move);
                }
            }
        }
    }
}

void StartUciSearch(int depth, int nodes, long long movetime) {
    search.thinking = true;
    search.stop = false;
    search.time = movetime;
    search.depth = depth;

    SearchPosition();
}

void HandleGo(const std::string& command) {
    std::istringstream iss(command);
    std::string token;
    iss >> token;

    int depth = MAX_DEPTH, nodes = -1;
    long long movetime = -1;
    while (iss >> token) {
        if (token == "depth") iss >> depth;
        if (token == "nodes") iss >> nodes;
        if (token == "movetime") iss >> movetime;
    }

    StartUciSearch(depth, nodes, movetime);
}

void HandleStop() {
    search.stop = true;
}

void ParseUciCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string token;
    iss >> token;
    
    if (token == "uci") {
        HandleUci();
    } else if (token == "isready") {
        HandleIsReady();
    } else if (token == "ucinewgame") {
        HandleUciNewGame();
    } else if (token == "position") {
        HandlePosition(command);
    } else if (token == "go") {
        HandleGo(command);
    } else if (token == "stop") {
        HandleStop();
    } else if (token == "quit") {
        exit(0);
    }
}

void UciLoop() {
    std::string command;
    
    while (std::getline(std::cin, command)) {
        ParseUciCommand(command);
    }
}

int main() {
    init();
    UciLoop();
    return 0;
}