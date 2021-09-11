#include <pulse/pulseaudio.h>

#define NONE 0

typedef enum {
    SINK = 1,
    SOURCE,
} device_value;

typedef enum {
    MUTE = 1,
    LIST,
    SET_VOLUME,
    INCREASE_VOLUME,
    DECREASE_VOLUME,
} action_value;

typedef struct {
  device_value device;
  action_value action;
} user_input;

static user_input* pass_options(int argc, char *argv[]);
static pa_proplist* create_proplist(void);
static void house_clean(pa_context *ctx, void *userdata);
static void list_sinks_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata);
static void list_sources_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata);
static void process_action(pa_context *ctx, user_input *action);
static void context_state_callback(pa_context *ctx, void *userdata);
