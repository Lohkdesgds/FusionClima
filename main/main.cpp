extern "C" {
    void app_main(void);
}

#include <iostream>

void app_main(void)
{
    std::cout << "Hello world!" << std::endl;
    while(1) {}
}
