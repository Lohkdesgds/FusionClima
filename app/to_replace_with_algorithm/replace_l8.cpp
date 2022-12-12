#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <sstream>
#include <Windows.h>

class vecc : public std::vector<std::string> {
public:
	std::string get_safe(const size_t p) { return p >= size() ? "" : *(begin() + p); }
};

// windows chato
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hpInstance, LPSTR nCmdLine, int iCmdShow)
{
	std::cout << "Hello there. Loading up...\n";

	std::stringstream ss(nCmdLine); // main_deploy.py pred "11182013 1300 27.3 80"
	vecc args;

	std::string str;
	std::string comboed;
	while (ss >> str) {
		if (str.size() && str.front() == '\"') {
			comboed += str;
		}
		else if (str.size() && comboed.size() && *str.rbegin() == '\"' && (*(str.rbegin() + 1)) != '\\') {
			comboed += " " + str;
			args.push_back(std::move(comboed));
			comboed = {};
		}
		else if (comboed.size()) {
			comboed += " " + str;
		}
		else {
			args.push_back(str);
		}
	}

	const int argc = args.size();

	if (args.size() != 3) {
		std::cout << "Arguments failed: call:\n";
		for (int a = 0; a < argc; ++a) {
			std::cout << a << " -> " << args[a] << std::endl;
		}
		return -1;
	}

	std::cout << "Arguments loaded. Thinking\n";

	//for (int a = 1; a < argc; ++a) {
	//	args.push_back(argv[a]);
	//}

	//if (args.get_safe(0) == "add") {
	// args: day time hum temp (invertido pq assim foi feito no python, tah difícil)
	std::cout << "Args passed. Opening file...\n";

	std::fstream fp("pred_history.txt", std::ios::binary | std::ios::out);
	if (!fp) return 1;

	char varsss[4][96];

	sscanf_s(args[argc - 1].c_str(), "\"%s %s %s %s\"", varsss[0], 96, varsss[1], 96, varsss[2], 96, varsss[3], 96);


	fp << varsss[2] << " " << varsss[3] << "\nDEBUG: date hour " << varsss[0] << " " << varsss[1] << "\nrandom text just to ikr idc lmao [";

	float prev = powf(std::atof(varsss[3]) * 0.01f + 0.1f, 2.0f);
	if (prev > 1.0f) prev = 1.0f;

	fp << (100.0f * prev) << "]";

	fp.close();

	std::cout << "All good!\n";
	//}
	//if (args.get_safe(0) == "show") {
	//	std::fstream fp("dump.txt", std::ios::binary | std::ios::in);
	//	float vals[2];
	//
	//	fp >> vals[0] >> vals[1];
	//
	//	float prev = powf(vals[1] * 0.01f + 0.1f, 2.0f);
	//	if (prev > 1.0f) prev = 1.0f;
	//
	//	std::cout << vals[0] << " " << vals[1] << " " << (prev * 100.0f);
	//}

}