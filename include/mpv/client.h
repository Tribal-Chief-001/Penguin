/*
 * MPV Client API Header
 * Standard libmpv C interface
 */

#ifndef MPV_CLIENT_H_
#define MPV_CLIENT_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MPV_ENABLE_DEPRECATED 0

typedef struct mpv_handle mpv_handle;

typedef enum mpv_error {
    MPV_ERROR_SUCCESS           = 0,
    MPV_ERROR_EVENT_QUEUE_FULL  = -1,
    MPV_ERROR_NOMEM             = -2,
    MPV_ERROR_UNINITIALIZED     = -3,
    MPV_ERROR_INVALID_PARAMETER = -4,
    MPV_ERROR_OPTION_NOT_FOUND  = -5,
    MPV_ERROR_OPTION_FORMAT     = -6,
    MPV_ERROR_OPTION_ERROR      = -7,
    MPV_ERROR_PROPERTY_NOT_FOUND= -8,
    MPV_ERROR_PROPERTY_FORMAT   = -9,
    MPV_ERROR_PROPERTY_UNAVAILABLE= -10,
    MPV_ERROR_PROPERTY_ERROR    = -11,
    MPV_ERROR_COMMAND           = -12,
    MPV_ERROR_LOADING_FAILED    = -13,
    MPV_ERROR_AO_INIT_FAILED    = -14,
    MPV_ERROR_VO_INIT_FAILED    = -15,
    MPV_ERROR_NOTHING_TO_PLAY   = -16,
    MPV_ERROR_UNKNOWN_FORMAT    = -17,
    MPV_ERROR_UNSUPPORTED       = -18,
    MPV_ERROR_NOT_IMPLEMENTED   = -19,
    MPV_ERROR_GENERIC           = -20
} mpv_error;

const char *mpv_error_string(int error);
void mpv_free(void *data);
const char *mpv_client_name(mpv_handle *ctx);
int64_t mpv_client_id(mpv_handle *ctx);
mpv_handle *mpv_create(void);
int mpv_initialize(mpv_handle *ctx);
void mpv_destroy(mpv_handle *ctx);
void mpv_terminate_destroy(mpv_handle *ctx);
mpv_handle *mpv_create_client(mpv_handle *ctx, const char *name);
mpv_handle *mpv_create_weak_client(mpv_handle *ctx, const char *name);
int mpv_load_config_file(mpv_handle *ctx, const char *filename);
int64_t mpv_get_time_us(mpv_handle *ctx);

typedef enum mpv_format {
    MPV_FORMAT_NONE             = 0,
    MPV_FORMAT_STRING           = 1,
    MPV_FORMAT_OSD_STRING       = 2,
    MPV_FORMAT_FLAG             = 3,
    MPV_FORMAT_INT64            = 4,
    MPV_FORMAT_DOUBLE           = 5,
    MPV_FORMAT_NODE             = 6,
    MPV_FORMAT_NODE_ARRAY       = 7,
    MPV_FORMAT_NODE_MAP         = 8,
    MPV_FORMAT_BYTE_ARRAY       = 9
} mpv_format;

typedef struct mpv_node mpv_node;

typedef struct mpv_node_list {
    int num;
    mpv_node *values;
    char **keys;
} mpv_node_list;

typedef struct mpv_byte_array {
    void *data;
    size_t size;
} mpv_byte_array;

struct mpv_node {
    union {
        char *string;
        int flag;
        int64_t int64_;
        double double_;
        mpv_node_list *list;
        mpv_byte_array *ba;
    } u;
    mpv_format format;
};

void mpv_free_node_contents(mpv_node *node);

int mpv_set_option(mpv_handle *ctx, const char *name, mpv_format format, void *data);
int mpv_set_option_string(mpv_handle *ctx, const char *name, const char *data);

int mpv_command(mpv_handle *ctx, const char **args);
int mpv_command_node(mpv_handle *ctx, mpv_node *args, mpv_node *result);
int mpv_command_ret(mpv_handle *ctx, const char **args, mpv_node *result);
int mpv_command_string(mpv_handle *ctx, const char *args);
int mpv_command_async(mpv_handle *ctx, uint64_t reply_userdata, const char **args);
int mpv_command_node_async(mpv_handle *ctx, uint64_t reply_userdata, mpv_node *args);

void mpv_abort_async_command(mpv_handle *ctx, uint64_t reply_userdata);

int mpv_set_property(mpv_handle *ctx, const char *name, mpv_format format, void *data);
int mpv_set_property_string(mpv_handle *ctx, const char *name, const char *data);
int mpv_set_property_async(mpv_handle *ctx, uint64_t reply_userdata, const char *name, mpv_format format, void *data);
int mpv_get_property(mpv_handle *ctx, const char *name, mpv_format format, void *data);
char *mpv_get_property_string(mpv_handle *ctx, const char *name);
char *mpv_get_property_osd_string(mpv_handle *ctx, const char *name);
int mpv_get_property_async(mpv_handle *ctx, uint64_t reply_userdata, const char *name, mpv_format format);
int mpv_observe_property(mpv_handle *mpv, uint64_t reply_userdata, const char *name, mpv_format format);
int mpv_unobserve_property(mpv_handle *mpv, uint64_t registered_reply_userdata);

typedef enum mpv_event_id {
    MPV_EVENT_NONE              = 0,
    MPV_EVENT_SHUTDOWN          = 1,
    MPV_EVENT_LOG_MESSAGE       = 2,
    MPV_EVENT_GET_PROPERTY_REPLY= 3,
    MPV_EVENT_SET_PROPERTY_REPLY= 4,
    MPV_EVENT_COMMAND_REPLY     = 5,
    MPV_EVENT_START_FILE        = 6,
    MPV_EVENT_END_FILE          = 7,
    MPV_EVENT_FILE_LOADED       = 8,
    MPV_EVENT_IDLE              = 11,
    MPV_EVENT_TICK              = 14,
    MPV_EVENT_CLIENT_MESSAGE    = 16,
    MPV_EVENT_VIDEO_RECONFIG    = 17,
    MPV_EVENT_AUDIO_RECONFIG    = 18,
    MPV_EVENT_SEEK              = 20,
    MPV_EVENT_PLAYBACK_RESTART  = 21,
    MPV_EVENT_PROPERTY_CHANGE   = 22,
    MPV_EVENT_QUEUE_OVERFLOW    = 24,
    MPV_EVENT_HOOK              = 25
} mpv_event_id;

typedef struct mpv_event_property {
    const char *name;
    mpv_format format;
    void *data;
} mpv_event_property;

typedef enum mpv_log_level {
    MPV_LOG_LEVEL_NONE  = 0,
    MPV_LOG_LEVEL_FATAL = 10,
    MPV_LOG_LEVEL_ERROR = 20,
    MPV_LOG_LEVEL_WARN  = 30,
    MPV_LOG_LEVEL_INFO  = 40,
    MPV_LOG_LEVEL_V     = 50,
    MPV_LOG_LEVEL_DEBUG = 60,
    MPV_LOG_LEVEL_TRACE = 70
} mpv_log_level;

typedef struct mpv_event_log_message {
    const char *prefix;
    const char *level;
    const char *text;
    mpv_log_level log_level;
} mpv_event_log_message;

typedef enum mpv_end_file_reason {
    MPV_END_FILE_REASON_EOF      = 0,
    MPV_END_FILE_REASON_STOP     = 2,
    MPV_END_FILE_REASON_QUIT     = 3,
    MPV_END_FILE_REASON_ERROR    = 4,
    MPV_END_FILE_REASON_REDIRECT = 5
} mpv_end_file_reason;

typedef struct mpv_event_end_file {
    int reason;
    int error;
    int64_t playlist_entry_id;
    int64_t playlist_insert_id;
    int playlist_insert_num_entries;
} mpv_event_end_file;

typedef struct mpv_event {
    mpv_event_id event_id;
    int error;
    uint64_t reply_userdata;
    void *data;
} mpv_event;

const char *mpv_event_name(mpv_event_id event);
mpv_event *mpv_wait_event(mpv_handle *ctx, double timeout);
void mpv_wakeup(mpv_handle *ctx);
void mpv_set_wakeup_callback(mpv_handle *ctx, void (*cb)(void *d), void *d);
void mpv_wait_async_requests(mpv_handle *ctx);
int mpv_request_log_messages(mpv_handle *ctx, const char *min_level);

#ifdef __cplusplus
}
#endif

#endif
