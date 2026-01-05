/*
	Copyright (c) 2026 Devon Artmeier

	Permission to use, copy, modify, and /or distribute this software
	for any purpose with or without fee is hereby granted.

	THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
	WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIE
	WARRANTIES OF MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE
	AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
	DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
	PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#define BUFFER_LENGTH 0x1000
#define ROM_HEADER_LENGTH 0x200
#define MAX_ROM_LENGTH 0x400000
#define MAX_MAPPER_ROM_LENGTH 0x2000000
#define MAPPER_ROM_BANK_LENGTH 0x80000

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int i;
	FILE *fp;
	unsigned char buffer[BUFFER_LENGTH];
	long io_length;
	long original_length;
	long padded_length;
	long remain_length;
	long header_rom_length;
	unsigned short checksum = 0;

	const char *rom_filename = NULL;
	int mapper_mode = 0;
	int dont_pad = 0;
	long pad_value = 0;
	int has_pad_value = 0;

	if (argc < 2) {
		printf("Usage: mdromfix (-m) (-d) (-p [value]) [filename]\n"
		      "            -m         - Set mapper mode\n"
		      "            -d         - Don't apply padding (only alignment)\n"
		      "            -p [value] - Set padding/alignment value (0-255)\n"
		      "            [filename] - ROM filename\n");
		return 0;
	}

	for (i = 1; i < argc; i++) {
		if (strlen(argv[i]) == 2 && argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'm':
					mapper_mode = 1;
					break;

				case 'd':
					dont_pad = 1;
					break;

				case 'p':
					if (has_pad_value) {
						fprintf(stderr, "Padding value already set.\n");
						return 1;
					}

					pad_value = strtol(argv[++i], NULL, 0);
					if (pad_value < 0 || pad_value > 0xFF) {
						fprintf(stderr, "Invalid padding value.\n");
						return 1;
					}
					has_pad_value = 1;
					break;
			}
		} else {
			if (rom_filename) {
				fprintf(stderr, "ROM filename already set.\n");
				return 1;
			}
			rom_filename = argv[i];
		}
	}

	memset(buffer, pad_value, BUFFER_LENGTH);

	if (!rom_filename) {
		fprintf(stderr, "ROM filename not set.\n");
		return 1;
	}

	fp = fopen(rom_filename, "rb+");
	if (!fp) {
		fprintf(stderr, "Failed to open \"%s\".\n", rom_filename);
		return 1;
	}

	if (fseek(fp, 0, SEEK_END)) {
		fprintf(stderr, "Failed to seek to end in \"%s\".\n", rom_filename);
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		}
		return 1;
	}

	original_length = ftell(fp);
	if (original_length == -1L) {
		fprintf(stderr, "Failed to get length of \"%s\".\n", rom_filename);
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		}
		return 1;
	} else if (original_length < ROM_HEADER_LENGTH) {
		fprintf(stderr, "\"%s\" is too small.\n", rom_filename);
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		}
		return 1;
	} else if (original_length > (mapper_mode ? MAX_MAPPER_ROM_LENGTH : MAX_ROM_LENGTH)) {
		fprintf(stderr, "\"%s\" is too large.\n", rom_filename);
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		}
		return 1;
	}

	padded_length = original_length + (original_length & 1);
	if (padded_length < ROM_HEADER_LENGTH) {
		padded_length = ROM_HEADER_LENGTH;
	}
	if (!dont_pad) {
		if (!mapper_mode) {
			padded_length = 1 << ((int)log2(padded_length - 1) + 1);
		} else {
			padded_length += (MAPPER_ROM_BANK_LENGTH - (padded_length % MAPPER_ROM_BANK_LENGTH)) % MAPPER_ROM_BANK_LENGTH;
		}
	}

	remain_length = padded_length - original_length;
	while (remain_length > 0) {
		if ((io_length = fwrite(buffer, 1, (remain_length > BUFFER_LENGTH) ? BUFFER_LENGTH : remain_length, fp)) == 0) {
			fprintf(stderr, "Failed to write to \"%s\".\n", rom_filename);
			if (fclose(fp)) {
				fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
			}
			return 1;
		}
		remain_length -= io_length;
	}

	if (fseek(fp, ROM_HEADER_LENGTH, SEEK_SET)) {
		fprintf(stderr, "Failed to seek to data in \"%s\".\n", rom_filename);
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		}
		return 1;
	}

	remain_length = ((padded_length > MAX_ROM_LENGTH) ? MAX_ROM_LENGTH : padded_length) - ROM_HEADER_LENGTH;
	header_rom_length = (remain_length + ROM_HEADER_LENGTH) - 1;
	while (remain_length > 0) {
		if ((io_length = fread(buffer, 1, (remain_length > BUFFER_LENGTH) ? BUFFER_LENGTH : remain_length, fp)) == 0) {
			fprintf(stderr, "Failed to read from \"%s\".\n", rom_filename);
			if (fclose(fp)) {
				fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
			}
			return 1;
		}
		for (i = 0; i < io_length; i += 2) {
			checksum = (checksum + ((buffer[i] << 8) | buffer[i + 1])) & 0xFFFF;
		}
		remain_length -= io_length;
	}

	if (fseek(fp, 0, SEEK_SET)) {
		fprintf(stderr, "Failed to seek to header in \"%s\".\n", rom_filename);
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		}
		return 1;
	}
	if ((io_length = fread(buffer, 1, ROM_HEADER_LENGTH, fp)) != ROM_HEADER_LENGTH) {
		fprintf(stderr, "Failed to read header from \"%s\".\n", rom_filename);
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		}
		return 1;
	}

	buffer[0x18E] = (checksum >> 8) & 0xFF;
	buffer[0x18F] = checksum & 0xFF;
	buffer[0x1A4] = (header_rom_length >> 24) & 0xFF;
	buffer[0x1A5] = (header_rom_length >> 16) & 0xFF;
	buffer[0x1A6] = (header_rom_length >> 8) & 0xFF;
	buffer[0x1A7] = header_rom_length & 0xFF;

	rewind(fp);
	if ((io_length = fwrite(buffer, 1, ROM_HEADER_LENGTH, fp)) != ROM_HEADER_LENGTH) {
		fprintf(stderr, "Failed to write header to \"%s\".\n", rom_filename);
		if (fclose(fp)) {
			fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		}
		return 1;
	}
	
	if (fclose(fp)) {
		fprintf(stderr, "Failed to close \"%s\".\n", rom_filename);
		return 1;
	}
	return 0;
}