#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define SIZE 21
#define MARKER_SIZE 7

// QR Code generator
// We start with the smallest 21x21 with EC level of L (-> 25 Alphanumeric in total)
// https://www.thonky.com/qr-code-tutorial/module-placement-matrix

enum ec_level {EC_LEVEL_L = 3, EC_LEVEL_M = 2, EC_LEVEL_Q = 1, EC_LEVEL_H = 0};

void add_single_marker(int side_size, bool* qrcode, bool* reserved, int x, int y) {
	int reserved_size_x = MARKER_SIZE;	
	int i = 0;

	if(x == 0) {
		reserved_size_x +=2;	
	} else {

		i = -1;
	}

	for(i; i < reserved_size_x ; i++) {
		int reserved_size_y = MARKER_SIZE;	
		int j = 0;
		if(y == 0) {
			reserved_size_y +=2;
		} else {
			j = -1;
		}
		for(j; j < reserved_size_y ; j++) {

			if(((i == 0 || j == 0 || i == MARKER_SIZE - 1 || j==MARKER_SIZE - 1) || 
					(i != 1 && i != MARKER_SIZE -2 && j != 1 && j!= MARKER_SIZE - 2)) &&
					j < MARKER_SIZE && i < MARKER_SIZE && i >= 0 && j >= 0) {
				qrcode[j+i*side_size + (x*side_size + y)] = true;
			}

			reserved[j+i*side_size + (x*side_size + y)] = true;
		}
	}
}


// @Incomplete for  V >= 2 add alignement patterns
void add_markers(int side_size, bool* qrcode, bool* reserved) {
	add_single_marker(side_size, qrcode, reserved, 0,0);
	add_single_marker(side_size, qrcode, reserved, 0, side_size - MARKER_SIZE);
	add_single_marker(side_size, qrcode, reserved, side_size - MARKER_SIZE, 0);

	// Timing lines
	for(int i = MARKER_SIZE + 1; i < side_size - MARKER_SIZE; i++) {
		// Real timing line
		if(i%2 == 0) {
			qrcode[i*side_size + MARKER_SIZE - 1] = true;
			qrcode[i + side_size * (MARKER_SIZE - 1)] = true;
		}

		// Reserved area
		reserved[i*side_size + MARKER_SIZE - 1] = true;
		reserved[i + side_size * (MARKER_SIZE - 1)] = true;
	} 

	// Dark Module
	qrcode[(4*1 + 9) *side_size + 8] = true; // @Hardcoded : 4*1 should be 4 * V where V is the QRCode version
}


// @Incomplete for  V >= 7 reserve the special 3x6 bloc
void add_indicators(int side_size, bool* qrcode, bool* reserved, int ec_level, int mask_pattern) {


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
	bool qrcode[SIZE*SIZE]  = {}; // The final qrcode
	bool reserved[SIZE*SIZE]  = {}; // A temporary array showing reserved array


	add_markers(SIZE, qrcode, reserved);
	add_indicators(SIZE, qrcode, reserved, EC_LEVEL_L, 4);

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
	printf("\n");

	for(int i = 0; i < SIZE ; i++) {
		for(int j = 0; j < SIZE ; j++) {
			if(reserved[i*SIZE+j] == true) {
				printf("# ");
			} else {

				printf(". ");
			}
		}
		printf("\n");

	}
	return 0;
}
