#include <iostream>
#include <chrono>
#include <algorithm>
#include <stdlib.h> 
#include <conio.h>
#include <windows.h> 

#define DEFAULT "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define PERFT2  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -" 
#define PERFT2e "r3k2r/p1ppqpb1/bn2pnp1/3PN3/Pp2P3/2N2Q1p/1PPBBPPP/R3K2R b KQkq a3"
#define PERFT2n "4k2r/pPppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQk -"
#define PERFT3  "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"
#define PERFT3c "8/2p5/3p4/KP5r/5R1k/8/4P1P1/8 b - -"
#define PERFT5  "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"

#define BLACK 0
#define WHITE 1

#define KCASTLE 0b0001
#define kCASTLE 0b0010
#define QCASTLE 0b0100
#define qCASTLE 0b1000

//piece scores for evaluation
#define p 100
#define b 300
#define n 400
#define r 500
#define q 1000
#define at 100
#define pr 50

typedef unsigned long long u64;
typedef uint8_t            u8;
typedef uint16_t           u16;


//mailbox for movechecking with borders
bool mailbox[120] = {
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int mailboxLookup[64] = {
	21, 22, 23, 24, 25, 26, 27, 28,
	31, 32, 33, 34, 35, 36, 37, 38,
	41, 42, 43, 44, 45, 46, 47, 48,
	51, 52, 53, 54, 55, 56, 57, 58,
	61, 62, 63, 64, 65, 66, 67, 68,
	71, 72, 73, 74, 75, 76, 77, 78,
	81, 82, 83, 84, 85, 86, 87, 88,
	91, 92, 93, 94, 95, 96, 97, 98
};

//bitboards for player colours and empties
u64 white   = 0b1111111111111111000000000000000000000000000000000000000000000000;
u64 black   = 0b0000000000000000000000000000000000000000000000001111111111111111;
u64 empty   = ~(black | white);

//bitboards for every piece
u64 pawns   = 0b0000000011111111000000000000000000000000000000001111111100000000;
u64 rooks   = 0b1000000100000000000000000000000000000000000000000000000010000001;
u64 knights = 0b0100001000000000000000000000000000000000000000000000000001000010;
u64 bishops = 0b0010010000000000000000000000000000000000000000000000000000100100;
u64 kings   = 0b0001000000000000000000000000000000000000000000000000000000010000;
u64 queens  = 0b0000100000000000000000000000000000000000000000000000000000001000;


//masks
u64 doubleMoveW = 0b0000000011111111000000000000000000000000000000000000000000000000;
u64 doubleMoveB = 0b0000000000000000000000000000000000000000000000001111111100000000;

u64 bKcastle    = 0b0000000000000000000000000000000000000000000000000000000001100000;
u64 bQcastle    = 0b0000000000000000000000000000000000000000000000000000000000001110;

u64 wKcastle    = 0b0110000000000000000000000000000000000000000000000000000000000000;
u64 wQcastle    = 0b0000111000000000000000000000000000000000000000000000000000000000;

u64 rRankMask   = 0b0000000100000001000000010000000100000001000000010000000100000001;
u64 lRankMask   = 0b1000000010000000100000001000000010000000100000001000000010000000;

u64 wPromotion  = 0b0000000000000000000000000000000000000000000000000000000011111111;
u64 bPromotion  = 0b1111111100000000000000000000000000000000000000000000000000000000;


bool side = WHITE;
bool playerSide = WHITE;
int fiftyMove = 0;
int moveNum = 1;

const int pieceMoves[5][8] = {
	/*rook*/   {-1,1,8,-8,0,0,0,0},
	/*bishop*/ {-7,7,9,-9,0,0,0,0},
	/*knight*/ {-17,17,15,-15,-10,10,6,-6},
	/*king*/   {-7,7,9,-9,-1,1,8,-8},
	/*queen*/  {-7,7,9,-9,-1,1,8,-8}
};
const int mailboxMoves[5][8] = {
	/*rook*/   {-1,1,10,-10,0,0,0,0},
	/*bishop*/ {-9,9,11,-11,0,0,0,0},
	/*knight*/ {-21,21,19,-19,-12,12,8,-8},
	/*king*/   {-9,9,11,-11,-1,1,10,-10},
	/*queen*/  {-9,9,11,-11,-1,1,10,-10}
};

const u8 pieceMoveSizes[5] = {4,4,8,8,8};
const bool slidingPiece[5] = {1,1,0,0,1};

struct move {
	u8 to;
	u8 from;
	u8 moveInfo = 0;
	unsigned int ply;
};

struct hmove {
	move m;
	u8 piece;
	int ep;
	bool side;
	u8 castle;
};

int movePointer = 0;
int hmovePointer = 0;
int ep = -1;
u8 castle = 15;

move generated[1024];
hmove history[256];

int perftCounter = 0;
int perftCap = 0;
int perftEp = 0;
int perftCastle = 0;
int perftProm = 0;
int nodesChecked = 0;

int px = 0;
int py = 0;
int col = 0;

static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void printBoard() {
	u64 pointer = 1;
	std::cout << "8  ";
	for (int i = 0; i < 64; i++) {
		char piece;

		if (empty & pointer)
			piece = '-';
		else {
			if (pawns & pointer)
				piece = 'p';
			else if (rooks & pointer)
				piece = 'r';
			else if (knights & pointer)
				piece = 'n';
			else if (bishops & pointer)
				piece = 'b';
			else if (kings & pointer)
				piece = 'k';
			else if (queens & pointer)
				piece = 'q';
			else
				piece = 'e';

			if (white & pointer)
				piece = toupper(piece);
		}

		std::cout << piece << ' ';
		pointer <<= 1;
		if (!((i + 1) % 8)) {
			int num = 8 - (i + 1) / 8;
			if (num) 
				std::cout << '\n' << num << "  ";
		}
	}
	std::cout << "\n\n   A B C D E F G H\n";
}

void addMove(u8 to, u8 from, u8 bits) {
	generated[movePointer].from = from;
	generated[movePointer].to = to;
	generated[movePointer].moveInfo = bits;
	//test[movePointer] = to | from >> 5 | bits >> 10;
	movePointer++;
}

void gen (){
	u64 pointer = 1;
	u64 slidingPointer = 1;
	u8 piece = 0;
	u8 j, i, moves;
	movePointer = 0;

	for (i = 0; i < 64; i++) {
		if (side && white & pointer) {
			if (pointer & pawns) {
				//if the piece is not on the left edge and there is a enemy to the diagonal right add to list of pseudo legal moves
				if (!(lRankMask & pointer) && black & pointer >> 7) {
					if (wPromotion & pointer >> 7) {
						addMove(i - 7, i, 17);
					}
					else
						addMove(i - 7, i, 1);
				}
				if (!(rRankMask & pointer) && black & pointer >> 9) {
					if (wPromotion & pointer >> 9) {
						addMove(i - 9, i, 17);
					}
					else
						addMove(i - 9, i, 1);
				}
				if (!(rRankMask & pointer) && (u64)1 << ep & pointer >> 7)
					addMove(i - 7, i, 8);
				if (!(lRankMask & pointer) && (u64)1 << ep & pointer >> 9)
					addMove(i - 9, i, 8);
				if (empty & pointer >> 8) {
					if (wPromotion & pointer >> 8) {
						addMove(i - 8, i, 16);
					}
					else 
						addMove(i - 8, i, 0);
				}
				if (pointer & doubleMoveW && empty & pointer >> 16 && empty & pointer >> 8)
					addMove(i - 16, i, 0);
			}
			/*

			if (pointer & kings) {
				if ((wKcastle & empty) == wKcastle && castle & KCASTLE)
					addMove(62, i, 2);
				if ((wQcastle & empty) == wQcastle && castle & QCASTLE)
					addMove(58, i, 2);
			}

			if (pointer & rooks)
				piece = 0;
			else if (pointer & bishops)
				piece = 1;
			else if (pointer & knights)
				piece = 2;
			else if (pointer & queens)
				piece = 4;
			else if (pointer & kings)
				piece = 3;


			for (moves = 0; moves < pieceMoveSizes[piece]; moves++) {
				for (j = i;;) {
					if (!mailbox[mailboxLookup[j] + mailboxMoves[piece][moves]])
						break;
					j += pieceMoves[piece][moves];

					if ((u64)1 << j & white)
						break;
					else if ((u64)1 << j & empty)
						addMove(j, i, 0);
					else {
						addMove(j, i, 1);
						break;
					}
					if (!slidingPiece[piece])
						break;
				}
			}
			*/
			
			else if (pointer & rooks) {
				for (moves = 0; moves < 4; moves++) {
					for (j = i;;) {
						if (!mailbox[mailboxLookup[j] + mailboxMoves[0][moves]])
							break;
						j += pieceMoves[0][moves];

						if ((u64)1 << j & white)
							break;
						else if ((u64)1 << j & empty)
							addMove(j, i, 0);
						else {
							addMove(j, i, 1);
							break;
						}
					}
				}
			}
			
			else if (pointer & bishops) {
				for (moves = 0; moves < 4; moves++) {
					for (j = i;;) {
						if (!mailbox[mailboxLookup[j] + mailboxMoves[1][moves]])
							break;
						j += pieceMoves[1][moves];

						if ((u64)1 << j & white)
							break;
						else if ((u64)1 << j & empty)
							addMove(j, i, 0);
						else {
							addMove(j, i, 1);
							break;
						}
					}
				}
			}

			else if (pointer & knights) {
				for (moves = 0; moves < 8; moves++) {
					j = i;
 					if (!mailbox[mailboxLookup[j] + mailboxMoves[2][moves]])
						continue;
					j += pieceMoves[2][moves];

					if ((u64)1 << j & white)
						continue;
					else if ((u64)1 << j & empty)
						addMove(j, i, 0);
					else {
						addMove(j, i, 1);
						continue;
					}
				}
			}

			else if (pointer & queens) {
				for (moves = 0; moves < 8; moves++) {
					for (j = i;;) {
						if (!mailbox[mailboxLookup[j] + mailboxMoves[3][moves]])
							break;
						j += pieceMoves[3][moves];
						if ((u64)1 << j & white)
							break;
						else if ((u64)1 << j & empty)
							addMove(j, i, 0);
						else {
							addMove(j, i, 1);
							break;
						}
					}
				}
			}

			else {
				for (moves = 0; moves < 8; moves++) {
					j = i;
					if (!mailbox[mailboxLookup[j] + mailboxMoves[3][moves]])
						continue;
					j += pieceMoves[3][moves];
					if ((u64)1 << j & white)
						continue;
					else if ((u64)1 << j & empty)
						addMove(j, i, 0);
					else {
						addMove(j, i, 1);
						continue;
					}
				}
				if ((wKcastle & empty) == wKcastle && castle & KCASTLE) 
					addMove(62, i, 2);
				if ((wQcastle & empty) == wQcastle && castle & QCASTLE) 
					addMove(58, i, 4);
			}
			
		}
		else if (!side && black & pointer) {
			if (pointer & pawns) {
				if (!(rRankMask & pointer) && white & pointer << 7) {
					if (bPromotion & pointer << 7) {
						addMove(i + 7, i, 17);
					}
					else
						addMove(i + 7, i, 1);
				}
				if (!(lRankMask & pointer) && white & pointer << 9) {
					if (bPromotion & pointer << 9) {
						addMove(i + 9, i, 17);
					}
					else
						addMove(i + 9, i, 1);
				}
				if (!(lRankMask & pointer) && (u64)1 << ep & pointer << 7)
					addMove(i + 7, i, 8);
				if (!(rRankMask & pointer) && (u64)1 << ep & pointer << 9)
					addMove(i + 9, i, 8);
				if (empty & pointer << 8) {
					if (bPromotion & pointer << 8) {
						addMove(i + 8, i, 16);
					}
					else
						addMove(i + 8, i, 0);
				}
				if (pointer & doubleMoveB && empty & pointer << 16 && empty & pointer << 8)
					addMove(i + 16, i, 0);
			}
			/*
			if (pointer & kings) {
				if ((bKcastle & empty) == bKcastle && castle & kCASTLE)
					addMove(6, i, 2);
				if ((bQcastle & empty) == bQcastle && castle & qCASTLE)
					addMove(2, i, 2);
			}

			if (pointer & rooks)
				piece = 0;
			else if (pointer & bishops)
				piece = 1;
			else if (pointer & knights)
				piece = 2;
			else if (pointer & queens)
				piece = 4;
			else if (pointer & kings)
				piece = 3;


			for (moves = 0; moves < pieceMoveSizes[piece]; moves++) {
				for (j = i;;) {
					if (!mailbox[mailboxLookup[j] + mailboxMoves[piece][moves]])
						break;
					j += pieceMoves[piece][moves];

					if ((u64)1 << j & black)
						break;
					else if ((u64)1 << j & empty)
						addMove(j, i, 0);
					else {
						addMove(j, i, 1);
						break;
					}
					if (!slidingPiece[piece])
						break;
				}
			}
			*/
			
			else if (pointer & rooks) {
				for (moves = 0; moves < 4; moves++) {
					for (j = i;;) {
						if (!mailbox[mailboxLookup[j] + mailboxMoves[0][moves]])
							break;
						j += pieceMoves[0][moves];

						if ((u64)1 << j & black)
							break;
						else if ((u64)1 << j & empty)
							addMove(j, i, 0);
						else {
							addMove(j, i, 1);
							break;
						}
					}
				}
			}

			else if (pointer & bishops) {
				for (moves = 0; moves < 4; moves++) {
					for (j = i;;) {
						if (!mailbox[mailboxLookup[j] + mailboxMoves[1][moves]])
							break;
						j += pieceMoves[1][moves];

						if ((u64)1 << j & black)
							break;
						else if ((u64)1 << j & empty)
							addMove(j, i, 0);
						else {
							addMove(j, i, 1);
							break;
						}
					}
				}
			}

			else if (pointer & knights) {
				for (moves = 0; moves < 8; moves++) {
					j = i;
					if (!mailbox[mailboxLookup[j] + mailboxMoves[2][moves]])
						continue;
					j += pieceMoves[2][moves];

					if ((u64)1 << j & black)
						continue;
					else if ((u64)1 << j & empty)
						addMove(j, i, 0);
					else {
						addMove(j, i, 1);
						continue;
					}
				}
			}

			else if (pointer & queens) {
				for (moves = 0; moves < 8; moves++) {
					for (j = i;;) {
						if (!mailbox[mailboxLookup[j] + mailboxMoves[3][moves]])
							break;
						j += pieceMoves[3][moves];
						if ((u64)1 << j & black)
							break;
						else if ((u64)1 << j & empty)
							addMove(j, i, 0);
						else {
							addMove(j, i, 1);
							break;
						}
					}
				}
			}

			else {
				for (moves = 0; moves < 8; moves++) {
					j = i;
					if (!mailbox[mailboxLookup[j] + mailboxMoves[3][moves]])
						continue;
					j += pieceMoves[3][moves];
					if ((u64)1 << j & black)
						continue;
					else if ((u64)1 << j & empty)
						addMove(j, i, 0);
					else {
						addMove(j, i, 1);
						continue;
					}
				}
				if ((bKcastle & empty) == bKcastle && castle & kCASTLE)
					addMove(6, i, 2);
				if ((bQcastle & empty) == bQcastle && castle & qCASTLE)
					addMove(2, i, 4);
			}
			
		}
		pointer <<= 1;
	}
}

bool attacked(int square) {
	u8 j, moves;
	u64 pointer = (u64)1 << square;

	u64* col1 = nullptr;
	u64* col2 = nullptr;

	if (square == 61)
		int x = 0;

	if (side) {
		if (pointer & black)
			col1 = &black;
		else
			col1 = &white;

		col2 = &black;
		if (pointer & empty) {
			if (!(lRankMask & pointer) && ~white & pointer >> 7 && pawns & pointer >> 7)
				return true;
			if (!(rRankMask & pointer) && ~white & pointer >> 9 && pawns & pointer >> 9)
				return true;
		}
		else {
			if (!(lRankMask & pointer) && ~*col1 & pointer >> 7 && pawns & pointer >> 7)
				return true;
			if (!(rRankMask & pointer) && ~*col1 & pointer >> 9 && pawns & pointer >> 9)
				return true;
		}
	}
	else  {
		if (pointer & white)
			col1 = &white;
		else
			col1 = &black;
		col2 = &white;
		if (pointer & empty) {
			if (!(rRankMask & pointer) && ~black & pointer << 7 && pawns & pointer << 7)
				return true;
			if (!(lRankMask & pointer) && ~black & pointer << 9 && pawns & pointer << 9)
				return true;
		}
		else {
			if (!(rRankMask & pointer) && ~*col1 & pointer << 7 && pawns & pointer << 7)
				return true;
			if (!(lRankMask & pointer) && ~*col1 & pointer << 9 && pawns & pointer << 9)
				return true;
		}
	}
	for (int piece = 0; piece < 4; piece++) {
		for (moves = 0; moves < pieceMoveSizes[piece]; moves++) {
			for (j = square;;) {
				if (!mailbox[mailboxLookup[j] + mailboxMoves[piece][moves]])
					break;
				j += pieceMoves[piece][moves];

				if (col1)
					if ((u64)1 << j & *col1)
						break;
				if ((u64)1 << j & *col2) {
					switch (piece) {
					case 0:
						if ((u64)1 << j & rooks || (u64)1 << j & queens)
							return true;
						break;
					case 1:
						if ((u64)1 << j & bishops || (u64)1 << j & queens)
							return true;
						break;
					case 2:
						if ((u64)1 << j & knights)
							return true;
						break;
					case 3:
						if ((u64)1 << j & kings)
							return true;
						break;
					default:
						break;
					}
					break;
				}
				if (!slidingPiece[piece])
					break;
			}
		}
	}
	return false;
}

bool protectedSq(int square) {
	u8 j, moves;
	u64 pointer = (u64)1 << square;

	if (pointer & empty)
		return false;

	u64* col1 = nullptr;
	u64* col2 = nullptr;

	if (square == 19)
		int x = 0;

	if (side) {
		if (pointer & white)
			col1 = &white;
		else if (pointer & black)
			col1 = &black;
		col2 = &black;
		if (!(rRankMask & pointer) && ~*col2 & pointer >> 7 && pawns & pointer >> 7)
			return true;
		if (!(lRankMask & pointer) && ~*col2 & pointer >> 9 && pawns & pointer >> 9)
			return true;
	}
	else {
		if (pointer & white)
			col1 = &white;
		else if (pointer & black)
			col1 = &black;
		col2 = &white;
		if (!(lRankMask & pointer) && ~*col2 & pointer << 7 && pawns & pointer >> 7)
			return true;
		if (!(rRankMask & pointer) && ~*col2 & pointer << 9 && pawns & pointer >> 9)
			return true;
	}
	for (int piece = 0; piece < 4; piece++) {
		for (moves = 0; moves < pieceMoveSizes[piece]; moves++) {
			for (j = square;;) {
				if (!mailbox[mailboxLookup[j] + mailboxMoves[piece][moves]])
					break;
				j += pieceMoves[piece][moves];

				if (col2)
					if ((u64)1 << j & *col2)
						break;
				if ((u64)1 << j & *col1) {
					switch (piece) {
					case 0:
						if ((u64)1 << j & rooks || (u64)1 << j & queens)
							return true;
						break;
					case 1:
						if ((u64)1 << j & bishops || (u64)1 << j & queens)
							return true;
						break;
					case 2:
						if ((u64)1 << j & knights)
							return true;
						break;
					case 3:
						if ((u64)1 << j & kings)
							return true;
						break;
					default:
						break;
					}
					break;
				}
				if (!slidingPiece[piece])
					break;
			}
		}
	}
	return false;
}

bool checkCheck() {
	u64 pointer = 1;
	//find the king
	for (int square = 0; square < 64; square++) {
		if (!side) {
			if (pointer & kings && pointer & black)
				return attacked(square); //return if the king is being attacked
		}
		else {
			if (pointer & kings && pointer & white)
				return attacked(square); //return if the king is being attacked
		}
		pointer <<= 1;
	}
}

void undoMove() {
	if (hmovePointer == 0)
		return;

	//creates pointers
	u64 toPointer = (u64)1 << history[hmovePointer - 1].m.to;
	u64 pointer = (u64)1 << history[hmovePointer - 1].m.from;

	u8 piece;

	//find what piece moved
	if (toPointer & rooks)
		piece = 0;
	else if (toPointer & bishops)
		piece = 1;
	else if (toPointer & knights)
		piece = 2;
	else if (toPointer & queens)
		piece = 4;
	else if (toPointer & kings)
		piece = 3;
	else
		piece = 5;

	//remove the piece
	u64 mask = ~toPointer;
	black &= mask;
	white &= mask;
	pawns &= mask;
	rooks &= mask;
	knights &= mask;
	bishops &= mask;
	kings &= mask;
	queens &= mask;

	//put the piece back where it was
	switch (piece) {
	case 0:
		rooks |= pointer;
		break;
	case 1:
		bishops |= pointer;
		break;
	case 2:
		knights |= pointer;
		break;
	case 3:
		kings |= pointer;
		break;
	case 4:
		queens |= pointer;
		break;
	case 5:
		pawns |= pointer;
		break;
	}
	
	//update the colours
	if (history[hmovePointer - 1].side)
		white |= pointer;
	else
		black |= pointer;
	
	//capture
	if (history[hmovePointer - 1].m.moveInfo & 1) {
		switch (history[hmovePointer - 1].piece) {
		case 0:
			rooks |= toPointer;
			break;
		case 1:
			bishops |= toPointer;
			break;
		case 2:
			knights |= toPointer;
			break;
		case 3:
			kings |= toPointer;
			break;
		case 4:
			queens |= toPointer;
			break;
		case 5:
			pawns |= toPointer;
			break;
		}

		if (history[hmovePointer - 1].side)
			black |= toPointer;
		else
			white |= toPointer;
	}
	//castling
	else if (history[hmovePointer - 1].m.moveInfo & 2) {
		if (history[hmovePointer - 1].side) {
			u64 rook = (u64)1 << 63;
			rooks |= rook;
			white |= rook;
			rook = ~(rook >> 2);
			rooks &= rook;
			white &= rook;
		}
		else {
			u64 rook = (u64)1 << 7;
			rooks |= rook;
			black |= rook;
			rook = ~(rook >> 2);
			rooks &= rook;
			black &= rook;
		}
	}//q side
	else if (history[hmovePointer - 1].m.moveInfo & 4) {
		if (history[hmovePointer - 1].side) {
			u64 rook = (u64)1 << 56;
			rooks |= rook;
			white |= rook;
			rook = ~(rook << 3);
			rooks &= rook;
			white &= rook;
		}
		else {
			u64 rook = (u64)1;
			rooks |= rook;
			black |= rook;
			rook = ~(rook << 3);
			rooks &= rook;
			black &= rook;
		}
	}

	//ep
	else if (history[hmovePointer - 1].m.moveInfo & 8) {
		if (history[hmovePointer - 1].side) {
			pawns |= toPointer << 8;
			black |= toPointer << 8;
		}
		else {
			pawns |= toPointer >> 8;
			white |= toPointer >> 8;
		}
	}

	//roll back promotions
	if (history[hmovePointer - 1].m.moveInfo & 16) {
		queens &= ~pointer;
		pawns |= pointer;
	}
	else if (history[hmovePointer - 1].m.moveInfo & 32) {
		knights &= ~pointer;
		pawns |= pointer;
	}
	else if (history[hmovePointer - 1].m.moveInfo & 64) {
		rooks &= ~pointer;
		pawns |= pointer;
	}
	else if (history[hmovePointer - 1].m.moveInfo & 128) {
		bishops &= ~pointer;
		pawns |= pointer;
	}

	//roll back variables
	ep = history[hmovePointer - 1].ep;
	castle = history[hmovePointer - 1].castle;
	empty = ~(white | black);
	side = history[hmovePointer - 1].side;
	hmovePointer--;

	//regenerate the moves (i know this is inefficient but i haven't got around to fixing it yet)
	movePointer = 0;
	gen();
}

bool makeMove(int to, int from) {
	//search for the move the player wants to make
	for (int i = 0; i < movePointer; i++) {
		if (generated[i].from == from){
			if (generated[i].to == to) {
				//makes pointers to the moved squares
				u64 toPointer = (u64)1 << to;
				u64 pointer = (u64)1 << from;

				u8 piece;

				hmove h;
				h.m = generated[i];
				h.piece = 6;
				h.side = side;

				if (generated[i].moveInfo & 16) {
					//std::cout << "\nWhat would you like to promote to (Q,N,R,B): \n";
					//char a;
					//std::cin >> a;
					char a = 'q';
					switch (tolower(a)) {
					case 'n':
						generated[i].moveInfo &= 247;
						generated[i].moveInfo |= 32;
						pawns &= ~pointer;
						knights |= pointer;
						break;
					case 'r':
						generated[i].moveInfo &= 247;
						generated[i].moveInfo |= 64;
						pawns &= ~pointer;
						rooks |= pointer;
						break;
					case 'b':
						generated[i].moveInfo &= 247;
						generated[i].moveInfo |= 128;
						pawns &= ~pointer;
						bishops |= pointer;
						break;
					default:
						pawns &= ~pointer;
						queens |= pointer;
						break;
					}
				}

				//capture
				if (generated[i].moveInfo & 1) {
					u64 mask = ~toPointer;
					//remove the piece
					black &= mask;
					white &= mask;

					//find out what piece is being taken so it can be put back if the move is undone
					if (toPointer & rooks)
						piece = 0;
					else if (toPointer & bishops)
						piece = 1;
					else if (toPointer & knights)
						piece = 2;
					else if (toPointer & queens)
						piece = 4;
					else if (toPointer & kings)
						piece = 3;
					else
						piece = 5;

					//remove the piece
					pawns &= mask;
					rooks &= mask;
					knights &= mask;
					bishops &= mask;
					kings &= mask;
					queens &= mask;

					//store what piece it is
					h.piece = piece;
				}
				//king side castle
				else if (generated[i].moveInfo & 2) {
					if (side) {
						if (attacked(62) || attacked(61))
							return false;
						u64 rook = (u64)1 << 63;
						rook = ~rook;
						rooks &= rook;
						white &= rook;
						rook = (~rook) >> 2;
						rooks |= rook;
						white |= rook;
					}
					else {
						if (attacked(6) || attacked(5))
							return false;
						u64 rook = (u64)1 << 7;
						rook = ~rook;
						rooks &= rook;
						black &= rook;
						rook = (~rook) >> 2;
						rooks |= rook;
						black |= rook;
					}
				}
				//queen side castle
				else if (generated[i].moveInfo & 4) {
					if (side) {
						if (attacked(57) || attacked(58) || attacked(59))
							return false;
						u64 rook = (u64)1 << 56;
						rook = ~rook;
						rooks &= rook;
						white &= rook;
						rook = (~rook) << 3;
						rooks |= rook;
						white |= rook;
					}
					else {
						if (attacked(1) || attacked(2) || attacked(3))
							return false;
						u64 rook = (u64)1;
						rook = ~rook;
						rooks &= rook;
						black &= rook;
						rook = (~rook) << 3;
						rooks |= rook;
						black |= rook;
					}
				}
				//ep
				if (generated[i].moveInfo & 8) {
					u64 mask;
					if (white & pointer)
						mask = toPointer << 8;
					else
						mask = toPointer >> 8;
					mask = ~mask;
					black &= mask;
					white &= mask;
					pawns &= mask;
				}
				

				if (pointer & rooks)
					piece = 0;
				else if (pointer & bishops)
					piece = 1;
				else if (pointer & knights)
					piece = 2;
				else if (pointer & queens)
					piece = 4;
				else if (pointer & kings)
					piece = 3;
				else {
					piece = 5;
					//double move
					if (abs(generated[i].to - generated[i].from) == 16) {
						u64 mask;
						if (white & pointer)
							mask = toPointer << 8;
						else
							mask = toPointer >> 8;
						ep = (int)log2(mask);
					}
				}
				
				//removes castling if the rook or king moves
				if (piece == 0) {
					if (pointer & (u64)1 << 63)
						castle &= ~KCASTLE;

					if (pointer & (u64)1 << 56)
						castle &= ~QCASTLE;

					if (pointer & (u64)1 << 7)
						castle &= ~kCASTLE;

					if (pointer & (u64)1 << 0)
						castle &= ~qCASTLE;
				}
				if (piece == 3) {
					if (pointer & white) {
						castle &= ~KCASTLE;
						castle &= ~QCASTLE;
					}
					if (pointer & black) {
						castle &= ~kCASTLE;
						castle &= ~qCASTLE;
					}
				}

				//places the piece where it is going
				switch (piece) {
				case 0:
					rooks |= toPointer;
					break;
				case 1:
					bishops |= toPointer;
					break;
				case 2:
					knights |= toPointer;
					break;
				case 3:
					kings |= toPointer;
					break;
				case 4:
					queens |= toPointer;
					break;
				case 5:
					pawns |= toPointer;
					break;
				}

				//add the piece to the colour
				if (side)
					white |= toPointer;
				else
					black |= toPointer;

				//update history
				h.castle = castle;
				history[hmovePointer++] = h;
				u64 mask = ~pointer;

				//remove the piece from the place it was
				black   &= mask;
				white   &= mask;
				pawns   &= mask;
				rooks   &= mask;
				knights &= mask;
				bishops &= mask;
				kings   &= mask;
				queens  &= mask;

				//update empty squares
				empty = ~(white | black);

				h.ep = ep;

				//if the pseudo-legal does not cause check
				if (!checkCheck()) {
					//update size and regenerate moves
					side = !side;
					movePointer = 0;
					gen();
					ep = -1;
					return true;
				}
				else
					undoMove(); //the pseudo-legal move wasn't legal so undo it
				return false;
			}
		}
	}
	return false;
}

void printBoardwAttacks() {
	u64 pointer = 1;
	side = !side;
	std::cout << "8  ";
	for (int i = 0; i < 64; i++) {
		char piece;
		if (attacked(i)) 
			SetConsoleTextAttribute(hConsole, 0x0004);
		if (protectedSq(i))
			SetConsoleTextAttribute(hConsole, 0x0002);
		if (empty & pointer)
			piece = '-';
		else {
			if (pawns & pointer)
				piece = 'p';
			else if (rooks & pointer)
				piece = 'r';
			else if (knights & pointer)
				piece = 'n';
			else if (bishops & pointer)
				piece = 'b';
			else if (kings & pointer)
				piece = 'k';
			else if (queens & pointer)
				piece = 'q';
			else
				piece = 'e';

			if (white & pointer)
				piece = toupper(piece);
		}

		std::cout << piece << ' ';
		SetConsoleTextAttribute(hConsole, 0x0007);
		pointer <<= 1;
		if (!((i + 1) % 8)) {
			int num = 8 - (i + 1) / 8;
			if (num)
				std::cout << '\n' << num << "  ";
		}
	}
	std::cout << "\n\n   A B C D E F G H\n";
	side = !side;
}

void loadBoardFromFen(std::string fen) {
	pawns = 0;
	rooks = 0;
	bishops = 0;
	queens = 0;
	kings = 0;
	knights = 0;
	black = 0;
	white = 0;
	castle = 0;
	ep = -1;
	fiftyMove = 0;
	moveNum = 0;

	int square = 0;

	u64 pointer = 1;
	int parameterPointer = 0;

	int letter = 0;

	//for each thing in the fen
	for (char i : fen) {
		letter++;
		if (square < 64) {
			//switch for each piece
			switch (tolower(i)) {
			case 'r':
				rooks |= pointer;
				break;
			case 'k':
				kings |= pointer;
				break;
			case 'p':
				pawns |= pointer;
				break;
			case 'n':
				knights |= pointer;
				break;
			case 'b':
				bishops |= pointer;
				break;
			case 'q':
				queens |= pointer;
				break;
				//new line
			case '/':
				continue;
				//skip open squares
			default:
				if (i - 48 > 0 && i - 48 < 9) {
					pointer <<= i - 48;
					square += i -48;
					continue;
				}
				break;
			};
			if (i != toupper(i))
				black |= pointer;
			else
				white |= pointer;

			pointer <<= 1;
			square++;
		}
		//parameters
		else {
			//colour of the piece
			switch (i) {
				if (parameterPointer == 0) {
			case 'w':
				side = WHITE;
				parameterPointer++;
				break;
			case 'b':
				side = BLACK;
				parameterPointer++;
				break;
				}
			}


			//checks for castling
			if (parameterPointer >= 1 && parameterPointer < 5) {
				if (i == 'K') {
					if (parameterPointer == 2)
						parameterPointer++;
					castle |= 0b0001;
				}
				else if (i == 'k') {
					if (parameterPointer == 3)
						parameterPointer++;
					castle |= 0b0010;
				}
				else if (i == 'Q') {
					if (parameterPointer == 2)
						parameterPointer++;
					castle |= 0b0100;
				}
				else if (i == 'q') {
					if (parameterPointer == 3)
						parameterPointer++;
					castle |= 0b1000;
				}
				else {
					parameterPointer++;
				}
			}
			if (parameterPointer == 5) {
				if (i == '-') {
					ep = -1;
					parameterPointer += 2;
				}
				else {
					ep = 0;
					switch (i)
					{
					case 'h':
						ep++;
					case 'g':
						ep++;
					case 'f':
						ep++;
					case 'e':
						ep++;
					case 'd':
						ep++;
					case 'c':
						ep++;
					case 'b':
						ep++;
					case 'a':
						parameterPointer++;
					default:
						break;
					}
				}
			}
			else if (parameterPointer == 6){
				switch (i)
				{
				case '1':
					ep += 8;
				case '2':
					ep += 8;
				case '3':
					ep += 8;
				case '4':
					ep += 8;
				case '5':
					ep += 8;
				case '6':
					ep += 8;
				case '7':
					ep += 8;
				case '8':
					parameterPointer++;
				default:
					break;
				}
			}
			else if (parameterPointer == 7) {
				if (i != ' ') {
					if (fiftyMove != 0)
						fiftyMove *= 10;
					fiftyMove += i - 48;
				}
				if (fen[letter] == ' ')
					parameterPointer++;
			}
			else if (parameterPointer == 8) {
				if (i != ' ') {
					if (moveNum != 0)
						moveNum *= 10;
					moveNum += i - 48;
				}
			}
		}
	}
	empty = ~(black | white);
}

void printBoardPerft(int to, int from, int cols) {
	if (col < cols && col != 0) {
		px += 22;
	}
	else if (col != 0) {
		px = 0;
		py += 11;
		col = 0;
	}

	SetConsoleCursorPosition(hConsole, COORD{ (short)px,(short)(py) });

	u64 pointer = 1;
	std::cout << "8  ";
	for (int i = 0; i < 64; i++) {
		if (i == to || i == from)
			SetConsoleTextAttribute(hConsole, 0x0002);
		else
			SetConsoleTextAttribute(hConsole, 0x0007);
		if (hmovePointer-1)
			if (history[hmovePointer - 2].m.to == i || history[hmovePointer - 2].m.from == i)
				SetConsoleTextAttribute(hConsole, 0x0006);
		
		char piece;

		if (empty & pointer)
			piece = '-';
		else {
			if (pawns & pointer)
				piece = 'p';
			else if (rooks & pointer)
				piece = 'r';
			else if (knights & pointer)
				piece = 'n';
			else if (bishops & pointer)
				piece = 'b';
			else if (kings & pointer)
				piece = 'k';
			else if (queens & pointer)
				piece = 'q';
			else
				piece = 'e';

			if (white & pointer)
				piece = toupper(piece);
		}

		std::cout << piece << ' ';
		pointer <<= 1;
		if (!((i + 1) % 8)) {
			int num = 8 - (i + 1) / 8;
			if (num) {
				SetConsoleCursorPosition(hConsole, COORD{ (short)px,(short)(++py) });
				std::cout << num << "  ";
			}
		}
	}

	SetConsoleTextAttribute(hConsole, 0x0007);
	col++;
	SetConsoleCursorPosition(hConsole, COORD{ (short)px,(short)(++py) });
	std::cout << "   A B C D E F G H";
	SetConsoleCursorPosition(hConsole, COORD{ (short)px,(short)(++py) });
}

int perft(int ply) {
	if (ply == 0) {
		return 1;
	}
	for (int i = 0; i < movePointer; i++) {
		if (makeMove(generated[i].to, generated[i].from)) {
			if (ply > 1)
				py -= 9;
			
			py -= 9;
			SetConsoleCursorPosition(hConsole, COORD{ (short)px,(short)(py) });

			printBoardPerft(history[hmovePointer - 1].m.to, history[hmovePointer - 1].m.from, 12);

			std::cout << (char)((history[hmovePointer - 1].m.from % 8) + 97)
				<< (8 - (history[hmovePointer - 1].m.from >> 3))
				<< (char)((history[hmovePointer - 1].m.to % 8) + 97)
				<< (8 - (history[hmovePointer - 1].m.to >> 3));
			
			perftCounter += perft(ply - 1);
			if (history[hmovePointer - 1].m.moveInfo & 1) {
				perftCap++;
				SetConsoleTextAttribute(hConsole, 0x0004);
				std::cout << " Cap ";
			}
			else if (history[hmovePointer - 1].m.moveInfo & 8) {
				perftEp++;
				SetConsoleTextAttribute(hConsole, 0x0006);
				std::cout << " EP ";
			}
			else if (history[hmovePointer - 1].m.moveInfo & 4 || history[hmovePointer - 1].m.moveInfo & 2) {
				perftCastle++;
				SetConsoleTextAttribute(hConsole, 0x0005);
				std::cout << " Cas ";
			}
			if (history[hmovePointer - 1].m.moveInfo > 8) {
				perftProm++;
				SetConsoleTextAttribute(hConsole, 0x0003);
				std::cout << " Prom ";
			}
			SetConsoleTextAttribute(hConsole, 0x0007);
			undoMove();
		}
	}
	py += 9;
	SetConsoleCursorPosition(hConsole, COORD{ (short)px,(short)(py) });
}

int staticEval() {
	int score = 0;
	u64 pointer = 1;
	int m = 1;
	//for each piece on the board
	for (int i = 0; i < 64; i++) {
		//add or subtract the value of the piece from the score based on the side 
		m = 1;
		if (pointer & white)
			m = -1;
		if (pawns & pointer)
			score += p * m;
		else if (rooks & pointer)
			score += r * m;
		else if (knights & pointer)
			score += n * m;
		else if (bishops & pointer)
			score += b * m;
		else if (queens & pointer)
			score += q * m;

		if (protectedSq(i))
			score += pr * m;
		if (attacked(i))
			score += at * m;
		if (checkCheck())
			score += -1000 * m;
		pointer <<= 1;
	}
	return score;
}

int minimax(int depth, int alpha, int beta, bool min) {
	nodesChecked++;
	if (depth == 0) {
		return staticEval();
	}
	int score = 0;

	if (!min) {
		for (int i = 0; i < movePointer; i++) {
			if (makeMove(generated[i].to, generated[i].from)) {
				//printBoard();
				score = minimax(depth - 1, alpha, beta, 1);
				undoMove();
				if (score >= beta)
					return beta;
				if (score > alpha)
					alpha = score;
			}
		}
		return alpha;
	}
	else {
		for (int i = 0; i < movePointer; i++) {
			if (makeMove(generated[i].to, generated[i].from)) {
				//printBoard();
				score = minimax(depth - 1, alpha, beta, 0);
				undoMove();
				if (score <= alpha)
					return alpha;
				if (score < beta)
					beta = score;
			}
		}
		return beta;
	}
}

void timeTest() {
	int tries = 1000000;

	std::chrono::high_resolution_clock::time_point start, end;

	std::cout << "\ngen\n";
	start = std::chrono::steady_clock::now();
	for (int i = 0; i < tries; i++) {
		movePointer = 0;
		gen();
	}
	end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() / tries << "ns\n";

	std::cout << movePointer << '\n';


	std::cout << "\ncheck\n";
	start = std::chrono::steady_clock::now();
	for (int i = 0; i < tries; i++) {
		attacked(i % 64);
	}
	end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() / tries << "ns\n";

	std::cout << checkCheck() << "\n\n\n";

	printBoardwAttacks();

	std::cout << "\nmakemove\n";
	start = std::chrono::steady_clock::now();
	for (int i = 0; i < tries; i++) {
		makeMove(44, 52);
		undoMove();
	}
	end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() / tries << "ns\n";
}

void perft() {
	std::chrono::steady_clock::time_point end, start;

	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	if (GetConsoleScreenBufferInfo(hConsole, &cbsi))
	{
		px = cbsi.dwCursorPosition.X;
		py = cbsi.dwCursorPosition.Y;
	}
	start = std::chrono::steady_clock::now();
	perft(3);
	end = std::chrono::steady_clock::now();
	std::cout << "\nPerft results";
	std::cout << "\n" << perftCounter << " moves\n";
	std::cout << perftCap << " captures\n";
	std::cout << perftEp << " en passents\n";
	std::cout << perftCastle << " castles\n";
	std::cout << perftProm << " promotions\n";
	std::cout << "time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds> (end - start).count() << "ms\n\n";
}

int main() {
	loadBoardFromFen(DEFAULT);
	gen();

	std::chrono::high_resolution_clock::time_point start, end;

	while (true) {
		printBoard();

		if (side == playerSide) {
			std::cout << "\n\n";
			std::cout << "\nEnter your move: \n";
			char move[4];
			std::cin >> move;
			char f[2] = { move[0],move[1] };
			char t[2] = { move[2],move[3] };

			int from = 0;

			f[0] = tolower(f[0]);

			if (f[0] == 'u') {
				undoMove();
				undoMove();
			}
			else {
				switch (f[0])
				{
				case 'h':
					from++;
				case 'g':
					from++;
				case 'f':
					from++;
				case 'e':
					from++;
				case 'd':
					from++;
				case 'c':
					from++;
				case 'b':
					from++;
				default:
					break;
				}

				switch (f[1])
				{
				case '1':
					from += 8;
				case '2':
					from += 8;
				case '3':
					from += 8;
				case '4':
					from += 8;
				case '5':
					from += 8;
				case '6':
					from += 8;
				case '7':
					from += 8;
				default:
					break;
				}

				int to = 0;

				t[0] = tolower(t[0]);

				switch (t[0])
				{
				case 'h':
					to++;
				case 'g':
					to++;
				case 'f':
					to++;
				case 'e':
					to++;
				case 'd':
					to++;
				case 'c':
					to++;
				case 'b':
					to++;
				default:
					break;
				}

				switch (t[1])
				{
				case '1':
					to += 8;
				case '2':
					to += 8;
				case '3':
					to += 8;
				case '4':
					to += 8;
				case '5':
					to += 8;
				case '6':
					to += 8;
				case '7':
					to += 8;
				default:
					break;
				}

				start = std::chrono::steady_clock::now();

				makeMove(to, from);

				end = std::chrono::steady_clock::now();
				std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() << "ns\n";
			}
			//printBoard();
		}
		else {
			int bestScore = -999999999;
			int bestmove[2];
			int move[2];
			int score = 0;

			nodesChecked = 0;

			for (int i = 0; i < movePointer; i++) {
				move[0] = generated[i].to;
				move[1] = generated[i].from;
				if (makeMove(generated[i].to, generated[i].from)) {
					score = minimax(3, -9999999, 9999999, 1);
					if (score > bestScore) {
						bestScore = score;
						bestmove[0] = move[0];
						bestmove[1] = move[1];
					}
					//printBoard();
					undoMove();
				}
			}
			makeMove(bestmove[0], bestmove[1]);
			std::cout << (char)((history[hmovePointer - 1].m.from % 8) + 97)
				<< (8 - (history[hmovePointer - 1].m.from >> 3))
				<< (char)((history[hmovePointer - 1].m.to % 8) + 97)
				<< (8 - (history[hmovePointer - 1].m.to >> 3));
			std::cout << "   " << nodesChecked << '\n';
		}
	}
}
