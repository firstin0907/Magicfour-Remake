#include "../include/RandomClass.hh"


std::mt19937 RandomClass::generator = std::mt19937(std::random_device()());
