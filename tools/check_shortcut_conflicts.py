#!/usr/bin/env python3
"""Detect duplicate keyboard shortcuts across the application actions.

Why: actions are created in several files (src/Create/*, src/main/*). A duplicate
shortcut inside the same QActionGroup silently makes one of the actions
unreachable by keyboard (for example the historical Scale Bar / Guidelines
collision on Shift+F8).

The check is static: it scans the C++ sources for
    <action>->setShortcut (QKeySequence (tr ("Shift+F8")));
and for Qt standard keys, skipping the latter (they never collide by design).

Exit code 0 = no duplicates, 1 = duplicates found (CI gate).
"""

from __future__ import annotations

import re
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SOURCE_ROOT = REPO_ROOT / "src"

NEW_ACTION_RE = re.compile(r"new\s+QAction\s*\(\s*(?:tr\s*\(\s*)?\"(?P<name>[^\"]*)\"")
SET_SHORTCUT_TR_RE = re.compile(
    r"(?P<action>\w+)\s*->\s*setShortcut\s*\(\s*QKeySequence\s*\(\s*tr\s*\(\s*\"(?P<key>[^\"]*)\"\s*\)\s*\)\s*\)"
)
SET_SHORTCUT_STANDARD_RE = re.compile(
    r"(?P<action>\w+)\s*->\s*setShortcut\s*\(\s*QKeySequence\s*::\s*\w+"
)


def normalize(key: str) -> str:
    return re.sub(r"\s+", "", key).lower()


def action_label(lines: list[str], index: int, action: str) -> str:
    """Best-effort human readable name for the action owning line `index`."""
    for back in range(index, max(-1, index - 12), -1):
        match = NEW_ACTION_RE.search(lines[back])
        if match:
            return match.group("name")
    return action


def scan() -> tuple[dict[str, list[str]], int]:
    owners: dict[str, list[str]] = defaultdict(list)
    standard = 0

    for path in sorted(SOURCE_ROOT.rglob("*.cpp")):
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        rel = path.relative_to(REPO_ROOT).as_posix()
        for number, line in enumerate(lines):
            if SET_SHORTCUT_TR_RE.search(line):
                match = SET_SHORTCUT_TR_RE.search(line)
                assert match is not None
                label = action_label(lines, number, match.group("action"))
                owners[normalize(match.group("key"))].append(
                    f"{rel}:{number + 1} {label}"
                )
            elif SET_SHORTCUT_STANDARD_RE.search(line):
                standard += 1

    return owners, standard


def main() -> int:
    owners, standard = scan()

    duplicates = {key: hits for key, hits in owners.items() if len(hits) > 1}

    print(f"scanned shortcuts: {sum(len(v) for v in owners.values())} explicit"
          f" + {standard} Qt standard keys")
    for key in sorted(owners):
        for hit in owners[key]:
            print(f"  {key:<12} {hit}")

    if duplicates:
        print("\nDUPLICATE SHORTCUTS DETECTED:")
        for key, hits in sorted(duplicates.items()):
            print(f"  {key}:")
            for hit in hits:
                print(f"    - {hit}")
        return 1

    print("\nno duplicate shortcuts found")
    return 0


if __name__ == "__main__":
    sys.exit(main())
