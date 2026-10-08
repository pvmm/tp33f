# tp33f

Turbo Pascal 3.3f is a reverse engineered project from the original Turbo Pascal 3.0 '.com' file by Frits Hilderink.

Several additions were made:
    - long integer
    - memman
    - a graphical libary called GIOS

For more info and a complete manual see:
    https://www.generation-msx.nl/software/frits-hilderink/turbo-pascal-v33f/release/6425/

## Repository layout

- `TURBO/` - the compiler source tree. `TURBO/TURBO.MSX` is the Make project that builds the MSX version (`TURBO.COM`); `TURBO/TURBO.PC` (only in `TURBO.7z`) builds the DOS cross-compiler `TP3.EXE`.
- `LIBRARY/` - the shared runtime file-I/O library (`LIBRARY/FILEIO.MAC` and its Make project `LIBRARY/LIBRARY.MSX`).
- `BIN/` - the DOS build tools. `MAKE.EXE`, `TASMX.EXE` and `TLINK.EXE` run in 16-bit protected mode, so the DPMI files below have to stay with them:
    - `MAKE.EXE` - MAKE version 3.6, 1992 Borland International.
    - `TASMX.EXE` - the MSX Z80 assembler (protected-mode, needs `DPMILOAD.EXE` + `DPMI16BI.OVL`).
    - `TLIB.EXE` - TLIB version 3.02, 1992 Borland International (library manager).
    - `TLINK.EXE` - Turbo Link version 5.1, 1992 Borland International (linker; protected-mode, needs `DPMILOAD.EXE` + `DPMI16BI.OVL`).
    - `DPMILOAD.EXE` - DPMI Loader version 1.0, 1990-1991 Borland International. Loads the DPMI server and then starts the protected-mode tool; it is invoked (and required) by Borland's protected-mode programs.
    - `DPMI16BI.OVL` - the 16-bit DPMI server ("Ergo DPMI (286)"), the DPMI host that provides the DOS Protected Mode Interface (INT 31h, extended memory, ...) to the tool running in protected mode.
    - `DPMIMEM.DLL` - DPMI memory manager DLL used by the DPMI server. It reserves/allocks blocks of memory, manages EMS/expanded-memory use, and honors the `DPMIMEM=MAXMEM nnnn` environment variable to cap how much extended memory the DPMI kernel takes.
    - `DPMIRES.EXE` - the DPMI "install/reserve" program. Preloads the DPMI server as a TSR ("DPMI services resident") so protected-mode tools start faster; type EXIT to uninstall.
    - `DOS4GW.EXE` - DOS/4G, Copyright (c) Rational Systems, Inc. 1987-1993. The 32-bit protected-mode DOS extender runtime shipped with the Borland toolchain; loads/executes 32-bit extended programs and provides VCPI/DPMI services.
- `BIN.7z`, `LIBRARY.7z`, `TURBO.7z` - the original archives the folders above were extracted from. `TURBO.7z` additionally contains `TURBO.PC` and the assembler listing files (`*.LST`).
- `dosbox.sh` + `dosbox.cfg` - local DOSBox setup used to run the DOS build tools. Run `./dosbox.sh` to get a DOS prompt rooted at the repository.
- `.gitattributes` - forces CRLF on text files and marks binaries, to match the DOS/CP/M heritage of the sources.

## Building

Open `./dosbox.sh`, then in `LIBRARY/` run one of:
- `MAKEMSX.BAT` - `make -f library.msx`, converts `FILEIO.MAC` and assembles `LIBRARY/MSX_OBJ/FILEIO.OBJ`;
- `MAKEPC.BAT` - `make -f library.pc`, converts `FILEIO.MAC` and assembles `LIBRARY/PC_OBJ/FILEIO.OBJ`.

The shared runtime file-I/O library is built from `LIBRARY/` and is needed for `TURBO.COM` compilation.

Then go to `TURBO/` and run one of:
- `COMP.BAT` - compile the Pascal sources (`LONG.PAS`, `DIV32.PAS`, ...) with the PC cross-compiler `TP3.EXE`;
- `MAKEMSX.BAT` - `make -f turbo.msx`, builds `RUNTIME.COM` + `TURBO.COM` for MSX;
- `MAKEPC.BAT` - `make -f turbo.pc`, builds `TP3.EXE` for DOS (requires `TURBO.PC`, kept only in `TURBO.7z`).

Run `MAKEMSX.BAT` or `MAKEPC.BAT` in `LIBRARY/` to produce the `FILEIO.OBJ` needed by `TURBO.COM`.

## The `z80` converter and the `.mac` / `.mc` sources

Each module is written once as a Z80-oriented master source with a `.mac` extension: `TURBO/COMPILER.MAC`, `TURBO/RUNTIME.MAC`, `TURBO/INIT.MAC`, `TURBO/SLIB.MAC`, `TURBO/GLIB.MAC`, `TURBO/END.MAC` and `LIBRARY/FILEIO.MAC`. They cover both targets through `IFDEF MSX` / `IFDEF MAKEPC`, and lines that only make sense for the *other* target are written as comments prefixed with `;!`. `TURBO/GLIB.MAC` holds all the GIOS (graphical library) function signatures.

The `z80` binary is just Frits Hilderink's "2pc" converter (v1.6) under a different name; the Make files invoke it as `z80`, but it is the same 2pc program. It rewrites each `.mac` into one target-specific assembly file:

    z80 <module>.mac <output> <msx|pc>

- with the `msx` argument, `TURBO/RUNTIME.MAC` becomes `TURBO/MSX_GEN/RUNTIME.MC` (likewise `COMPILER.MC`, `INIT.MC`, `SLIB.MC`, `GLIB.MC`, `END.MC`). These `.mc` files are "processed assembly": the Z80 mnemonics are emitted as `DB`/`DW` statements, labels and `INCLUDE` names are uppercased, `DS n` is rewritten as `db n dup (0)`, and the `;!` lines stay comments. They are then `INCLUDE`d by the thin wrappers in `TURBO/MSX_ASM/` (e.g. `RTL_RTL.ASM`, `TUR_COMP.ASM`) and assembled by `TASMX.EXE` into `TURBO/MSX_OBJ/*.OBJ`.
- with the `pc` argument, the same `.mac` becomes a 8086 source in `TURBO/PC_GEN/*.ASM` (Z80 mnemonics translated to 80x86, e.g. `LD`->`MOV`, `JP`->`JMP`, `HL`->`BX`, and the `;!` lines activated, e.g. `pushf`/`popf`). These are `INCLUDE`d by `TURBO/PC_ASM/TP3_*.ASM` and assembled/linked into `TP3.EXE`. `LIBRARY/FILEIO.MAC` is converted straight to `LIBRARY/MSX_ASM/FILEIO.ASM` or `LIBRARY/PC_ASM/FILEIO.ASM` for direct assembly.

The MSX link is driven by `TURBO/MSX_OBJ/` targets: `RTL_RTL.OBJ` + `LIBRARY/MSX_OBJ/FILEIO.OBJ` + `RTL_END.OBJ` link into `RUNTIME.COM`, and `TUR_INIT/TUR_COMP/TUR_SLIB/TUR_GLIB/TUR_END` link into `TURBO.COM` (object list in `TURBO/LINK.MSX`). `LIBCONST.INC` is regenerated from `RUNTIME.MAP` by `TURBO/EXTERN.EXE` (source in `TURBO/EXTERN.C`) via the `libconst.inc : runtime.com` rule in the Makefile.

**Note:** the `z80` binary is included in this repository as `2PC/Z80.EXE` (together with `2PC/CWSDPMI.EXE` for DPMI) — it is simply the 2pc tool (v1.6, by F. Hilderink) under another name, and the Make files call it as `..\2pc\z80`, which resolves into the `2PC/` folder here. Its source lives in [fhil/2pc](https://github.com/fhil/2pc) ("create 2pc.exe", v1.6 by F. Hilderink), and a from-source rebuild for MS-DOS (DJGPP) is maintained in the companion `2pc` repository. The `TURBO/MSX_GEN/*.MC`, `TURBO/PC_GEN/*.ASM` and `LIBRARY/*_ASM/FILEIO.ASM` files in this repo are its checked-in outputs.
