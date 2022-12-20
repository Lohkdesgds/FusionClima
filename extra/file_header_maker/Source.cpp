#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cout << "One file at a time please. Drop a file into the exe to create a file_header.h" << std::endl;
		return 0;
	}

	std::ifstream in(argv[1], std::ios::binary);
	if (!in || in.bad()) {
		std::cout << "Could not open input." << std::endl;
		return 1;
	}

	std::ofstream out("file_header.h", std::ios::binary);
	if (!out || out.bad()) {
		std::cout << "Could not open output." << std::endl;
		return 1;
	}

	out << "#pragma once\n\ninline const char fp[] = {";

	char buf[512]{};
	in.seekg(0, std::ios::end);
	const size_t endd = in.tellg();
	size_t curr = 0;
	in.seekg(0, std::ios::beg);

	std::thread thr([&] {
		do {
			printf_s("Copying... %.1f%% [%zu of %zu]  \r", (100.0f * curr / endd), curr, endd);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		} while (curr < endd);
		std::cout << std::endl;
	});

	while (in && in.good() && !in.eof())
	{
		const size_t readd = in.read(buf, std::size(buf)).gcount();
		curr += readd;
		for (size_t p = 0; p < readd; ++p) {
			out << "0x" << std::hex << (uint16_t)((uint8_t)buf[p]) << ",";
		}
	}
	curr = endd;
	thr.join();

	out.seekp(static_cast<size_t>(out.tellp()) - 1);
	out << "};";

	in.close();
	out.close();

	std::cout << "End!" << std::endl;

	std::this_thread::sleep_for(std::chrono::seconds(5));
	return 0;
}