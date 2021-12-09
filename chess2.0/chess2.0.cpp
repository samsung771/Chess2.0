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


u64 doubleMoveW = 0b0000000011111111000000000000000000000000000000000000000000000000;
u64 doubleMoveB = 0b0000000000000000000000000000000000000000000000001111111100000000;

u64 bKcastle    = 0b0000000000000000000000000000000000000000000000000000000001100000;
u64 bQcastle    = 0b0000000000000000000000000000000000000000000000000000000000001110;

u64 wKcastle    = 0b0110000000000000000000000000000000000000000000000000000000000000;
u64 wQcastle    = 0b0000111000000000000000000000000000000000000000000000000000000000;

u64 rRankMask   = 0b0000000100000001000000010000000100000001000000010000000100000001;
u64 lRankMask   = 0b1000000010000000100000001000000010000000100000001000000010000000;


bool side = WHITE;
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

u16 test[1024] = {0};

struct move {
	u8 to;
	u8 from;
	u8 moveInfo = 0;
	unsigned int ply;
};

struct hmove {
	move m;
	u8 piece;
	bool side;
};

int movePointer = 0;
int hmovePointer = 0;
int ep = -1;
u8 castle = 15;

move generated[1024];
hmove history[500];

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

	for (i = 0; i < 64; i++) {
		if (side && white & pointer) {
			if (pointer & pawns) {
				if (!(rRankMask & pointer) && black & pointer >> 7)
					addMove(i - 7, i, 1);
				if (!(lRankMask & pointer) && black & pointer >> 9)
					addMove(i - 9, i, 1);
				if (!(rRankMask & pointer) && (u64)1 << ep & pointer >> 7)
					addMove(i - 7, i, 4);
				if (!(lRankMask & pointer) && (u64)1 << ep & pointer >> 9)
					addMove(i - 9, i, 4);
				if (!(lRankMask & pointer) && black & pointer >> 9)
					addMove(i - 9, i, 1);
				if (empty & pointer >> 8)
					addMove(i - 8, i, 0);
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
					addMove(58, i, 2);
			}
			
		}
		else if (!side && black & pointer) {
			if (pointer & pawns) {
				if (!(lRankMask & pointer) && white & pointer << 7)
					addMove(i + 7, i, 1);
				if (!(rRankMask & pointer) && white & pointer << 9)
					addMove(i + 9, i, 1);
				if (!(lRankMask & pointer) && (u64)1 << ep & pointer << 7)
					addMove(i + 7, i, 4);
				if (!(rRankMask & pointer) && (u64)1 << ep & pointer << 9)
					addMove(i + 9, i, 4);
				if (empty & pointer << 8)
					addMove(i + 8, i, 0);
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
					addMove(2, i, 2);
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

	if (!side) {
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
	for (int square = 0; square < 64; square++) {
		if (!side) {
			if (pointer & kings && pointer & black)
				return attacked(square);
		}
		else {
			if (pointer & kings && pointer & white)
				return attacked(square);
		}
		pointer <<= 1;
	}
}

void undoMove() {
	u64 toPointer = (u64)1 << history[hmovePointer - 1].m.to;
	u64 pointer = (u64)1 << history[hmovePointer - 1].m.from;

	u8 piece;

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

	u64 mask = ~toPointer;
	black &= mask;
	white &= mask;
	pawns &= mask;
	rooks &= mask;
	knights &= mask;
	bishops &= mask;
	kings &= mask;
	queens &= mask;

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
	

	if (history[hmovePointer - 1].side)
		white |= pointer;
	else
		black |= pointer;

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

	empty = ~(white | black);
	hmovePointer--;
}

bool makeMove(int to, int from) {
	for (int i = 0; i < movePointer; i++) {
		if (generated[i].from == from){
			if (generated[i].to == to) {
				u64 toPointer = (u64)1 << to;

				u8 piece;

				hmove h;
				h.m = generated[i];
				h.piece = 6;
				h.side = side;

				if (generated[i].moveInfo & 1) {
					u64 mask = ~toPointer;
					black &= mask;
					white &= mask;

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

					pawns &= mask;
					rooks &= mask;
					knights &= mask;
					bishops &= mask;
					kings &= mask;
					queens &= mask;

					h.piece = piece;
				}

				u64 pointer = (u64)1 << from;

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
				else 
					piece = 5;


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


				if (side)
					white |= toPointer;
				else
					black |= toPointer;

				history[hmovePointer] = h;
				hmovePointer++;

				u64 mask = ~pointer;

				black &= mask;
				white &= mask;
				pawns &= mask;
				rooks &= mask;
				knights &= mask;
				bishops &= mask;
				kings &= mask;
				queens &= mask;

				empty = ~(white | black);

				if (checkCheck)
					return true;
				else
					undoMove();
				return false;
			}
		}
	}
	return false;
}

void printBoardwAttacks() {
	u64 pointer = 1;
	std::cout << "8  ";
	for (int i = 0; i < 64; i++) {
		char piece;
		if (attacked(i)) 
			SetConsoleTextAttribute(hConsole, 0x0004);
		else if (protectedSq(i))
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

int main() {
	int tries = 1000000;
	printBoard();
	loadBoardFromFen(PERFT3);
	printBoard();

	std::chrono::steady_clock::time_point end, start;
	std::cout << "\ngen\n";
	start = std::chrono::steady_clock::now();
	for (int i = 0; i < tries; i++) {
		movePointer = 0;
		gen();
	}
	end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count()/tries << "ns\n";

	std::sort(&generated[0],&generated[movePointer], [](move a, move b) {return a.moveInfo > b.moveInfo; });

	std::cout << movePointer << '\n';


	std::cout << "\ncheck\n";
	start = std::chrono::steady_clock::now();
	for (int i = 0; i < tries; i++) {
		attacked(i%64);
	}
	end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() / tries << "ns\n";
	
	std::cout << checkCheck() << "\n\n\n";

	printBoardwAttacks();
	/*
	std::cout << "\nmakemove\n";
	start = std::chrono::steady_clock::now();
	for (int i = 0; i < tries; i++) {
		makeMove(44,52);
		undoMove();
	}
	end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() / tries << "ns\n";
	*/
	std::cout << '\n' << makeMove(44, 52) << '\n';
	
	printBoard();
	undoMove();
	printBoard();

}
