---
name: dosbox-testing
description: Use when running or debugging the tp33f DOS build tools (MAKE.EXE, TASMX.EXE, TLINK.EXE, TLIB.EXE, the `z80`/2pc converter) under DOSBox, or headlessly launching DOSBox batches. Triggers on: DOSBox, dosbox.sh, dosbox.cfg, COMMAND.COM, headless, UPPERCASED host files, MAKE 3.6, TASMX, TLINK, FILEIO.OBJ, TURBO.COM and the `..\2pc\z80` call.
---

# DOSBox headless testing (tp33f build tools)

The whole tp33f build (GNU make 3.6, TASMX.EXE, TLINK.EXE, TLIB.EXE and the
`z80`/2pc converter, in `BIN/`/`2PC/`) runs inside DOSBox. `dosbox.sh` +
`dosbox.cfg` mount the repository as `C:` and give an interactive DOS
prompt; the facts below are for scripting/headless use.

## Headless harness

- Launch a batch headlessly with:

  `dosbox --noprimaryconf --nolocalconf /abs/path/run.bat`

  The parent directory of the .bat is mounted as `C:` and the emulator exits
  0 when the batch finishes. Wrap with `timeout 30` — exit 124 means a hang,
  usually a program waiting on input (e.g. XCOPY prompting).
- Files written by DOS appear UPPERCASED on the host filesystem and land in
  the mounted directory — check the right folder; a "missing" file is often
  just uppercase (e.g. `OUT1.PC` or `C:\SRC\O5.PC`, not some repo root).
- `%errorlevel%` is not expanded by DOSBox's COMMAND.COM (it prints blank),
  and `2>&1` redirection is unsupported — do not use either for result
  signalling; finish each batch by writing a sentinel file instead.
- DOSBox COMMAND.COM cannot execute a program or batch through a command
  path that starts with `..\`, so batches must use absolute `C:\...` paths or
  plain names found via PATH. This is a shell limitation only: the tp33f
  builds run under GNU make (DJGPP), which resolves `..\2pc\z80` itself.
- `dosbox.cfg` mounts only the tp33f tree as `C:`, so `..\2pc\z80` from
  `C:\TURBO`/`C:\LIBRARY` resolves to the shipped `2PC/Z80.EXE` (+ its
  `2PC/CWSDPMI.EXE`), and `BIN/` carries Borland's DPMI files
  (DPMILOAD.EXE + DPMI16BI.OVL) for the protected-mode MAKE/TASMX/TLINK.

## Reference verification batch

Converting the 2pc converter's test suite under DOSBox (input at
`2pc/src/TEST`, expected outputs `2pc/src/TESTOK.PC` and `TESTOK.MSX` in
the companion 2pc repository):

```
@echo off
path c:\
2pc.exe C:\SRC\TEST C:\OUT.PC pc
z80.exe C:\SRC\TEST C:\OUT.MSX msx
echo done > \DONE.TXT
```

Then compare on the host — output file names come back uppercase
(`OUT.PC`, `OUT.MSX`, `DONE.TXT`):

```
cmp <2pc>/src/TESTOK.PC <2pc>/OUT.PC
cmp <2pc>/src/TESTOK.MSX <2pc>/OUT.MSX
```

Both must be byte-identical (the 2pc DJGPP build forces `_O_BINARY` so no
CRLFs are injected).