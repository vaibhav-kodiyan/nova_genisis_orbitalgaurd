#pragma once // to avoid multiple initializations
using namespace std;

// centralized header file
#include <vector>
#include <array>
#include <string>
#include <cmath>
#include <numeric>
#include <limits>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cctype>
#include <iomanip>
#include <new>

// threshold distance in km for conjunction screening
const double THRESHOLD_DISTANCE = 100.0;

// basic time constants
const double SECONDS_PER_DAY = 86400.0;
const double MINUTES_PER_DAY = 1440.0;
