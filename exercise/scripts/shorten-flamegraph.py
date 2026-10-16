#!/usr/bin/env python3

import re
import subprocess
import sys

TEMPLATE = re.compile(r"<[^<>]*>")
PARAMS = re.compile(r"\(.*\)\s*$")
QUALIFIERS = re.compile(r"\b(const|volatile|noexcept)\b")

DROP = {
    "_start",
    "__libc_start_main",
    "__libc_start_call_main",
    "__libc_csu_init",
    "__libc_csu_fini",
    "_dl_start",
    "_dl_start_final",
    "_dl_init",
}
DROP_RE = re.compile(r"^(__libc_start|_dl_|\[.*\])")


def strip_templates(name):
    previous = None
    while previous != name:
        previous = name
        name = TEMPLATE.sub("", name)
    return name


def shorten(name):
    name = strip_templates(name)
    name = PARAMS.sub("", name)
    name = QUALIFIERS.sub("", name).strip()
    parts = [part for part in name.split("::") if part and part != "std" and not part.startswith("__")]
    if not parts:
        return name
    return "::".join(parts[-2:])


def drop(name):
    return name in DROP or bool(DROP_RE.match(name))


def demangle(names):
    mangled = sorted({name for name in names if name.startswith("_Z")})
    if not mangled:
        return {}
    try:
        result = subprocess.run(["c++filt", *mangled], capture_output=True, text=True)
    except OSError:
        return {}
    if result.returncode != 0:
        return {}
    demangled = result.stdout.splitlines()
    if len(demangled) != len(mangled):
        return {}
    return dict(zip(mangled, demangled))


def main():
    stacks = []
    frames = []
    for line in sys.stdin.read().splitlines():
        if not line.strip():
            continue
        stack, _, count = line.rpartition(" ")
        if not stack or not count:
            continue
        names = stack.split(";")
        stacks.append((names, count))
        frames.extend(names)

    mapping = demangle(frames)

    out = []
    for names, count in stacks:
        kept = []
        for name in names:
            name = mapping.get(name, name)
            if drop(name):
                continue
            kept.append(shorten(name))
        if kept:
            out.append(";".join(kept) + " " + count)

    sys.stdout.write("\n".join(out))
    if out:
        sys.stdout.write("\n")


if __name__ == "__main__":
    main()
