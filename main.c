#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define SIZE 21
#define MARKER_SIZE 7

// QR Code generator
// We start with the smallest 21x21 with EC level of L (-> 25 Alphanumeric in total)
// https://www.thonky.com/qr-code-tutorial/module-placement-matrix

enum ec_level {EC_LEVEL_L = 3, EC_LEVEL_M = 2, EC_LEVEL_Q = 1, EC_LEVEL_H = 0};

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

void add_markers(int side_size, bool* qrcode) {
	add_single_marker(side_size, qrcode, 0,0);
	add_single_marker(side_size, qrcode, 0, side_size - MARKER_SIZE);
	add_single_marker(side_size, qrcode, side_size - MARKER_SIZE, 0);

	// calibration lines
	for(int i = MARKER_SIZE + 1; i < side_size - MARKER_SIZE; i += 2) {
		qrcode[i*side_size + MARKER_SIZE - 1] = true;
		qrcode[i + side_size * (MARKER_SIZE - 1)] = true;
	} 
}


// @Incomplete
void add_indicators(int side_size, bool* qrcode, int ec_level, int mask_pattern) {

	// static_value is the 5 bits representing ec_levels and mask_pattern
	int static_value = ec_level << 3 | mask_pattern;

	for(int i = 4; i >= 0; i--) { // @Hardcoded -> 5 bits so 4
		//printf("%d ", 1<<i);
		if((static_value & 1<<i) > 0) {
			qrcode[(MARKER_SIZE + 1) * side_size + (4-i)] = true; // Upper static values
			qrcode[side_size*(side_size-4+i)-(side_size-MARKER_SIZE - 1)] = true; // Lower static values
		}
	}

}



int main(void) {
	// Init qrcode array
	bool qrcode[SIZE*SIZE]  = {};


	add_markers(SIZE, qrcode);
	add_indicators(SIZE, qrcode, EC_LEVEL_L, 4);

	// Display
	for(int i = 0; i < SIZE ; i++) {
		for(int j = 0; j < SIZE ; j++) {
			if(qrcode[i*SIZE+j] == true) {
				printf("# ");
			} else {

				printf(". ");
			}
		}
		printf("\n");

	}
	return 0;
}
