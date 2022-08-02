#include <stdio.h>
#include <stdbool.h>

#define SIZE 21
#define MARKER_SIZE 7

// QR Code genrator
// We start with the smallest 21x21 with ECC level of L (-> 25 Alphanumeric in total)

void add_single_marker(int side_size, bool* qrcode, int x, int y) {
	for(int i = 0; i < MARKER_SIZE ; i++) {
		for(int j = 0; j < MARKER_SIZE ; j++) {
			if((i == 0 || j == 0 || i == MARKER_SIZE - 1 || j==MARKER_SIZE - 1) || 
					(i != 1 && i != MARKER_SIZE -2 && j != 1 && j!= MARKER_SIZE - 2)) {
				qrcode[j+i*side_size + (x*side_size + y)] = true;
			}
		}
	}

}

void add_markers(int side_size, bool *qrcode) {
	add_single_marker(side_size, qrcode, 0,0);
	add_single_marker(side_size, qrcode, 0,14);
	add_single_marker(side_size, qrcode, 14, 0);

}



int main(void) {
	// Init qrcode array
	bool qrcode[SIZE*SIZE]  = {};


	add_markers(SIZE, qrcode);
	// Display
	for(int i = 0; i < SIZE ; i++) {
		for(int j = 0; j < SIZE ; j++) {
			printf("%d ", qrcode[i*SIZE+j]);
		}
		printf("\n");

	}
	return 0;
}
