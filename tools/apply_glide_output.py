"""Reproducible RGBA8 output metadata adapters for both original Glide drivers."""
from pathlib import Path
import re

def apply(root):
    for filename, prefix, about, probe, table in (
        ("nfs3hp.43.cpp", "Application::", "504e00", "504c80", "0x567efc"),
        ("voodoo2a.0.cpp", "", "a83c60", "a83ac0", "0xa92018")):
        path = root / "src/nfs3hp/disassembly" / filename
        text = path.read_text()
        if '#include <lib/glide_output.h>' not in text:
            text = '#include <lib/glide_output.h>\n' + text
        for name in (about, probe):
            pattern = r'void ' + re.escape(prefix) + 'sub_' + name + r'\([\s\S]*?(?=\nvoid |\Z)'
            match = re.search(pattern, text)
            if not match: raise RuntimeError("Missing output adapter site: " + name)
            body = match[0]
            if '// Port RGBA8 mode adapter' in body: continue
            body = body.replace('{\n', '{\n    // Port RGBA8 mode adapter: output depth is independent of RGB565 LFB.\n', 1)
            pointer = '&app->getMemory<x86::reg32>(' + table + ')'
            if name == probe:
                # Original Voodoo RAM budgeting must retain its original 16-bit accounting.
                body = body.replace('// Port RGBA8 mode adapter: output depth is independent of RGB565 LFB.\n',
                    '// Port RGBA8 mode adapter: output depth is independent of RGB565 LFB.\n    win32::setGlideOutputDepth(' + pointer + ', 16);\n', 1)
            body = body.replace('    return;','    win32::setGlideOutputDepth(' + pointer + ', 32);\n    return;')
            text = text[:match.start()] + body + text[match.end():]
        path.write_text(text)

if __name__ == "__main__": apply(Path(__file__).resolve().parents[1])
