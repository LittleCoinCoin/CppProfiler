#pragma once

#ifdef PROFILE_LIB_SHARED
#define PROFILE_API __declspec(dllexport)
#else
#define PROFILE_API __declspec(dllimport)
#endif