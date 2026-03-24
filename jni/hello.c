#include <android_native_app_glue.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

struct engine {
    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
};

// Fungsi pembaca CPU Load dari sistem Linux
float get_cpu_load() {
    static long long last_user, last_idle;
    long long user, nice, system, idle, iowait, irq, softirq;
    FILE* fp = fopen("/proc/stat", "r");
    if (!fp) return 0.5f;
    fscanf(fp, "cpu %lld %lld %lld %lld %lld %lld %lld", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    fclose(fp);
    long long current_user = user + nice + system + iowait + irq + softirq;
    long long diff_user = current_user - last_user;
    long long diff_idle = idle - last_idle;
    last_user = current_user; last_idle = idle;
    float total = diff_user + diff_idle;
    return (total > 0) ? (float)diff_user / total : 0.0f;
}

// Fungsi gambar kotak warna (Bar) menggunakan glScissor
void draw_bar(float pct, int row, float r, float g, float b, int screen_w, int screen_h) {
    int bar_h = screen_h / 8;
    int spacing = screen_h / 12;
    int y_pos = screen_h - (row * (bar_h + spacing));
    int bar_w = (int)(pct * (screen_w - 100));

    glEnable(GL_SCISSOR_TEST);
    glScissor(50, y_pos, bar_w, bar_h);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) return;

    // Hitung waktu untuk efek denyut (Pulse)
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    float t = res.tv_sec + (float)res.tv_nsec / 1e9f;
    float pulse = (sinf(t * 4.0f) * 0.3f) + 0.7f; // Denyut antara 0.4 - 1.0

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Background abu gelap
    glClear(GL_COLOR_BUFFER_BIT);

    // Render 4 Bar System
    draw_bar(get_cpu_load(), 1, 1.0f * pulse, 0.2f, 0.2f, engine->width, engine->height); // CPU (Red Pulse)
    draw_bar(0.65f, 2, 0.2f, 1.0f, 0.2f, engine->width, engine->height); // RAM (Green)
    draw_bar(0.85f, 3, 0.2f, 0.5f, 1.0f, engine->width, engine->height); // BAT (Blue)
    draw_bar(0.40f, 4, 0.7f, 0.7f, 0.7f, engine->width, engine->height); // STR (White)

    eglSwapBuffers(engine->display, engine->surface);
}

static void engine_init_display(struct engine* engine) {
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    EGLConfig config; EGLint numConfigs;
    const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    EGLSurface surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    EGLContext context = eglCreateContext(display, config, NULL, NULL);
    eglMakeCurrent(display, surface, surface, context);
    eglQuerySurface(display, surface, EGL_WIDTH, &engine->width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &engine->height);
    engine->display = display; engine->surface = surface; engine->context = context;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
    }
}

void android_main(struct android_app* state) {
    struct engine engine;
    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    engine.app = state;

    while (1) {
        int ident, events;
        struct android_poll_source* source;
        while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) return;
        }
        engine_draw_frame(&engine); // Animasi terus berjalan
    }
}

