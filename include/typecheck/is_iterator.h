#pragma once
#include <type_traits>
#include <iterator>

// ui
namespace LongUI {
    // type healper
    namespace type_helper {
        // is_iterator: FALSE
        template<typename U, typename = void>
        struct is_iterator { enum : bool { value = false }; };
        // is_iterator: TRUE
        template<typename U>
        struct is_iterator<U, typename std::enable_if<!std::is_same<typename
            std::iterator_traits<U>::value_type, void>::value>::type>
        {
            enum : bool { value = true };
        };
    }
}