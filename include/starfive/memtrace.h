#ifndef _MEM_TEST_DATA_H_
#define _MEM_TEST_DATA_H_

#define MEM_TEST_ENTRY_MAX_REQ_BLK_SIZE (6)

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

struct DirectedMemTestEntry
{
    uint64_t paddr;
    uint8_t memCmd;
    uint8_t blkSize;
    uint8_t data[MEM_TEST_ENTRY_MAX_REQ_BLK_SIZE];
};
typedef struct DirectedMemTestEntry DirectedMemTestEntry;

int create_data_file(const char *path);
int open_data_file(const char *path);
bool close_data_file(int fd);

// Command string
const char *cmdStrArray[] = {
    "InvalidCmd",
    "ReadReq",
    "ReadResp",
    "ReadRespWithInvalidate",
    "WriteReq",
    "WriteResp",
    "WriteCompleteResp",
    "WritebackDirty",
    "WritebackClean",
    "WriteClean", // writes dirty data below without evicting
    "CleanEvict",
    "SoftPFReq",
    "SoftPFExReq",
    "HardPFReq",
    "SoftPFResp",
    "HardPFResp",
    "WriteLineReq",
    "UpgradeReq",
    "SCUpgradeReq", // Special "weak" upgrade for StoreCond
    "UpgradeResp",
    "SCUpgradeFailReq", // Failed SCUpgradeReq in MSHR (never sent)
    "UpgradeFailResp",  // Valid for SCUpgradeReq only
    "ReadExReq",
    "ReadExResp",
    "ReadCleanReq",
    "ReadSharedReq",
    "LoadLockedReq",
    "StoreCondReq",
    "StoreCondFailReq", // Failed StoreCondReq in MSHR (never sent)
    "StoreCondResp",
    "LockedRMWReadReq",
    "LockedRMWReadResp",
    "LockedRMWWriteReq",
    "LockedRMWWriteResp",
    "SwapReq",
    "SwapResp",
    "UNKNOWN",
    "UNKNOWN",
    // MessageReq and MessageResp are deprecated.
    "MemFenceReq",
    "MemSyncReq",  // memory synchronization request (e.g., cache invalidate)
    "MemSyncResp", // memory synchronization response
    "MemFenceResp",
    "CleanSharedReq",
    "CleanSharedResp",
    "CleanInvalidReq",
    "CleanInvalidResp",
    // Error responses
    // @TODO these should be classified as responses rather than
    // requests; coding them as requests initially for backwards
    // compatibility
    "InvalidDestError",     // packet dest field invalid
    "BadAddressError",      // memory address invalid
    "FunctionalReadError",  // unable to fulfill functional read
    "FunctionalWriteError", // unable to fulfill functional write
    // Fake simulator-only commands
    "PrintReq",      // Print state matching address
    "FlushReq",      // request for a cache flush
    "InvalidateReq", // request for address to be invalidated
    "InvalidateResp",
    // hardware transactional memory
    "HTMReq",
    "HTMReqResp",
    "HTMAbort",
    // Tlb shootdown
    "TlbiExtSync",
    "NUM_MEM_CMDS"};

// Copied from `packet.hh`
enum Command
{
    InvalidCmd,
    ReadReq,
    ReadResp,
    ReadRespWithInvalidate,
    WriteReq,
    WriteResp,
    WriteCompleteResp,
    WritebackDirty,
    WritebackClean,
    WriteClean, // writes dirty data below without evicting
    CleanEvict,
    SoftPFReq,
    SoftPFExReq,
    HardPFReq,
    SoftPFResp,
    HardPFResp,
    WriteLineReq,
    UpgradeReq,
    SCUpgradeReq, // Special "weak" upgrade for StoreCond
    UpgradeResp,
    SCUpgradeFailReq, // Failed SCUpgradeReq in MSHR (never sent)
    UpgradeFailResp,  // Valid for SCUpgradeReq only
    ReadExReq,
    ReadExResp,
    ReadCleanReq,
    ReadSharedReq,
    LoadLockedReq,
    StoreCondReq,
    StoreCondFailReq, // Failed StoreCondReq in MSHR (never sent)
    StoreCondResp,
    LockedRMWReadReq,
    LockedRMWReadResp,
    LockedRMWWriteReq,
    LockedRMWWriteResp,
    SwapReq,
    SwapResp,
    // MessageReq and MessageResp are deprecated.
    MemFenceReq = SwapResp + 3,
    MemSyncReq,  // memory synchronization request (e.g., cache invalidate)
    MemSyncResp, // memory synchronization response
    MemFenceResp,
    CleanSharedReq,
    CleanSharedResp,
    CleanInvalidReq,
    CleanInvalidResp,
    // Error responses
    // @TODO these should be classified as responses rather than
    // requests; coding them as requests initially for backwards
    // compatibility
    InvalidDestError,     // packet dest field invalid
    BadAddressError,      // memory address invalid
    FunctionalReadError,  // unable to fulfill functional read
    FunctionalWriteError, // unable to fulfill functional write
    // Fake simulator-only commands
    PrintReq,      // Print state matching address
    FlushReq,      // request for a cache flush
    InvalidateReq, // request for address to be invalidated
    InvalidateResp,
    // hardware transactional memory
    HTMReq,
    HTMReqResp,
    HTMAbort,
    // Tlb shootdown
    TlbiExtSync,
    NUM_MEM_CMDS
};

#endif