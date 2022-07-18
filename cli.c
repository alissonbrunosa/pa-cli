#include <math.h>
#include <stdio.h>
#include <string.h>

#include <cli.h>
#include <util.h>

static pa_mainloop_api *mainloop_api = NULL;

int main(int argc, char *argv[]) {
    if (argc == 1) {
        errorf("No command specified\n");
        print_help();
        return 1;
    }

    user_data *data = parse_options(argc, argv);

    pa_proplist *proplist = create_proplist();
    pa_mainloop *mainloop = pa_mainloop_new();
    mainloop_api = pa_mainloop_get_api(mainloop);
    pa_context *context = pa_context_new_with_proplist(mainloop_api, NULL, proplist);

    pa_context_set_state_callback(context, context_state_callback, data);
    pa_context_connect(context, NULL, 0, NULL);

    int ret = 1;
    if (pa_mainloop_run(mainloop, &ret) < 0) {
        errorf("Failed to start mainloop.");
        return ret;
    }

    pa_proplist_free(proplist);
    pa_mainloop_free(mainloop);
    return ret;
}

static void context_state_callback(pa_context *ctx, void *userdata) {
    switch (pa_context_get_state(ctx)) {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
        break;
    case PA_CONTEXT_READY:
        process_action(ctx, userdata);
        break;
    case PA_CONTEXT_TERMINATED:
        exit(0);
        break;
    case PA_CONTEXT_FAILED:
    default:
        errorf("Could not connect: %s", pa_strerror(pa_context_errno(ctx)));
        drain_context(ctx, userdata);
        return;
    }
}

static user_data *parse_options(int argc, char *argv[]) {
    device_t device = NONE;
    action_t action = NONE;
    int volume_pecertage;
    char *device_name;

    if (streql(argv[1], "sink")) {
        device = SINK;

    } else if (streql(argv[1], "sink-input")) {
        device = SINK_INPUT;

    } else if (streql(argv[1], "source")) {
        device = SOURCE;

    } else if (streql(argv[1], "help")) {
        print_help();
        exit(0);
    }

    if (argc >= 3) {
        if (streql(argv[2], "list")) {
            action = LIST;

        } else if (streql(argv[2], "adjust-volume")) {
            action = ADJUST_VOLUME;

            if (argc != 4) {
                errorf("adjust-volume takes 1 argument\n", argv[3]);
                exit(1);
            }

            volume_pecertage = atoi(argv[3]);
            if (!volume_pecertage) {
                errorf("%s is not a valid argument\n", argv[3]);
                exit(1);
            }
        } else if (streql(argv[2], "set-default")) {
            action = SET_DEFAULT;

            if (argc != 4) {
                errorf("set-default takes 1 argument\n", argv[3]);
                exit(1);
            }


           device_name = strcopy(argv[3]);
        } else if (streql(argv[2], "mute")) {
            action = MUTE;

        }
    }

    user_data *data = malloc(sizeof(user_data));
    data->device = device;
    data->action = action;
    data->device_name = device_name;
    data->volume_pecertage = volume_pecertage;
    return data;
}

static pa_proplist *create_proplist(void) {
    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, "application.name", "cli");
    pa_proplist_sets(proplist, "application.id", "xyz.carcara.cli");
    pa_proplist_sets(proplist, "application.version", "0.0.1");
    pa_proplist_sets(proplist, "application_icon_name", "audio-card");

    return proplist;
}

static void clean(pa_context *ctx, user_data *userdata) {
    if (!userdata->device_name)
        free(userdata->device_name);

    free(userdata);

    pa_context_disconnect(ctx);
    pa_context_unref(ctx);
    mainloop_api->quit(mainloop_api, 0);
}

static void list_sinks_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata) {
    if (done) {
        drain_context(ctx, userdata);
        return;
    }

    printf("%-5d\t%-90s\t%s\n", info->index, info->name, info->description);
}

static void list_sources_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata) {
    if (done) {
        drain_context(ctx, userdata);
        return;
    }

    printf("%-5d\t%-90s\t%s\n", info->index, info->name, info->description);
}

static void process_action(pa_context *ctx, void *userdata) {
    pa_operation *o = NULL;
    user_data *data = userdata;

    switch (data->device) {
    case SINK:
        switch (data->action) {
        case LIST:
            o = pa_context_get_sink_info_list(ctx, list_sinks_callback, data);
            break;
        case SET_DEFAULT:
            o = pa_context_set_default_sink(ctx, data->device_name, success_callback, data);
            break;
        case MUTE:
        case ADJUST_VOLUME:
            o = pa_context_get_server_info(ctx, get_server_info_callback, data);
            break;
        default:
            errorf("Unkown action sink\n");
            drain_context(ctx, data);
        }
        break;
    case SOURCE:
        switch (data->action) {
        case LIST:
            o = pa_context_get_source_info_list(ctx, list_sources_callback, data);
            break;
        case SET_DEFAULT:
            o = pa_context_set_default_source(ctx, data->device_name, success_callback, data);
            break;
        case MUTE:
        case ADJUST_VOLUME:
            o = pa_context_get_server_info(ctx, get_server_info_callback, data);
            break;
        default:
            errorf("Action not supported for source\n");
            drain_context(ctx, data);
        }
        break;
    case SINK_INPUT:
        switch (data->action) {
        case LIST:
            o = pa_context_get_sink_input_info_list(ctx, list_sink_inputs_callback, data);
            break;
        case MOVE:
            break;
        default:
            errorf("Action not supported for sink-input\n");
            drain_context(ctx, data);
        }
        break;
    default:
        o = pa_context_get_server_info(ctx, print_server_info_call_callback, data);
    }

    pa_operation_unref(o);
}


static void get_server_info_callback(pa_context *ctx, const pa_server_info *info, void *userdata) {
    pa_operation *o;
    user_data *data = userdata;

    if (!info) {
        errorf("Failed to get server info: %s\n", pa_strerror(pa_context_errno(ctx)));
        drain_context(ctx, userdata);
        exit(1);
    }

    switch(data->device) {
        case SINK:
            o = pa_context_get_sink_info_by_name(ctx, info->default_sink_name, sink_info_callback, data);
            break;
        case SOURCE:
            o = pa_context_get_source_info_by_name(ctx, info->default_source_name, source_info_callback, data);
            break;
        default:
            errorf("Unkown device\n");
            drain_context(ctx, userdata);
            exit(1);
    }

    pa_operation_unref(o);
}

static void sink_info_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata) {
    if (done < 0) {
        errorf("Failed to get sink information: %s", pa_strerror(pa_context_errno(ctx)));
        drain_context(ctx, userdata);
        exit(1);
    }

    if (done)
        return;

    user_data *data = userdata;
    pa_operation *o;
    switch (data->action) {
    case MUTE:
        o = pa_context_set_sink_mute_by_index(ctx, info->index, !info->mute, success_callback, data);
        break;
    case ADJUST_VOLUME:
        o = adjust_sink_volume(ctx, info, data);
        break;
    default:
        errorf("Unkown action\n");
        drain_context(ctx, userdata);
        exit(1);
    }

    pa_operation_unref(o);
}

static void source_info_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata) {
    pa_operation *o;
    user_data *data = userdata;

    if (done < 0) {
        errorf("Failed to get source information: %s", pa_strerror(pa_context_errno(ctx)));
        drain_context(ctx, userdata);
        exit(1);
    }

    if (done)
        return;

    switch (data->action) {
    case MUTE:
        o = pa_context_set_source_mute_by_index(ctx, info->index, !info->mute, success_callback, data);
        break;
    case ADJUST_VOLUME:
        o = adjust_source_volume(ctx, info, data);
        break;
    default:
        errorf("Unkown action\n");
        drain_context(ctx, userdata);
        exit(1);
    }

    pa_operation_unref(o);
}

static pa_operation* adjust_sink_volume(pa_context *ctx, const pa_sink_info *info, user_data *data) {
    pa_cvolume *vol = calculate_volume(&info->volume, data->volume_pecertage);

    return pa_context_set_sink_volume_by_index(ctx, info->index, vol, success_callback, data);
}

static pa_operation* adjust_source_volume(pa_context *ctx, const pa_source_info *info, user_data *data) {
    pa_cvolume *vol = calculate_volume(&info->volume, data->volume_pecertage);

    return pa_context_set_source_volume_by_index(ctx, info->index, vol, success_callback, data);
}

static pa_cvolume* calculate_volume(const pa_cvolume *volume, int percentage) {
    float current_percentage = pa_cvolume_max(volume) * 100.0 / PA_VOLUME_NORM;

    pa_volume_t value = (current_percentage + percentage) * PA_VOLUME_NORM / 100.0;
    return pa_cvolume_scale((pa_cvolume*)volume, clamp(value, PA_VOLUME_MUTED, PA_VOLUME_NORM));
}

static void success_callback(pa_context *ctx, int success, void *userdata) {
    if (!success)
        errorf("Failure: %s", pa_strerror(pa_context_errno(ctx)));

    drain_context(ctx, userdata);
}

static void list_sink_inputs_callback(pa_context *ctx, const pa_sink_input_info *info, int done, void *userdata) {
    if (done) {
        drain_context(ctx, userdata);
        return;
    }

    printf("%-5d\t%s\n", info->index, info->name);
}

static void print_server_info_call_callback(pa_context *ctx, const pa_server_info *info, void *userdata) {
    pa_operation_unref(pa_context_get_sink_info_by_name(ctx, info->default_sink_name, pretty_print_sink_callback, userdata));
    pa_operation_unref(pa_context_get_source_info_by_name(ctx, info->default_source_name, pretty_print_source_callback, userdata));
}

static void pretty_print_sink_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata) {
    if (done < 0) {
        errorf("Failed to get sink information: %s", pa_strerror(pa_context_errno(ctx)));
        drain_context(ctx, userdata);
        exit(1);
    }

    if (done) {
        drain_context(ctx, userdata);
        return;
    }

    float volume = pa_cvolume_max(&info->volume) * 100.0 / PA_VOLUME_NORM;
    printf("Sink #%u\n\tName: %s\n\tDescription: %s\n\tDriver: %s\n\tVolume: %g%%\n", info->index, info->name, strnull(info->description), strnull(info->driver), roundf(volume));
}

static void pretty_print_source_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata) {
    if (done < 0) {
        errorf("Failed to get sink information: %s", pa_strerror(pa_context_errno(ctx)));
        drain_context(ctx, userdata);
        exit(1);
    }

    if (done) {
        drain_context(ctx, userdata);
        return;
    }

    printf("Source #%u\n\tName: %s\n\tDescription: %s\n\tDriver: %s\n\tVolume: %s\n", info->index, info->name, strnull(info->description), strnull(info->driver), "100%");
}

static void context_drain_complete_callback(pa_context *ctx, void *userdata) {
    clean(ctx, userdata);
}

static void drain_context(pa_context *ctx, void *userdata) {
    pa_operation *op = pa_context_drain(ctx, context_drain_complete_callback, userdata);

    if (!op)
        clean(ctx, userdata);
    else
        pa_operation_unref(op);
}


