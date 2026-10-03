from . import arguments
from capstone import x86


# A call hands the callee the registers (cpu.sync()) and takes back what it left
# (cpu.reload()): see x86::Local in include/cpu.h.  In a function that works on
# the CPU itself both are nothing.
def cg_call(instruction, function_bounds, function_names, destination):
    if destination.type == x86.X86_OP_IMM:
        name = function_names.get(destination.imm, 'sub_%x' % destination.imm)
        return ['cpu.esp -= 4;',
                '%s(app, cpu.sync());' % name,
                'cpu.reload();',
                'if (cpu.terminate) return;']
    else:
        return ['cpu.ip = %s;' % arguments.get_value(instruction, destination),
                'cpu.esp -= 4;',
                'app->dynamic_call(cpu.ip, cpu.sync());',
                'cpu.reload();',
                'if (cpu.terminate) return;']


def cg_ret(instruction, function_bounds, function_names, *value):
    if value:
        return ['cpu.esp += 4+%s;' % arguments.get_value(instruction, value[0]),
                'return;']
    else:
        return ['cpu.esp += 4;', 'return;']
