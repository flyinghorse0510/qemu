
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
#include <malloc.h>
#include <memalign.h>


#include <qemu-plugin.h>
#include "memtrace.h"

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

static GString *outputDir = NULL;
static GArray *traceMemMapArray = NULL;

// Create binary data file
static int create_data_file(const char *path)
{
    return open(path, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
}

// Close file
static bool close_data_file(int fd)
{
    return (close(fd) == 0);
}

// record memory trace
static bool record_mem_trace(unsigned int vcpuIndex, DirectedMemTestEntry* trace) {
    if (traceMemMapArray->len <= vcpuIndex) {
        fprintf(stdout, "Expanding vCPU MemTrace Map ==> vCPU(%u)\n", vcpuIndex);
        g_array_set_size(traceMemMapArray, vcpuIndex+1);
        TraceMemMap* map = &g_array_index(traceMemMapArray, TraceMemMap, vcpuIndex);
        map->trace = malloc(TRACE_MEM_MAP_INIT_SIZE * sizeof(DirectedMemTestEntry));
        map->maxLength = TRACE_MEM_MAP_INIT_SIZE;
        map->length = 0;
        
        if (map->trace == NULL) {
            fprintf(stderr, "Fatal Error! Allocating memory failed! ==> vCPU(%u)\n", vcpuIndex);
            return false;
        }
    }

    TraceMemMap* map = &g_array_index(traceMemMapArray, TraceMemMap, vcpuIndex);
    if (map->trace == NULL) {
        return false;
    }

    if (map->length >= map->maxLength) {
        fprintf(stdout, "Reallocating vCPU MemTrace Map ==> vCPU(%u):%lu -> %lu\n", vcpuIndex, map->maxLength, 2 * map->maxLength);
        map->trace = reallocarray(map->trace, 2 * map->maxLength, sizeof(DirectedMemTestEntry));
        map->maxLength = 2 * map->maxLength;
        if (map->trace == NULL) {
            fprintf(stderr, "Fatal Error! Allocating memory failed! ==> vCPU(%u)\n", vcpuIndex);
            return false;
        }
    }

    memcpy((map->trace + map->length), trace, sizeof(DirectedMemTestEntry));
    map->length++;

    return true;
}

// Trigger when instructions access memory
static void vcpu_mem_trigger(unsigned int vcpuIndex, qemu_plugin_meminfo_t info, uint64_t vaddr, void *userdata)
{
    // Construct MemTrace
    DirectedMemTestEntry memTrace = {};
    memTrace.paddr = vaddr;

    // Calculate access size
    unsigned int accessSizeShift = qemu_plugin_mem_size_shift(info);
    unsigned int accessSize = 1 << accessSizeShift;
    memTrace.blkSize = accessSize;

    // Judge ReadReqeust / WriteRequest
    if (qemu_plugin_mem_is_store(info))
    {
        // WriteRequest
        memTrace.memCmd = WriteReq;
    }
    else
    {
        // Read Request
        memTrace.memCmd = ReadReq;
    }

    bool ret = record_mem_trace(vcpuIndex, &memTrace);
    if (!ret) {
        fprintf(stderr, "Record MemTrace error! Terminating...\n");
        exit(EXIT_FAILURE);
    }
}

// Trigger when TB translation happens
static void vcpu_tb_trans_trigger(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    // Total instruction count
    size_t insCount = qemu_plugin_tb_n_insns(tb);
    for (size_t i = 0; i < insCount; i++)
    {
        struct qemu_plugin_insn *insn = qemu_plugin_tb_get_insn(tb, i);
        // Register memory event callback
        qemu_plugin_register_vcpu_mem_cb(
            insn,
            vcpu_mem_trigger,
            QEMU_PLUGIN_CB_NO_REGS,
            QEMU_PLUGIN_MEM_RW,
            NULL);
    }
}

// Sync MemTrace to disks
static void sync_mem_trace(qemu_plugin_id_t id, void *data) {
    unsigned long vCPUCount = traceMemMapArray->len;
    char fileNamebuffer[128];
    fprintf(stdout, "Syncing MemTrace to disks under %s...\n", outputDir->str);
    for (unsigned long i = 0; i < vCPUCount; i++) {
        // Construct output path
        g_autoptr(GString) outputPath = g_string_new(outputDir->str);
        snprintf(fileNamebuffer, 127, "/vCPU_%lu.bin", i);
        outputPath = g_string_append(outputPath, fileNamebuffer);
        fprintf(stdout, "[%lu/%lu] Syncing vCPU(%lu) MemTrace...\n", i+1, vCPUCount, i);
        // Create data file
        int fd = create_data_file(outputPath->str);
        // Failed to create file
        if (fd < 0) {
            fprintf(stderr, "Fatal Error! Failed to sync MemTrace ==> %s\n", outputPath->str);
            exit(EXIT_FAILURE);
        }
        // Sync
        TraceMemMap* map = &g_array_index(traceMemMapArray, TraceMemMap, i);
        ssize_t writeBytes = write(fd, map->trace, map->length * sizeof(DirectedMemTestEntry));
        if (writeBytes != map->length * sizeof(DirectedMemTestEntry)) {
            fprintf(stderr, "Fatal Error! Failed to sync MemTrace ==> %s\n", outputPath->str);
            exit(EXIT_FAILURE);
        }
        // Clean
        close_data_file(fd);
    }
    fprintf(stdout, "All MemTrace synced\n");
}

QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id,
                                           const qemu_info_t *info,
                                           int argc, char **argv)
{
    fprintf(stdout, "StarFive MemTrace Plugin\n");
    fprintf(stdout, "Installing plugin...\n");
    // Parse arguments
    for (int i = 0; i < argc; i++)
    {
        char *opt = argv[i];
        g_auto(GStrv) tokens = g_strsplit(opt, "=", 2);
        if (g_strcmp0(tokens[0], "dir") == 0)
        {
            outputDir = g_string_new(tokens[1]);
        }
        else
        {
            fprintf(stderr, "Unknown MemTrace plugin parameters: %s!\n", tokens[0]);
            return -1;
        }
    }

    if (!outputDir)
    {
        outputDir = g_string_new("mem_trace");
    }

    traceMemMapArray = g_array_new(false, true, sizeof(TraceMemMap));

    qemu_plugin_register_vcpu_tb_trans_cb(
        id,
        vcpu_tb_trans_trigger
    );

    qemu_plugin_register_atexit_cb(
        id,
        sync_mem_trace,
        NULL
    );

    fprintf(stdout, "Memory trace files will be generated under %s\n", outputDir->str);
    fprintf(stdout, "Maximum vCPUs: %d, Current vCPUs: %d\n", info->system.max_vcpus, info->system.smp_vcpus);

    return 0;
}
