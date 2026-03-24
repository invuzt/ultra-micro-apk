#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES/gl.h>

void android_main(struct android_app* state) {
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);

    EGLConfig config; 
    EGLint numConfigs;
    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
        EGL_NONE
    };

    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    EGLSurface surface = eglCreateWindowSurface(display, config, state->window, NULL);
    EGLContext context = eglCreateContext(display, config, NULL, NULL);
    eglMakeCurrent(display, surface, surface, context);

    while (1) {
        int ident, events;
        struct android_poll_source* source;
        while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) return;
        }

        // Background Putih Bersih
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        eglSwapBuffers(display, surface);
    }
}
