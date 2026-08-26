#pragma once

// SNVOXENG_EXPORTS is defined only by the snvoxeng DLL project itself
// (see snvoxeng.vcxproj -> PreprocessorDefinitions).
// Consumers (app, tests) compile this header without it and get dllimport.
#ifdef SNVOXENG_EXPORTS

#define SNVOXENG_API __declspec(dllexport)
#define SNVOXENG_API_POD extern "C" __declspec(dllexport)

#else // ^ SNVOXENG_EXPORTS ^

#define SNVOXENG_API __declspec(dllimport)
#define SNVOXENG_API_POD extern "C" __declspec(dllimport)

#endif // ^ ~SNVOXENG_EXPORTS ^
