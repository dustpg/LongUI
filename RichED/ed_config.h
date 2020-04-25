#pragma once

// modify this config-file if you want

// x86 CPU cache-line = 64
#define RED_LUT_ALIGNED alignas(64)
#define RED_RICHED_ALIGNED alignas(8)

#ifndef PCN_NOINLINE

#ifndef PCN_NOVTABLE
#ifdef _MSC_VER
#define PCN_NOVTABLE _declspec(novtable)
#else
#define PCN_NOVTABLE
#endif
#endif

#ifndef PCN_NOINLINE
#ifdef _MSC_VER
#define PCN_NOINLINE _declspec(noinline)
#else
#define PCN_NOINLINE
#endif
#endif

#endif

//#define RED_UNDOREDO

#include <cstdint>

//#include <new>

namespace RichED {
    // constant
    enum {
#ifdef NDEBUG
        // fixed length, 1 for surrogate
        TEXT_CELL_STR_MAXLEN = 63,
#else
        // fixed length, 1 for surrogate
        TEXT_CELL_STR_MAXLEN = 9,
#endif
        // merge min len
        TEXT_MERGE_LEN = 4,
    };
    // small object
    struct /*alignas()*/ CEDSmallObject {

    };
    // cell context, init-ed as null
    struct CellContext {
        // context
        void*       context;
    };


    // unit used in riched
    using unit_t = float;
    // line offset type
    //using lineoff_t = float;
    // get as unit
    constexpr unit_t unit_one(long double x) noexcept { return float(x); }
    // get as max
    constexpr unit_t max_unit() noexcept { return float(1e10); }
    // times
    inline unit_t times(unit_t a, uint32_t x) noexcept { return float(x) * a; }
    // half
    inline unit_t half(unit_t a) noexcept { return a * 0.5f; }
    // half
    inline unit_t mask(unit_t a) noexcept { return a * 0.5f; }
}


