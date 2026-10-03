from . import arguments


def cg_fninit(instruction, function_bounds, function_names):
    return ['cpu.fpu.init();']


def cg_wait(instruction, function_bounds, function_names):
    return ['/*nothing*/;']
cg_fwait = cg_wait
cg_fclex = cg_wait
cg_fnclex = cg_wait

def cg_fst(instruction, function_bounds, function_names, destination):
    return ['%s = %s(cpu.fpu.st(0));' % (arguments.get_float(instruction, destination), arguments.get_float_type(destination.size))]


def cg_fstp(instruction, function_bounds, function_names, destination):
    return cg_fst(instruction, function_bounds, function_names, destination) + ['cpu.fpu.pop();']


def cg_fld(instruction, function_bounds, function_names, value):
    return ['cpu.fpu.push(x86::Float(%s));' % arguments.get_float(instruction, value)]


def cg_fild(instruction, function_bounds, function_names, value):
    return ['cpu.fpu.push(x86::Float(x86::sreg%d(%s)));' % (value.size*8, arguments.get_value(instruction, value))]


def cg_fldz(instruction, function_bounds, function_names):
    return ['cpu.fpu.push(0.0);']


def cg_fld1(instruction, function_bounds, function_names):
    return ['cpu.fpu.push(1.0);']


def cg_fldpi(instruction, function_bounds, function_names):
    return ['cpu.fpu.push(3.1415926535897932);']


def cg_fldl2e(instruction, function_bounds, function_names):
    return ['cpu.fpu.push(1.4426950408889634);']

def cg_fldln2(instruction, function_bounds, function_names):
    return ['cpu.fpu.push(0.6931471805599453);']

def cg_fyl2x(instruction, function_bounds, function_names):
    return [
        'cpu.fpu.st(1) = cpu.fpu.log2(cpu.fpu.st(0)) * cpu.fpu.st(1);',
        'cpu.fpu.pop();'
    ]

# Whether arithmetic rounds as the x87's precision control asks (FPU::add, sub,
# mul, div).  The game's own code does: it runs its races in single precision,
# and its physics has to come out as a PC's does.  The driver DLLs -- drawing
# and sound -- keep plain double arithmetic: rounding there changes nothing on
# the screen and costs a little on every vertex.  Set per module by the writers
# (application.py, dll.py).
EXACT = True

_SYMBOLS = {'add': '+', 'sub': '-', 'mul': '*', 'div': '/'}


def _op(name, destination, a, b):
    """`destination` = `a` name `b`, rounded where EXACT says."""
    if EXACT:
        return '%s = cpu.fpu.%s(%s, %s);' % (destination, name, a, b)
    return '%s = %s %s %s;' % (destination, a, _SYMBOLS[name], b)


def _st(instruction, operand):
    return arguments.get_float(instruction, operand)


def _mem(instruction, operand):
    return 'x86::Float(%s)' % arguments.get_float(instruction, operand)


# st(0) op= memory or register (one operand), or first op= second (two).
def _plain(name, instruction, argument):
    if len(argument) == 1:
        return [_op(name, 'cpu.fpu.st(0)', 'cpu.fpu.st(0)', _mem(instruction, argument[0]))]
    elif len(argument) == 2:
        destination = _st(instruction, argument[0])
        return [_op(name, destination, destination, _mem(instruction, argument[1]))]
    assert False


# The popping form: st(1), or the named register, op= st(0), then pop.
def _popping(name, instruction, argument):
    if len(argument) == 0:
        return [_op(name, 'cpu.fpu.st(1)', 'cpu.fpu.st(1)', 'cpu.fpu.st(0)'), 'cpu.fpu.pop();']
    elif len(argument) == 1:
        destination = _st(instruction, argument[0])
        return [_op(name, destination, destination, 'cpu.fpu.st(0)'), 'cpu.fpu.pop();']
    assert False


# The reversed form: st(0) = memory op st(0), or first = second op first.
def _reversed(name, instruction, argument):
    if len(argument) == 1:
        return [_op(name, 'cpu.fpu.st(0)', _mem(instruction, argument[0]), 'cpu.fpu.st(0)')]
    elif len(argument) == 2:
        destination = _st(instruction, argument[0])
        return [_op(name, destination, _mem(instruction, argument[1]), 'x86::Float(%s)' % destination)]
    assert False, instruction.op_str


# The reversed popping form: st(1), or the named register, = st(0) op itself, then pop.
def _reversed_popping(name, instruction, argument):
    if len(argument) == 0:
        return [_op(name, 'cpu.fpu.st(1)', 'cpu.fpu.st(0)', 'cpu.fpu.st(1)'), 'cpu.fpu.pop();']
    elif len(argument) == 1:
        destination = _st(instruction, argument[0])
        return [_op(name, destination, 'cpu.fpu.st(0)', 'x86::Float(%s)' % destination), 'cpu.fpu.pop();']
    assert False


def cg_fadd(instruction, function_bounds, function_names, *argument):
    return _plain('add', instruction, argument)


def cg_faddp(instruction, function_bounds, function_names, *argument):
    return _popping('add', instruction, argument)


def cg_fsub(instruction, function_bounds, function_names, *argument):
    return _plain('sub', instruction, argument)


def cg_fsubp(instruction, function_bounds, function_names, *argument):
    return _popping('sub', instruction, argument)


def cg_fsubr(instruction, function_bounds, function_names, *argument):
    return _reversed('sub', instruction, argument)


def cg_fsubrp(instruction, function_bounds, function_names, *argument):
    return _reversed_popping('sub', instruction, argument)


def cg_fmul(instruction, function_bounds, function_names, *argument):
    if len(argument) == 2:
        # Register by register, as it always was: no x86::Float around the second.
        destination = _st(instruction, argument[0])
        return [_op('mul', destination, destination, _st(instruction, argument[1]))]
    return _plain('mul', instruction, argument)


def cg_fmulp(instruction, function_bounds, function_names, *argument):
    return _popping('mul', instruction, argument)


def cg_fdiv(instruction, function_bounds, function_names, *argument):
    return _plain('div', instruction, argument)


def cg_fdivp(instruction, function_bounds, function_names, *argument):
    return _popping('div', instruction, argument)


def cg_fdivr(instruction, function_bounds, function_names, *argument):
    if len(argument) == 2:
        # As it always was: second / first, with no x86::Float around either.
        destination = _st(instruction, argument[0])
        return [_op('div', destination, _st(instruction, argument[1]), destination)]
    return _reversed('div', instruction, argument)


def cg_fdivrp(instruction, function_bounds, function_names, *argument):
    return _reversed_popping('div', instruction, argument)


def cg_frndint(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.rndint(cpu.fpu.st(0));']


def cg_fistp(instruction, function_bounds, function_names, destination):
    # toInteger stores the x87's integer indefinite for a NaN or a value out of
    # range, where a plain C++ conversion is undefined (and differs on arm64).
    return ['%s = x86::reg%d(cpu.fpu.toInteger<x86::sreg%d>(cpu.fpu.st(0)));' % (arguments.get_value(instruction, destination),
                                                                              destination.size * 8,
                                                                              destination.size * 8),
            'cpu.fpu.pop();']


def cg_fxch(instruction, function_bounds, function_names, *argument):
    if len(argument) == 0:
        dest = 'cpu.fpu.st(1)'
    elif len(argument) == 1:
        dest = arguments.get_float(instruction, argument[0])
    elif len(argument) == 2:
        dest = arguments.get_float(instruction, argument[1])
    else:
        assert False
    return ['{',
            '    x86::Float tmp = cpu.fpu.st(0);',
            '    cpu.fpu.st(0) = %s;' % (dest),
            '    %s = tmp;' % (dest),
            '}']


def cg_fchs(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = -cpu.fpu.st(0);']


def cg_ftst(instruction, function_bounds, function_names):
    return ['cpu.fpu.compare(cpu.fpu.st(0), 0.0);']


def cg_fcom(instruction, function_bounds, function_names, operand):
    return ['cpu.fpu.compare(cpu.fpu.st(0), x86::Float(%s));' % arguments.get_float(instruction, operand)]


def cg_fcomp(instruction, function_bounds, function_names, operand):
    return cg_fcom(instruction, function_bounds, function_names, operand) + ['cpu.fpu.pop();']


def cg_fcompp(instruction, function_bounds, function_names):
    return ['cpu.fpu.compare(cpu.fpu.st(0), cpu.fpu.st(1));',
            'cpu.fpu.pop();',
            'cpu.fpu.pop();']


def cg_fnstsw(instruction, function_bounds, function_names, destination):
    return ['%s = cpu.fpu.status.word;' % arguments.get_value(instruction, destination)]
cg_fstsw = cg_fnstsw


def cg_fldcw(instruction, function_bounds, function_names, operand):
    return ['cpu.fpu.setControl(%s);' % arguments.get_value(instruction, operand)]


def cg_fstcw(instruction, function_bounds, function_names, destination):
    return ['%s = cpu.fpu.control.word;' % arguments.get_value(instruction, destination)]
cg_fnstcw = cg_fstcw


def cg_fptan(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.tan(cpu.fpu.st(0));',
            'cpu.fpu.push(1.0);']


def cg_fsin(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.sin(cpu.fpu.st(0));']


def cg_fcos(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.cos(cpu.fpu.st(0));']


def cg_fabs(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.abs(cpu.fpu.st(0));']


def cg_fsqrt(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.sqrt(cpu.fpu.st(0));']


def cg_fpatan(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(1) = cpu.fpu.atan(cpu.fpu.st(0), cpu.fpu.st(1));',
            'cpu.fpu.pop();']


def cg_fprem(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.rem(cpu.fpu.st(0), cpu.fpu.st(1));']


def cg_fscale(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.scale(cpu.fpu.st(0), cpu.fpu.st(1));']


def cg_f2xm1(instruction, function_bounds, function_names):
    return ['cpu.fpu.st(0) = cpu.fpu.f2xm1(cpu.fpu.st(0));']

