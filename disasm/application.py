import io
import os
from .module import Module, function_opening, loop_heads, SAFEPOINT
from .codegen import fpu as fpu_codegen
from .codegen import fpu_frame
from .codegen import fpu_stack

class Application(Module):
    def __init__(self, application_name, exe_path, rebase_after):
        Module.__init__(self, application_name, exe_path, rebase_after)
        self._x87_frames = None
        # In plain doubles, for tools/game_checks: the game's code for natives
        # that are written in doubles.
        self.double_functions = set()

    def write(self, thread_segments=[], skip_instructions=[], dlls=[], function_names={}, natives={}):
        """`natives`: the game's functions the port has natively, {address:
        name} -- void (WinApplication*, CPU&) in the application's namespace,
        or one inside it the name says (watcom::strcmp).
        No code is written for them: the generated code calls the native where
        it called the function, and the address is registered to it, for the
        calls through pointers."""
        # The game's own arithmetic rounds as the x87 would (codegen/fpu.py).
        fpu_codegen.EXACT = True
        entries = set(s.get_entry_point() for s in self.subroutines)
        for address, name in natives.items():
            if address not in entries:
                raise RuntimeError('%s: no function at 0x%x for %s' % (self.application_name, address, name))
            if address in function_names:
                raise RuntimeError('%s: 0x%x is both %s and native %s' % (self.application_name, address,
                                                                         function_names[address], name))
        calls = self.native_calls(function_names, natives, self.application_name)
        try:
            os.makedirs('src/%s/disassembly' % (self.application_name))
        except OSError:
            pass
        with open('src/%s/disassembly/%s.cpp' % (self.application_name, self.application_name), 'w') as src:
            with open('src/%s/disassembly/%s.h' % (self.application_name,  self.application_name), 'w') as h:
                app_name = self.application_name
                APP_NAME = self.application_name.upper()
                src.write('#include "%s.h"\n'
                          '#include <x86.h>\n'
                          '#include <winapi/wrapper.h>\n' % (app_name))
                for dll in dlls:
                    src.write('#include "%s.h"\n' % (dll.application_name))
                for dll_name, dll_methods in self.dll_imports:
                    src.write('#include <winapi/%s.h>\n' % dll_name.decode())
                for dll in dlls:
                    for dll_name, dll_methods in dll.dll_imports:
                        src.write('#include <winapi/%s.h>\n' % dll_name.decode())
                src.write('\nnamespace %s\n'
                            '{\n\n' % (app_name))
                h.write('#ifndef %s_H_\n'
                        '#define %s_H_\n'
                        '#include <x86.h>\n'
                        '#include <lib/winapp.h>\n'
                        '#include <lib/memmap.h>\n'
                        '#include <map>\n'  % (APP_NAME, APP_NAME))
                h.write('\nnamespace %s\n'
                        '{\n\n' % (app_name))
                if natives:
                    h.write('/* The game\'s functions the port has natively, where their code was\n'
                            ' * (disassemble_nfs3hp.py): the generated code calls them by name. */\n')
                    for address, name in sorted(natives.items()):
                        h.write(self.native_declaration(address, name))
                    h.write('\n')
                h.write('class Application : public win32::WinApplication\n'
                        '{\n'
                        'public:\n'
                        '    Application(const char* appName);\n'
                        '    void execute();\n'
                        'private:\n')
                for data, address, length, name, ro, code in self.sections:
                    if data:
                        self._dump_raw_section(name, data, length)
                        h.write('    static const x86::reg8 s_%sSegment[];\n' % name.decode())
                section_segments = ['{ %s, 0x%x, %d }' % (('s_%sSegment'%name.decode()) if data else 'nullptr',
                                                           address,
                                                           length) for data, address, length, name, ro, code in self.sections]
                for dll in dlls:
                    section_segments += ['{ %s, 0x%x, %d }' % (('%s::s_%sSegment'%(dll.application_name, name.decode())) if data else 'nullptr',
                                                                address,
                                                                length) for data, address, length, name, ro, code in dll.sections]
                sections = ', '.join(section_segments)
                src.write('Application::Application(const char* appName)\n'
                            '    :   WinApplication(appName, 0x%x, { %s })\n'
                            '{\n' % (self.sections[0][1], sections))
                for dll in dlls:
                    print(dll)
                    src.write('        %s::s_registry->registerSymbols(this);\n' % dll.application_name)
                for subroutine in self.subroutines:
                    function_entry = subroutine.get_entry_point()
                    if function_entry in natives:
                        src.write('    registerMethod(0x%08x, {"<%s>%s", &%s});\n' % (
                            function_entry, app_name, natives[function_entry], calls[function_entry]))
                        continue
                    name = function_names.get(function_entry, 'sub_%x' % function_entry)
                    src.write('    registerMethod(0x%08x, {"<Application>%s", &Application::%s});\n' % (function_entry, name, name))
                for dll in dlls:
                    if dll.natives is not None:
                        # no code of its own: its exports are natives, and so is
                        # what they hand out to be called
                        for address, function in sorted(dll.native_callbacks.items()):
                            src.write('    registerMethod(0x%08x, {"<%s>%s", &%s::%s});\n' % (
                                address, dll.application_name, function, dll.natives_namespace, function))
                        continue
                    for subroutine in dll.subroutines:
                        function_entry = subroutine.get_entry_point()
                        name = function_names.get(function_entry, 'sub_%x' % function_entry)
                        src.write('    registerMethod(0x%08x, {"<%s>%s", &%s::%s});\n' % (function_entry, dll.application_name, name, dll.application_name, name))
                for dll_name, dll_methods in self.dll_imports:
                    for method, address in dll_methods:
                        src.write('    registerMethod(0x%08x, {"<%s>%s", &win32::Wrapper<decltype(&win32::%s::%s), &win32::%s::%s>::stdcall});\n' % (address, dll_name.decode(), method.decode(), dll_name.decode(), method.decode(), dll_name.decode(), method.decode()))
                for dll in dlls:
                    for dll_name, dll_methods in dll.dll_imports:
                        for method, address in dll_methods:
                            src.write('    registerMethod(0x%08x, {"<%s>%s", &win32::Wrapper<decltype(&win32::%s::%s), &win32::%s::%s>::stdcall});\n' % (address, dll_name.decode(), method.decode(), dll_name.decode(), method.decode(), dll_name.decode(), method.decode()))

                src.write('}\n\n')
                src.write('void Application::execute()\n'
                            '{\n'
                            '    runThread(m_cpu, 0x%x);\n'
                            '}\n\n' % (self.entry_point))
                src.write('}\n')

                for index in range(0, 1+int(len(self.subroutines)/100)):
                    methods = open('src/%s/disassembly/%s.%d.cpp' % (app_name, app_name, index), 'w')
                    methods.write('#include "%s.h"\n'
                                  '#include <lib/thread.h>\n\n'
                                  'namespace %s\n'
                                  '{\n\n' % (app_name, app_name))
                    for subroutine in self.subroutines[100*index:100*(index+1)]:
                        function_entry = subroutine.get_entry_point()
                        if function_entry in natives:
                            methods.write('/* sub_%x: native, %s */\n\n' % (function_entry, calls[function_entry]))
                            continue
                        name = function_names.get(function_entry, 'sub_%x' % function_entry)
                        h.write('    static void %s(WinApplication* app, x86::CPU& cpu);\n' % name)
                        self.write_function(methods, subroutine, name, calls, thread_segments, skip_instructions)
                    methods.write('}\n')
                h.write('};\n\n'
                        '}\n\n'
                        '#endif /* !%s_H_ */\n' % (APP_NAME))
        for dll in dlls:
            dll.write(self.application_name, thread_segments, function_names)

    @staticmethod
    def native_declaration(address, name):
        """A native's declaration, in the application's namespace: one named in
        a namespace of its own inside that, as watcom::strcmp, in that one."""
        *namespaces, function = name.split('::')
        declaration = 'void %s(win32::WinApplication* app, x86::CPU& cpu);' % function
        for namespace in reversed(namespaces):
            declaration = 'namespace %s { %s }' % (namespace, declaration)
        return '%s  // 0x%x\n' % (declaration, address)

    @staticmethod
    def native_calls(function_names, natives, namespace):
        """What the generated code calls each function by: its name, and a
        native's qualified with the namespace it is in."""
        calls = dict(function_names)
        calls.update((address, '%s::%s' % (namespace, name)) for address, name in natives.items())
        return calls

    def write_function(self, methods, subroutine, name, function_names, thread_segments=[], skip_instructions=[]):
        """One function's code as `name`, a member of Application, into
        `methods`; the functions it calls by `function_names`."""
        function_start = subroutine.get_start_address()
        function_end = subroutine.get_end_address()
        function_entry = subroutine.get_entry_point()
        fallthrough = False
        methods.write('/* align: skip %s */\n' % (' '.join(['0x%02x'%c for c in subroutine.skipped_blob])))
        if subroutine.data_blob:
            methods.write('/* data blob: %s */\n' % (''.join(['%02x'%c for c in subroutine.data_blob])))
        for jump_entry in subroutine.jump_table:
            methods.write('/* jump table: 0x%08x */\n' % jump_entry)
        parameters, opening = function_opening(subroutine, 'WinApplication')
        methods.write('void Application::%s%s\n'
                      '{\n'
                      '%s'
                      '  NFS2_USE(cpu);\n'
                      '  NFS2_USE(app);\n' % (name, parameters, opening))
        # The function's code goes through a buffer: which x87 locals it needs,
        # declared above it all, is known once it has been written.
        outer = methods
        methods = io.StringIO()
        frame = self._x87_frame(function_entry, skip_instructions)
        fpu_codegen.EXACT = function_entry not in self.double_functions
        fpu_codegen.BY_CONTROL_WORD = (function_entry in fpu_frame.UNDER_CONTROL_WORD
                                       or any(i.mnemonic == 'fldcw' for i in subroutine.instructions))
        if subroutine.thread_unsafe:
            methods.write('  win32::LockContext lock(*app);\n')
        if subroutine.dynamic_labels:
            methods.write('  goto start;\n'
                            'dynamic_jump:\n'
                            '  switch(cpu.ip)\n'
                            '  {\n'
                            'start:\n')
        if function_entry != function_start:
            methods.write('    goto L_entry_0x%08x;\n' % function_entry)
        run = fpu_stack.Run()
        dynamic_addresses = set(x for _, x in subroutine.dynamic_labels)
        static_addresses = set(x for _, x in subroutine.static_labels)
        loops = loop_heads(subroutine)
        for instruction in subroutine.instructions:
            lines = self.generate(instruction, (function_start, function_end), function_names)
            labelled = (instruction.address in dynamic_addresses
                        or instruction.address in static_addresses
                        or (function_entry != function_start and instruction.address == function_entry))
            skipped = instruction.address in skip_instructions
            carried = (frame is None and fpu_stack.ENABLED and not skipped
                       and not fpu_stack.Run.boundary(instruction) and fpu_stack.Run.understood(lines))
            if run.open and (labelled or not carried):
                methods.write(fpu_stack.text(run.close()))
            if instruction.address in dynamic_addresses:
                if fallthrough:
                    methods.write('  [[fallthrough]];\n')
                methods.write('  case 0x%08x:\n' % (instruction.address))
            if instruction.address in static_addresses:
                methods.write('L_0x%08x:\n' % (instruction.address))
            if function_entry != function_start and instruction.address == function_entry:
                methods.write('L_entry_0x%08x:\n' % (instruction.address))
            if instruction.address in thread_segments:
                methods.write('    app->unlockContext(cpu);\n'
                              '    win32::Thread::sleep(0);\n'
                              '    app->lockContext(cpu);\n')
            if instruction.address in loops:
                methods.write(SAFEPOINT)
            if skipped:
                lines = []
            elif carried:
                lines = run.transform(lines)
            elif frame is not None and not skipped and frame.depth_at(instruction) is not None:
                lines = frame.transform(instruction, lines)
            methods.write('    // %s\n'
                          '    %s\n'  % (self._raw(subroutine.section, instruction),
                                         '\n    '.join(lines)))
            fallthrough = instruction.mnemonic not in ['jmp', 'ret']
        if run.open:
            methods.write(fpu_stack.text(run.close()))
        if subroutine.dynamic_labels:
            methods.write('  default:\n'
                          '    NFS2_ASSERT(false);\n'
                          '  }\n')
        methods.write('}\n\n')
        if frame is not None:
            outer.write(frame.opening())
        outer.write(methods.getvalue())

    def _x87_frame(self, entry, skip_instructions):
        """The x87 stack of the function at `entry` in locals (codegen/fpu_frame.py);
        None for one whose depth does not follow from its code, which keeps it
        in the emulated FPU."""
        if not fpu_stack.ENABLED:
            return None
        if self._x87_frames is None:
            self._x87_frames, _ = fpu_frame.analyse(self, skip_instructions)
        frame = self._x87_frames.get(entry)
        return frame if frame is not None and frame.valid else None
