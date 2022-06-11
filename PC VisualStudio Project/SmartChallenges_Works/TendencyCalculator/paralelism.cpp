#include "paralelism.h"

void Paralleler::summon(std::function<void(void)> f)
{
	std::lock_guard<std::mutex> l(m);
	thrs.push_back(std::thread(f));
}

void Paralleler::wait_all()
{
	std::lock_guard<std::mutex> l(m);
	for (auto& i : thrs) i.join();
	thrs.clear();
}
