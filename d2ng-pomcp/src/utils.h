#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "coord.h"
#include "memorypool.h"
#include <algorithm>
#include <iostream>
#include <map>

#define LargeInteger 1000000
#define Infinity 1e+10
#define Tiny 1e-10

#ifdef DEBUG
#define safe_cast dynamic_cast
#else
#define safe_cast static_cast
#endif

#define PRINT_ERROR(error) \
    do { \
        std::cerr << __FILE__ << ":" << __LINE__ << " : " << error << std::endl; \
    } while(0)

#define PRINT_VALUE(x) \
    do { \
    	std::cerr << #x " = '" << x << "'" << std::endl; \
    } while(0)

namespace UTILS
{

inline int Sign(int x)
{
    return (x > 0) - (x < 0);
}

inline bool Near(double x, double y, double tol)
{
    return fabs(x - y) <= tol;
}

inline bool CheckFlag(int flags, int bit) { return (flags & (1 << bit)) != 0; }

inline void SetFlag(int& flags, int bit) { flags = (flags | (1 << bit)); }

template<class T>
inline bool Contains(std::vector<T>& vec, const T& item)
{
    return std::find(vec.begin(), vec.end(), item) != vec.end();
}

void UnitTest();

}

#endif // UTILS_H
