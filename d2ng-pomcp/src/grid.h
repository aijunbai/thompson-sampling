#ifndef GRID_H
#define GRID_H

#include "coord.h"

template <class T>
class GRID
{
public:

    GRID(int xsize = 0, int ysize = 0)
    :   XSize(xsize),
        YSize(ysize)
    {
        Grid.resize(xsize * ysize);
    }

    void Resize(int xsize, int ysize)
    {
        XSize = xsize;
        YSize = ysize;
        Grid.resize(xsize * ysize);
    }

    int GetXSize() const { return XSize; }
    int GetYSize() const { return YSize; }

    T& operator()(int index)
    {
        assert(index >= 0 && index < XSize * YSize);
        return Grid[index];
    }

    const T& operator()(int index) const
    {
        assert(index >= 0 && index < XSize * YSize);
        return Grid[index];
    }

    T& operator()(const COORD& coord)
    {
        assert(Inside(coord));
        return Grid[Index(coord)];
    }

    const T& operator()(const COORD& coord) const
    {
        assert(Inside(coord));
        return Grid[Index(coord)];
    }

    T& operator()(int x, int y)
    {
        assert(Inside(COORD(x, y)));
        return Grid[Index(x, y)];
    }

    const T& operator()(int x, int y) const
    {
        assert(Inside(COORD(x, y)));
        return Grid[Index(x, y)];
    }

    int Index(const COORD& coord) const
    {
        return XSize * coord.Y + coord.X;
    }

    int Index(int x, int y) const
    {
        assert(Inside(COORD(x, y)));
        return XSize * y + x;
    }

    bool Inside(const COORD& coord) const
    {
        return coord.X >= 0 && coord.Y >= 0
            && coord.X < XSize && coord.Y < YSize;
    }

    int DistToEdge(const COORD& coord, int direction)
    {
        assert(Inside(coord));
        switch (direction)
        {
        case COORD::E_NORTH:
            return YSize - 1 - coord.Y;
        case COORD::E_EAST:
            return XSize - 1 - coord.X;
        case COORD::E_SOUTH:
            return coord.Y;
        case COORD::E_WEST:
            return coord.X;
        default:
            assert(false); return -1;
        }
    }

    void SetAllValues(const T& value)
    {
        for (int x = 0; x < XSize; x++)
            for (int y = 0; y < YSize; y++)
                Grid[Index(x, y)] = value;
    }

    void SetRow(int y, T* values)
    {
        for (int x = 0; x < XSize; x++)
            Grid[Index(x, y)] = values[x];
    }

    void SetCol(int x, T* values)
    {
        for (int y = 0; y < YSize; y++)
            Grid[Index(x, y)] = values[y];
    }

    COORD Coord(int index) const
    {
        assert(index >= 0 && index < XSize * YSize);
        return COORD(index % XSize, index / XSize);
    }

public:
    int XSize, YSize;
    std::vector<T> Grid;
};

template <class T>
inline std::size_t hash_value(const GRID<T> &v) {
    using boost::hash_value;
    using boost::hash_combine;

    // Start with a hash value of 0    .
    std::size_t seed = 0;

    // Modify 'seed' by XORing and bit-shifting in
    // one member of 'Key' after the other:
    hash_combine(seed, boost::hash_value(v.XSize));
    hash_combine(seed, boost::hash_value(v.YSize));
    hash_combine(seed, boost::hash_value(v.Grid));

    // Return the result.
    return seed;
}

#endif // GRID_H
