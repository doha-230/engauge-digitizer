# Offline Update Procedure

This document is for administrators of machines without network access. It describes how to update
Engauge Digitizer when a new release is available, using removable media.

## When to update

Engauge never checks the network. Update when you learn about a new release through your own
channels (a software distribution process, an internal announcement, or a USB hand-off).

## What to bring in

A new release consists of the same two files as the original installation:

| File | Purpose |
|---|---|
| `Engauge-Digitizer-Windows-Setup-<version>.exe` | Installer for per-machine or per-user installation |
| `Engauge-Digitizer-Windows-Portable-<version>.zip` | Portable zip, no installation, no registry use |

Both come with a `SHA256SUMS.txt` checksum file from the release page. Verify the checksums before
deploying:

```
certutil -hashfile Engauge-Digitizer-Windows-Setup-<version>.exe SHA256
```

and compare with the entry in `SHA256SUMS.txt`.

## Update steps

1. Close every running Engauge window.
2. **Installer route**: run the new Setup executable. It updates the existing installation in place;
   settings and recent files are preserved (they live in the per-user profile, not in the install
   directory).
3. **Portable route**: extract the new zip into a new directory, copy any local configuration you
   want to keep (`Engauge.ini` next to the executable, when it exists), and retire the old directory.
   The portable build keeps everything inside its folder.
4. Start the new version once and open `Help > About`. The version there must match the release you
   deployed. This is the only verification Engauge offers by design, since it does no network calls.

## Rollback

Keep the previous version until the new one has been in use for a while:

- Installer route: re-run the old Setup executable over the new one.
- Portable route: keep the old directory until the new one is verified, then delete it.

## Documents never migrate

Engauge documents (`.dig` files) are independent of the program version. Newer versions read older
documents. A document written by a newer version may contain data an older version cannot show, so
the usual direction of the upgrade matters.

## Diagnostics without network

`Help > About` shows the version, the Qt runtime and the build date.
When reporting a problem, copy what these show into the report.
