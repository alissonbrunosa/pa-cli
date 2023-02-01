#include <pulse/pulseaudio.h>

#define NONE 0

typedef enum {
    SINK = 1,
    SINK_INPUT,
    SOURCE,
} device_t;

typedef enum {
    MUTE = 1,
    LIST,
    MOVE,
    GET_VOLUME,
    ADJUST_VOLUME,
    SET_DEFAULT,
} action_t;

typedef struct {
    int volume_pecertage;
    device_t device;
    action_t action;
    char *device_name;
} user_data;


static user_data* parse_options(int argc, char *argv[]);

static pa_proplist* create_proplist(void);

static void clean(pa_context *ctx, user_data *input);

static void success_callback(pa_context *ctx, int success, void *userdata);

static pa_operation* adjust_sink_volume(pa_context *ctx, const pa_sink_info *info, user_data *data);

static void print_sink_volume(pa_context *ctx, const pa_sink_info *info, user_data *data);

static void print_source_volume(pa_context *ctx, const pa_source_info *info);

static pa_operation* adjust_source_volume(pa_context *ctx, const pa_source_info *info, user_data *data);

static void sink_info_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata);

static void source_info_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata);

static void list_sinks_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata);

static void list_sink_inputs_callback(pa_context *ctx, const pa_sink_input_info *info, int done, void *userdata);

static void list_sources_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata);

static void process_action(pa_context *ctx, void *userdata);

static void context_state_callback(pa_context *ctx, void *userdata);

static void get_server_info_callback(pa_context *ctx, const pa_server_info *info, void *userdata);

static pa_cvolume* calculate_volume(const pa_cvolume *volume, int percentage);

static void print_server_info_call_callback(pa_context *ctx, const pa_server_info *info, void *userdata);

static void pretty_print_sink_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata);

static void pretty_print_source_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata);

static void context_drain_complete_callback(pa_context *ctx, void *userdata);

static void drain_context(pa_context *ctx, void *userdata);

static void move_sink_input_callback(pa_context *ctx, const pa_sink_input_info *info, int done, void *userdata);

static void default_sink_success_callback(pa_context *ctx, int success, void *userdata);

static void default_source_success_callback(pa_context *ctx, int success, void *userdata);

static void move_source_output_callback(pa_context *ctx, const pa_source_output_info *info, int done, void *userdata);
