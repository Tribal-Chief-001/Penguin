/*
 * MPV OpenGL Render API Header
 */

#ifndef MPV_RENDER_GL_H_
#define MPV_RENDER_GL_H_

#include "render.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mpv_opengl_init_params {
    void *(*get_proc_address)(void *ctx, const char *name);
    void *get_proc_address_ctx;
    const char *extra_exts;
} mpv_opengl_init_params;

typedef struct mpv_opengl_fbo {
    int fbo;
    int w;
    int h;
    int internal_format;
} mpv_opengl_fbo;

typedef struct mpv_opengl_drm_params {
    int fd;
    int crtc_id;
    int connector_id;
    struct _drmModeAtomicReq **atomic_request_ptr;
    int render_fd;
} mpv_opengl_drm_params;

typedef struct mpv_opengl_drm_params_v2 {
    int fd;
    int crtc_id;
    int connector_id;
    struct _drmModeAtomicReq **atomic_request_ptr;
    int render_fd;
} mpv_opengl_drm_params_v2;

#ifdef __cplusplus
}
#endif

#endif
