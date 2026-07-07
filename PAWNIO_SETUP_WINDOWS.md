# PawnIO Setup Guide for RyzenAdj on Windows

## Prerequisites

- Windows 10 or later
- Administrator privileges (required for kernel driver installation)
- Visual C++ Runtime (MSVCRT) installed

## Step 1: Download PawnIO

1. Visit https://github.com/namazso/PawnIO/releases
2. Download the latest release package (look for `PawnIO-vX.X.X.zip`)
3. Extract the ZIP file to a location of your choice (e.g., `C:\Tools\PawnIO`)

## Step 2: Install PawnIO Driver

PawnIO provides installation scripts. Choose one method:

### Method A: Automated Installation (Recommended)

1. Open Command Prompt **as Administrator**
2. Navigate to the PawnIO installation directory:
   ```batch
   cd C:\Tools\PawnIO
   ```
3. Run the installation script:
   ```batch
   install.bat
   ```
4. Reboot your system if prompted

### Method B: Manual Installation

1. Open Device Manager
2. Right-click "Other devices" → "Add legacy hardware"
3. Choose "Install the hardware that I manually select from a list"
4. Click "Have Disk" and browse to the PawnIO driver file
5. Follow the installation wizard
6. Reboot when complete

### Method C: Command Line

```batch
# Copy driver to drivers directory
copy PawnIO.sys C:\Windows\System32\drivers\

# Register the driver
pnputil /a C:\Windows\System32\drivers\PawnIO.sys

# Start the driver service
sc create PawnIO binPath= "C:\Windows\System32\drivers\PawnIO.sys"
sc start PawnIO
```

## Step 3: Verify Installation

### Check Driver Status

1. Open Command Prompt **as Administrator**:
   ```batch
   sc query PawnIO
   ```
   You should see:
   ```
   SERVICE_NAME: PawnIO
           TYPE               : 1  KERNEL_DRIVER
           STATE              : 4  RUNNING
   ```

2. Or check Device Manager:
   - Open Device Manager
   - Look for "PawnIO" under "System devices"
   - Should show no error icons

### Verify Device Access

1. Check that the device exists:
   ```batch
   dir \\.\PawnIO
   ```
   This should succeed without errors.

## Step 4: Build RyzenAdj with PawnIO Support

### Using CMake (Recommended)

1. Open Command Prompt and navigate to your RyzenAdj repository:
   ```batch
   cd C:\Path\To\RyzenAdj
   ```

2. Create a build directory:
   ```batch
   mkdir build
   cd build
   ```

3. Configure CMake with PawnIO path:
   ```batch
   cmake -DPAWNIO_ROOT="C:\Tools\PawnIO" ..
   ```

4. Build the project:
   ```batch
   cmake --build . --config Release
   ```

### Manual Configuration

If CMake doesn't find PawnIO automatically:

1. Ensure the following files exist:
   - `C:\Tools\PawnIO\include\pawnio_um.h`
   - `C:\Tools\PawnIO\lib\PawnIOLib.lib`

2. Add PawnIO include directory to your IDE/compiler settings
3. Link against `PawnIOLib.lib`

## Step 5: Run RyzenAdj

### Important: Administrator Privileges Required

RyzenAdj must always run with administrator privileges when using PawnIO.

**Method 1: Command Line**
```batch
cd "C:\Path\To\RyzenAdj"
ryzenadj.exe --info
```
(Run Command Prompt as Administrator first)

**Method 2: Create Shortcut**

1. Right-click `ryzenadj.exe` → Create shortcut
2. Right-click the shortcut → Properties
3. Go to "Advanced"
4. Check "Run as administrator"
5. Click OK, then OK again
6. Double-click the shortcut to run

**Method 3: Task Scheduler**

1. Open Task Scheduler
2. Create Basic Task → "RyzenAdj"
3. Set trigger to "At startup"
4. Action: Start a program → `ryzenadj.exe`
5. Check "Run with highest privileges"

## Troubleshooting

### "Cannot open PawnIO device"

**Problem:** RyzenAdj shows "PawnIO Err: Failed to open PawnIO device"

**Solutions:**

1. **Verify driver is loaded:**
   ```batch
   sc query PawnIO
   ```
   Should show STATE: RUNNING

2. **Restart the driver service:**
   ```batch
   sc stop PawnIO
   sc start PawnIO
   ```

3. **Reinstall the driver:**
   ```batch
   sc delete PawnIO
   # Then run install.bat again
   ```

4. **Check Windows Event Viewer:**
   - Open Event Viewer
   - Go to Windows Logs → System
   - Look for PawnIO-related errors

### "Access Denied" or "Permission Error"

**Problem:** RyzenAdj runs but fails with permission errors

**Solutions:**

1. **Run as Administrator:**
   - Right-click `ryzenadj.exe`
   - Select "Run as administrator"

2. **Disable UAC (Not Recommended):**
   - This reduces security and is not recommended
   - Consider using Task Scheduler instead (see above)

3. **Check user account type:**
   - Must be Administrator or Power User
   - Standard user accounts cannot access kernel drivers

### "PawnIO driver not found" during build

**Problem:** CMake cannot find PawnIO during compilation

**Solutions:**

1. **Set PAWNIO_ROOT explicitly:**
   ```batch
   cmake -DPAWNIO_ROOT="C:\Tools\PawnIO" ..
   ```

2. **Verify file paths:**
   ```batch
   # Check that these files exist:
   dir "C:\Tools\PawnIO\include\pawnio_um.h"
   dir "C:\Tools\PawnIO\lib\PawnIOLib.lib"
   ```

3. **Update environment variables:**
   - Set `PAWNIO_ROOT` as a system environment variable
   - Restart CMake/IDE after setting

### RyzenAdj crashes or hangs

**Problem:** Application crashes when accessing hardware

**Solutions:**

1. **Check Windows compatibility:**
   - Ensure Windows is fully updated
   - Run Windows Update

2. **Verify PCI device accessibility:**
   ```batch
   # Use Device Manager to verify PCI devices are visible
   # Right-click Computer → Device Manager
   # Look for AMD devices
   ```

3. **Check driver version compatibility:**
   - Ensure PawnIO version matches your Windows version
   - Visit https://github.com/namazso/PawnIO for compatibility info

4. **Collect debug output:**
   ```batch
   ryzenadj.exe --dump-table > debug.log 2>&1
   # Share this log when reporting issues
   ```

## Uninstalling PawnIO

If you need to uninstall PawnIO:

1. Open Command Prompt **as Administrator**:
   ```batch
   sc stop PawnIO
   sc delete PawnIO
   del C:\Windows\System32\drivers\PawnIO.sys
   ```

2. Reboot your system

3. Delete the PawnIO installation directory

## Performance Considerations

- PawnIO kernel driver has minimal performance overhead
- Initial driver load may take a few seconds on first use
- Subsequent calls are fast (microseconds)
- No impact on system performance when idle

## Security Notes

- PawnIO requires administrator privileges for a reason: it accesses kernel-mode hardware
- Only download PawnIO from official GitHub repository
- Verify driver signature: `sigcheck -v C:\Windows\System32\drivers\PawnIO.sys`
- PawnIO is open-source; you can verify the driver code

## Support and Resources

- **PawnIO Issues:** https://github.com/namazso/PawnIO/issues
- **RyzenAdj Issues:** https://github.com/FlyGoat/RyzenAdj/issues
- **This Fork Issues:** https://github.com/finalotogaming/RyzenAdj/issues

---

**Last Updated:** 2026-07-07
