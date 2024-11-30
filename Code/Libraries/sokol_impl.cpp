#define SOKOL_IMPL
#if _WIN32
	#define SOKOL_GLCORE
#else
	#define SOKOL_GLES3
#endif
#define SOKOL_IMGUI_IMPL

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_time.h"

#include "imgui.h"
#include "sokol_imgui.h"