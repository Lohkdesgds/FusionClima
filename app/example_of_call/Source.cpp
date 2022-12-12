#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <Lunaris-Process/process.h>

int main(int argc, char* argv[])
{
	if (argc > 1) {
		std::cout << "Called with arguments:\nCall argument list:\n\n";
		for (int p = 0; p < argc; ++p) {
			std::cout << "[" << (p) << "] " << argv[p] << std::endl;
		}
		return 0;
	}

	while (1) {
		std::cout << "> ";

		std::string buf;
		std::getline(std::cin, buf);

		std::stringstream ss(buf);
		std::vector<std::string> margs;

		{
			std::string tmp;
			while (ss >> tmp) {
				margs.push_back(tmp);
			}
		}
		for (auto& i : margs) {
			size_t p = 0;
			while ((p = i.find("\\", p)) != std::string::npos) {
				i.erase(i.begin() + p);
				++p;
			}
		}

		if (margs.empty()) continue;

		std::string appnam = margs.front();
		margs.erase(margs.begin());

		std::cout << "=================================\nCall: " << appnam;

		size_t _c = 0;
		for (const auto& i : margs) {
			std::cout << " " << i;
		}

		//margs.insert(margs.begin(), "/c");

		std::cout << "\n\nApp response:\n\n";

		//Lunaris::process_sync proc("cmd", margs, Lunaris::process_sync::mode::READWRITE);
		try {
			Lunaris::process_sync proc(appnam, margs, Lunaris::process_sync::mode::READWRITE);

			while (proc.is_running()) std::this_thread::sleep_for(std::chrono::milliseconds(100));

			if (proc.has_read()) {
				_c = 0;
				while (proc.has_read()) {
					std::cout << "Line #" << std::setfill('0') << std::setw(4) << ++_c << " | " << proc.read() << std::endl;
				}
			}
		}
		catch (const std::exception& e) {
			std::cout << "Error: " << e.what() << std::endl;
		}
		catch (...) {
			std::cout << "Error: unknown" << std::endl;
		}

		std::cout << "=================================\n";
	}

}