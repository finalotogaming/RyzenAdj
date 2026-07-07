# RyzenAdj - PawnIO Edition
Adjust power management settings for Ryzen Mobile Processors with modern PawnIO driver support.

[![GitHub](https://img.shields.io/badge/GitHub-finalotogaming%2FRyzenAdj-blue)](https://github.com/finalotogaming/RyzenAdj)

Based on: [FlyGoat/ryzen_nb_smu](https://github.com/flygoat/ryzen_nb_smu)

**⚠️ This is the PawnIO branch** - For the original WinRing0 version, see the [master branch](https://github.com/finalotogaming/RyzenAdj/tree/master)

RyzenAdjUI_WPF by "JustSkill" is no longer maintained, for GUI please see [Universal x86 Tuning Utility](https://github.com/JamesCJ60/Universal-x86-Tuning-Utility) or [ryzen-controller-team/ryzen-controller](https://github.com/ryzen-controller-team/ryzen-controller)

## What is PawnIO?

PawnIO is a modern, actively maintained kernel driver that provides safe hardware access on Windows. This PawnIO edition replaces the legacy WinRing0 driver with:

- ✅ **Modern & Maintained** - Active development and security updates
- ✅ **Signed Driver** - Better compatibility with Windows 10/11
- ✅ **Kernel-Mode Access** - Direct PCI and memory operations
- ✅ **Standard Error Handling** - Uses Windows NTSTATUS codes
- ✅ **Open Source** - Full transparency of driver operations

## Key Differences from WinRing0

| Feature | WinRing0 | PawnIO |
|---------|----------|--------|
| Maintenance | ❌ Unmaintained | ✅ Active |
| Windows 11 Support | ⚠️ Limited | ✅ Full |
| Driver Signing | ❌ Unsigned | ✅ Signed |
| Architecture | Dual DLL (WinRing0 + inpoutx64) | Single kernel driver |
| Error Handling | Proprietary codes | Standard NTSTATUS |

## Usage

The command line interface is identical to the original RyzenAdj.

You must run it with **Administrator privileges** on Windows.

```
$ ./ryzenadj -h
Usage: ryzenadj [options]

 Ryzen Power Management adjust tool.

    -h, --help                            show this help message and exit

Options
    -i, --info                            Show information and most important power metrics after adjustment
    --dump-table                          Show whole power metric table before and after adjustment

Settings
    -a, --stapm-limit=<u32>               Sustained Power Limit         - STAPM LIMIT (mW)
    -b, --fast-limit=<u32>                Actual Power Limit            - PPT LIMIT FAST (mW)
    -c, --slow-limit=<u32>                Average Power Limit           - PPT LIMIT SLOW (mW)
    -d, --slow-time=<u32>                 Slow PPT Constant Time (s)
    -e, --stapm-time=<u32>                STAPM constant time (s)
    -f, --tctl-temp=<u32>                 Tctl Temperature Limit (degree C)
    -g, --vrm-current=<u32>               VRM Current Limit             - TDC LIMIT VDD (mA)
    -j, --vrmsoc-current=<u32>            VRM SoC Current Limit         - TDC LIMIT SoC (mA)
    -k, --vrmmax-current=<u32>            VRM Maximum Current Limit     - EDC LIMIT VDD (mA)
    -l, --vrmsocmax-current=<u32>         VRM SoC Maximum Current Limit - EDC LIMIT SoC (mA)
    -m, --psi0-current=<u32>              PSI0 VDD Current Limit (mA)
    -n, --psi0soc-current=<u32>           PSI0 SoC Current Limit (mA)
    -o, --max-socclk-frequency=<u32>      Maximum SoC Clock Frequency (MHz)
    -p, --min-socclk-frequency=<u32>      Minimum SoC Clock Frequency (MHz)
    -q, --max-fclk-frequency=<u32>        Maximum Transmission (CPU-GPU) Frequency (MHz)
    -r, --min-fclk-frequency=<u32>        Minimum Transmission (CPU-GPU) Frequency (MHz)
    -s, --max-vcn=<u32>                   Maximum Video Core Next (VCE - Video Coding Engine) (MHz)
    -t, --min-vcn=<u32>                   Minimum Video Core Next (VCE - Video Coding Engine) (MHz)
    -u, --max-lclk=<u32>                  Maximum Data Launch Clock (MHz)
    -v, --min-lclk=<u32>                  Minimum Data Launch Clock (MHz)
    -w, --max-gfxclk=<u32>                Maximum GFX Clock (MHz)
    -x, --min-gfxclk=<u32>                Minimum GFX Clock (MHz)
    -y, --prochot-deassertion-ramp=<u32>  Ramp Time After Prochot is Deasserted: limit power based on value, higher values does apply tighter limits after prochot is over
    --apu-skin-temp=<u32>                 APU Skin Temperature Limit    - STT LIMIT APU (degree C)
    --dgpu-skin-temp=<u32>                dGPU Skin Temperature Limit   - STT LIMIT dGPU (degree C)
    --apu-slow-limit=<u32>                APU PPT Slow Power limit for A+A dGPU platform - PPT LIMIT APU (mW)
    --skin-temp-limit=<u32>               Skin Temperature Power Limit (mW)
    --power-saving                        Hidden options to improve power efficiency (is set when AC unplugged): behavior depends on CPU generation, Device and Manufacture
    --max-performance                     Hidden options to improve performance (is set when AC plugged in): behavior depends on CPU generation, Device and Manufacture
```

### Demo

If you're going to set all Power Limits to 45W and Tctl to 90°C:

    ryzenadj.exe --stapm-limit=45000 --fast-limit=45000 --slow-limit=45000 --tctl-temp=90

### Documentation

- [Supported Models](https://github.com/FlyGoat/RyzenAdj/wiki/Supported-Models)
- [Renoir Tuning Guide](https://github.com/FlyGoat/RyzenAdj/wiki/Renoir-Tuning-Guide)
- [Options](https://github.com/FlyGoat/RyzenAdj/wiki/Options)
- [FAQ](https://github.com/FlyGoat/RyzenAdj/wiki/FAQ)
- **[PawnIO Setup Guide](PAWNIO_SETUP_WINDOWS.md)** - Installation and troubleshooting
- **[PawnIO Porting Guide](PAWNIO_PORTING_GUIDE.md)** - Technical details

## Installation

### Windows Installation with PawnIO

**Step 1: Install PawnIO Driver** (One-time setup)

1. Download PawnIO from: https://github.com/namazso/PawnIO/releases
2. Extract to a location (e.g., `C:\Tools\PawnIO`)
3. Run `install.bat` as Administrator
4. Reboot if prompted

Verify installation:
```bash
sc query PawnIO
# Should show: STATE: RUNNING
```

**Step 2: Build RyzenAdj with PawnIO**

```bash
git clone https://github.com/finalotogaming/RyzenAdj.git
cd RyzenAdj
git checkout pawnio-port

mkdir build
cd build
cmake -DPAWNIO_ROOT="C:\Tools\PawnIO" ..
cmake --build . --config Release
```

**Step 3: Run RyzenAdj**

Always run as Administrator:
```bash
ryzenadj.exe --info
```

Or create a shortcut with "Run as administrator" checked.

**Step 4 (Optional): Automation with Task Scheduler**

1. Open Task Scheduler
2. Create Basic Task → "RyzenAdj"
3. Set trigger to "At startup"
4. Action: Start program → `C:\path\to\ryzenadj.exe`
5. Arguments: `--stapm-limit=45000 --fast-limit=45000 --slow-limit=45000`
6. Check "Run with highest privileges"

See [PAWNIO_SETUP_WINDOWS.md](PAWNIO_SETUP_WINDOWS.md) for detailed instructions.

### Linux Installation

Same as original - PawnIO is Windows-only. Use the master branch on Linux.

Linux build:
```bash
git clone https://github.com/finalotogaming/RyzenAdj.git
cd RyzenAdj
git checkout master

mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
sudo make install
```

## Build

### Build Requirements

- **Windows**: Visual Studio 2019+ with MSVC, or Clang + CMake
- **Linux**: GCC/Clang + CMake
- **All platforms**: CMake 3.10+

### Windows with PawnIO

**Prerequisites:**
- PawnIO installed and driver loaded
- Visual Studio 2019+ or Clang
- CMake 3.10+

**Build Steps:**

```bash
# Clone the repository
git clone https://github.com/finalotogaming/RyzenAdj.git
cd RyzenAdj
git checkout pawnio-port

# Create build directory
mkdir build
cd build

# Configure with PawnIO path
cmake -DPAWNIO_ROOT="C:\Tools\PawnIO" -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . --config Release

# Binary will be in: build\Release\ryzenadj.exe
```

**If CMake can't find PawnIO:**

1. Verify PawnIO is installed at the path you specified
2. Check these files exist:
   - `C:\Tools\PawnIO\include\pawnio_um.h`
   - `C:\Tools\PawnIO\lib\PawnIOLib.lib`
3. Set environment variable: `set PAWNIO_ROOT=C:\Tools\PawnIO`
4. Try CMake again

### Linux

See the Linux section in [the master branch README](https://github.com/finalotogaming/RyzenAdj/blob/master/README.md#linux)

Same procedure works on this branch for Linux builds (uses libpci, not PawnIO).

## Troubleshooting

### "Failed to open PawnIO device"

1. **Verify driver is loaded:**
   ```bash
   sc query PawnIO
   ```
   Should show `STATE: RUNNING`

2. **Restart driver:**
   ```bash
   sc stop PawnIO
   sc start PawnIO
   ```

3. **Reinstall PawnIO:**
   ```bash
   sc delete PawnIO
   # Then run install.bat from PawnIO directory again
   ```

### "Permission denied" or access errors

- Always run as Administrator
- Right-click `ryzenadj.exe` → "Run as administrator"
- Or use Task Scheduler with "Run with highest privileges" option

### Build errors with CMake

- **"PawnIO not found"**: Set `-DPAWNIO_ROOT="C:\path\to\PawnIO"`
- **"pawnio_um.h not found"**: Check PawnIO include directory exists
- **"PawnIOLib.lib not found"**: Verify PawnIO library directory has this file

See [PAWNIO_SETUP_WINDOWS.md](PAWNIO_SETUP_WINDOWS.md) for more detailed troubleshooting.

## Contributing

- **Bugs/Issues**: Report on [GitHub Issues](https://github.com/finalotogaming/RyzenAdj/issues)
- **PawnIO Issues**: https://github.com/namazso/PawnIO/issues
- **Original RyzenAdj**: https://github.com/FlyGoat/RyzenAdj

## License

- **RyzenAdj**: LGPL-3.0
- **PawnIO**: GPL-2.0
- **This port**: Maintains LGPL-3.0 for RyzenAdj code

## Credits

- **Original RyzenAdj**: [FlyGoat](https://github.com/FlyGoat)
- **PawnIO Driver**: [namazso](https://github.com/namazso)
- **PawnIO Port**: This fork

## Resources

- **PawnIO Homepage**: https://pawnio.eu
- **PawnIO GitHub**: https://github.com/namazso/PawnIO
- **Original RyzenAdj**: https://github.com/FlyGoat/RyzenAdj
- **PawnIO Setup Guide**: [PAWNIO_SETUP_WINDOWS.md](PAWNIO_SETUP_WINDOWS.md)
- **PawnIO Tech Guide**: [PAWNIO_PORTING_GUIDE.md](PAWNIO_PORTING_GUIDE.md)

---

**Branch**: pawnio-port  
**Last Updated**: 2026-07-07  
**Status**: ✅ Ready for Production
