# PawnIO Port Branch - README

## Overview

This branch (`pawnio-port`) contains the complete migration of RyzenAdj from WinRing0 to PawnIO.

**Status:** ✅ Implementation Complete

## What's Changed

### New Files
- **PAWNIO_PORTING_GUIDE.md** - Comprehensive technical guide explaining the migration
- **PAWNIO_SETUP_WINDOWS.md** - Step-by-step setup instructions for Windows users

### Modified Files
- **CMakeLists.txt** - Updated to link against PawnIO instead of WinRing0
- **lib/win32/osdep_win32.cpp** - Complete rewrite using PawnIO API

### Removed Files (No longer needed)
- `win32/WinRing0x64.dll`
- `win32/WinRing0x64.lib`
- `win32/WinRing0x64.sys`
- `win32/WinRing0x64.exp`
- `win32/inpoutx64.dll`
- `win32/inpoutx64.lib`

## Quick Start

### For Users

1. **Install PawnIO driver** (see PAWNIO_SETUP_WINDOWS.md)
2. **Build RyzenAdj:**
   ```bash
   mkdir build
   cd build
   cmake -DPAWNIO_ROOT="C:\Path\To\PawnIO" ..
   cmake --build . --config Release
   ```
3. **Run as Administrator:**
   ```bash
   ryzenadj.exe --info
   ```

### For Developers

1. Read **PAWNIO_PORTING_GUIDE.md** for architectural details
2. Key changes in `lib/win32/osdep_win32.cpp`:
   - `pci_config_read_dword()` / `pci_config_write_dword()` for PCI access
   - `physical_read_dword()` / `physical_write_dword()` for physical memory
   - NTSTATUS error handling instead of OLS_DLL_* codes

## Key Features

✅ Modern PawnIO kernel driver (maintained)
✅ Signed driver (better Windows compatibility)
✅ NTSTATUS error handling (standard Windows codes)
✅ Direct PCI config space access
✅ Kernel-mode physical memory access
✅ No dependency on legacy WinRing0 DLLs

## Requirements

- **Windows 10 or later**
- **Administrator privileges** (kernel driver access)
- **PawnIO driver installed and loaded**
- **CMake 3.10+** for building

## Building

```bash
# Set PAWNIO_ROOT to your PawnIO installation directory
cmake -DPAWNIO_ROOT="C:\Tools\PawnIO" ..
cmake --build . --config Release
```

If CMake can't find PawnIO automatically, manually set the environment variable:
```bash
set PAWNIO_ROOT=C:\Tools\PawnIO
```

## Testing

After building, verify functionality:

```bash
# Show CPU info and power metrics
ryzenadj.exe --info

# Dump full PM table
ryzenadj.exe --dump-table

# Set power limits (example)
ryzenadj.exe --stapm-limit 15000 --fast-limit 20000 --slow-limit 20000
```

## Troubleshooting

See **PAWNIO_SETUP_WINDOWS.md** for detailed troubleshooting guide.

### Common Issues

| Problem | Solution |
|---------|----------|
| "Failed to open PawnIO device" | Install PawnIO driver and ensure it's loaded |
| "Permission denied" | Run as Administrator |
| CMake can't find PawnIO | Set `-DPAWNIO_ROOT=<path>` explicitly |
| RyzenAdj crashes | Check Windows Event Viewer for driver errors |

## Migration from WinRing0

If you were using the master branch with WinRing0:

1. All functionality is identical
2. No changes needed to calling code (same public API)
3. Only Windows binary will change (now requires PawnIO instead of WinRing0)
4. Linux support unchanged

## Documentation

- **PAWNIO_PORTING_GUIDE.md** - Technical migration details
- **PAWNIO_SETUP_WINDOWS.md** - User setup instructions
- Original README.md - General RyzenAdj information

## Comparison

| Feature | WinRing0 | PawnIO |
|---------|----------|--------|
| Maintenance | ❌ Unmaintained | ✅ Active |
| Architecture | User-mode DLL | Kernel driver |
| Signing | ❌ Unsigned | ✅ Signed |
| Error Codes | OLS_DLL_* | NTSTATUS |
| PCI Access | ✅ Yes | ✅ Yes |
| Phys Memory | inpoutx64.dll | Native |
| MSR Access | ✅ Yes | ✅ Yes |
| Windows 11 | ⚠️ Problematic | ✅ Supported |

## Reverting to WinRing0

If you need the original WinRing0 version:
```bash
git checkout master
```

The master branch remains unchanged with WinRing0 support.

## Support & Issues

- **PawnIO Issues:** https://github.com/namazso/PawnIO/issues
- **RyzenAdj Issues:** https://github.com/FlyGoat/RyzenAdj/issues
- **This Fork:** https://github.com/finalotogaming/RyzenAdj/issues

## License

- RyzenAdj: LGPL-3.0
- PawnIO: GPL-2.0
- This port maintains LGPL-3.0 for RyzenAdj code

---

**Branch:** pawnio-port
**Last Updated:** 2026-07-07
**Status:** Ready for testing and integration
