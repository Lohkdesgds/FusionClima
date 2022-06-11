#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <vector>

class Paralleler {
	std::mutex m;
	std::vector<std::thread> thrs;
public:
	void summon(std::function<void(void)>);
	void wait_all();
};