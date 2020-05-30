#pragma once

#define LUI_COMPILER_UNK    0
#define LUI_COMPILER_GCC    1
#define LUI_COMPILER_MSVC   2
#define LUI_COMPILER_CLANG  3


#if defined(__clang__)
#define LUI_COMPILER LUI_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define LUI_COMPILER LUI_COMPILER_GCC
#elif defined(_MSC_VER)
#define LUI_COMPILER LUI_COMPILER_MSVC
#else
#define LUI_COMPILER LUI_COMPILER_UNK
#endif


#if (LUI_COMPILER == LUI_COMPILER_MSVC)
#if _MSC_VER < 1900
#ifdef __cplusplus
#define noexcept
#define constexpr const
#define alignas(x)
#define alignof(x) sizeof(void*)
#endif
#define __restrict
#endif
#endif

#define UNICALL __stdcall
#define luiref

#ifndef PCN_NOINLINE

#if  LUI_COMPILER == LUI_COMPILER_MSVC
#define PCN_FOINLINE __forceinline
#define PCN_NOINLINE __declspec(noinline)
#define PCN_NOVTABLE __declspec(novtable)
#define PCN_DLLEXPRT
#else
#define PCN_FOINLINE __inline__ __attribute__((always_inline))
#define PCN_NOINLINE __attribute__((noinline))
#define PCN_NOVTABLE
#define PCN_DLLEXPRT
#endif

#define PCN_DLL_PUBLIC  PCN_DLLEXPRT
#define PCN_DLL_PROTECT

#endif