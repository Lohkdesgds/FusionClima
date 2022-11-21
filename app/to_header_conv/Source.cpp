#include <iostream>
#include <fstream>

constexpr size_t buf = 4096;

int main(int argc, char* argv[])
{
	if (argc != 2) return 1;

	std::fstream f(argv[1], std::ios::binary | std::ios::in);
	if (!f || f.bad()) return 2;

	std::fstream o("out.h", std::ios::binary | std::ios::out);
	if (!o || o.bad()) return 3;

	o << "#pragma once\n#include<stdint.h>\nnamespace FileSpace {constexpr uint8_t fp[]={\n";
	o << std::hex;

	std::cout << "Begin.\n";

	while (!f.eof() && f) {
		uint8_t blk[buf];
		const auto _rd = f.read((char*)&blk, std::size(blk)).gcount();
		for(size_t p = 0; p < _rd; ++p)
			o << "0x" << std::hex << (uint16_t)blk[p] << ",";
	}
	o.seekp(-1, std::ios::cur);
	o << "};\n}\n";
	f.close();
	o.close();

	std::cout << "End.\n";
	std::cin.get();
	return 0;
}