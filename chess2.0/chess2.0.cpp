#include <iostream>

typedef unsigned long long u64;
typedef uint8_t u8;

//mailbox for movechecking with borders
u8 mailbox[120] = {
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

int main() {
	printBoard();
}
