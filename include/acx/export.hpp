// SPDX-License-Identifier: Apache-2.0
#pragma once
#if defined(_WIN32) && !defined(__GNUC__)
  #if defined(ACX_BUILDING_LIBRARY)
    #define ACX_API __declspec(dllexport)
  #else
    #define ACX_API __declspec(dllimport)
  #endif
#else
  #define ACX_API __attribute__((visibility("default")))
#endif
