
#include "../graphics.h"
#include "../memory.h"
#include "../assert.h"

#include <windows.h>

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef int GLint;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef double GLdouble;
typedef double GLclampd;
typedef void* GLeglClientBufferEXT;
typedef void* GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;

#ifdef _WIN64
typedef signed   long long int khronos_ssize_t;
typedef unsigned long long int khronos_usize_t;
#else
typedef signed   long  int     khronos_ssize_t;
typedef unsigned long  int     khronos_usize_t;
#endif
typedef          float         khronos_float_t;

typedef khronos_float_t GLfloat;
typedef khronos_ssize_t GLsizeiptr;
static_assert (sizeof(khronos_ssize_t) == sizeof(void*), "size_t is wrong");

#define GL_NONE 0
#define GL_FALSE 0
#define GL_TRUE 1
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_DONT_CARE 0x1100
#define GL_FASTEST 0x1101
#define GL_NICEST 0x1102
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_CLEAR 0x1500
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_MAX_VARYING_FLOATS 0x8B4B
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE 0x8B4F
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_RGBA32UI 0x8D70
#define GL_RGB32UI 0x8D71
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_RGBA16UI 0x8D76
#define GL_RGB16UI 0x8D77
#define GL_RGBA8UI 0x8D7C
#define GL_RGB8UI 0x8D7D
#define GL_RGBA32I 0x8D82
#define GL_RGB32I 0x8D83
#define GL_RGBA16I 0x8D88
#define GL_RGB16I 0x8D89
#define GL_RGBA8I 0x8D8E
#define GL_RGB8I 0x8D8F
#define GL_RED_INTEGER 0x8D94
#define GL_GREEN_INTEGER 0x8D95
#define GL_BLUE_INTEGER 0x8D96
#define GL_RGB_INTEGER 0x8D98
#define GL_RGBA_INTEGER 0x8D99
#define GL_BGR_INTEGER 0x8D9A
#define GL_BGRA_INTEGER 0x8D9B
#define GL_RGB4 0x804F
#define GL_RGB5 0x8050
#define GL_RGB8 0x8051
#define GL_RGB10 0x8052
#define GL_RGB12 0x8053
#define GL_RGB16 0x8054
#define GL_RGBA2 0x8055
#define GL_RGBA4 0x8056
#define GL_RGB5_A1 0x8057
#define GL_RGBA8 0x8058
#define GL_RGB10_A2 0x8059
#define GL_RGBA12 0x805A
#define GL_RGBA16 0x805B
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_RG 0x8227
#define GL_RG_INTEGER 0x8228
#define GL_R8 0x8229
#define GL_R16 0x822A
#define GL_RG8 0x822B
#define GL_RG16 0x822C
#define GL_R16F 0x822D
#define GL_R32F 0x822E
#define GL_RG16F 0x822F
#define GL_RG32F 0x8230
#define GL_R8I 0x8231
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
#define GL_R32I 0x8235
#define GL_R32UI 0x8236
#define GL_RG8I 0x8237
#define GL_RG8UI 0x8238
#define GL_RG16I 0x8239
#define GL_RG16UI 0x823A
#define GL_RG32I 0x823B
#define GL_RG32UI 0x823C
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_DEPTH32F_STENCIL8 0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_POINT 0x1B00
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01
#define GL_INCR 0x1E02
#define GL_DECR 0x1E03
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_REPEAT 0x2901
#define GL_TEXTURE 0x1702
#define GL_COLOR 0x1800
#define GL_DEPTH 0x1801
#define GL_STENCIL 0x1802
#define GL_STENCIL_INDEX 0x1901
#define GL_DEPTH_COMPONENT 0x1902
#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_BORDER_COLOR 0x1004
#define GL_DONT_CARE 0x1100
#define GL_FASTEST 0x1101
#define GL_NICEST 0x1102
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_RENDERBUFFER_WIDTH 0x8D42
#define GL_RENDERBUFFER_HEIGHT 0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT 0x8D44
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_BLEND_COLOR 0x8005
#define GL_BLEND_EQUATION 0x8009
#define GL_CONSTANT_COLOR 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_FUNC_ADD 0x8006
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_FUNC_SUBTRACT 0x800A
#define GL_MIN 0x8007
#define GL_MAX 0x8008
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_TEXTURE0 0x84C0
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_BLEND_DST 0x0BE0
#define GL_BLEND_SRC 0x0BE1
#define GL_BLEND 0x0BE2
#define GL_CULL_FACE 0x0B44
#define GL_CULL_FACE_MODE 0x0B45
#define GL_FRONT_FACE 0x0B46
#define GL_DEPTH_RANGE 0x0B70
#define GL_DEPTH_TEST 0x0B71
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT_RIGHT 0x0401
#define GL_BACK_LEFT 0x0402
#define GL_BACK_RIGHT 0x0403
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LEFT 0x0406
#define GL_RIGHT 0x0407
#define GL_FRONT_AND_BACK 0x0408
#define GL_NO_ERROR 0
#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_SCISSOR_BOX 0x0C10
#define GL_SCISSOR_TEST 0x0C11
#define GL_FRAMEBUFFER_DEFAULT 0x8218
#define GL_FRAMEBUFFER_UNDEFINED 0x8219
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8

typedef void (WINAPI* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
PFNGLGENBUFFERSPROC Gfx_glGenBuffers;
#define glGenBuffers Gfx_glGenBuffers

typedef void (WINAPI* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
PFNGLBINDBUFFERPROC Gfx_glBindBuffer;
#define glBindBuffer Gfx_glBindBuffer

typedef void (WINAPI* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
PFNGLBUFFERDATAPROC Gfx_glBufferData;
#define glBufferData Gfx_glBufferData

typedef GLuint(WINAPI* PFNGLCREATESHADERPROC)(GLenum type);
PFNGLCREATESHADERPROC Gfx_glCreateShader;
#define glCreateShader Gfx_glCreateShader

typedef void (WINAPI* PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
PFNGLSHADERSOURCEPROC Gfx_glShaderSource;
#define glShaderSource Gfx_glShaderSource

typedef void (WINAPI* PFNGLCOMPILESHADERPROC)(GLuint shader);
PFNGLCOMPILESHADERPROC Gfx_glCompileShader;
#define glCompileShader Gfx_glCompileShader

typedef void (WINAPI* PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
PFNGLGETSHADERIVPROC Gfx_glGetShaderiv;
#define glGetShaderiv Gfx_glGetShaderiv

typedef void (WINAPI* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
PFNGLDELETEBUFFERSPROC Gfx_glDeleteBuffers;
#define glDeleteBuffers Gfx_glDeleteBuffers

typedef void (WINAPI* PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint* framebuffers);
PFNGLDELETEFRAMEBUFFERSPROC Gfx_glDeleteFramebuffers;
#define glDeleteFramebuffers Gfx_glDeleteFramebuffers

typedef void (WINAPI* PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
PFNGLGETSHADERINFOLOGPROC Gfx_glGetShaderInfoLog;
#define glGetShaderInfoLog Gfx_glGetShaderInfoLog

typedef void (WINAPI* PFNGLDELETESHADERPROC)(GLuint shader);
PFNGLDELETESHADERPROC Gfx_glDeleteShader;
#define glDeleteShader Gfx_glDeleteShader

typedef GLuint(WINAPI* PFNGLCREATEPROGRAMPROC)(void);
PFNGLCREATEPROGRAMPROC Gfx_glCreateProgram;
#define glCreateProgram Gfx_glCreateProgram

typedef void (WINAPI* PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
PFNGLATTACHSHADERPROC Gfx_glAttachShader;
#define glAttachShader Gfx_glAttachShader

typedef void (WINAPI* PFNGLLINKPROGRAMPROC)(GLuint program);
PFNGLLINKPROGRAMPROC Gfx_glLinkProgram;
#define glLinkProgram Gfx_glLinkProgram

typedef void (WINAPI* PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
PFNGLGETPROGRAMIVPROC Gfx_glGetProgramiv;
#define glGetProgramiv Gfx_glGetProgramiv

typedef void (WINAPI* PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
PFNGLGETPROGRAMINFOLOGPROC Gfx_glGetProgramInfoLog;
#define glGetProgramInfoLog Gfx_glGetProgramInfoLog

typedef void (WINAPI* PFNGLDELETEPROGRAMPROC)(GLuint program);
PFNGLDELETEPROGRAMPROC Gfx_glDeleteProgram;
#define glDeleteProgram Gfx_glDeleteProgram

typedef GLint(WINAPI* PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
PFNGLGETUNIFORMLOCATIONPROC Gfx_glGetUniformLocation;
#define glGetUniformLocation Gfx_glGetUniformLocation

typedef void (WINAPI* PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
PFNGLGENVERTEXARRAYSPROC Gfx_glGenVertexArrays;
#define glGenVertexArrays Gfx_glGenVertexArrays

typedef void (WINAPI* PFNGLBINDVERTEXARRAYPROC)(GLuint array);
PFNGLBINDVERTEXARRAYPROC Gfx_glBindVertexArray;
#define glBindVertexArray Gfx_glBindVertexArray

typedef GLint(WINAPI* PFNGLGETATTRIBLOCATIONPROC)(GLuint program, const GLchar* name);
PFNGLGETATTRIBLOCATIONPROC Gfx_glGetAttribLocation;
#define glGetAttribLocation Gfx_glGetAttribLocation

typedef void (WINAPI* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
PFNGLVERTEXATTRIBPOINTERPROC Gfx_glVertexAttribPointer;
#define glVertexAttribPointer Gfx_glVertexAttribPointer

typedef void (WINAPI* PFNGLVERTEXATTRIBIPOINTERPROC)(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer);
PFNGLVERTEXATTRIBIPOINTERPROC Gfx_glVertexAttribIPointer;
#define glVertexAttribIPointer Gfx_glVertexAttribIPointer

typedef void (WINAPI* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
PFNGLENABLEVERTEXATTRIBARRAYPROC Gfx_glEnableVertexAttribArray;
#define glEnableVertexAttribArray Gfx_glEnableVertexAttribArray

typedef void (WINAPI* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
PFNGLDELETEVERTEXARRAYSPROC Gfx_glDeleteVertexArrays;
#define glDeleteVertexArrays Gfx_glDeleteVertexArrays

typedef void (WINAPI* PFNGLGENTEXTURESPROC)(GLsizei n, GLuint* textures);
PFNGLGENTEXTURESPROC Gfx_glGenTextures;
#define glGenTextures Gfx_glGenTextures

typedef void (WINAPI* PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
PFNGLBINDTEXTUREPROC Gfx_glBindTexture;
#define glBindTexture Gfx_glBindTexture

typedef void (WINAPI* PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
PFNGLTEXIMAGE2DPROC Gfx_glTexImage2D;
#define glTexImage2D Gfx_glTexImage2D

typedef void (WINAPI* PFNGLGENERATEMIPMAPPROC)(GLenum target);
PFNGLGENERATEMIPMAPPROC Gfx_glGenerateMipmap;
#define glGenerateMipmap Gfx_glGenerateMipmap

typedef void (WINAPI* PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint* textures);
PFNGLDELETETEXTURESPROC Gfx_glDeleteTextures;
#define glDeleteTextures Gfx_glDeleteTextures

typedef void (WINAPI* PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
PFNGLTEXPARAMETERIPROC Gfx_glTexParameteri;
#define glTexParameteri Gfx_glTexParameteri

typedef void (WINAPI* PFNGLACTIVETEXTUREPROC)(GLenum texture);
PFNGLACTIVETEXTUREPROC Gfx_glActiveTexture;
#define glActiveTexture Gfx_glActiveTexture

typedef void (WINAPI* PFNGLUSEPROGRAMPROC)(GLuint program);
PFNGLUSEPROGRAMPROC Gfx_glUseProgram;
#define glUseProgram Gfx_glUseProgram

typedef void (WINAPI* PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
PFNGLBINDFRAMEBUFFERPROC Gfx_glBindFramebuffer;
#define glBindFramebuffer Gfx_glBindFramebuffer

typedef void (WINAPI* PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint* framebuffers);
PFNGLGENFRAMEBUFFERSPROC Gfx_glGenFramebuffers;
#define glGenFramebuffers Gfx_glGenFramebuffers

typedef void (WINAPI* PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
PFNGLFRAMEBUFFERTEXTURE2DPROC Gfx_glFramebufferTexture2D;
#define glFramebufferTexture2D Gfx_glFramebufferTexture2D

typedef GLenum(WINAPI* PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
PFNGLCHECKFRAMEBUFFERSTATUSPROC Gfx_glCheckFramebufferStatus;
#define glCheckFramebufferStatus Gfx_glCheckFramebufferStatus

typedef void (WINAPI* PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
PFNGLUNIFORM1IPROC Gfx_glUniform1i;
#define glUniform1i Gfx_glUniform1i

typedef void (WINAPI* PFNGLDRAWELEMENTSINSTANCEDPROC)(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
PFNGLDRAWELEMENTSINSTANCEDPROC Gfx_glDrawElementsInstanced;
#define glDrawElementsInstanced Gfx_glDrawElementsInstanced

typedef void (WINAPI* PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void* indices);
PFNGLDRAWELEMENTSPROC Gfx_glDrawElements;
#define glDrawElements Gfx_glDrawElements

typedef void (WINAPI* PFNGLDRAWARRAYSINSTANCEDPROC)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
PFNGLDRAWARRAYSINSTANCEDPROC Gfx_glDrawArraysInstanced;
#define glDrawArraysInstanced Gfx_glDrawArraysInstanced

typedef void (WINAPI* PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
PFNGLDRAWARRAYSPROC Gfx_glDrawArrays;
#define glDrawArrays Gfx_glDrawArrays

typedef void (WINAPI* PFNGLUNIFORM1IVPROC)(GLint location, GLsizei count, const GLint* value);
PFNGLUNIFORM1IVPROC Gfx_glUniform1iv;
#define glUniform1iv Gfx_glUniform1iv

typedef void (WINAPI* PFNGLUNIFORM2IVPROC)(GLint location, GLsizei count, const GLint* value);
PFNGLUNIFORM2IVPROC Gfx_glUniform2iv;
#define glUniform2iv Gfx_glUniform2iv

typedef void (WINAPI* PFNGLUNIFORM3IVPROC)(GLint location, GLsizei count, const GLint* value);
PFNGLUNIFORM3IVPROC Gfx_glUniform3iv;
#define glUniform3iv Gfx_glUniform3iv

typedef void (WINAPI* PFNGLUNIFORM4IVPROC)(GLint location, GLsizei count, const GLint* value);
PFNGLUNIFORM4IVPROC Gfx_glUniform4iv;
#define glUniform4iv Gfx_glUniform4iv

typedef void (WINAPI* PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
PFNGLVIEWPORTPROC Gfx_glViewport;
#define glViewport Gfx_glViewport

typedef void (WINAPI* PFNGLDEPTHMASKPROC)(GLboolean flag);
PFNGLDEPTHMASKPROC Gfx_glDepthMask;
#define glDepthMask Gfx_glDepthMask

typedef void (WINAPI* PFNGLCOLORMASKPROC)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
PFNGLCOLORMASKPROC Gfx_glColorMask;
#define glColorMask Gfx_glColorMask

typedef void (WINAPI* PFNGLUNIFORM1FVPROC)(GLint location, GLsizei count, const GLfloat* value);
PFNGLUNIFORM1FVPROC Gfx_glUniform1fv;
#define glUniform1fv Gfx_glUniform1fv

typedef void (WINAPI* PFNGLUNIFORM2FVPROC)(GLint location, GLsizei count, const GLfloat* value);
PFNGLUNIFORM2FVPROC Gfx_glUniform2fv;
#define glUniform2fv Gfx_glUniform2fv

typedef void (WINAPI* PFNGLUNIFORM3FVPROC)(GLint location, GLsizei count, const GLfloat* value);
PFNGLUNIFORM3FVPROC Gfx_glUniform3fv;
#define glUniform3fv Gfx_glUniform3fv

typedef void (WINAPI* PFNGLUNIFORM4FVPROC)(GLint location, GLsizei count, const GLfloat* value);
PFNGLUNIFORM4FVPROC Gfx_glUniform4fv;
#define glUniform4fv Gfx_glUniform4fv

typedef void (WINAPI* PFNGLUNIFORMMATRIX2FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
PFNGLUNIFORMMATRIX2FVPROC Gfx_glUniformMatrix2fv;
#define glUniformMatrix2fv Gfx_glUniformMatrix2fv

typedef void (WINAPI* PFNGLUNIFORMMATRIX3FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
PFNGLUNIFORMMATRIX3FVPROC Gfx_glUniformMatrix3fv;
#define glUniformMatrix3fv Gfx_glUniformMatrix3fv

typedef void (WINAPI* PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
PFNGLUNIFORMMATRIX4FVPROC Gfx_glUniformMatrix4fv;
#define glUniformMatrix4fv Gfx_glUniformMatrix4fv

typedef void (WINAPI* PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
PFNGLCLEARCOLORPROC Gfx_glClearColor;
#define glClearColor Gfx_glClearColor

typedef void (WINAPI* PFNGLCLEARDEPTHPROC)(GLdouble depth);
PFNGLCLEARDEPTHPROC Gfx_glClearDepth;
#define glClearDepth Gfx_glClearDepth

typedef void (WINAPI* PFNGLCLEARPROC)(GLbitfield mask);
PFNGLCLEARPROC Gfx_glClear;
#define glClear Gfx_glClear

typedef void (WINAPI* PFNGLENABLEPROC)(GLenum cap);
PFNGLENABLEPROC Gfx_glEnable;
#define glEnable Gfx_glEnable

typedef void (WINAPI* PFNGLDISABLEPROC)(GLenum cap);
PFNGLDISABLEPROC Gfx_glDisable;
#define glDisable Gfx_glDisable

typedef void (WINAPI* PFNGLBLENDCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
PFNGLBLENDCOLORPROC Gfx_glBlendColor;
#define glBlendColor Gfx_glBlendColor

typedef void (WINAPI* PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
PFNGLBLENDFUNCPROC Gfx_glBlendFunc;
#define glBlendFunc Gfx_glBlendFunc

typedef void (WINAPI* PFNGLBLENDFUNCSEPARATEPROC)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
PFNGLBLENDFUNCSEPARATEPROC Gfx_glBlendFuncSeparate;
#define glBlendFuncSeparate Gfx_glBlendFuncSeparate

typedef void (WINAPI* PFNGLBLENDEQUATIONPROC)(GLenum mode);
PFNGLBLENDEQUATIONPROC Gfx_glBlendEquation;
#define glBlendEquation Gfx_glBlendEquation

typedef void (WINAPI* PFNGLBLENDEQUATIONSEPARATEPROC)(GLenum modeRGB, GLenum modeAlpha);
PFNGLBLENDEQUATIONSEPARATEPROC Gfx_glBlendEquationSeparate;
#define glBlendEquationSeparate Gfx_glBlendEquationSeparate

typedef void (WINAPI* PFNGLCULLFACEPROC)(GLenum mode);
PFNGLCULLFACEPROC Gfx_glCullFace;
#define glCullFace Gfx_glCullFace

typedef void (WINAPI* PFNGLFRONTFACEPROC)(GLenum mode);
PFNGLFRONTFACEPROC Gfx_glFrontFace;
#define glFrontFace Gfx_glFrontFace

typedef void (WINAPI* PFNGLDEPTHFUNCPROC)(GLenum func);
PFNGLDEPTHFUNCPROC Gfx_glDepthFunc;
#define glDepthFunc Gfx_glDepthFunc

typedef void (WINAPI* PFNGLDEPTHRANGEPROC)(GLdouble n, GLdouble f);
PFNGLDEPTHRANGEPROC Gfx_glDepthRange;
#define glDepthRange Gfx_glDepthRange

typedef void (WINAPI* PFNGLSCISSORPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
PFNGLSCISSORPROC Gfx_glScissor;
#define glScissor Gfx_glScissor

typedef void (WINAPI* PFNGLFINISHPROC)(void);
PFNGLFINISHPROC Gfx_glFinish;
#define glFinish Gfx_glFinish

typedef void (WINAPI* PFNGLTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
PFNGLTEXSUBIMAGE2DPROC Gfx_glTexSubImage2D;
#define glTexSubImage2D Gfx_glTexSubImage2D

typedef void (WINAPI* PFNGLPIXELSTOREIPROC)(GLenum pname, GLint param);
PFNGLPIXELSTOREIPROC Gfx_glPixelStorei;
#define glPixelStorei Gfx_glPixelStorei

typedef void (WINAPI* PFNGLTEXPARAMETERIVPROC)(GLenum target, GLenum pname, const GLint* params);
PFNGLTEXPARAMETERIVPROC Gfx_glTexParameteriv;
#define glTexParameteriv Gfx_glTexParameteriv

typedef void* (WINAPI* PFNWGLGETPROCADDRESSPROC_PRIVATE)(const char*);

// OpenGL context setup
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);

// V Synch
typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);

struct GfxInternal_ShaderBoundTextures {
    u32 shaderId;
    i32 uniformSlots[8];
    //u32 textureUnits[8]; // Not using texture id for anything
};

struct GfxInternal_State {
    // NOTE: Map is misleading. It's all vectors with linear search. Consider changing to actual maps if it becomes a bottle neck.
    u32 resourceCountBuffer;
    u32 resourceCountShader;
    u32 resourceCountVertexLayout;
    u32 resourceCountTexture;
    u32 resourceCountFbo;

    u64* indexBufferTypeMap; // (u32 - buffer id, GLenum - GL_NONE, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT)
    u32 indexBufferMapCount; 
    u32 indexBufferMapCapacity;

    u64* vaoShaderMap; // (u32 - vao, u32 - shader)
    u32 vaoShaderMapCount;
    u32 vaoShaderMapCapacity;

    u64* vaoIboMap;
    u32 vaoIboMapCount;
    u32 vaoIboMapCapacity;

    u32* renderTargetFboMap; // (u32 - color texture, u32 - depth texture, u32 fbo id) (aka stride of 3)
    u32 renderTargetFboMapCount;
    u32 renderTargetFboMapCapacity;

    GfxInternal_ShaderBoundTextures* shaderBoundTextureMap;
    u32 shaderBoundTextureMapCount;
    u32 shaderBoundTextureMapCapacity;

    u32 boundShader;
    u32 boundFbo;

    HMODULE libGL;

    bool enableTracking;
    u32 indexCount;
    u32 drawCount;
};

GfxInternal_State gGfxState;

static void* GfxInternal_GetGlProcAddress(const char* name) {
    void* p = (void*)wglGetProcAddress(name);
    if (p == 0 ||
        (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
        (p == (void*)-1))
    {
        p = (void*)GetProcAddress(gGfxState.libGL, name);
    }

    return p;
}

static GfxInternal_ShaderBoundTextures* GfxInternal_GetShaderBoundTextures(u32 program) {
    for (u32 i = 0, size = gGfxState.shaderBoundTextureMapCount; i < size; ++i) {
        if (gGfxState.shaderBoundTextureMap[i].shaderId == program) {
            return &gGfxState.shaderBoundTextureMap[i];
        }
    }
    return 0;
}


static u32 GfxInternal_GetRenderTargetMap(u32 _color, u32 _depth) {
    for (u32 i = 0, count = gGfxState.renderTargetFboMapCount; i < count; ++i) {
        u32 color = gGfxState.renderTargetFboMap[i * 3 + 0];
        u32 depth = gGfxState.renderTargetFboMap[i * 3 + 1];

        if (color == _color && depth == _depth) {
            return gGfxState.renderTargetFboMap[i * 3 + 2];
        }
    }

    return 0;
}

static void GfxInternal_SetRenderTargetMap(u32 _color, u32 _depth, u32 _fbo) {
    bool foundFirst = false;
    u32 firstIndex = 0;

    for (u32 i = 0, count = gGfxState.renderTargetFboMapCount; i < count; ++i) {
        u32 color = gGfxState.renderTargetFboMap[i * 3 + 0];
        u32 depth = gGfxState.renderTargetFboMap[i * 3 + 1];
        u32 fbo =   gGfxState.renderTargetFboMap[i * 3 + 2];

        if (fbo == 0) {
            if (!foundFirst) {
                foundFirst = true;
                firstIndex = i;
            }
        }
        
        if (color == _color && depth == _depth) {
            gGfxState.renderTargetFboMap[i * 3 + 2] = _fbo;
            return; // Updated existing
        }
    }

    if (foundFirst) {
        gGfxState.renderTargetFboMap[firstIndex * 3 + 0] = _color;
        gGfxState.renderTargetFboMap[firstIndex * 3 + 1] = _depth;
        gGfxState.renderTargetFboMap[firstIndex * 3 + 2] = _fbo;
        return;
    }

    if (gGfxState.renderTargetFboMapCapacity == gGfxState.renderTargetFboMapCount) {
        gGfxState.renderTargetFboMapCapacity = (gGfxState.renderTargetFboMapCapacity == 0) ? 16 : gGfxState.renderTargetFboMapCapacity * 2;
        gGfxState.renderTargetFboMap = (u32*)MemRealloc(gGfxState.renderTargetFboMap, gGfxState.renderTargetFboMapCapacity * sizeof(u32) * 3);
    }

    gGfxState.renderTargetFboMap[gGfxState.renderTargetFboMapCount * 3 + 0] = _color;
    gGfxState.renderTargetFboMap[gGfxState.renderTargetFboMapCount * 3 + 1] = _depth;
    gGfxState.renderTargetFboMap[gGfxState.renderTargetFboMapCount * 3 + 2] = _fbo;
    gGfxState.renderTargetFboMapCount += 1;
}

static GLenum GfxInternal_GetIndexBufferType(u32 bufferId) {
    for (u32 i = 0, count = gGfxState.indexBufferMapCount; i < count; ++i) {
        u64 bufferType = gGfxState.indexBufferTypeMap[i];

        u32 key = (u32)(bufferType >> 32);
        GLenum value = (u32)bufferType;

        if (key == bufferId) {
            return value;
        }
    }

    PlatformAssert(false, __LOCATION__);
    return GL_NONE;
}

static void GfxInternal_SetIndexBufferType(u32 bufferId, GLenum type) {
    u64 newItem = (u64(bufferId) << 32) | u64(type);
    
    // Search for existing
    u32 firstFree = 0;
    bool foundFirstFree = false;

    for (u32 i = 0, count = gGfxState.indexBufferMapCount; i < count; ++i) {
        u32 key = (u32)(gGfxState.indexBufferTypeMap[i] >> 32);

        if (key == bufferId) {
            gGfxState.indexBufferTypeMap[i] = newItem;
            return;
        }
        else if (key == 0) {
            if (!foundFirstFree) {
                foundFirstFree = true;
                firstFree = i;
            }
        }
    }

    // Grab a free one if none found
    if (foundFirstFree) {
        gGfxState.indexBufferTypeMap[firstFree] = newItem;
        return;
    }

    // Add new one if no free one was available
    if (gGfxState.indexBufferMapCapacity == gGfxState.indexBufferMapCount) {
        gGfxState.indexBufferMapCapacity = (gGfxState.indexBufferMapCapacity == 0)? 16 : gGfxState.indexBufferMapCapacity * 2;
        gGfxState.indexBufferTypeMap = (u64*)MemRealloc(gGfxState.indexBufferTypeMap, gGfxState.indexBufferMapCapacity * sizeof(u64));
    }

    gGfxState.indexBufferTypeMap[gGfxState.indexBufferMapCount++] = newItem;
}

static void GfxInternal_RemoveIndexBufferType(u32 bufferId) {
    PlatformAssert(gGfxState.indexBufferMapCount >= 1, __LOCATION__);

    for (u32 i = 0, count = gGfxState.indexBufferMapCount; i < count; ++i) {
        u32 key = (u32)(gGfxState.indexBufferTypeMap[i] >> 32);

        if (key == bufferId) {
            u64 updatedItem = 0;
            gGfxState.indexBufferTypeMap[i] = updatedItem;
            return;
        }
    }

    PlatformAssert(false, __LOCATION__);
}

static u32 GfxInternal_GetVaoShader(u32 _vao) {
    for (u32 i = 0, count = gGfxState.vaoShaderMapCount; i < count; ++i) {
        u64 vaoShader = gGfxState.vaoShaderMap[i];

        u32 vao = (u32)(vaoShader >> 32);
        u32 shader = (u32)vaoShader;

        if (vao == _vao) {
            return shader;
        }
    }

    PlatformAssert(false, __LOCATION__);
    return 0;
}

static void GfxInternal_SetVaoShader(u32 _vao, u32 _shader) {
    u64 newItem = (u64(_vao) << 32) | u64(_shader);

    u32 firstFree = 0;
    bool firstFreeFound = false;

    // Search for existing
    for (u32 i = 0, count = gGfxState.vaoShaderMapCount; i < count; ++i) {
        u32 key = (u32)(gGfxState.vaoShaderMap[i] >> 32);

        if (key == _vao) {
            gGfxState.vaoShaderMap[i] = newItem;
            return;
        }
        else if (key == 0) {
            if (!firstFreeFound) {
                firstFreeFound = true;
                firstFree = i;
            }
        }
    }

    // None found, recycle if possible
    if (firstFreeFound) {
        gGfxState.vaoShaderMap[firstFree] = newItem;
        return;
    }

    // Last resort, grow list
    if (gGfxState.vaoShaderMapCapacity == gGfxState.vaoShaderMapCount) {
        gGfxState.vaoShaderMapCapacity = (gGfxState.vaoShaderMapCapacity == 0) ? 16 : gGfxState.vaoShaderMapCapacity * 2;
        gGfxState.vaoShaderMap = (u64*)MemRealloc(gGfxState.vaoShaderMap, gGfxState.vaoShaderMapCapacity * sizeof(u64));
    }

    gGfxState.vaoShaderMap[gGfxState.vaoShaderMapCount++] = newItem;
}

static void GfxInternal_RemoveVaoShader(u32 vao) {
    PlatformAssert(gGfxState.vaoShaderMapCount >= 1, __LOCATION__);

    for (u32 i = 0, count = gGfxState.vaoShaderMapCount; i < count; ++i) {
        u32 key = (u32)(gGfxState.vaoShaderMap[i] >> 32);

        if (key == vao) {
            gGfxState.vaoShaderMap[i] = 0;
            return;
        }
    }

    PlatformAssert(false, __LOCATION__);
}

static u32 GfxInternal_GetVaoIbo(u32 _vao) {
    for (u32 i = 0, count = gGfxState.vaoIboMapCount; i < count; ++i) {
        u64 vaoIbo = gGfxState.vaoIboMap[i];

        u32 vao = (u32)(vaoIbo >> 32);
        u32 ibo = (u32)vaoIbo;

        if (vao == _vao) {
            return ibo;
        }
    }

    PlatformAssert(false, __LOCATION__);
    return 0;
}

static void GfxInternal_SetVaoIbo(u32 _vao, u32 _ibo) {
    u64 newItem = (u64(_vao) << 32) | u64(_ibo);

    bool firstFound = false;
    u32 first = 0;

    for (u32 i = 0, count = gGfxState.vaoIboMapCount; i < count; ++i) {
        u32 key = (u32)(gGfxState.vaoIboMap[i] >> 32);

        if (key == _vao) {
            gGfxState.vaoIboMap[i] = newItem;
            return;
        }
        else if (key == 0) {
            if (!firstFound) {
                firstFound = true;
                first = i;
            }
        }
    }

    if (firstFound) {
        gGfxState.vaoIboMap[first] = newItem;
        return;
    }

    if (gGfxState.vaoIboMapCapacity == gGfxState.vaoIboMapCount) {
        gGfxState.vaoIboMapCapacity = (gGfxState.vaoIboMapCapacity == 0) ? 16 : gGfxState.vaoIboMapCapacity * 2;
        gGfxState.vaoIboMap = (u64*)MemRealloc(gGfxState.vaoIboMap, gGfxState.vaoIboMapCapacity * sizeof(u64));
    }

    gGfxState.vaoIboMap[gGfxState.vaoIboMapCount++] = newItem;
}

static void GfxInternal_RemoveVaoIbo(u32 vao) {
    PlatformAssert(gGfxState.vaoIboMapCount >= 1, __LOCATION__);

    for (u32 i = 0, count = gGfxState.vaoIboMapCount; i < count; ++i) {
        u32 key = (u32)(gGfxState.vaoIboMap[i] >> 32);

        if (key == vao) {
            gGfxState.vaoIboMap[i] = 0;
            return;
        }
    }

    PlatformAssert(false, __LOCATION__);
}

extern "C" int GfxInitialize(void* inUserData, void* outUserData) {
    HDC hdc = *(HDC*)inUserData; 
    HGLRC* result = (HGLRC*)outUserData;

    MemSet(&gGfxState, 0, sizeof(GfxInternal_State));
    gGfxState.libGL = LoadLibraryW(L"opengl32.dll");
    PlatformAssert(gGfxState.libGL != NULL, __LOCATION__);

    // Init OpenGL context
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixelFormat, &pfd);

    HGLRC tempRC = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempRC);
    
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
    const int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    *result = wglCreateContextAttribsARB(hdc, 0, attribList);
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempRC);
    wglMakeCurrent(hdc, *result);

    Gfx_glGenBuffers = (PFNGLGENBUFFERSPROC )GfxInternal_GetGlProcAddress("glGenBuffers");
    Gfx_glBindBuffer = (PFNGLBINDBUFFERPROC )GfxInternal_GetGlProcAddress("glBindBuffer");
    Gfx_glBufferData = (PFNGLBUFFERDATAPROC )GfxInternal_GetGlProcAddress("glBufferData");
    Gfx_glCreateShader = (PFNGLCREATESHADERPROC )GfxInternal_GetGlProcAddress("glCreateShader");
    Gfx_glShaderSource = (PFNGLSHADERSOURCEPROC )GfxInternal_GetGlProcAddress("glShaderSource");
    Gfx_glCompileShader = (PFNGLCOMPILESHADERPROC )GfxInternal_GetGlProcAddress("glCompileShader");
    Gfx_glGetShaderiv = (PFNGLGETSHADERIVPROC )GfxInternal_GetGlProcAddress("glGetShaderiv");
    Gfx_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC )GfxInternal_GetGlProcAddress("glDeleteBuffers");
    Gfx_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC )GfxInternal_GetGlProcAddress("glDeleteFramebuffers");
    Gfx_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC )GfxInternal_GetGlProcAddress("glGetShaderInfoLog");
    Gfx_glDeleteShader = (PFNGLDELETESHADERPROC )GfxInternal_GetGlProcAddress("glDeleteShader");
    Gfx_glCreateProgram = (PFNGLCREATEPROGRAMPROC )GfxInternal_GetGlProcAddress("glCreateProgram");
    Gfx_glAttachShader = (PFNGLATTACHSHADERPROC )GfxInternal_GetGlProcAddress("glAttachShader");
    Gfx_glLinkProgram = (PFNGLLINKPROGRAMPROC )GfxInternal_GetGlProcAddress("glLinkProgram");
    Gfx_glGetProgramiv = (PFNGLGETPROGRAMIVPROC )GfxInternal_GetGlProcAddress("glGetProgramiv");
    Gfx_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC )GfxInternal_GetGlProcAddress("glGetProgramInfoLog");
    Gfx_glDeleteProgram = (PFNGLDELETEPROGRAMPROC )GfxInternal_GetGlProcAddress("glDeleteProgram");
    Gfx_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC )GfxInternal_GetGlProcAddress("glGetUniformLocation");
    Gfx_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC )GfxInternal_GetGlProcAddress("glGenVertexArrays");
    Gfx_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC )GfxInternal_GetGlProcAddress("glBindVertexArray");
    Gfx_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC )GfxInternal_GetGlProcAddress("glGetAttribLocation");
    Gfx_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GfxInternal_GetGlProcAddress("glVertexAttribPointer");
    Gfx_glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC )GfxInternal_GetGlProcAddress("glVertexAttribIPointer");
    Gfx_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC )GfxInternal_GetGlProcAddress("glEnableVertexAttribArray");
    Gfx_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC )GfxInternal_GetGlProcAddress("glDeleteVertexArrays");
    Gfx_glGenTextures = (PFNGLGENTEXTURESPROC )GfxInternal_GetGlProcAddress("glGenTextures");
    Gfx_glBindTexture = (PFNGLBINDTEXTUREPROC )GfxInternal_GetGlProcAddress("glBindTexture");
    Gfx_glTexImage2D = (PFNGLTEXIMAGE2DPROC )GfxInternal_GetGlProcAddress("glTexImage2D");
    Gfx_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC )GfxInternal_GetGlProcAddress("glGenerateMipmap");
    Gfx_glDeleteTextures = (PFNGLDELETETEXTURESPROC )GfxInternal_GetGlProcAddress("glDeleteTextures");
    Gfx_glTexParameteri = (PFNGLTEXPARAMETERIPROC )GfxInternal_GetGlProcAddress("glTexParameteri");
    Gfx_glActiveTexture = (PFNGLACTIVETEXTUREPROC )GfxInternal_GetGlProcAddress("glActiveTexture");
    Gfx_glUseProgram = (PFNGLUSEPROGRAMPROC )GfxInternal_GetGlProcAddress("glUseProgram");
    Gfx_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC )GfxInternal_GetGlProcAddress("glBindFramebuffer");
    Gfx_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC )GfxInternal_GetGlProcAddress("glGenFramebuffers");
    Gfx_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC )GfxInternal_GetGlProcAddress("glFramebufferTexture2D");
    Gfx_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC )GfxInternal_GetGlProcAddress("glCheckFramebufferStatus");
    Gfx_glUniform1i = (PFNGLUNIFORM1IPROC )GfxInternal_GetGlProcAddress("glUniform1i");
    Gfx_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC )GfxInternal_GetGlProcAddress("glDrawElementsInstanced");
    Gfx_glDrawElements = (PFNGLDRAWELEMENTSPROC )GfxInternal_GetGlProcAddress("glDrawElements");
    Gfx_glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC )GfxInternal_GetGlProcAddress("glDrawArraysInstanced");
    Gfx_glDrawArrays = (PFNGLDRAWARRAYSPROC )GfxInternal_GetGlProcAddress("glDrawArrays");
    Gfx_glUniform1iv = (PFNGLUNIFORM1IVPROC )GfxInternal_GetGlProcAddress("glUniform1iv");
    Gfx_glUniform2iv = (PFNGLUNIFORM2IVPROC )GfxInternal_GetGlProcAddress("glUniform2iv");
    Gfx_glUniform3iv = (PFNGLUNIFORM3IVPROC )GfxInternal_GetGlProcAddress("glUniform3iv");
    Gfx_glUniform4iv = (PFNGLUNIFORM4IVPROC )GfxInternal_GetGlProcAddress("glUniform4iv");
    Gfx_glViewport = (PFNGLVIEWPORTPROC )GfxInternal_GetGlProcAddress("glViewport");
    Gfx_glDepthMask = (PFNGLDEPTHMASKPROC )GfxInternal_GetGlProcAddress("glDepthMask");
    Gfx_glColorMask = (PFNGLCOLORMASKPROC )GfxInternal_GetGlProcAddress("glColorMask");
    Gfx_glUniform1fv = (PFNGLUNIFORM1FVPROC )GfxInternal_GetGlProcAddress("glUniform1fv");
    Gfx_glUniform2fv = (PFNGLUNIFORM2FVPROC )GfxInternal_GetGlProcAddress("glUniform2fv");
    Gfx_glUniform3fv = (PFNGLUNIFORM3FVPROC )GfxInternal_GetGlProcAddress("glUniform3fv");
    Gfx_glUniform4fv = (PFNGLUNIFORM4FVPROC )GfxInternal_GetGlProcAddress("glUniform4fv");
    Gfx_glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC )GfxInternal_GetGlProcAddress("glUniformMatrix2fv");
    Gfx_glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC )GfxInternal_GetGlProcAddress("glUniformMatrix3fv");
    Gfx_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC )GfxInternal_GetGlProcAddress("glUniformMatrix4fv");
    Gfx_glClearColor = (PFNGLCLEARCOLORPROC )GfxInternal_GetGlProcAddress("glClearColor");
    Gfx_glClearDepth = (PFNGLCLEARDEPTHPROC )GfxInternal_GetGlProcAddress("glClearDepth");
    Gfx_glClear = (PFNGLCLEARPROC )GfxInternal_GetGlProcAddress("glClear");
    Gfx_glEnable = (PFNGLENABLEPROC )GfxInternal_GetGlProcAddress("glEnable");
    Gfx_glDisable = (PFNGLDISABLEPROC )GfxInternal_GetGlProcAddress("glDisable");
    Gfx_glBlendColor = (PFNGLBLENDCOLORPROC )GfxInternal_GetGlProcAddress("glBlendColor");
    Gfx_glBlendFunc = (PFNGLBLENDFUNCPROC )GfxInternal_GetGlProcAddress("glBlendFunc");
    Gfx_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC )GfxInternal_GetGlProcAddress("glBlendFuncSeparate");
    Gfx_glBlendEquation = (PFNGLBLENDEQUATIONPROC )GfxInternal_GetGlProcAddress("glBlendEquation");
    Gfx_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC )GfxInternal_GetGlProcAddress("glBlendEquationSeparate");
    Gfx_glCullFace = (PFNGLCULLFACEPROC )GfxInternal_GetGlProcAddress("glCullFace");
    Gfx_glFrontFace = (PFNGLFRONTFACEPROC )GfxInternal_GetGlProcAddress("glFrontFace");
    Gfx_glDepthFunc = (PFNGLDEPTHFUNCPROC )GfxInternal_GetGlProcAddress("glDepthFunc");
    Gfx_glDepthRange = (PFNGLDEPTHRANGEPROC )GfxInternal_GetGlProcAddress("glDepthRange");
    Gfx_glScissor = (PFNGLSCISSORPROC )GfxInternal_GetGlProcAddress("glScissor");
    Gfx_glFinish = (PFNGLFINISHPROC)GfxInternal_GetGlProcAddress("glFinish");
    Gfx_glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC)GfxInternal_GetGlProcAddress("glTexSubImage2D");
    Gfx_glPixelStorei = (PFNGLPIXELSTOREIPROC)GfxInternal_GetGlProcAddress("glPixelStorei");
    Gfx_glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)GfxInternal_GetGlProcAddress("glTexParameteriv");

    // Enable v synch
    int vsynch = 0;
    { // Enable Vsynch
        PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)GfxInternal_GetGlProcAddress("wglGetExtensionsStringEXT");
        bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;

        if (swapControlSupported) {
            PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)GfxInternal_GetGlProcAddress("wglSwapIntervalEXT");
            PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)GfxInternal_GetGlProcAddress("wglGetSwapIntervalEXT");

            if (wglSwapIntervalEXT(1)) {
                //std::cout << "Enabled vsynch\n";
                vsynch = wglGetSwapIntervalEXT();
            }
            else {
                //std::cout << "Could not enable vsynch\n";
            }
        }
        else { // !swapControlSupported
            //std::cout << "WGL_EXT_swap_control not supported\n";
        }
    }

    return vsynch;
}

extern "C" void GfxShutdown(void* userData) { 
    PlatformAssert(gGfxState.resourceCountBuffer == 0, __LOCATION__);
    PlatformAssert(gGfxState.resourceCountShader == 0, __LOCATION__);
    PlatformAssert(gGfxState.resourceCountVertexLayout == 0, __LOCATION__);
    PlatformAssert(gGfxState.resourceCountTexture == 0, __LOCATION__);
    PlatformAssert(gGfxState.resourceCountFbo == 0, __LOCATION__);

    if (gGfxState.indexBufferTypeMap != 0) {
#if _DEBUG
        for (u32 i = 0; i < gGfxState.indexBufferMapCount; ++i) {
            PlatformAssert(gGfxState.indexBufferTypeMap[i] == 0, __LOCATION__);
        }
#endif
        MemRelease(gGfxState.indexBufferTypeMap);
    }

    if (gGfxState.vaoShaderMap != 0) {
#if _DEBUG
        for (u32 i = 0; i < gGfxState.vaoShaderMapCount; ++i) {
            PlatformAssert(gGfxState.vaoShaderMap[i] == 0, __LOCATION__);
        }
#endif
        MemRelease(gGfxState.vaoShaderMap);
    }

    if (gGfxState.vaoIboMap != 0) {
#if _DEBUG
        for (u32 i = 0; i < gGfxState.vaoIboMapCount; ++i) {
            PlatformAssert(gGfxState.vaoIboMap[i] == 0, __LOCATION__);
        }
#endif
        MemRelease(gGfxState.vaoIboMap);
    }

    if (gGfxState.renderTargetFboMap != 0) {
        for (u32 i = 0, count = gGfxState.renderTargetFboMapCount; i < count; ++i) {
            u32 fbo = gGfxState.renderTargetFboMap[i * 3 + 2];
            PlatformAssert(fbo == 0, __LOCATION__);
            if (fbo != 0) {
                glDeleteFramebuffers(1, &fbo);
                gGfxState.resourceCountFbo--;
            }
        }
        MemRelease(gGfxState.renderTargetFboMap);
    }

    if (gGfxState.shaderBoundTextureMap != 0) {
#if _DEBUG
        for (u32 i = 0, size = gGfxState.shaderBoundTextureMapCount; i < size; ++i) {
            PlatformAssert(gGfxState.shaderBoundTextureMap[i].shaderId == 0, __LOCATION__);
            // If this assert hits, not all shaders are destroyed
        }
#endif
        MemRelease(gGfxState.shaderBoundTextureMap);
    }

    FreeLibrary(gGfxState.libGL);
    MemSet(&gGfxState, 0, sizeof(GfxInternal_State));
}

extern "C" u32 GfxCreateBuffer() {
    GLuint result = 0;
    glGenBuffers(1, &result);

    if (result == 0) {
        PlatformAssert(false, __LOCATION__); // 0 needs to be invalid
        glGenBuffers(1, &result);
    }
    
    GfxInternal_SetIndexBufferType(result, GL_NONE);
    gGfxState.resourceCountBuffer++;
    return result;
}

extern "C" void GfxDestroyBuffer(u32 bufferId) {
    gGfxState.resourceCountBuffer--;
    GfxInternal_RemoveIndexBufferType(bufferId);
    glDeleteBuffers(1, &bufferId);
}

extern "C" void GfxFillArrayBuffer(u32 bufferId, void* input, u32 bytes, bool _static) {
    GfxInternal_SetIndexBufferType(bufferId, GL_NONE);
    
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ARRAY_BUFFER, bytes, input, _static? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

extern "C" void GfxFillIndexBuffer(u32 bufferId, void* input, u32 bytes, u32 indexType, bool _static) {
    if (indexType == GfxIndexTypeByte) {
        GfxInternal_SetIndexBufferType(bufferId, GL_UNSIGNED_BYTE);
    }
    else if (indexType == GfxIndexTypeShort) {
        GfxInternal_SetIndexBufferType(bufferId, GL_UNSIGNED_SHORT);
    }
    else if (indexType == GfxIndexTypeInt) {
        GfxInternal_SetIndexBufferType(bufferId, GL_UNSIGNED_INT);
    }
    else {
        PlatformAssert(false, __LOCATION__); // Invalid index type
    }
   
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes, input, _static ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

extern "C" u32 GfxCreateShader(const char* vsource, const char* fsource) {
    int success = 0;
   
    u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vsource, 0);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[256];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		glDeleteShader(vertexShader);
        PlatformAssert(false, __LOCATION__); // Error compiling vertex shader
        return 0;
    }

    u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fsource, 0);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[256];
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
        PlatformAssert(false, __LOCATION__); // Error compiling vertex shader
        return 0;
    }

    u32 shaderProgram = glCreateProgram();
    if (shaderProgram == 0) {
        PlatformAssert(false, __LOCATION__); // 0 needs to be invalid
        shaderProgram = glCreateProgram();
    }
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[256];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        PlatformAssert(false, __LOCATION__); // Error linking shader
        shaderProgram = 0;
    }

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    gGfxState.resourceCountShader++;
    return shaderProgram;
}

extern "C" void GfxDestroyShader(u32 shaderId) {
    for (u32 i = 0, size = gGfxState.shaderBoundTextureMapCount; i < size; ++i) {
        if (gGfxState.shaderBoundTextureMap[i].shaderId == shaderId) {
            gGfxState.shaderBoundTextureMap[i].shaderId = 0;
            for (u32 j = 0; j < 8; ++j) {
                gGfxState.shaderBoundTextureMap[i].uniformSlots[j] = -1;
            }
        }
    }
    gGfxState.resourceCountShader--;
    glDeleteProgram(shaderId);
}

extern "C" i32 GfxGetUniformSlot(u32 shaderId, const char* name) {
    i32 uniform = glGetUniformLocation(shaderId, name);
    PlatformAssert(uniform >= 0, __LOCATION__);
#if 1
    if (uniform < 0) {
        char infoLog[128] = {'i', 'n', 'v', 'a', 'l', 'i', 'd', ' ', 'u', 'n', 'i', 'f', 'o', 'r', 'm', ':', ' ', 0};
        for (char* iter = (char*)name, *write = infoLog + 17; *iter != '\0'; *write = *iter, iter += 1, write += 1);
        PrintDebugString(infoLog);
        return -1;
    }
#endif
    
    return uniform;
}

extern "C" i32 GfxGetAttributeSlot(u32 shaderId, const char* name) {
    i32 attribute = glGetAttribLocation(shaderId, name);
    PlatformAssert(attribute >= 0, __LOCATION__);
#if 1
    if (attribute < 0) {
        char infoLog[128] = { 'i', 'n', 'v', 'a', 'l', 'i', 'd', ' ', 'a', 't', 't', 'r', 'i', 'b', ':', ' ', 0 };
        for (char* iter = (char*)name, *write = infoLog + 16; *iter != '\0'; *write = *iter, iter += 1, write += 1);
        PrintDebugString(infoLog);
        return -1;
    }
#endif

    return attribute;
}

extern "C" u32 GfxCreateShaderVertexLayout(u32 shaderId) {
    u32 result = 0;
    glGenVertexArrays(1, &result);
    if (result == 0) {
        PlatformAssert(false, __LOCATION__); // 0 needs to be invalid
        glGenVertexArrays(1, &result);
    }
    GfxInternal_SetVaoShader(result, shaderId);
    GfxInternal_SetVaoIbo(result, 0); // Non indexed
    gGfxState.resourceCountVertexLayout++;
    return result;
}

extern "C" void GfxAddBufferToLayoutByName(u32 layoutId, const char* name, u32 bufferId, u32 numComponents, u32 strideBytes, u32 bufferType, u32 dataOffsetBytes) {
    GLenum indexBufferType = GfxInternal_GetIndexBufferType(bufferId);
    if (indexBufferType != GL_NONE && name != 0) {
        PlatformAssert(false, __LOCATION__); // Index buffer should be un-named
    }
    else if (name == 0 && indexBufferType == GL_NONE) {
        PlatformAssert(false, __LOCATION__); // Only index buffer can be un-named
    }
    
    u32 shaderId = GfxInternal_GetVaoShader(layoutId);
    i32 attribLocation = -1;
    if (name != 0) {
        attribLocation = glGetAttribLocation(shaderId, name);
    }
    GfxAddBufferToLayout(layoutId, attribLocation, bufferId, numComponents, strideBytes, bufferType, dataOffsetBytes);
}

extern "C" void GfxAddBufferToLayout(u32 layoutId, i32 attribLocation, u32 bufferId, u32 numComponents, u32 strideBytes, u32 bufferType, u32 dataOffsetBytes) {
    glBindVertexArray(layoutId);

    if (GfxInternal_GetIndexBufferType(bufferId) != GL_NONE) { // Adding index
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
        GfxInternal_SetVaoIbo(layoutId, bufferId); // Indexed
    }
    else { // Adding arrays
        GLenum type = 0;
        if (bufferType == GfxBufferTypeFloat32) {
            type = GL_FLOAT;
        }
        else if (bufferType == GfxBufferTypeInt16) {
            type = GL_SHORT;
        }
        else  if (bufferType == GfxBufferTypeInt32) {
            type = GL_INT;
        }

        PlatformAssert(attribLocation >= 0, __LOCATION__); // Not supporting invalid buffers
        if (attribLocation >= 0) {
            glBindBuffer(GL_ARRAY_BUFFER, bufferId);
            if (bufferType == GfxBufferTypeInt16 || bufferType == GfxBufferTypeInt32) {
                glVertexAttribIPointer(attribLocation, numComponents, type, strideBytes, (void*)dataOffsetBytes);
            }
            else {
                glVertexAttribPointer(attribLocation, numComponents, type, false, strideBytes, (void*)dataOffsetBytes);
            }
            glEnableVertexAttribArray(attribLocation);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
    glBindVertexArray(0);
}

extern "C" void GfxDestroyShaderVertexLayout(u32 layoutId) {
    GfxInternal_RemoveVaoShader(layoutId);
    GfxInternal_RemoveVaoIbo(layoutId);
    gGfxState.resourceCountVertexLayout--;
    glDeleteVertexArrays(1, &layoutId);
}

extern "C" u32 GfxCreateTexture(void* data, u32 width, u32 height, u32 sourceFormat, u32 targetFormat, bool genMips) {
    if (targetFormat == 4) {
        if (data != 0) {
            PlatformAssert(false, __LOCATION__); // GfxCreateTexture: can't provide data for depth texture
        }
    }

    GLenum internalFormat = 0;
    if (sourceFormat == GfxTextureFormatRGB8) { // GfxTextureFormatRGB8
        internalFormat = GL_RGB8;
    }
    else if (sourceFormat == GfxTextureFormatRGBA8) { // GfxTextureFormatRGBA8
        internalFormat = GL_RGBA8;
    }
    else if (sourceFormat == GfxTextureFormatR32F) { // GfxTextureFormatR32F
        internalFormat = GL_R32F;
    }
    else if (sourceFormat == GfxTextureFormatRGB32F) { // GfxTextureFormatRGB32F
        internalFormat = GL_RGB32F;
    }
    else if (sourceFormat == GfxTextureFormatDepth) {
        internalFormat = GL_DEPTH_COMPONENT32F;
    }
    else if (sourceFormat == GfxTextureFormatR8) {
        internalFormat = GL_R8;
    }
    else {
        PlatformAssert(false, __LOCATION__); // GfxCreateTexture: invalid internal format
    }

    GLenum textureFormat = 0;
    GLenum textureDataType = 0;
    if (targetFormat == GfxTextureFormatRGB8) { // GfxTextureFormatRGB8
        textureFormat = GL_RGB;
        textureDataType = GL_UNSIGNED_BYTE;
    }
    else if (targetFormat == GfxTextureFormatRGBA8) { // GfxTextureFormatRGBA8
        textureFormat = GL_RGBA;
        textureDataType = GL_UNSIGNED_BYTE;
    }
    else if (targetFormat == GfxTextureFormatR32F) { // GfxTextureFormatR32F
        textureFormat = GL_RED;
        textureDataType = GL_FLOAT;
    }
    else if (targetFormat == GfxTextureFormatRGB32F) { // GfxTextureFormatRGB32F
        textureFormat = GL_RGB;
        textureDataType = GL_FLOAT;
    }
    else if (targetFormat == GfxTextureFormatDepth) { // GfxTextureFormatDepth
        textureFormat = GL_DEPTH_COMPONENT;
        textureDataType = GL_FLOAT;
    }
    else if (targetFormat == GfxTextureFormatR8) {
        textureFormat = GL_RED;
        textureDataType = GL_UNSIGNED_BYTE;
    }
    else {
        PlatformAssert(false, __LOCATION__); // GfxCreateTexture: invalid target format
    }

    u32 textureId = 0;
    glGenTextures(1, &textureId);

    glBindTexture(GL_TEXTURE_2D, textureId); 
    if (targetFormat == GfxTextureFormatDepth) {
        data = 0;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, textureFormat, textureDataType, data);
    if (genMips) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0); 

    gGfxState.resourceCountTexture++;
    return textureId;
}

extern "C" void GfxDestroyTexture(u32 textureId) {
    for (u32 i = 0, size = gGfxState.renderTargetFboMapCount; i < size; ++i) {
        //u32* renderTargetFboMap; // (u32 - color texture, u32 - depth texture, u32 fbo id) (aka stride of 3)
        u32 fbo = gGfxState.renderTargetFboMap[i * 3 + 2];
        if (fbo != 0) {
            u32 color = gGfxState.renderTargetFboMap[i * 3 + 0];
            u32 depth = gGfxState.renderTargetFboMap[i * 3 + 1];

            if (color == textureId || depth == textureId) {
                glDeleteFramebuffers(1, &fbo);
                gGfxState.resourceCountFbo--;
                gGfxState.renderTargetFboMap[i * 3 + 0] = 0;
                gGfxState.renderTargetFboMap[i * 3 + 1] = 0;
                gGfxState.renderTargetFboMap[i * 3 + 2] = 0;
                break;
            }
        }
    }
    gGfxState.resourceCountTexture--;
    glDeleteTextures(1, &textureId);
}

extern "C" void GfxWriteToTexture(u32 textureId, void* data, u32 targetFormat, u32 x, u32 y, u32 w, u32 h) {
    GLenum textureFormat = 0;
    GLenum textureDataType = 0;
    if (targetFormat == GfxTextureFormatRGB8) { // GfxTextureFormatRGB8
        textureFormat = GL_RGB;
        textureDataType = GL_UNSIGNED_BYTE;
    }
    else if (targetFormat == GfxTextureFormatRGBA8) { // GfxTextureFormatRGBA8
        textureFormat = GL_RGBA;
        textureDataType = GL_UNSIGNED_BYTE;
    }
    else if (targetFormat == GfxTextureFormatR32F) { // GfxTextureFormatR32F
        textureFormat = GL_RED;
        textureDataType = GL_FLOAT;
    }
    else if (targetFormat == GfxTextureFormatRGB32F) { // GfxTextureFormatRGB32F
        textureFormat = GL_RGB;
        textureDataType = GL_FLOAT;
    }
    else if (targetFormat == GfxTextureFormatDepth) { // GfxTextureFormatDepth
        textureFormat = GL_DEPTH_COMPONENT;
        textureDataType = GL_FLOAT;
    }
    else if (targetFormat == GfxTextureFormatR8) {
        textureFormat = GL_RED;
        textureDataType = GL_UNSIGNED_BYTE;
    }
    else {
        PlatformAssert(false, __LOCATION__); // GfxCreateTexture: invalid target format
    }

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, textureFormat, textureDataType, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

extern "C" void GfxSetTextureSampler(u32 textureId, u32 wrapS, u32 wrapT, u32 minFilter, u32 mipFilter, u32 magFilter) {
    GLenum _min = 0;
    if (minFilter == GfxFilterNearest) { //GfxFilterNearest
        if (mipFilter == 0) { //GfxFilterNearest
            _min = GL_NEAREST_MIPMAP_NEAREST;
        }
        else if (mipFilter == 1) {// GfxFilterLinear
            _min = GL_NEAREST_MIPMAP_LINEAR;
        }
        else if (mipFilter == 2) { // GfxFilterNone
            _min = GL_NEAREST;
        }
        else {
            PlatformAssert(false, __LOCATION__); // invalid min / mip combo
        }
    }
    else if (minFilter == GfxFilterLinear) { // GfxFilterLinear
        if (mipFilter == GfxFilterNearest) { //GfxFilterNearest
            _min = GL_LINEAR_MIPMAP_NEAREST;
        }
        else if (mipFilter == GfxFilterLinear) {// GfxFilterLinear
            _min = GL_LINEAR_MIPMAP_LINEAR;
        }
        else if (mipFilter == GfxFilterNone) { // GfxFilterNone
            _min = GL_LINEAR;
        }
        else {
            PlatformAssert(false, __LOCATION__); // invalid 
        }
    }
    else {
        PlatformAssert(false, __LOCATION__); // invalid   min filter
    }

    GLenum _mag = 0;
    if (magFilter == 0) { //GfxFilterNearest
        _mag = GL_NEAREST;
    }
    else if (magFilter == 1) { // GfxFilterLinear
        _mag = GL_LINEAR;
    }
    else {
        PlatformAssert(false, __LOCATION__); // invalid   mag filter
    }

    GLenum _wrapS = 0;
    if (wrapS == 0) { // GfxWrapRepeat
        _wrapS = GL_REPEAT;
    }
    else if (wrapS == 1) { // GfxWrapClamp
        _wrapS = GL_CLAMP_TO_EDGE;
    }
    else {
        PlatformAssert(false, __LOCATION__); // invalid wrap s
    }

    GLenum _wrapT = 0;
    if (wrapT == 0) { // GfxWrapRepeat
        _wrapT = GL_REPEAT;
    }
    else if (wrapT == 1) { // GfxWrapClamp
        _wrapT = GL_CLAMP_TO_EDGE;
    }
    else {
        PlatformAssert(false, __LOCATION__); // invalid wrap t
    }

    glBindTexture(GL_TEXTURE_2D, textureId); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _min);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _mag);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrapT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void GfxInternal_BindShader(u32 shaderId) {
    if (shaderId != gGfxState.boundShader) {
        if (gGfxState.boundShader != 0) {
            GfxInternal_ShaderBoundTextures* textures = GfxInternal_GetShaderBoundTextures(shaderId);
            if (textures != 0) {
                for (u32 i = 0; i < 8; ++i) {
                    if (textures->uniformSlots[i] != -1) {
                        glActiveTexture(GL_TEXTURE0 + i);
                        glBindTexture(GL_TEXTURE_2D, 0);
                        textures->uniformSlots[i] = -1;
                    }
                }
            }
            glActiveTexture(GL_TEXTURE0);
        }

        glUseProgram(shaderId);
        gGfxState.boundShader = shaderId;
    }
}


static void GfxInternal_SetTexture(u32 program, u32 uniformSlot, u32 textureObject) {
    GfxInternal_ShaderBoundTextures* tracker = 0;
    GfxInternal_ShaderBoundTextures* firstFree = 0;

    // Look for the shader
    for (u32 i = 0, size = gGfxState.shaderBoundTextureMapCount; i < size; ++i) {
        if (gGfxState.shaderBoundTextureMap[i].shaderId == program) {
            tracker = &gGfxState.shaderBoundTextureMap[i];
            break;  // Found a slot!
        }
        else if (gGfxState.shaderBoundTextureMap[i].shaderId == 0) {
            if (firstFree == 0) {
                firstFree = &gGfxState.shaderBoundTextureMap[i];
            }
        }
    }

    if (tracker == 0) { // Shader not found
        if (firstFree != 0) { // Recycle
            tracker = firstFree;
            for (u32 i = 0; i < 8; ++i) {
                //tracker->textureUnits[i] = 0;
                tracker->uniformSlots[i] = -1;
            }
        }
        else { // Create new
            if (gGfxState.shaderBoundTextureMapCount == gGfxState.shaderBoundTextureMapCapacity) {
                gGfxState.shaderBoundTextureMapCapacity = (gGfxState.shaderBoundTextureMapCapacity == 0) ? 16 : gGfxState.shaderBoundTextureMapCapacity * 2;
                gGfxState.shaderBoundTextureMap = (GfxInternal_ShaderBoundTextures*)MemRealloc(gGfxState.shaderBoundTextureMap, gGfxState.shaderBoundTextureMapCapacity * sizeof(GfxInternal_ShaderBoundTextures));
                for (u32 i = gGfxState.shaderBoundTextureMapCount, size = gGfxState.shaderBoundTextureMapCapacity; i < size; ++i) {
                    GfxInternal_ShaderBoundTextures* clear = &gGfxState.shaderBoundTextureMap[i];
                    clear->shaderId = 0;
                    for (u32 i = 0; i < 8; ++i) {
                        //clear->textureUnits[i] = 0;
                        clear->uniformSlots[i] = -1;
                    }
                }
            }
            tracker = &gGfxState.shaderBoundTextureMap[gGfxState.shaderBoundTextureMapCount++];
        }
    }

    PlatformAssert(tracker != 0, __LOCATION__);
    tracker->shaderId = program;

    i32 uniformIndex = -1;
    for (u32 i = 0; i < 8; ++i) {
        if (tracker->uniformSlots[i] == uniformSlot) {
            uniformIndex = i;
            break;
        }
    }
    if (uniformIndex == -1) {
        for (u32 i = 0; i < 8; ++i) {
            if (tracker->uniformSlots[i] == -1) {
                uniformIndex = i;
                break;
            }
        }
    }
    PlatformAssert(uniformIndex >= 0, __LOCATION__);
    PlatformAssert(uniformIndex < 8, __LOCATION__);

    //tracker->textureUnits[uniformIndex] = textureObject;
    tracker->uniformSlots[uniformIndex] = uniformSlot;

    GfxInternal_BindShader(program);

    // Activate texture unit first...
    glActiveTexture(GL_TEXTURE0 + uniformIndex);
    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureObject);
    // Set the "sample name" uniform to Texture unit index
    glUniform1i(uniformSlot, uniformIndex);
    glActiveTexture(GL_TEXTURE0);
}

static void GfxInternal_BindRenderTarget(u32 colorTextureId, u32 depthTextureId) { 
    u32 fbo = 0;

    if (colorTextureId == 0 && depthTextureId == 0) {
        if (gGfxState.boundFbo != fbo) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    else {
        fbo = GfxInternal_GetRenderTargetMap(colorTextureId, depthTextureId);
        if (fbo == 0) { // Create new FBO
            glGenFramebuffers(1, &fbo);
            gGfxState.resourceCountFbo++;
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            GfxInternal_SetRenderTargetMap(colorTextureId, depthTextureId, fbo);

            if (colorTextureId != 0) {
                glBindTexture(GL_TEXTURE_2D, colorTextureId);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextureId, 0);
            }
            else {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
            }

            if (depthTextureId != 0) {
                glBindTexture(GL_TEXTURE_2D, depthTextureId);
                /*if (pfc_shadow) {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                }
                else */ {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
                }
                glFramebufferTexture2D(GL_FRAMEBUFFER, 	GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0);
            }
            else {
                glFramebufferTexture2D(GL_FRAMEBUFFER, 	GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
            }
        }
        else if (fbo != gGfxState.boundFbo) {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
            GLenum dbg0 = GL_FRAMEBUFFER_UNDEFINED;
            GLenum dbg1 = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
            GLenum dbg2 = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
            GLenum dbg3 = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER;
            GLenum dbg4 = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER;
            GLenum dbg5 = GL_FRAMEBUFFER_UNSUPPORTED;
            GLenum dbg6 = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE;
            GLenum dbg8 = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS;

            PlatformAssert(false, __LOCATION__); // Framebuffer not complete
        }
    }

    gGfxState.boundFbo = fbo;
}

static GLenum GfxInternal_DrawModeToEnum(u32 mode) {
    if (mode == GfxDrawModePoints) {
        return GL_POINTS;
    }
    else if (mode == GfxDrawModeLines) {
        return GL_LINES;     
    }
    else if (mode == GfxDrawModeLineStrip) {
        return GL_LINE_STRIP;
    }
    else if (mode == GfxDrawModeTriangles) {
        return GL_TRIANGLES;
    }
    else if (mode == GfxDrawModeTriangleStrip) {
        return GL_TRIANGLE_STRIP;
    }
    else if (mode == GfxDrawModeTriangleFan) {
        return GL_TRIANGLE_FAN;
    }
    PlatformAssert(false, __LOCATION__); // invalid draw mode
    return 0;
}

extern "C" void GfxDraw(u32 colorTargetTextureId, u32 depthTargetTextureId, u32 vertexLayoutId, u32 _drawMode, u32 startIndex, u32 indexCount, u32 instanceCount) {
    GfxInternal_BindRenderTarget(colorTargetTextureId, depthTargetTextureId);
    u32 shaderId = GfxInternal_GetVaoShader(vertexLayoutId);

    GfxInternal_BindShader(shaderId);
    GLenum drawMode = GfxInternal_DrawModeToEnum(_drawMode);

    glBindVertexArray(vertexLayoutId);
    u32 indexBufferId = GfxInternal_GetVaoIbo(vertexLayoutId);
    if (indexBufferId != 0) { // Is indexed
        GLenum indexBufferType = GfxInternal_GetIndexBufferType(indexBufferId);
        if (instanceCount <= 1) {
            glDrawElements(drawMode, indexCount, indexBufferType, (void*)startIndex);
        }
        else {
            glDrawElementsInstanced(drawMode, indexCount, indexBufferType, (void*)startIndex, instanceCount);
        }
    }
    else { // Not indexed (draw arrays)
        if (instanceCount <= 1) {
            glDrawArrays(drawMode, startIndex, indexCount);
        }
        else {
            glDrawArraysInstanced(drawMode, startIndex, indexCount, instanceCount);
        }
    }

    if (gGfxState.enableTracking) {
        gGfxState.indexCount += indexCount;
        gGfxState.drawCount += 1;
    }
    glBindVertexArray(0); 
}

extern "C" void GfxSetUniform(u32 shaderId, u32 uniformSlot, void* data, u32 uniformType, u32 count) {
    if (uniformType == 10) { // GfxUniformTypeTexture
        GfxInternal_SetTexture(shaderId, uniformSlot, (u32)data);
        return;
    }
    
    GfxInternal_BindShader(shaderId);

    if (uniformType == GfxUniformTypeInt1) {
        glUniform1iv(uniformSlot, count, (GLint*)data);
    }
    else if (uniformType == GfxUniformTypeInt2) { 
        glUniform2iv(uniformSlot, count, (GLint*)data);
    }
    else if (uniformType == GfxUniformTypeInt3) {
        glUniform3iv(uniformSlot, count, (GLint*)data);
    }
    else if (uniformType == GfxUniformTypeInt4) {
        glUniform4iv(uniformSlot, count, (GLint*)data);
    }
    else if (uniformType == GfxUniformTypeFloat1) {
        glUniform1fv(uniformSlot, count, (GLfloat*)data);
    }
    else if (uniformType == GfxUniformTypeFloat2) {
        glUniform2fv(uniformSlot, count, (GLfloat*)data);
    }
    else if (uniformType == GfxUniformTypeFloat3) {
        glUniform3fv(uniformSlot, count, (GLfloat*)data);
    }
    else if (uniformType == GfxUniformTypeFloat4) {
        glUniform4fv(uniformSlot, count, (GLfloat*)data);
    }
    else if (uniformType == GfxUniformTypeFloat9) {
        glUniformMatrix3fv(uniformSlot, count, false, (GLfloat*)data);
    }
    else if (uniformType == GfxUniformTypeFloat16) {
        glUniformMatrix4fv(uniformSlot, count, false, (GLfloat*)data);
    }
    else {
        PlatformAssert(false, __LOCATION__); // invalid uniform type
    }
}

extern "C" void GfxClearAll(u32 colorTargetId, u32 depthTargetId, float r, float g, float b, float d) {
    GfxInternal_BindRenderTarget(colorTargetId, depthTargetId);

    glClearColor(r, g, b, 1.0);
    glClearDepth(d);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

extern "C" void GfxClearColor(u32 colorTargetTextureId, u32 depthTargetTextureId, float r, float g, float b) {
    GfxInternal_BindRenderTarget(colorTargetTextureId, depthTargetTextureId);

    glClearColor(r, g, b, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

extern "C" void GfxClearDepth(u32 colorTargetTextureId, u32 depthTargetTextureId, float depth) {
    GfxInternal_BindRenderTarget(colorTargetTextureId, depthTargetTextureId);

    glClearDepth(depth);
    glClear(GL_DEPTH_BUFFER_BIT);
}

static GLenum GfxInternal_BlendfuncToEnum(u32 func) {
    if (func == GfxBlendFuncZero) {   
        return GL_ZERO;           
    }
    else if (func == GfxBlendFuncOne) {               
        return GL_ONE;
    }
    else if (func == GfxBlendFuncSrcColor) {     
        return GL_SRC_COLOR;     
    }
    else if (func == GfxBlendFuncOneMinusSrcColor) {  
        return GL_ONE_MINUS_SRC_COLOR;
    }
    else if (func == GfxBlendFuncDstColor) {     
        return GL_DST_COLOR;     
    }
    else if (func == GfxBlendFuncOneMinusDstColor) {  
        return GL_ONE_MINUS_DST_COLOR;
    }
    else if (func == GfxBlendFuncSrcAlpha) {          
        return GL_SRC_ALPHA;
    }
    else if (func == GfxBlendFuncOneMinusSrcAlpha) { 
        return GL_ONE_MINUS_SRC_ALPHA; 
    }
    else if (func == GfxBlendFuncDstAlpha) {          
        return GL_DST_ALPHA;
    }
    else if (func == GfxBlendFuncOneMinusDstAlpha) {  
        return GL_ONE_MINUS_DST_ALPHA;
    }
    else if (func == GfxBlendFuncConstColor) {  
        return GL_CONSTANT_COLOR;
    }
    else if (func == GfxBlendFuncOneMinusConstColor) {
        return GL_ONE_MINUS_CONSTANT_COLOR;
    }
    else if (func == GfxBlendFuncConstAlpha) {        
        return GL_CONSTANT_ALPHA;
    }
    else if (func == GfxBlendFuncOneMinusconstAlpha) {
        return GL_ONE_MINUS_CONSTANT_ALPHA;
    }
    else if (func == GfxBlendFuncSrcAlphaSaturate) {  
        return GL_SRC_ALPHA_SATURATE;
    }

    PlatformAssert(false, __LOCATION__); //  Invalid blend state
    return 0;
}

static GLenum GfxInternal_BlendEqToEnum(u32 blendEq) {
    if (blendEq == GfxBlendEquationAdd) {    
        return GL_FUNC_ADD;   
    }
    else if (blendEq == GfxBlendEquationSubtract) {  
        return GL_FUNC_SUBTRACT;  
    }
    else if (blendEq == GfxBlendEquationReverseSubtract) {
        return GL_FUNC_REVERSE_SUBTRACT;
    }
    else if (blendEq == GfxBlendEquationMin) {     
        return GL_MIN;
    }
    else if (blendEq == GfxBlendEquationMax) {  
        return GL_MAX;        
    }

    PlatformAssert(false, __LOCATION__); //  Invalid blend equation
    return 0;
}

extern "C" void GfxSetBlendState(bool blend, f32* optBlendColor, u32 blendDstRgb, u32 blendDstAlpha, u32 blendEquationRgb, u32 blendEquationAlpha, u32 blendSrcRgb, u32 blendSrcAlpha) {
    if (blend) {
        glEnable(GL_BLEND);
    }
    else {
        glDisable(GL_BLEND);
    }

    if (optBlendColor != 0) {
        glBlendColor(optBlendColor[0], optBlendColor[1], optBlendColor[2], optBlendColor[3]);
    }

    GLenum srcAlpha = GfxInternal_BlendfuncToEnum(blendSrcAlpha);
    GLenum srcRgb = GfxInternal_BlendfuncToEnum(blendSrcRgb);
    GLenum dstAlpha = GfxInternal_BlendfuncToEnum(blendDstAlpha);
    GLenum dstRgb = GfxInternal_BlendfuncToEnum(blendDstRgb);

    if (blendDstAlpha == blendDstRgb && blendSrcAlpha == blendSrcRgb) {
        glBlendFunc(srcRgb, dstRgb);  // Same
    }
    else { // Seperate
        glBlendFuncSeparate(srcRgb, dstRgb, srcAlpha, dstAlpha);
    }

    GLenum alphaEquation = GfxInternal_BlendEqToEnum(blendEquationAlpha);
    GLenum rgbEquation = GfxInternal_BlendEqToEnum(blendEquationRgb);

    if (blendEquationAlpha == blendEquationRgb) {
        glBlendEquation(rgbEquation);  // Same
    }
    else { // Seperate
        glBlendEquationSeparate(rgbEquation, alphaEquation);
    }
}

extern "C" void GfxSetCullState(u32 cullFace, u32 faceWind) {
     if (cullFace == GfxCullFaceOff) {
        glDisable(GL_CULL_FACE);
    }
    else if (cullFace == GfxCullFaceBack) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else if (cullFace == GfxCullFaceFront) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    }
    else if (cullFace == GfxCullFaceFrontAndBack) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT_AND_BACK);
    }
    else {
         PlatformAssert(false, __LOCATION__); // invalid cull face
    }

    if (faceWind == GfxFaceWindCounterClockwise) {
        glFrontFace(GL_CCW);
    }
    else if (faceWind == GfxFaceWindClockwise) {
        glFrontFace(GL_CW);
    }
    else {
        PlatformAssert(false, __LOCATION__); // invalid face winding
    }
}

extern "C" void GfxSetDepthState(bool enable, u32 u32_depthFunc, f32* depthRange) {
    if (enable) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
    
    GLenum depthFunc = 0;
    if (u32_depthFunc == 0) { // GfxDepthFuncAlways
        depthFunc = GL_ALWAYS;
    }
    else if (u32_depthFunc == 1) { // GfxDepthFuncNever
        depthFunc = GL_NEVER;
    }
    else if (u32_depthFunc == 2) { // GfxDepthFuncEqual
        depthFunc = GL_EQUAL;
    }
    else if (u32_depthFunc == 3) { // GfxDepthFuncLEqual
        depthFunc = GL_LEQUAL;
    }
    else if (u32_depthFunc == 4) { // GfxDepthFuncGreater
        depthFunc = GL_GREATER;
    }
    else if (u32_depthFunc == 5) { // GfxDepthFuncGEqual
        depthFunc = GL_GEQUAL;
    }
    else if (u32_depthFunc == 6) { // GfxDepthFuncNotEqual
        depthFunc = GL_NOTEQUAL;
    }
    else if (u32_depthFunc == 7) { // GfxDepthFuncLess
        depthFunc = GL_LESS;
    }
    else {
        PlatformAssert("invalid depth func", __LOCATION__);
    }
    glDepthFunc(depthFunc);

    if (depthRange != 0) {
        glDepthRange(depthRange[0], depthRange[1]);
    }
}

extern "C" void GfxSetScissorState(bool enable, u32 x, u32 y, u32 w, u32 h) {
    if (enable) {
        glEnable(GL_SCISSOR_TEST);
    }
    else {
        glDisable(GL_SCISSOR_TEST);
    }

    glScissor(x, y, w, h);
}

extern "C" void GfxSetWriteMask(bool r, bool g, bool b, bool a, bool depth) {
    glColorMask(r, g, b, a);
    glDepthMask(depth);
}

extern "C" void GfxSetViewport(u32 x, u32 y, u32 w, u32 h) {
    glViewport(x, y, w, h);
}

extern "C" void GfxFinish() {
    glFinish();
}

extern "C" void GfxEnableStats(bool enabled) {
    gGfxState.enableTracking = enabled;
}

extern "C" int GfxStatsIndexCount() {
    int result = gGfxState.indexCount;
    gGfxState.indexCount = 0;
    return result;
}

extern "C" int GfxStatsDrawCount() {
    int result = gGfxState.drawCount;
    gGfxState.drawCount = 0;
    return result;
}