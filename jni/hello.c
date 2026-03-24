#include <android_native_app_glue.h>
#include <android/log.h>

#define LOG_TAG "OdfizNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void android_main(struct android_app* state) {
    // Memastikan app tidak langsung exit
    LOGI("Hello World dari Native C!");

    while (1) {
        int ident, events;
        struct android_poll_source* source;
        while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) source->process(state, source);
            if (state->destroyRequested != 0) return;
        }
    }
}

