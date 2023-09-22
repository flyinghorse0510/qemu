
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <glib.h>

#include <qemu-plugin.h>
#include "memtrace.h"

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

static GString* outputDir = NULL;

QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id,
                                           const qemu_info_t *info,
                                           int argc, char **argv)
{
    // Parse arguments
    for (int i = 0; i < argc; i++)
    {
        char *opt = argv[i];
        g_auto(GStrv) tokens = g_strsplit(opt, "=", 2);
        if (g_strcmp0(tokens[0], "dir") == 0) {
            outputDir = g_string_new(tokens[1]);
        } else {
            fprintf(stderr, "Unknown plugin parameters: %s!\n", tokens[0]);
            return -1;
        }
    }
    
    if (outputDir) {
        outputDir = g_string_new("mem_trace/");
    }
    fprintf(stdout, "Memory trace files will be generated under %s\n", outputDir->str);

    return 0;
}

// Create binary data file
int create_data_file(const char *path)
{
    return open(path, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
}

int open_data_file(const char *path)
{
    return open(path, O_RDONLY);
}

bool close_data_file(int fd) {
    return (close(fd) == 0);
}