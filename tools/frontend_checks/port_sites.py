"""The port's patch sites, by address: who keeps a site once its function goes native.

Every tools/apply_*.py changes generated code at the instructions its lists name,
and raises if one is gone.  A function in disassemble_nfs3hp.py's MENU_NATIVES has no
generated code, so its sites must not reach the tool; and the native is held to the
patched game, so the same change has to be made to the code generate.py makes afresh.
Both are done here from the same lists, so that a group's merge needs neither by hand:

    leave_natives(functions, natives)      disassemble_nfs3hp.py, before the tools run:
                                           those sites taken out of the tools' lists
    sites_of_natives(functions, natives)   generate.py: {function: [Patch, ...]}

The lists read (formats vary with the tool's age):

  (file, "004406d7  c74064...", old, new[, declaration])  SITES, NATIVE_SITES and any list
                                           of that shape, MODE_LABEL and ZOOM among them
  (file, "0041a1c6", bytes, register)      apply_alpha_intensity.SITES, by its own test
  (file, "00448651")                       apply_menu_unlock.SITES, by CAP_TEST
  (instruction, field)                     apply_cabin_fit.READS
  (address, (old, new))                    apply_hud_scale.READS
  (module, "sub_40de30", hook)             the hooks (apply_native_vertices.py's and those
                                           that use its apply): nothing to make in the
                                           fresh code, extract.py takes hooks out; said
                                           (hooks_of_natives), as the native must then
                                           do what the hook did itself

Tools that write their sites inside apply() (apply_movie_tap, apply_native_movie's
player, apply_glide_output) are in WHOLE_FUNCTION, for merge_group.py to refuse.
"""
import importlib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# In the tools' lists and meant to stay out of the fresh code: {instruction address: why}.
LEFT_OUT = {}

# Functions the tools change with text of their own, not by a list.
WHOLE_FUNCTION = {0x495bc0: "tools/apply_movie_tap.py, tools/apply_native_movie.py"}


class Patch:
    """A change at one instruction: `old` made `new` under the comment for `instruction`."""
    __slots__ = ("tool", "instruction", "address", "old", "new", "declaration")

    def __init__(self, tool, instruction, old, new, declaration=""):
        self.tool, self.instruction, self.old, self.new, self.declaration = tool, instruction, old, new, declaration
        self.address = int(instruction[:8], 16)


def tools():
    """Every tools/apply_*.py, imported by name as disassemble_nfs3hp.py imports them."""
    if str(ROOT) not in sys.path:
        sys.path.insert(0, str(ROOT))
    return {path.stem: importlib.import_module("tools." + path.stem) for path in sorted((ROOT / "tools").glob("apply_*.py"))}


def _is_edit(site):
    return (isinstance(site, tuple) and len(site) >= 4 and isinstance(site[1], str) and re.match(r"[0-9a-f]{8}\b", site[1])
            and isinstance(site[2], str) and isinstance(site[3], str))


def _lists(module):
    """(name, list) of what in a tool could hold sites."""
    return [(name, value) for name, value in vars(module).items()
            if name.isupper() and isinstance(value, list) and all(isinstance(s, tuple) for s in value)]


def patches_of(stem, module, site, list_name):
    """The Patch a site makes; None for a row that is not a change at an instruction (hook_of's)."""
    if stem == "apply_alpha_intensity" and list_name == "SITES":
        _, address, code, register = site
        test = module.MEMORY_TEST if register is None else module.REGISTER_TEST % register
        was = "the 0x7a3a58 probe" if register is None else "%s & 0x40, %s read from 0x7a3a58" % (register, register)
        return Patch(stem, "%s  %s" % (address, code), test, module.FORCED % was)
    if stem == "apply_menu_unlock" and list_name == "SITES":
        return Patch(stem, "%s  f605583a7a0040" % site[1], module.CAP_TEST, module.CAP_FORCED)
    if stem == "apply_cabin_fit" and list_name == "READS":
        instruction, field = site
        decimal, index = module.FIELDS[field]
        return Patch(stem, instruction, "app->getMemory<x86::reg32>(x86::reg32(%d) /* %s */)" % (decimal, field),
                     "cabinRect(app, %d) /* port: the cabin fitted to 4:3, was [%s] */" % (index, field), module.DECLARATION)
    if stem == "apply_hud_scale" and list_name == "READS":
        address, (old, new) = site
        return Patch(stem, address + "  ", old, new, module.DECLARATIONS)
    if _is_edit(site):
        return Patch(stem, site[1], site[2], site[3], site[4] if len(site) > 4 else "")
    return None


def hook_of(site):
    """(module, function's address, hook) of a hook's row, or None."""
    if (isinstance(site, tuple) and len(site) == 3 and all(isinstance(part, str) for part in site)
            and re.fullmatch(r"sub_[0-9a-f]+", site[1])):
        return site[0], int(site[1][4:], 16), site[2]
    return None


def every_list():
    """(tool's name, module, list's name, list) of all tools."""
    for stem, module in tools().items():
        for name, value in _lists(module):
            yield stem, module, name, value


def native_addresses(functions, natives):
    """The instructions in the natives and in no other function: functions is {entry: addresses}.
    Code the generator still writes for another entry (a split's parent) keeps its sites."""
    written = set()
    for entry, addresses in functions.items():
        if entry not in natives:
            written.update(addresses)
    return {address: entry for entry in natives if entry in functions for address in functions[entry] if address not in written}


def of_module(module):
    """{entry: its instructions' addresses} of the generator's module (disasm's Application)."""
    return {s.get_entry_point(): [i.address for i in s.instructions] for s in module.subroutines}


def leave_natives(functions, natives):
    """Out of the tools' lists, in this process only: every site in a native's code, hooks
    on a native too.  The tools then find only what still is there.  What was taken."""
    inside = native_addresses(functions, natives)
    taken = []
    for stem, module, name, value in every_list():
        kept = []
        for site in value:
            patch = patches_of(stem, module, site, name)
            hook = hook_of(site)
            if (patch is not None and patch.address in inside) or (hook is not None and hook[1] in natives):
                taken.append((stem, name, site))
            else:
                kept.append(site)
        # In place: `from x import SITES` and the tools' own globals see it alike.
        value[:] = kept
    return taken


def sites_of_natives(functions, natives):
    """{a native's entry: its Patches, in the tools' order}, each instruction once: a site can
    stand in two of a tool's lists (a tool's whole list and the part it moved)."""
    inside = native_addresses(functions, natives)
    found = {}
    seen = set()
    for stem, module, name, value in every_list():
        for site in value:
            patch = patches_of(stem, module, site, name)
            if patch is None or patch.address not in inside or patch.address in LEFT_OUT:
                continue
            key = (patch.instruction, patch.old, patch.new)
            if key not in seen:
                seen.add(key)
                found.setdefault(inside[patch.address], []).append(patch)
    return found


def hooks_of_natives(natives):
    """(tool, list, function's address, hook) of the hooks' rows on natives."""
    found = []
    for stem, module, name, value in every_list():
        for site in value:
            hook = hook_of(site)
            if hook is not None and hook[1] in natives:
                found.append((stem, name, hook[1], hook[2]))
    return found


def by_address(text, names):
    """`nfs3hp::render::detailSettings` as `sub_472d10`: the tree's native names in a site's
    text, in the fresh code's terms.  names: {address: name in nfs3hp}."""
    for address, name in sorted(names.items(), key=lambda item: -len(item[1])):
        qualified = "nfs3hp::" + name
        if qualified in text:
            text = re.sub(re.escape(qualified) + r"\b", "sub_%x" % address, text)
    return text


def make_in(text, patches, names):
    """The patches made to a function's fresh code.  `old` must be in the instruction's own
    block, as the tools ask: the comment can come twice (a run of x87 kept in locals writes
    it outside and inside), then the block that has it."""
    for patch in patches:
        old = by_address(patch.old, names)
        # In `new` what stands in `old` goes the same way; what only `new` calls stays as the
        # tree has it, for extract.py to take by address like any other native's call.
        new = patch.new
        for address, name in names.items():
            qualified = "nfs3hp::" + name
            if qualified in patch.old:
                new = re.sub(re.escape(qualified) + r"\b", "sub_%x" % address, new)
        anchor = "// " + patch.instruction
        at = text.find(anchor)
        while True:
            if at < 0:
                # Its text is the tree's: a native renamed since, for one (apply_music_stream.py's).
                raise SystemExit("%s: at %s the fresh code has no %r" % (patch.tool, patch.instruction, old))
            stop = text.find("\n    // 00", at + len(anchor))
            stop = len(text) if stop < 0 else stop
            if old in text[at:stop]:
                break
            at = text.find(anchor, stop)
        text = text[:at] + text[at:stop].replace(old, new, 1) + text[stop:]
    return text
