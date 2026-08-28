/*
_X(name, Name, ext, VK_TYPE, shaderc_type)
* VK_TYPE -> VK_SHADER_STAGE_ ## VK_TYPE ## _BIT
* shaderc_type -> shaderc_ ## shaderc_type ## _shader

_XKHR(name, Name, ext, VK_TYPE, shaderc_type)
* VK_TYPE -> VK_SHADER_STAGE_ ## VK_TYPE ## _BIT_KHR
* shaderc_type -> shaderc_ ## shaderc_type ## _shader

_XEXT(name, Name, ext, VK_TYPE, shaderc_type)
* VK_TYPE -> VK_SHADER_STAGE_ ## VK_TYPE ## _BIT_EXT
* shaderc_type -> shaderc_ ## shaderc_type ## _shader
*/

#ifdef _X
_X(vertex,   Vertex,   vert, VERTEX,   vertex)
_X(geometry, Geometry, geom, GEOMETRY, geometry)
_X(fragment, Fragment, frag, FRAGMENT, fragment)
_X(compute,  Compute,  comp, COMPUTE,  compute)

_X(tess_control,    TessControl,    tesc, TESSELLATION_CONTROL,    tess_control)
_X(tess_evaluation, TessEvaluation, tese, TESSELLATION_EVALUATION, tess_evaluation)
#endif

#ifdef _XKHR
_XKHR(raygen,       Raygen,       rgen,  RAYGEN,       raygen)
_XKHR(any_hit,      AnyHit,       rahit, ANY_HIT,      anyhit)
_XKHR(closest_hit,  ClosestHit,   rchit, CLOSEST_HIT,  closesthit)
_XKHR(miss,         Miss,         rmiss, MISS,         miss)
_XKHR(intersection, Intersection, rint,  INTERSECTION, intersection)
_XKHR(callable,     Callable,     rcall, CALLABLE,     callable)
#endif

#ifdef _XEXT
_XEXT(task, Task, task, TASK, task)
_XEXT(mesh, Mesh, mesh, MESH, mesh)
#endif

#ifdef _X
#undef _X
#endif
#ifdef _XKHR
#undef _XKHR
#endif
#ifdef _XEXT
#undef _XEXT
#endif
