// SPDX-License-Identifier: LGPL
/* Copyright (C) 2018-2019 Jiaxun Yang <jiaxun.yang@flygoat.com> */
/* Copyright (C) 2026 Updated to use PawnIO instead of WinRing0 */
/* Access PCI Config Space - PawnIO */
extern "C" {
#include "../nb_smu_ops.h"
}
#include <cstdlib>
#include <cstring>
#include <windows.h>
#include <ntstatus.h>

// PawnIO User-Mode API
#include "pawnio_um.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Extract bus, device, function from WinRing0-style packed address
 * WinRing0 format: (bus << 16) | (device << 11) | (function << 8)
 */
static inline void decode_pci_address(uint32_t pci_address, uint32_t *bus, uint32_t *device, uint32_t *function)
{
    *bus = (pci_address >> 16) & 0xFF;
    *device = (pci_address >> 11) & 0x1F;
    *function = (pci_address >> 8) & 0x7;
}

/**
 * Initialize OS access object for PawnIO
 * 
 * @return Initialized os_access_obj_t, or NULL on failure
 */
os_access_obj_t *init_os_access_obj()
{
    // Allocate os_access_obj
    os_access_obj_t *obj = static_cast<os_access_obj_t *>(std::malloc(sizeof(os_access_obj_t)));
    if (obj == NULL) {
        DBG("Failed to allocate os_access_obj_t\n");
        return NULL;
    }

    memset(obj, 0, sizeof(os_access_obj_t));

    // Initialize PawnIO connection
    HANDLE device_handle = CreateFileA(
        "\\\\.\\PawnIO",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (device_handle == INVALID_HANDLE_VALUE) {
        DBG("PawnIO Err: Failed to open PawnIO device\n");
        DBG("PawnIO Err: Is the PawnIO driver installed and loaded?\n");
        free(obj);
        return NULL;
    }

    // Store device handle in the inpoutDll field (repurposed)
    obj->inpoutDll = (HINSTANCE)device_handle;

    DBG("PawnIO: Successfully initialized driver connection\n");
    return obj;
}

/**
 * Free OS access object and cleanup PawnIO resources
 * 
 * @param obj OS access object to free
 */
void free_os_access_obj(os_access_obj_t *obj)
{
    if (obj == NULL)
        return;

    HANDLE device_handle = (HANDLE)obj->inpoutDll;
    if (device_handle != INVALID_HANDLE_VALUE && device_handle != NULL) {
        CloseHandle(device_handle);
        DBG("PawnIO: Device connection closed\n");
    }

    free(obj);
}

/**
 * Read SMN register via PCI config space
 * 
 * @param obj OS access object
 * @param addr SMN address to read
 * @return SMN data value
 */
uint32_t smn_reg_read(const os_access_obj_t *obj, uint32_t addr)
{
    uint32_t bus, device, function;
    decode_pci_address(obj->pci_address, &bus, &device, &function);

    HANDLE device_handle = (HANDLE)obj->inpoutDll;

    // Write SMN address to PCI config register
    uint32_t write_value = addr & (~0x3);
    NTSTATUS status = pci_config_write_dword(
        device_handle,
        bus,
        device,
        function,
        NB_PCI_REG_ADDR_ADDR,
        write_value
    );

    if (status != STATUS_SUCCESS) {
        DBG("PawnIO Err: Failed to write SMN address register (0x%lx)\n", status);
        return 0;
    }

    // Read SMN data from PCI config register
    uint32_t read_value = 0;
    status = pci_config_read_dword(
        device_handle,
        bus,
        device,
        function,
        NB_PCI_REG_DATA_ADDR,
        &read_value
    );

    if (status != STATUS_SUCCESS) {
        DBG("PawnIO Err: Failed to read SMN data register (0x%lx)\n", status);
        return 0;
    }

    return read_value;
}

/**
 * Write SMN register via PCI config space
 * 
 * @param obj OS access object
 * @param addr SMN address to write
 * @param data Data to write
 */
void smn_reg_write(const os_access_obj_t *obj, uint32_t addr, uint32_t data)
{
    uint32_t bus, device, function;
    decode_pci_address(obj->pci_address, &bus, &device, &function);

    HANDLE device_handle = (HANDLE)obj->inpoutDll;

    // Write SMN address to PCI config register
    NTSTATUS status = pci_config_write_dword(
        device_handle,
        bus,
        device,
        function,
        NB_PCI_REG_ADDR_ADDR,
        addr
    );

    if (status != STATUS_SUCCESS) {
        DBG("PawnIO Err: Failed to write SMN address register (0x%lx)\n", status);
        return;
    }

    // Write SMN data to PCI config register
    status = pci_config_write_dword(
        device_handle,
        bus,
        device,
        function,
        NB_PCI_REG_DATA_ADDR,
        data
    );

    if (status != STATUS_SUCCESS) {
        DBG("PawnIO Err: Failed to write SMN data register (0x%lx)\n", status);
        return;
    }
}

/* Global state for physical memory mapping */
static HANDLE g_pawnio_device = NULL;
static uint32_t g_mapped_pa = 0;

/**
 * Initialize physical memory mapping for PM table access
 * 
 * @param os_access OS access object
 * @param physAddr Physical address to map
 * @return 0 on success, -1 on failure
 */
int init_mem_obj(os_access_obj_t *os_access, uintptr_t physAddr)
{
    HANDLE device_handle = (HANDLE)os_access->inpoutDll;
    if (device_handle == INVALID_HANDLE_VALUE || device_handle == NULL) {
        DBG("PawnIO Err: Invalid device handle\n");
        return -1;
    }

    g_pawnio_device = device_handle;
    g_mapped_pa = (uint32_t)physAddr;

    DBG("PawnIO: Physical memory prepared for mapping at PA: 0x%lx\n", physAddr);
    return 0;
}

/**
 * Copy PM table from physical memory using PawnIO
 * 
 * @param obj OS access object
 * @param buffer Destination buffer
 * @param size Number of bytes to copy
 * @return 0 on success, -1 on failure
 */
int copy_pm_table(const os_access_obj_t *obj, void *buffer, const size_t size)
{
    if (g_pawnio_device == NULL || g_pawnio_device == INVALID_HANDLE_VALUE) {
        DBG("PawnIO Err: Physical memory not initialized\n");
        return -1;
    }

    // Read physical memory directly using PawnIO
    for (size_t i = 0; i < size; i += sizeof(uint32_t)) {
        uint32_t value = 0;
        NTSTATUS status = physical_read_dword(
            g_pawnio_device,
            g_mapped_pa + i,
            &value
        );

        if (status != STATUS_SUCCESS) {
            DBG("PawnIO Err: Failed to read physical memory at offset 0x%lx (0x%lx)\n", i, status);
            return -1;
        }

        *(uint32_t *)((uint8_t *)buffer + i) = value;
    }

    return 0;
}

/**
 * Compare PM table with physical memory
 * 
 * @param buffer Buffer to compare
 * @param size Number of bytes to compare
 * @return 0 if equal, non-zero if different
 */
int compare_pm_table(const void *buffer, const size_t size)
{
    if (g_pawnio_device == NULL || g_pawnio_device == INVALID_HANDLE_VALUE) {
        DBG("PawnIO Err: Physical memory not initialized\n");
        return -1;
    }

    for (size_t i = 0; i < size; i += sizeof(uint32_t)) {
        uint32_t value = 0;
        NTSTATUS status = physical_read_dword(
            g_pawnio_device,
            g_mapped_pa + i,
            &value
        );

        if (status != STATUS_SUCCESS) {
            DBG("PawnIO Err: Failed to read physical memory at offset 0x%lx (0x%lx)\n", i, status);
            return -1;
        }

        if (value != *(uint32_t *)((uint8_t *)buffer + i)) {
            return 1; // Mismatch
        }
    }

    return 0; // Match
}

/**
 * Check if using SMU driver (always false for direct PCI access)
 * 
 * @return false
 */
bool is_using_smu_driver()
{
    return false;
}

#ifdef __cplusplus
}
#endif
