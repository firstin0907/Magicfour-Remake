#include "util/RandomClass.hh"


std::mt19937 RandomClass::generator_ = std::mt19937(std::random_device()());
