#include <pulse/pulseaudio.h>

#define NONE 0

typedef enum {
    SINK = 1,
    SOURCE,
} device_value;

typedef enum {
    MUTE = 1,
    LIST,
    ADJUST_VOLUME,
} action_value;

typedef struct {
    int volume_pecertage;
    device_value device;
    action_value action;
} user_data;


static user_data* pass_options(int argc, char *argv[]);

static pa_proplist* create_proplist(void);

static void clean(pa_context *ctx, user_data *input);

static void success_callback(pa_context *ctx, int success, void *userdata);

static pa_operation* adjust_sink_volume(pa_context *ctx, const pa_sink_info *info, user_data *data);

static pa_operation* adjust_source_volume(pa_context *ctx, const pa_source_info *info, user_data *data);

static void sink_info_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata);

static void source_info_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata);

static void list_sinks_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata);

static void list_sources_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata);

static void process_action(pa_context *ctx, void *userdata);

static void context_state_callback(pa_context *ctx, void *userdata);

static void get_server_info_callback(pa_context *ctx, const pa_server_info *info, void *userdata);

static pa_cvolume* calculate_volume(const pa_cvolume *volume, int percentage);
