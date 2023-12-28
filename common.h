#ifndef COMMON_H
#define COMMON_H
#include <iostream>
#include "myqueue.h"
#include <future>
#include <boost/algorithm/string.hpp>

using PromiseType=std::promise<std::string>;
using UnitType= std::pair<std::string, PromiseType>;



#endif // COMMON_H
