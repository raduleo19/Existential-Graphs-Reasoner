// Copyright 2019 Rica Radu Leonard
#include <iostream>
#include <string>
#include "./aegraph.h"

int main() {
    std::string input = "([[A]], [[[[A], D], [[A]], N]])";
    AEGraph test = AEGraph(input);
    auto output = test.possible_erasures();
    std::cout << "Got:" << std::endl;
    for (auto v : output) {
        for (auto e : v) {
            std::cout << e << " ";
        }
        std::cout << std::endl;
    }

}
