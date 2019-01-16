
#define GL_SILENCE_DEPRECATION

#include <CoreGraphics/CoreGraphics.h>
#include <CoreVideo/CoreVideo.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/gl3.h>

#include "private.h"

#include <time.h>

static const char *vertex_shader =
"#version 400\n"
"in vec3 vp;"
"void main() {"
"  gl_Position = vec4(vp, 1.0);"
"}";

static const char *fragment_shader =
"#version 400\n"
"out vec4 color;"
"uniform vec4 bounds;"
"uniform float time;"
"void main() {"
"  vec2 v = gl_FragCoord.xy / bounds.zw;"
"  vec2 c = vec2(0.5, 0.5);"
"  float p = (cos(time * 2.0) + 1.0) * 0.5;"
"  float d = distance(v, c) * 2.0;"
"  float a = clamp(abs(p - d), 0, 1);"
"  color = vec4(a, a, a, a);"
"}";

typedef struct {
	GLuint bounds;
	GLuint time;
}
ShaderParameters;

typedef struct {
	CGSConnectionID connection;
	CGSWindowID window;
	CGRect bounds;
	CGLContextObj context;
	ShaderParameters shader;
}
Environment;

static bool MakeGL(const char *vertex_shader_str, 
	const char *fragment_shader_str, GLuint *program_ref);
static ShaderParameters MakeShaderParameters(GLuint program);
static bool MakeCGLContext(CGSConnectionID, CGSWindowID, CGRect, CGLContextObj *);
static bool MakeWindow(CGSConnectionID, CGRect, CGSWindowID *);
static CVReturn DisplayLinkCallback(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, 
	CVOptionFlags, CVOptionFlags *, void *);

int main(void) {
	CGSConnectionID connection = CGSMainConnectionID();
	CGRect frame = CGRectMake(0, 0, 400, 400);
	CGSWindowID window = 0;
	MakeWindow(connection, frame, &window);

	CGRect bounds = CGRectMake(0, 0, CGRectGetWidth(frame), CGRectGetHeight(frame));
	CGContextRef context = CGWindowContextCreate(connection, window, 0);
	CGContextSetCompositeOperation(context, kCGCompositeCopy);
	CGContextSetRGBFillColor(context, 0.0, 0.0, 0.0, 0.0);
	CGContextFillRect(context, bounds);
	CGContextFlush(context);
	CGSOrderWindow(connection, window, kCGSWindowOrderingAbove, 0);
	
	CGLContextObj cgl_context;
	MakeCGLContext(connection, window, bounds, &cgl_context);
	CGLSetCurrentContext(cgl_context);
	GLuint shader;
	MakeGL(vertex_shader, fragment_shader, &shader);
	ShaderParameters params = MakeShaderParameters(shader);
	CGLSetCurrentContext(NULL);

	Environment *environment = &(Environment) {
		.connection = connection,
		.window = window,
		.bounds = bounds,
		.context = cgl_context,
		.shader = params
	};

	CVDisplayLinkRef display;
	CVDisplayLinkCreateWithCGDisplay(CGMainDisplayID(), &display);
	CVDisplayLinkSetOutputCallback(display, &DisplayLinkCallback, environment);
	CVDisplayLinkStart(display);
	sleep(7);
	CVDisplayLinkStop(display);
	return 0;
}


static CVReturn DisplayLinkCallback(CVDisplayLinkRef display, 
	const CVTimeStamp *time_now, const CVTimeStamp *time_output, 
	CVOptionFlags inputs, CVOptionFlags *outputs, void *user_context) {
	Environment *environment = user_context;
	CGLContextObj context = environment->context;
	ShaderParameters *shader = &environment->shader;
	CGRect bounds = environment->bounds;
	double time = time_output->hostTime / (double)CVGetHostClockFrequency();
	CGLLockContext(context);
	CGLSetCurrentContext(context);

	glClear(GL_COLOR_BUFFER_BIT);
	glUniform4f(shader->bounds, 0, 0, CGRectGetWidth(bounds), CGRectGetHeight(bounds));
	glUniform1f(shader->time, (float)time);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	CGLFlushDrawable(context);

	CGLSetCurrentContext(NULL);
	CGLUnlockContext(context);
	return 0;
}

static bool MakeGL(const char *vertex_shader_str, const char *fragment_shader_str, GLuint *shader_ref) {
	float vertices[] = {
    	-1.0f,  1.0f, 0.0f, // Top-left
    	 1.0f,  1.0f, 0.0f, // Top-right
    	 1.0f, -1.0f, 0.0f, // Bottom-right
    	-1.0f, -1.0f, 0.0f, // Bottom-left
	};
	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};
	// Vertex Array
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// Data
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	GLuint ebo = 0;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	// Vertex Attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	// Shader Compilation
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader_str, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader_str, NULL);
	glCompileShader(fs);
	GLuint shader = glCreateProgram();
	glAttachShader(shader, fs);
	glAttachShader(shader, vs);
	glLinkProgram(shader);
	// Settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(shader);
	glBindVertexArray(vao);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	*shader_ref = shader;
	return true;
}

static ShaderParameters MakeShaderParameters(GLuint program) {
	return (ShaderParameters) {
		.bounds = glGetUniformLocation(program, "bounds"),
		.time = glGetUniformLocation(program, "time")
	};
}

static bool MakeWindow(CGSConnectionID connection, CGRect frame, CGSWindowID *ref) {
	CGSRegion region;
	if (CGSNewRegionWithRect(&frame, &region) != kCGSErrorSuccess) {
		return false;
	}
	CGSWindowID window;
	if (CGSNewWindow(connection, kCGSBufferedBackingType, 0, 0, region, &window) != kCGSErrorSuccess) {
		CGSReleaseRegion(region);
		return false;
	}
	CGSReleaseRegion(region);
	if (CGSSetWindowOpacity(connection, window, kCGSFalse) != kCGSErrorSuccess) {
		return false;
	}
	*ref = window;
	return true;
}

static bool MakeCGLContext(CGSConnectionID connection, CGSWindowID window, CGRect bounds, CGLContextObj *ref) {
	CGLPixelFormatAttribute attributes[] = {
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
		kCGLPFAColorSize, 24,
		kCGLPFAAlphaSize, 8,
		kCGLPFADoubleBuffer,
		kCGLPFAAccelerated,
		kCGLPFASampleBuffers, 1,
		kCGLPFASamples, 4,
		(CGLPixelFormatAttribute)0
	};
	// Context Allocation
	CGLPixelFormatObj format;
	GLint nvirt; // number of virtual screens
 	if (CGLChoosePixelFormat(attributes, &format, &nvirt) != kCGLNoError) {
		return false;	 
	}
	CGLContextObj context;
	if (CGLCreateContext(format, NULL, &context) != kCGLNoError) {
		return false;
	}
	CGLDestroyPixelFormat(format);
	format = NULL;
	// Context Setup
	CGLSetCurrentContext(context);
	// V-Sync
	GLint v_sync_enabled = 1;
	CGLSetParameter(context, kCGLCPSwapInterval, &v_sync_enabled);
	// Opaque
	GLint opaque = 0;
	CGLSetParameter(context, kCGLCPSurfaceOpacity, &opaque);
	// Surface Setup
	CGSSurfaceID surface;
	if (CGSAddSurface(connection, window, &surface) != kCGSErrorSuccess) {
		return false;
	}
	CGSSetSurfaceBounds(connection, window, surface, bounds);
	CGSOrderSurface(connection, window, surface, 1, 0);
	CGLSetSurface(context, connection, window, surface);
	// Checks
	GLint drawable;
	if (CGLGetParameter(context, kCGLCPHasDrawable, &drawable) != kCGLNoError) {
		return false;
	}
	if (drawable == 0) {
		return false;
	}
	CGLSetCurrentContext(NULL);
	*ref = context;
	return true;
}