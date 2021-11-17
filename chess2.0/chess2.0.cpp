#include <iostream>
#include <chrono>

#define DEFAULT "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define PERFT2  "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -" 

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

u64 rRankMask    = 0b0000000100000001000000010000000100000001000000010000000100000001;
u64 lRankMask    = 0b1000000010000000100000001000000010000000100000001000000010000000;


bool side = WHITE;
int fiftyMove = 0;
int moveNum = 1;

const int pieceMoves[4][8] = {
	/*rook*/   {-1,1,8,-8,0,0,0,0},
	/*bishop*/ {-7,7,9,-9,0,0,0,0},
	/*knight*/ {-17,17,15,-15,-10,10,6,-6},
	/*queen*/  {-7,7,9,-9,-1,1,8,-8}
};
const int mailboxMoves[4][8] = {
	/*rook*/   {-1,1,10,-10,0,0,0,0},
	/*bishop*/ {-9,9,11,-11,0,0,0,0},
	/*knight*/ {-21,21,19,-19,-12,12,8,-8},
	/*queen*/  {-9,9,11,-11,-1,1,10,-10}
};

const u8 pieceMoveSizes[4] = {4,4,8,8};
const bool slidingPiece[4] = {1,1,0,1};

u16 test[1024] = {0};

struct move {
	u8 to;
	u8 from;
	u8 moveInfo = 0;
	unsigned int ply;
};

int movePointer = 0;
u8 ep = 0;
u8 castle = 15;

move generated[1024];
move history[100];

void printBoard() {
	unsigned long long pointer = 1;
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
				if (!(lRankMask & pointer >> 7) && black & pointer >> 7)
					addMove(i - 7, i, 1);
				if (!(rRankMask & pointer >> 9) && black & pointer >> 9)
					addMove(i - 9, i, 1);
				if (empty & pointer >> 8)
					addMove(i - 8, i, 0);
				if (pointer & doubleMoveW && empty & pointer >> 16 && empty & pointer >> 8)
					addMove(i - 16, i, 0);
			}
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
			}
		}
		else if (!side && black & pointer) {
			if (pointer & pawns) {
				if (!(lRankMask & pointer << 7) && white & pointer << 7)
					addMove(i + 7, i, 1);
				if (!(rRankMask & pointer << 9) && white & pointer << 9)
					addMove(i + 9, i, 1);
				if (empty & pointer << 8)
					addMove(i + 8, i, 0);
				if (pointer & doubleMoveB && empty & pointer << 16 && empty & pointer << 8)
					addMove(i + 16, i, 0);
			}
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
			}
		}
		pointer <<= 1;
	}
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
	ep = 0;
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
				if (i == '-')
					parameterPointer += 2;
				else
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
			else if (parameterPointer == 6){
				switch (i)
				{
				case '8':
					ep += 8;
				case '7':
					ep += 8;
				case '6':
					ep += 8;
				case '5':
					ep += 8;
				case '4':
					ep += 8;
				case '3':
					ep += 8;
				case '2':
					ep += 8;
				case '1':
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
	loadBoardFromFen(PERFT2);
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

	std::cout << movePointer << '\n';

	std::cout << "\naddmove\n";
	start = std::chrono::steady_clock::now();
	addMove(0,0,0);
	end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() << "ns\n";

	std::cout << "\nnothing\n";
	start = std::chrono::steady_clock::now();
	end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds> (end - start).count() << "ns\n";
}
