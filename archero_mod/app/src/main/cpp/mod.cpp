#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "And64InlineHook.hpp"

#define LOG_TAG "ArcheroMod"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Offsets taken from the v7.7.1 IL2CPP dump.
static constexpr uintptr_t kEntityDataEntityOffset = 0x18;
static constexpr uintptr_t kEntityBaseTypeOffset = 0x1184;
static constexpr int kEntityTypeHero = 1;

using GetHeadShotFn = bool (*)(void* thiz, void* source, void* data, void* method);
using GetMissFn = bool (*)(void* thiz, void* otherhs, void* method);

static GetHeadShotFn g_orig_get_headshot = nullptr;
static GetMissFn g_orig_get_miss = nullptr;

// Equivalent to the original menu state being toggled on.
static bool g_enable_headshot = true;
static bool g_enable_godmode = true;

uintptr_t get_base_address(const char *name) {
    FILE *f = fopen("/proc/self/maps", "r");
    if (!f) return 0;
    char line[512];
    uintptr_t base = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, name)) {
            sscanf(line, "%lx", &base);
            break;
        }
    }
    fclose(f);
    return base;
}

static int get_entity_type_from_entity_data(void* thiz) {
    if (!thiz) {
        return -1;
    }
    void* entity_base = *reinterpret_cast<void**>(
        reinterpret_cast<uintptr_t>(thiz) + kEntityDataEntityOffset
    );
    if (!entity_base) {
        return -1;
    }
    return *reinterpret_cast<int*>(
        reinterpret_cast<uintptr_t>(entity_base) + kEntityBaseTypeOffset
    );
}

// Mirrors the original mod's side-aware behavior:
// - force headshot only for non-hero targets.
static bool hk_get_headshot(void* thiz, void* source, void* data, void* method) {
    if (!g_orig_get_headshot) {
        return true;
    }
    if (!g_enable_headshot) {
        return g_orig_get_headshot(thiz, source, data, method);
    }
    int entity_type = get_entity_type_from_entity_data(thiz);
    if (entity_type < 0) {
        return g_orig_get_headshot(thiz, source, data, method);
    }
    if (entity_type != kEntityTypeHero) {
        return true;
    }
    return g_orig_get_headshot(thiz, source, data, method);
}

// Mirrors the original mod's side-aware behavior:
// - force miss only for hero targets.
static bool hk_get_miss(void* thiz, void* otherhs, void* method) {
    if (!g_orig_get_miss) {
        return true;
    }
    if (!g_enable_godmode) {
        return g_orig_get_miss(thiz, otherhs, method);
    }
    int entity_type = get_entity_type_from_entity_data(thiz);
    if (entity_type < 0) {
        return g_orig_get_miss(thiz, otherhs, method);
    }
    if (entity_type == kEntityTypeHero) {
        return true;
    }
    return g_orig_get_miss(thiz, otherhs, method);
}

void* hack_thread(void* arg) {
    LOGD("Hack thread started, waiting for libil2cpp.so...");
    uintptr_t il2cpp_base = 0;
    while ((il2cpp_base = get_base_address("libil2cpp.so")) == 0) {
        sleep(1);
    }
    LOGD("libil2cpp.so found at %p", (void*)il2cpp_base);

    // RVAs from report
    uintptr_t get_headshot_addr = il2cpp_base + 0x4F68364;
    uintptr_t get_miss_addr = il2cpp_base + 0x4F5B440;

    LOGD("Hooking EntityData$$GetHeadShot at %p", (void*)get_headshot_addr);
    A64HookFunction(
        (void*)get_headshot_addr,
        (void*)hk_get_headshot,
        reinterpret_cast<void**>(&g_orig_get_headshot)
    );

    LOGD("Hooking EntityData$$GetMiss at %p", (void*)get_miss_addr);
    A64HookFunction(
        (void*)get_miss_addr,
        (void*)hk_get_miss,
        reinterpret_cast<void**>(&g_orig_get_miss)
    );

    LOGD("Hooks applied (side-aware): headshot=%d godmode=%d", g_enable_headshot, g_enable_godmode);
    return nullptr;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGD("JNI_OnLoad called in Archero mod");
    pthread_t t;
    pthread_create(&t, nullptr, hack_thread, nullptr);
    return JNI_VERSION_1_6;
}