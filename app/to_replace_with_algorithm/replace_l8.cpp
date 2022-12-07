#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>

class vecc : public std::vector<std::string> {
public:
	std::string get_safe(const size_t p) { return p >= size() ? "" : *(begin() + p); }
};

int main(int argc, char* argv[])
{
	vecc args;
	for (int a = 1; a < argc; ++a) {
		args.push_back(argv[a]);
	}

	if (args.get_safe(0) == "add") {
		if (args.size() != 3) return 2;

		std::fstream fp("dump.txt", std::ios::binary | std::ios::out);
		if (!fp) return 1;

		fp << args.get_safe(1) << " " << args.get_safe(2);
		fp.close();
	}
	if (args.get_safe(0) == "show") {
		std::fstream fp("dump.txt", std::ios::binary | std::ios::in);
		float vals[2];

		fp >> vals[0] >> vals[1];

		float prev = powf(vals[1] * 0.01f + 0.1f, 2.0f);
		if (prev > 1.0f) prev = 1.0f;

		std::cout << vals[0] << " " << vals[1] << " " << (prev * 100.0f);
	}

}