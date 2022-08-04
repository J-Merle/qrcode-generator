#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define SIZE 21
#define MARKER_SIZE 7

// QR Code generator
// We start with the smallest 21x21 with EC level of L (-> 25 Alphanumeric in total)
// https://www.thonky.com/qr-code-tutorial/module-placement-matrix
//
//
// Improvements : stop using bool* because sizeof(bool) is 1 so we lose 7bits
// Cosider using this solution : https://stackoverflow.com/questions/2525310/how-to-define-and-work-with-an-array-of-bits-in-c
#define SetBit(A,k)     ( A[(k)/8] |= (1 << ((k)%8)) )
#define TestBit(A,k)    ( A[(k)/8] & (1 << ((k)%8)) )

enum ec_level {EC_LEVEL_L = 3, EC_LEVEL_M = 2, EC_LEVEL_Q = 1, EC_LEVEL_H = 0};
enum mode {NUMERIC = 1, ALPHA = 2, BYTE = 4, KANJI = 8, ECI = 7};

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
		if((static_value & 1<<i) > 0) {
			qrcode[(MARKER_SIZE + 1) * side_size + (4-i)] = true; // Upper static values
			qrcode[side_size*(side_size-4+i)-(side_size-MARKER_SIZE - 1)] = true; // Lower static values
		}
	}

}

void persist_data_in_qrcode(int data_size, uint8_t* data, int side_size, bool* qrcode, bool* reserved) {
	int x_offset = side_size -1;
	int y_offset = side_size -1;
	int local_write_offset = 0;
	int x_direction = -1; // We start by reading bottom to top
	int data_index = 0;
	while(data_index < data_size && y_offset >= 0) {
		// Always skip vertical timing bar as we can not write on it
		if(y_offset == 6) {
			y_offset--;
			continue;
		}

		int pos = x_offset * side_size + y_offset + local_write_offset;
		if(!reserved[pos]) {
			qrcode[pos] = TestBit(data, data_index);
			data_index++;
		}
		local_write_offset = (local_write_offset + 1) *-1;
		if(local_write_offset == 0) {
			x_offset += x_direction;
			if(x_offset < 0 || x_offset >= side_size) {
				x_offset -= x_direction; // We went too far, go back
				y_offset -= 2; 
				x_direction *= -1; // Reverse the writting direction
			}
		}

	}

}

// @Incomplete : add all masks
void apply_mask(int side_size, bool* qrcode, bool* reserved) {
	for(int i = 0; i < side_size; i++) {
		for(int j = 0; j < side_size; j++) {
			int pos = i*side_size + j;
			if(!reserved[pos] && (i+j)%3 == 0) {
				qrcode[pos] = !qrcode[pos];
			}
		}

	}

}

// @Bug we might want to check for overflows
void write_single_number(int array_size, uint8_t* array, int number, uint8_t number_of_bits, uint16_t* offset) {

	for(int i = 0; i <= number_of_bits - 1; i++) { 
		if((1 << (number_of_bits-1-i)) & number) {
			SetBit(array,  *offset);
		}
		(*offset)++;
	}

}

// @Incomplete add other special aplha char
int get_alpha(char c) {
	if(c >= 65 && c <= 90) {return c - 55;}

	int v = 0;
	switch(c) {
		// Space 
		case 32:
			v = 36;
			break;
		case '$':
			v = 37;
			break;
		case '%':
			v = 38;
			break;
		case '*':
			v = 39;
			break;
		case '+':
			v = 40;
			break;
		case '-':
			v = 41;
			break;
		case '.':
			v = 42;
			break;
		case '/':
			v = 43;
			break;
		case ':':
			v = 44;
			break;
	}
	return v;
			
}

// @Incomplete only handle V1 and alpha mode
void format_and_fill_data_in_qrcode(int data_size, const char* input_data, int side_size, bool* qrcode, bool* reserved) {

	int text_required_bits =data_size % 2 == 0 ? data_size / 2 * 11 : (data_size - 1) / 2 * 11 + 6 ;
	int needed_space = 4 + 9 + text_required_bits; // @Hardcoded 4=mode size and 9=character count size if V<=9
	while(needed_space % 8 != 0) {
		needed_space++;
	}
	uint8_t* created_data =  malloc(needed_space / 8);

	uint16_t offset = 0;

	// Add mode size
	write_single_number(needed_space, created_data, ALPHA, 4, &offset);
	// Add char count
	write_single_number(needed_space, created_data, 11, 9, &offset);

	for (int i = 0; i < data_size; i += 2) {
		char current = input_data[i];
		if (i + 1 < data_size) {
			char next = input_data[i + 1];
			write_single_number(needed_space, created_data, (45* get_alpha(current)) + get_alpha(next), 11, &offset);

		} else {
			write_single_number(needed_space, created_data, get_alpha(current), 6, &offset);
		}
	}

	persist_data_in_qrcode(needed_space, created_data, SIZE, qrcode, reserved);
	free(created_data);
	apply_mask(SIZE, qrcode, reserved);
}

int main(void) {
	// Init qrcode array
	bool qrcode[SIZE*SIZE]  = {}; // The final qrcode
	bool reserved[SIZE*SIZE]  = {}; // A temporary array showing reserved layer

	add_markers(SIZE, qrcode, reserved);
	add_indicators(SIZE, qrcode, reserved, EC_LEVEL_L, 6);

	format_and_fill_data_in_qrcode(15, "HTTP://GOOGLE.C", SIZE, qrcode, reserved);

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
