/*File: utf8validator.c*/

#include <stdio.h>
#define CLEAR 0
#define INVALID_HEADER 1
#define INVALID_TAIL 2
#define INVALID_CODEPOINT 3
#define OVERSIZED_CODEPOINT 4
#define EARLY_EOF 5

int main(void) {
	int ch, ascii_count = 0, multi_byte_count = 0, error_type = CLEAR, tail_bytes = 0, codepoint = 0, size; /*current character, amount of ascii characters detected, amount of multi-byte characters detected, error type, amount of tail bytes expected, codepoint calculated, size (amount of bytes used for encoding the current character)*/
	while ((ch = getchar()) != EOF) {                  /*read the input byte by byte and stop when you find EOF*/
		if (ch < 0x80) {                               /*if ch < 0x80 == 10000000, then it's of the form 0xxxxxxx. So it's a 1-byte (ascii-type) character*/
			++ascii_count;                             /*Add 1 to the ascii count*/
			if (tail_bytes) {                          /*If more tail bytes are expected*/
				error_type = INVALID_TAIL;             /*Set error type to "invalid tail byte"*/
				break;                                 /*Exit the loop*/
			}
		}
		else if (ch < 0xC0) {                          /*Since ch >= 0x80, if also ch < 0xC0 (== 11000000), then it has the form 10xxxxxx. So it's a tail byte*/
			if (!tail_bytes) {                         /*If there are no tail bytes expected*/
				error_type = INVALID_HEADER;           /*Set error type to "invalid header byte"*/
				break;                                 /*Exit the loop*/
			}
			--tail_bytes;                              /*Lessen tail bytes expected by 1*/
			codepoint <<= 6;                           /*Slide previous code point 6 places to the left*/
			codepoint += (ch & 0x3F);                  /*Add to it the current codepoint (the last 6 bits, so the byte with mask 0x3F==00111111)*/
		}
		else if (ch < 0xE0) {                          /*Since ch >= 0xC0, if also ch < 0xE0 (== 11100000), then it has the form 110xxxxx. So it's the header byte of a 2-byte character*/
			if (tail_bytes) {                          /*If more tail bytes are expected set the appropriate error type and exit the loop*/
				error_type = INVALID_TAIL;
				break;
			}
			size = 2;                                  /*Set amount of bytes used for the current character encoding to 2*/
			codepoint = (ch & 0x1F);                   /*Set the current codepoint as the last 5 bits (0x1F==00011111)*/
			++multi_byte_count;                        /*Add 1 to the multi-byte count*/
			tail_bytes = 1;                            /*Set expected tail bytes to 1*/
		}
		else if (ch < 0xF0) {                          /*Since ch >= 0xE0, if also ch < 0xF0 (== 11110000), then it has the form 1110xxxx. So it's the header byte of a 3-byte character*/
			if (tail_bytes) {                          /*If more tail bytes are expected set the appropriate error type and exit the loop*/
				error_type = INVALID_TAIL;
				break;
			}
			size = 3;                                  /*Set amount of bytes used for the current character encoding to 3*/
			codepoint = (ch & 0x0F);                   /*Set the current codepoint as the last 4 bits (0x0F==00001111)*/
			++multi_byte_count;                        /*Add 1 to the multi-byte count*/
			tail_bytes = 2;                            /*Set expected tail bytes to 2*/
		}
		else if (ch < 0xF8) {                          /*Since ch >= 0xF0, if also ch < 0xF8 (== 11111000), then it has the form 11110xxx. So it's the header byte of a 4-byte character*/
			if (tail_bytes) {                          /*If more tail bytes are expected set the appropriate error type and exit the loop*/
				error_type = INVALID_TAIL;
				break;
			}
			size = 4;                                  /*Set amount of bytes used for the current character encoding to 4*/
			codepoint = (ch & 0x07);                   /*Set the current codepoint as the last 3 bits (0x07==00001111)*/
			++multi_byte_count;                        /*Add 1 to the multi-byte count*/
			tail_bytes = 3;                            /*Set expected tail bytes to 3*/
		}
		else {                                         /*if ch >= 0xF8 (== 11111000), then it has the form 11111xxx. So it's an invalid byte.*/
			if (tail_bytes) {                          /*If tail bytes are expected*/
				error_type = INVALID_TAIL;             /*Set error type to "invalid tail byte"*/
				break;                                 /*Exit the loop*/
			}
			else {                                     /*If no tail bytes are expected*/
				error_type = INVALID_HEADER;           /*Set error type to "invalid header byte"*/
				break;                                 /*Exit the loop*/
			}
		}
		if (!tail_bytes) {                             /*It we expect no more tail bytes, we have constructed a full code point*/
			if (((codepoint >= 0xD800) && (codepoint <= 0xDFFF)) || (codepoint > 0x10FFFF)) { /*If the code point is in the invalid range*/
				error_type = INVALID_CODEPOINT;       /*Set error type to "invalid code point"*/
				break;                                /*Exit the loop*/
			}
			if ((((size == 2) && (codepoint < 0x0080)) || ((size == 3) && (codepoint < 0x0800)) || (size == 4) && (codepoint < 0x010000))) { /*If the size is bigger than the one needed for the code point*/
				error_type = OVERSIZED_CODEPOINT;     /*Set error type to "oversized code point"*/
				break;                                /*Exit the loop*/
			}
		}
	}
	if (tail_bytes && (!error_type)) {                /*If the loop ends with no error, and more tail bytes were expected, then an EOF was encountered at a wrong point*/
		error_type = EARLY_EOF;                       /*Set error type to "unexpected EOF"*/
	}
	switch (error_type) {                             /*Print the amount of ASCII and multi-byte UTF-8 characters it there was no error, else print the type of error and where it was found*/
		case CLEAR:
			printf("Found %d ASCII and %d multi-byte UTF-8 characters.\n", ascii_count, multi_byte_count);
			break;
		case INVALID_HEADER:
			printf("Invalid UTF-8 header byte: 0x%02X\n", ch);
			return 1;
		case INVALID_TAIL:
			printf("Invalid UTF-8 tail byte: 0x%02X\n", ch);
			return 2;
		case INVALID_CODEPOINT:
			printf("Invalid UTF-8 code point: 0x%04X\n", codepoint);
			return 4;
		case OVERSIZED_CODEPOINT:
			printf("Oversized UTF-8 code point: 0x%04X\n", codepoint);
			return 3;
		case EARLY_EOF:
			printf("Unexpected EOF\n");
			return 5;
	}
	return 0;                                         /*If the program finishes with no error, return 0*/
}
