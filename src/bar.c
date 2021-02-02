#define _GNU_SOURCE // for O_TMPFILE
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_BOOL
#define NK_IMPLEMENTATION
#undef NK_INCLUDE_FONT_BAKING

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nuklear.h"
#include "structs.h"
#include "utils.h"
#include "render.h"
#include "wayland.h"
#include "version.h"

#define ARG_HELP        "help"

#define HELP_MESSAGE    "NukeBAR %s\n" \
"Usage:\n  %s COMMAND - Start the bar\n" \
"Commands:\n"\
"\t" ARG_HELP "\t\tShow this help message.\n" \
""

enum log_levels _log_level;

const char* get_version()
{
#ifndef VERSION_HASH
#define VERSION_HASH "(unknown)"
#endif
    return VERSION_HASH;
}

void print_help(char* name)
{
    const char* help_msg;
    const char* version = get_version();

    help_msg = HELP_MESSAGE;

    fprintf(stdout, help_msg, version, name);
}

int main(int argc, char** argv)
{
    _log_level = log_tracing2 | log_tracing | log_debug | log_info | log_debug | log_error;

    if (argc < 0) {
        goto _failure;
    }

    char* name = argv[0];
    if (argc <= 1) {
        goto _help;
    }

    char *command = argv[1];
    if (strcmp(command, ARG_HELP) == 0) {
        goto _help;
    }

    struct nukebar bar = {0};
    _trace2("Started bar %p:%u", &bar, sizeof(bar));

    if (!wayland_init(&bar)) {
        goto _failure;
    }
    // Draw the first frame
    if (!render(&bar, 0)) {
        goto _failure;
    }

    while (wl_display_dispatch(bar.display) != -1 && !bar.stop) {
        // This space intentionally left blank
    }
    wayland_destroy(&bar);
_success:
    return EXIT_SUCCESS;
_failure:
    return EXIT_FAILURE;
_help:
    print_help(name);
    goto _success;
}
