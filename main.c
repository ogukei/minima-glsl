
#define GL_SILENCE_DEPRECATION

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreVideo/CoreVideo.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/gl3.h>
#include "private.h"

// Vertex Shader
static const char *kVertexShader =
"#version 400\n"
"in vec3 position;"
"void main() {"
"    gl_Position = vec4(position, 1.0);"
"}";

// Fragment Shader
static const char *kFragmentShader =
"#version 400\n"
"out vec4 color;"
"uniform vec4 bounds;"
"uniform float time;\n"
"void main() {"
"    vec2 v = gl_FragCoord.xy / bounds.zw;"
"    vec2 p = vec2(sin(time) * 0.1 + 0.5, sin(time + 1) * 0.1 + 0.5);"
"    float d = distance(v, p) * 2.0;"
"    float r = cos(time) * 0.05 + 0.075;"
"    vec3 k = vec3(1.0, 2.0 / r, 1.0 / (r * r));"
"    float a = 1.0 / (k.x + k.y*d + k.z*d*d);"
"    color = vec4(a, a, a, 0.9);"
"}";

typedef struct {
    GLuint bounds;
    GLuint time;
} ShaderParameters;

typedef struct {
    CGRect bounds;
    CGLContextObj context;
    ShaderParameters shader;
} Environment;

static bool MakeGL(const char *vertex_shader, const char *fragment_shader, GLuint *program);
static ShaderParameters MakeShaderParameters(GLuint program);
static bool VerifyShader(GLuint shader, const char *label);
static bool MakeCGL(CGSConnectionID, CGSWindowID, CGRect, CGLContextObj *);
static bool MakeWindow(CGSConnectionID, CGRect, CGSWindowID *);
static CVReturn RenderFrame(CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, 
    CVOptionFlags, CVOptionFlags *, void *);

int main(void) {
    CGSConnectionID connection = CGSMainConnectionID();
    CGRect frame = CGRectMake(0, 0, 400, 400);
    // creates a window
    CGSWindowID window = 0;
    bool ok = MakeWindow(connection, frame, &window);
    assert(ok);
    // adds an OpenGL surface onto the window
    CGRect bounds = CGRectMake(0, 0, CGRectGetWidth(frame), CGRectGetHeight(frame));
    CGLContextObj cgl_context;
    ok = MakeCGL(connection, window, bounds, &cgl_context);
    assert(ok);
    CGLSetCurrentContext(cgl_context);
    // compiles GLSL shader
    GLuint program;
    ok = MakeGL(kVertexShader, kFragmentShader, &program);
    assert(ok);
    ShaderParameters shader = MakeShaderParameters(program);
    CGLSetCurrentContext(NULL);
    Environment *environment = &(Environment) {
        .bounds = bounds,
        .context = cgl_context,
        .shader = shader
    };
    // starts frame rendering with Display Link
    CVDisplayLinkRef display = NULL;
    CVDisplayLinkCreateWithCGDisplay(CGSMainDisplayID(), &display);
    CVDisplayLinkSetOutputCallback(display, &RenderFrame, environment);
    CVDisplayLinkStart(display);
    sleep(10);
    CVDisplayLinkStop(display);
    return 0;
}

static CVReturn RenderFrame(CVDisplayLinkRef display, 
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

static bool MakeGL(const char *vertex_shader, const char *fragment_shader, GLuint *program_ref) {
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
    // Data Transfer
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
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    assert(VerifyShader(vs, "vertex"));
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    assert(VerifyShader(fs, "fragment"));
    GLuint program = glCreateProgram();
    glAttachShader(program, fs);
    glDeleteShader(fs);
    glAttachShader(program, vs);
    glDeleteShader(vs);
    glLinkProgram(program);
    // Settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(program);
    glBindVertexArray(vao);
    *program_ref = program;
    return true;
}

static ShaderParameters MakeShaderParameters(GLuint program) {
    return (ShaderParameters) {
        .bounds = glGetUniformLocation(program, "bounds"),
        .time = glGetUniformLocation(program, "time")
    };
}

static bool VerifyShader(GLuint shader, const char *label) {
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_FALSE) return true;
    GLint max_length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);
    char *buffer = malloc(max_length);
    glGetShaderInfoLog(shader, max_length, &max_length, buffer);
    fprintf(stderr, "%s shader compilation error: \n%s\n", label, buffer);
    free(buffer);
    return false;
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
    // Present Window
    {
        CGRect bounds = CGRectMake(0, 0, CGRectGetWidth(frame), CGRectGetHeight(frame));
        CGContextRef context = CGWindowContextCreate(connection, window, 0);
        CGContextSetCompositeOperation(context, kCGCompositeCopy);
        CGContextSetRGBFillColor(context, 0.0, 0.0, 0.0, 0.0);
        CGContextFillRect(context, bounds);
        CGContextFlush(context);
        CGContextRelease(context);
        context = NULL;
        CGSSetWindowLevel(connection, window, CGWindowLevelForKey(kCGOverlayWindowLevelKey));
        int tags = kCGSIgnoreMouseEventsTag;
        CGSSetWindowTags(connection, window, &tags, 32);
        CGSOrderWindow(connection, window, kCGSWindowOrderingAbove, 0);
    }
    *ref = window;
    return true;
}

static bool MakeCGL(CGSConnectionID connection, CGSWindowID window, CGRect bounds, CGLContextObj *ref) {
    CGLPixelFormatAttribute attributes[] = {
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
        kCGLPFAColorSize, 24,
        kCGLPFAAlphaSize, 8,
        kCGLPFADoubleBuffer,
        kCGLPFAAccelerated,
        //kCGLPFASampleBuffers, 1,
        //kCGLPFASamples, 4,
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