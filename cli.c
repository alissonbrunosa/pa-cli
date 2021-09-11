#include <stdio.h>
#include <string.h>

#include <yapc.h>
#include <helpers.h>

static pa_mainloop_api* mainloop_api = NULL;

int main(int argc, char *argv[]) {
    if (argc == 1) {
      printf("\033[0;31mNo command specified\033[0m\n\n");
      print_help();
      return 1;
    }

    user_input *input = pass_options(argc, argv);

    pa_proplist *proplist = create_proplist();
    pa_mainloop *mainloop = pa_mainloop_new();
    mainloop_api = pa_mainloop_get_api(mainloop);
    pa_context *context = pa_context_new_with_proplist(mainloop_api, NULL, proplist);
    pa_proplist_free(proplist);

    pa_context_set_state_callback(context, context_state_callback, input);
    pa_context_connect(context, NULL, 0, NULL);

    int ret = 1;
    pa_mainloop_run(mainloop, &ret);
    pa_mainloop_free(mainloop);

    return ret;
}

static user_input* pass_options(int argc, char *argv[]) {
    device_value device = NONE;
    action_value action = NONE;

    if (streql(argv[1], "sink")) {
      device = SINK;

      if (argc > 2) {
        if (streql(argv[2], "list")) {
          action = LIST;
        }
      }
    }

    if (device == NONE) {
        print_help();
        exit(1);
    }

    user_input *input = malloc(sizeof(user_input));
    input->device = device;
    input->action = action;
    return  input;
}

static pa_proplist* create_proplist(void) {
    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, "application.name", "yapc");
    pa_proplist_sets(proplist, "application.id", "xyz.carcara.yapc");
    pa_proplist_sets(proplist, "application.version", "0.0.1");
    pa_proplist_sets(proplist, "application_icon_name", "audio-card");

    return proplist;
}

static void house_clean(pa_context *ctx, void *userdata) {
    free(userdata);
    pa_context_disconnect(ctx);
    pa_context_unref(ctx);
    mainloop_api->quit(mainloop_api, 0);
}

static void list_sinks_callback(pa_context *ctx, const pa_sink_info *info, int done, void *userdata) {
  if (done > 0) {
    house_clean(ctx, userdata);
    return;
  }

  printf("%d %s\n", info->index, info->description);
}

static void list_sources_callback(pa_context *ctx, const pa_source_info *info, int done, void *userdata) {
  if (done > 0) {
    house_clean(ctx, userdata);
    return;
  }

  printf("%d %s\n", info->index, info->description);
}

static void process_action(pa_context *ctx, user_input *input) {
    pa_operation *o = NULL;

    switch(1) {
      case SINK:
        o = pa_context_get_sink_info_list(ctx, list_sinks_callback, NULL);
        break;
      case SOURCE:
        o = pa_context_get_source_info_list(ctx, list_sources_callback, NULL);
        break;
      default:
        printf("Ai nao\n");
    }

    pa_operation_unref(o);
}

static void context_state_callback(pa_context *ctx, void *userdata) {
    switch (pa_context_get_state(ctx)) {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;
        case PA_CONTEXT_READY:
            process_action(ctx, NULL);
            break;
        case PA_CONTEXT_FAILED:
            printf("Carai\n");
            break;
        case PA_CONTEXT_TERMINATED:
        default:
           return;
    }
}


