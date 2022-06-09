#include <iostream>
#include <chrono>
#include <thread>

#include "../shared/datahandler.h"

const std::string filenam = "data.txt";
const std::string confnam = "../shared/data_transf.txt";
const std::string confnam2 = "../shared/data_sliced.txt";
const double minchuva = 5.0;

int main()
{
	std::cout << "Working..." << std::endl;

	auto things = readfile(filenam);

	std::cout << "Read " << things.size() << " items. Converting..." << std::endl;

	//for (auto& i : things) i.transform_chuva_bool_cut(10);
	const auto things2 = get_preds_steps(things, minchuva);

	std::cout << "Exporting..." << std::endl;

	if (writefile(confnam, things) && writefile(confnam2, things2)) std::cout << "Success!" << std::endl;
	else std::cout << "Failed!" << std::endl;

	return 0;
}