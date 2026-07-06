#pragma once

#ifndef SNVOXENG_EXPORTS

#define SNVOXENG_API __declspec(dllexport)
#define SNVOXENG_API_POD extern "C" __declspec(dllexport)

#else // ^ SNVOXENG_EXPORTS ^

#define SNVOXENG_API __declspec(dllimport)
#define SNVOXENG_API_POD extern "C" __declspec(dllimport)

#endif // ^ ~SNVOXENG_EXPORTS ^
