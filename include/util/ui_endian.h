#pragma once


namespace LongUI {
    // helper
    namespace helper {
#ifdef __GNUC__
        // is little endian
        struct is_little_endian { enum : bool { value = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ }; };
        // is big endian
        struct is_big_endian { enum : bool { value = __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ }; };
        // is pdp endian
        struct is_pdp_endian { enum : bool { value = __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__ }; };
#else
        // is little endian
        struct is_little_endian { enum : bool { value = true }; };
        // is big endian
        struct is_big_endian { enum : bool { value = false }; };
        // is pdp endian
        struct is_pdp_endian { enum : bool { value = false }; };
#endif
        // color order
        template<int> struct color_order_ {};
        // color order: le
        template<> struct color_order_<(1 << 0)> { enum { r = 3, g = 2, b = 1, a = 0 }; };
        // color order: be
        template<> struct color_order_<(1 << 1)> { enum { r = 0, g = 1, b = 2, a = 3 }; };
        // color order: pe
        template<> struct color_order_<(1 << 2)> { enum { r = 1, g = 0, b = 3, a = 2 }; };
        // color order: host
        using color_order = color_order_<
            (is_little_endian::value << 0)
            | (is_big_endian::value << 1)
            | (is_pdp_endian::value << 2)
        >;
        // ascii offset
        template<size_t SIZE> struct ascii_offset;
        // ascii offset for 1
        template<> struct ascii_offset<1> { enum { value = color_order::a % 1 }; };
        // ascii offset for 2
        template<> struct ascii_offset<2> { enum { value = color_order::a % 2 }; };
        // ascii offset for 4
        template<> struct ascii_offset<4> { enum { value = color_order::a % 4 }; };
    }
}