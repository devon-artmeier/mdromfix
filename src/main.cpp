/*
	Copyright (c) 2025 Devon Artmeier

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

#include "shared.hpp"

int main(int argc, char* argv[])
{
	if (argc < 1) {
		std::cout << "Usage: mdromfix <-q> <-d> <-p [pad value]> [rom file]" << std::endl << std::endl <<
		             "           <-q>             - Quiet mode" << std::endl <<
		             "           <-d>             - Don't pad" << std::endl <<
		             "           <-p [pad value]> - Pad byte value" << std::endl <<
		             "           [rom file]       - ROM file" << std::endl << std::endl;
		return -1;
	}

	try {
		std::string input_file    = "";
		int         pad_value     = 0;
		bool        pad_value_set = false;
		bool        quiet_mode    = false;
		bool        dont_pad      = false;

		for (int i = 1; i < argc; i++) {
			if (StringToLower(argv[i]).compare("-q") == 0) {
				quiet_mode = true;
				continue;
			}

			if (StringToLower(argv[i]).compare("-s") == 0) {
				dont_pad = true;
				continue;
			}

			if (CheckArgument(argc, argv, i, "p")) {
				if (pad_value_set) {
					throw std::runtime_error("Pad value already defined.");
				}

				try {
					pad_value = std::stoi(argv[i]);
				} catch (...) {
					throw std::runtime_error(("Invalid pad value \"" + (std::string)argv[i] + "\".").c_str());
				}
				if (pad_value < 0 || pad_value > 255) {
					throw std::runtime_error(("Invalid pad value \"" + (std::string)argv[i] + "\".").c_str());
				}

				pad_value_set = true;
				continue;
			}

			if (!input_file.empty()) {
				throw std::runtime_error("ROM file already defined.");
			}
			input_file = argv[i];
		}

		if (input_file.empty()) {
			throw std::runtime_error("ROM file not defined.");
		}

		std::ifstream input(input_file, std::ios::in | std::ios::binary);
		if (!input.is_open()) {
			throw std::runtime_error(("Cannot open \"" + input_file + "\" for reading.").c_str());
		}

		input.seekg(0, std::ios::end);
		t_ull size = input.tellg();
		input.seekg(0, std::ios::beg);
		
		t_ull padded_size = 1ULL << (static_cast<int>(std::log2(size - 1)) + 1);
		if (dont_pad) {
			padded_size = size + (size & 1);
		}

		if (padded_size <= 0x200) {
			throw std::runtime_error(("\"" + input_file + "\" is too small.").c_str());
		}

		t_uc* buffer = new t_uc[padded_size];
		memset(buffer, pad_value, padded_size);

		input.read(reinterpret_cast<char*>(buffer), size);
		input.close();

		t_ull reported_start = (static_cast<t_ull>(buffer[0x1A0]) << 24) |
		                       (static_cast<t_ull>(buffer[0x1A1]) << 16) |
		                       (static_cast<t_ull>(buffer[0x1A2]) << 8) |
		                       static_cast<t_ull>(buffer[0x1A3]);
		t_ull reported_end   = (static_cast<t_ull>(buffer[0x1A4]) << 24) |
		                       (static_cast<t_ull>(buffer[0x1A5]) << 16) |
		                       (static_cast<t_ull>(buffer[0x1A6]) << 8) |
		                        static_cast<t_ull>(buffer[0x1A7]);
		t_ull reported_size  = reported_end - reported_start + 1;
		
		t_us  old_checksum   = (static_cast<t_us>(buffer[0x18E]) << 8) | static_cast<t_us>(buffer[0x18F]);
		t_us  new_checksum   = 0;

		t_ull limited_size   = std::min(padded_size, 0x400000ULL);

		for (t_ull i = 0x200; i < limited_size; i += 2) {
			new_checksum += (buffer[i] << 8) | buffer[i + 1];
		}

		buffer[0x18E] = static_cast<t_uc>(new_checksum >> 8);
		buffer[0x18F] = static_cast<t_uc>(new_checksum);

		buffer[0x1A0] = 0;
		buffer[0x1A1] = 0;
		buffer[0x1A2] = 0;
		buffer[0x1A3] = 0;

		buffer[0x1A4] = static_cast<t_uc>((limited_size - 1) >> 24);
		buffer[0x1A5] = static_cast<t_uc>((limited_size - 1) >> 16);
		buffer[0x1A6] = static_cast<t_uc>((limited_size - 1) >> 8);
		buffer[0x1A7] = static_cast<t_uc>(limited_size - 1);

		std::ofstream output(input_file, std::ios::out | std::ios::binary);
		if (!output.is_open()) {
			throw std::runtime_error(("Cannot open \"" + input_file + "\" for writing.").c_str());
		}
		output.write(reinterpret_cast<char*>(buffer), padded_size);
		output.close();

		if (!quiet_mode) {
			std::cout << "Reported ROM start: " << std::uppercase << std::hex << "0x" << reported_start << std::endl <<
				         "Reported ROM end:   " << "0x" << reported_end << std::endl <<
				         "New ROM start:      0x0" << std::endl <<
				         "New ROM end:        0x" << limited_size - 1 << std::endl <<
				         "Old checksum:       0x" << old_checksum << std::endl <<
				         "New checksum:       0x" << new_checksum << std::endl <<
				         "Old file size:      " << std::dec << size << " (" << std::hex << "0x" << size << ") byte(s)" << std::endl <<
				         "New file size:      " << std::dec << padded_size << " (" << std::hex << "0x" << padded_size << ") byte(s)";
		}
	} catch (std::exception& e) {
		std::cout << "Error: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}