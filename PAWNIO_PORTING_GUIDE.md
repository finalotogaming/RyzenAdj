# RyzenAdj: Porting from WinRing0 to PawnIO

## Overview

This document describes the complete migration of RyzenAdj from the legacy WinRing0 driver to the modern PawnIO kernel driver infrastructure.

## Why PawnIO?

**WinRing0 Limitations:**
- Legacy, unmaintained driver
- Limited to I/O port and MSR access patterns
- Dual-DLL architecture (WinRing0x64.dll + inpoutx64.dll)
- Requires manual driver installation and management
- Limited error handling and status reporting

**PawnIO Advantages:**
- Modern, actively maintained kernel driver
- Comprehensive API: PCI config space, physical memory, virtual memory, MSR
- Signed kernel driver (better compatibility)
- Single unified driver architecture
- NTSTATUS-based error reporting
- Kernel-mode execution for better hardware access
- Extensible module system

## Architecture Changes

### WinRing0 Architecture (Old)
```
RyzenAdj (User Mode)
    ↓
[OlsApi.h] ← InitializeOls(), ReadPciConfigDword(), WritePciConfigDword()
    ↓
[WinRing0x64.dll] ← Communicates with WinRing0x64.sys
    ↓
[inpoutx64.dll] ← Physical memory mapping
    ↓
[Hardware]
```

### PawnIO Architecture (New)
```
RyzenAdj (User Mode)
    ↓
[pawnio_um.h] ← PawnIOLib wrapper functions
    ↓
[PawnIO Device Driver] ← Unified kernel driver
    ↓
[Hardware]
```

## Functional Mapping

| WinRing0 Function | PawnIO Equivalent | Purpose |
|---|---|---|
| `InitializeOls()` | `PawnIO_Connect()` | Initialize driver connection |
| `DeinitializeOls()` | `CloseHandle()` | Clean up driver connection |
| `ReadPciConfigDword(addr, reg, &value)` | `pci_config_read_dword(bus, dev, fn, offset, &value)` | Read PCI config space |
| `WritePciConfigDword(addr, reg, value)` | `pci_config_write_dword(bus, dev, fn, offset, value)` | Write PCI config space |
| `MapPhysToLin(pa, size, &handle)` | `io_space_map(pa, size)` | Map physical memory |
| `UnmapPhysicalMemory(handle, va)` | `io_space_unmap(va, size)` | Unmap physical memory |
| `GetPhysLong(pa, &value)` | `physical_read_dword(pa, &value)` | Read physical memory |
| (Direct write to mapped VA) | `physical_write_dword(pa, value)` | Write physical memory |

## Key Implementation Details

### PCI Address Decoding

**WinRing0** uses a packed address format:
```c
// WinRing0: pci_address = (bus << 16) | (device << 11) | (function << 8)
WritePciConfigDword(pci_address, offset, value);
```

**PawnIO** uses explicit parameters:
```c
// PawnIO: Individual bus, device, function parameters
pci_config_write_dword(bus, device, function, offset, value);
```

The conversion is straightforward:
```c
uint32_t bus = (pci_address >> 16) & 0xFF;
uint32_t device = (pci_address >> 11) & 0x1F;
uint32_t function = (pci_address >> 8) & 0x7;
```

### Memory Mapping Changes

**WinRing0:**
```c
HINSTANCE hInpOutDll = LoadLibrary("inpoutx64.DLL");
gfpMapPhysToLin = GetProcAddress(hInpOutDll, "MapPhysToLin");
pdwLinAddr = gfpMapPhysToLin(physAddr, 0x1000, &physicalMemoryHandle);
// Direct pointer dereference
uint32_t value = *pdwLinAddr;
```

**PawnIO:**
```c
// Using PawnIOLib header
VA:va = io_space_map(physAddr, 0x1000);
if (va == VA:0) { /* error */ }
// Use PawnIO read function
NTSTATUS status = virtual_read_dword(va, &value);
if (status != STATUS_SUCCESS) { /* error */ }
io_space_unmap(va, 0x1000);
```

### Error Handling

**WinRing0** uses implicit error checking:
```c
DWORD dllStatus = GetDllStatus();
switch (dllStatus) {
    case OLS_DLL_NO_ERROR: /* success */ break;
    case OLS_DLL_DRIVER_NOT_FOUND: /* error */ break;
}
```

**PawnIO** uses NTSTATUS (Windows standard error codes):
```c
NTSTATUS status = pci_config_read_dword(bus, device, function, offset, &value);
if (status != STATUS_SUCCESS) {
    // Use standard Windows error codes
    if (status == STATUS_NOT_FOUND) { /* device not found */ }
    if (status == STATUS_DEVICE_DOES_NOT_EXIST) { /* invalid */ }
}
```

## File Changes

### Modified Files

1. **`lib/win32/osdep_win32.cpp`** (Complete rewrite)
   - Replace OlsApi.h includes with pawnio_um.h
   - Update device initialization/cleanup
   - Replace PCI config read/write functions
   - Implement new physical memory mapping strategy
   - Update error handling to NTSTATUS codes

2. **`CMakeLists.txt`**
   - Remove WinRing0x64 library linking
   - Add PawnIOLib linking
   - Update LINK_DIRECTORIES to PawnIO installation path

### Removed Files

- `win32/WinRing0x64.dll` (replaced by PawnIO driver)
- `win32/WinRing0x64.lib` (replaced by PawnIOLib)
- `win32/WinRing0x64.sys` (replaced by PawnIO driver)
- `win32/WinRing0x64.exp` (no longer needed)
- `win32/inpoutx64.dll` (PawnIO handles memory mapping)
- `win32/inpoutx64.lib` (PawnIO handles memory mapping)

## Testing Checklist

- [ ] PawnIO driver installed and loaded
- [ ] `pci_config_read_dword()` correctly reads SMN address register
- [ ] `pci_config_write_dword()` correctly writes SMN address register
- [ ] Physical memory mapping works for PM table access
- [ ] `init_os_access_obj()` properly initializes connection
- [ ] `free_os_access_obj()` properly cleans up resources
- [ ] Error codes are properly handled and reported
- [ ] No device access permission errors (ensure admin privileges)

## Troubleshooting

### "PawnIO driver not found"
- Ensure PawnIO is installed: Check Device Manager for "PawnIO" device
- Verify driver is loaded: `sc query PawnIO`
- Driver must be signed and installed before running RyzenAdj

### "Permission denied" errors
- RyzenAdj must run with administrator privileges
- PawnIO requires kernel-mode execution

### NTSTATUS error codes
- `STATUS_NOT_FOUND (0xC0000225)`: PCI device not found on specified bus/slot
- `STATUS_DEVICE_DOES_NOT_EXIST (0xC0000225)`: Invalid vendor ID (PCI device doesn't exist)
- `STATUS_UNSUCCESSFUL (0xC0000001)`: Generic operation failure

## Migration Notes for Developers

If you're maintaining code that uses the `os_access_obj_t` structure:

1. The structure may now contain a PawnIO device handle instead of an inpoutDll handle
2. All PCI accesses now require explicit bus/device/function extraction
3. Physical memory mapping now returns NTSTATUS codes instead of pointers
4. Error handling should use standard Windows NTSTATUS codes

## References

- PawnIO Repository: https://github.com/namazso/PawnIO
- PawnIO User-Mode API: https://github.com/namazso/PawnIO/blob/master/PawnIOLib/pawnio_um.h
- Windows NTSTATUS Codes: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebad55
- PCI Configuration Space: https://en.wikipedia.org/wiki/PCI_configuration_space

## Rollback Instructions

If you need to revert to WinRing0:

1. Checkout the `master` branch
2. The WinRing0 implementation remains unchanged on the main branch
3. Or revert commits: `git revert <commit-hash>`

---

**Document Version:** 1.0
**Last Updated:** 2026-07-07
**PawnIO Version Reference:** Latest (as of 2026-03-15)
