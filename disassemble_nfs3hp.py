import sys
sys.path.append('disasm')
from disasm import disassembler, dll


DATA_SEGMENTS = [(0x401004, 0x401010),
                 (0x405734, 0x405740),
                 (0x409ff0, 0x40a010),
                 (0x40cb00, 0x40cb20),
                 (0x412920, 0x412994),
                 (0x41a880, 0x41a8c8),
                 (0x420814, 0x420840),
                 (0x4212b0, 0x421360),
                 (0x423d50, 0x423d90),
                 (0x42d550, 0x42d618),
                 (0x42dd30, 0x42dd58),
                 (0x434ea0, 0x434f14),
                 (0x43f180, 0x43f1e4),
                 (0x44e150, 0x44e170),
                 (0x4489f0, 0x448ef2),
                 (0x454f10, 0x455020),
                 # Up to 0x45cdd0, not 0x45cdc0: the 16 bytes in between are
                 # alignment padding, and stopping short of them made the
                 # recompiler seed a function on the padding.  The real prologue
                 # is at 0x45cdd0, the game takes that address as a text-field
                 # callback, and the call found nothing registered.
                 (0x45cb60, 0x45cdd0),
                 (0x465d10, 0x465d60),
                 (0x46ccb0, 0x46cce0),
                 (0x472160, 0x4721f0),
                 (0x478310, 0x478320),
                 (0x47ba70, 0x47bbb8),
                 (0x488030, 0x4880b0),
                 (0x489630, 0x4896f0),
                 (0x491110, 0x491120),
                 (0x493360, 0x493380),
                 (0x493370, 0x493380),
                 (0x49e060, 0x49e080),
                 (0x4975b0, 0x497620),
                 (0x49c3e0, 0x49c410),
                 (0x49e060, 0x49e080),
                 (0x4a2f50, 0x4a3050),
                 (0x4a4050, 0x4a4060),
                 (0x4a6f80, 0x4a7080),
                 (0x4af3b0, 0x4b0700),
                 (0x4b5580, 0x4b56d0),
                 (0x4b6a10, 0x4b6a10),
                 (0x4b7a10, 0x4b7a60),
                 (0x4c9a70, 0x4c9a80),
                 (0x4d0650, 0x4d06f0),
                 (0x4d5740, 0x4d5840),
                 (0x4dfae5, 0x4dfaf0),
                 (0x4f7fa5, 0x4f8020),
                 (0x51679d, 0x5167b3),
                 (0x52efb0, 0x52f318),
                 (0x533ef0, 0x534000),]


MERGE_ROUTINES = [(0x405740, 0x405791),
                  (0x405740, 0x4057a0),
                  (0x420840, 0x42088a),
                  (0x420840, 0x420a71),
                  (0x420840, 0x420a7a),
                  (0x420840, 0x420a83),
                  (0x420840, 0x420aac),
                  (0x420840, 0x420b53),]

# Methods the game reaches only through its tables, so small (mov eax, 1;
# ret -- a bare ret -- a jmp) that they read as the tail of the function
# before them; a call to one found nothing registered and was skipped.
# 0x4f5fd0 is the IPX transport's (table 0x566e70): skipping it crashed both
# machines as an IPX race was joined.
KNOWN_SUBROUTINES = [0x4ff3d0, 0x4f5fd0, 0x505970, 0x509c60,]

# 0x4ff5c5 calls the test for the Pentium's FDIV bug (0x51f272), 0x4f0a06 is an emms.
SKIP_INSTRUCTIONS = [0x4a3aec, 0x4ff5c5, 0x4f0a06]


SPLIT_INSTRUCTIONS = [0x4e08af, 0x4e087a, 0x4eee0f]


THREAD_ROUTINES = []
# Busy waits on the main thread that only another thread can end.  Only one
# guest thread runs at a time, and a loop without API calls never lets go.
# 0x4e773f: sub_4e7720 waits for the game timer's tick [0x79c1a4] (the IPX
# library calls it on the main thread); 0x51436a: the IPX library waits for
# [edx+0xa8] to be cleared.  Without these the IPX lobby hangs.
THREAD_SEGMENTS = [0x43ac67, 0x43ac97, 0x46785e, 0x4f06f7, 0x4f2191, 0x495b68, 0x4d9158, 0x4c4e84, 0x4f7d0e,
                   0x4e773f, 0x51436a]

# The game's own functions that are native: each address is the port's
# function of that name in src/nfs3hp/native_game.cpp, no code is generated
# for it, and the generated code calls it by name: the game's side of THRASH
# and iSND, the logger, the lambdas iSNDloaddll hands eacsnd, and the fill the
# game's code clears its memory with; EA's names where the game's messages
# give them.  tools/game_checks generates their code
# all the same, to compare the natives with.
GAME_NATIVES = {
    0x401010: "Error",
    0x40faf0: "Is3DSoundAvailable",
    0x4df6c0: "Message",
    0x4e070c: "fillZero",               # the game's fill at its three entries: zeros,
    0x4e0716: "fillByte",               # bl spread,
    0x4e0721: "fillWord",               # the pattern in ebx
    0x4e1a00: "StopSoundBufferLambda",
    0x4e1a0c: "AcquireSoundBufferPositionLambda",
    0x4e1a24: "AcquireSoundDataLambda",
    0x4e1a34: "PrintSoundLambda",
    0x4e1a64: "TraceSoundLambda",
    0x4e1a88: "abortmessagewrapper",
    0x4e1adc: "TickSoundLambda",
    0x4e1ae4: "iSNDconvertsample",
    0x4e1d48: "audiofocushandler",
    0x4e1dd4: "iSNDloaddll",
    0x4e21b0: "SNDcaps",
    0x4e2234: "SoundServeHandler",
    0x4eba70: "AcquireWindow",
    0x4f8390: "THRASH_opendll",
}

# The Watcom C runtime's functions that are native: each address is the port's
# function of that name in src/nfs3hp/native_watcom.cpp, in nfs3hp::watcom --
# Open Watcom's names (src/nfs3hp/watcom.md maps the runtime in nfs3.exe).
# tools/game_checks compares them with their generated code as well.
WATCOM_NATIVES = {
    0x4df660: "watcom::mem_putc",
    0x4df674: "watcom::vsprintf",
    0x4df690: "watcom::sprintf",
    0x4dfd04: "watcom::exit",
    0x4dfd30: "watcom::_exit",
    0x4dfd56: "watcom::CHP",            # __CHP
    0x4e0640: "watcom::memset",
    0x4e06d9: "watcom::IF_DATAN2",      # IF@DATAN2
    0x4e0820: "watcom::strstr",
    0x4e0e30: "watcom::strncpy",
    0x4e28a0: "watcom::unlink",
    0x4e9030: "watcom::update_buffer",  # __update_buffer
    0x4e9070: "watcom::reset_buffer",   # __reset_buffer
    0x4e9088: "watcom::fseek",
    0x4e9200: "watcom::fread",
    0x4e9ec0: "watcom::stricmp",        # _stricmp
    0x4eb1d0: "watcom::mkdir",
    0x4eb1f0: "watcom::initrandnext",
    0x4eb1fc: "watcom::rand",
    0x4eb220: "watcom::srand",
    0x4eb390: "watcom::strncmp",
    0x4edd80: "watcom::open_flags",     # __open_flags
    0x4edebc: "watcom::doopen",         # __doopen
    0x4edfa4: "watcom::fsopen",         # _fsopen
    0x4edfe8: "watcom::fopen",
    0x4ee100: "watcom::fclose",
    0x4ee144: "watcom::shutdown_stream", # __shutdown_stream
    0x4ee15c: "watcom::hexDigit",       # __MkTmpFile's digit
    0x4ee168: "watcom::MkTmpFile",      # __MkTmpFile
    0x4ee214: "watcom::doclose",        # __doclose
    0x4ee2e0: "watcom::remove",
    0x4ee2f0: "watcom::strrchr",
    0x4ee310: "watcom::strcmp",
    0x4ee3c0: "watcom::chmod",
    0x4ee400: "watcom::rmdir",
    0x4ee420: "watcom::ftell",
    0x4ee480: "watcom::med3",
    0x4ee4d0: "watcom::byteswap",
    0x4ee4f8: "watcom::qsort",
    0x4eebd0: "watcom::fwrite",
    0x4eede0: "watcom::toupper",
    0x4eedf0: "watcom::strncat",
    0x4eff60: "watcom::strtok",
    0x4f0260: "watcom::chdir",
    0x4f0430: "watcom::atoi",
    0x4f0490: "watcom::atol",
    0x4f04f0: "watcom::cget_string",
    0x4f051c: "watcom::uncget_string",
    0x4f0520: "watcom::vsscanf",
    0x4f0548: "watcom::sscanf",
    0x4f0570: "watcom::fgets",
    0x4f05f0: "watcom::access",
    0x4f0710: "watcom::sleep",
    0x4f0730: "watcom::getcwd",
    0x4f07d0: "watcom::getdiskfree",    # _getdiskfree
    0x4f0ae0: "watcom::tolower",
    0x4f1c30: "watcom::floor",
    0x4f22b0: "watcom::time",
    0x4f2d78: "watcom::ceil",
    0x4f45c0: "watcom::stackavail",
    0x4f4610: "watcom::GRO",            # __GRO
    0x4f4b80: "watcom::fprintf",
    0x4f7900: "watcom::nmalloc",        # _nmalloc
    0x4f79f0: "watcom::nfree",          # _nfree
    0x4f7af8: "watcom::TryInit",        # __TryInit
    0x4f7b2a: "watcom::TryHandler",     # __TryHandler
    0x4f7b61: "watcom::TryScope",       # __TryHandler's scopes
    0x4f7b7d: "watcom::Except",         # __Except
    0x4f7b87: "watcom::TryOuter",       # __TryHandler's next scope
    0x4f7b8c: "watcom::TryDone",        # __TryHandler's end
    0x4f7ba7: "watcom::global_unwind",  # __global_unwind
    0x4f7bc3: "watcom::local_unwind",   # __local_unwind
    0x4f7c20: "watcom::atexit",
    0x4f7c90: "watcom::control87",      # _control87
    0x4f7ce0: "watcom::kbhit",
    0x4f7d70: "watcom::fflush",
    0x4f7d80: "watcom::fputs",
    0x4f7e20: "watcom::do_getch",
    0x4f7f24: "watcom::getch",
    0x4f7fa0: "watcom::cstart_",        # _cstart_
    0x4f8020: "watcom::strupr",         # _strupr
    0x4f8040: "watcom::localtime_",     # _localtime
    0x4f8094: "watcom::localtime",
    0x4f80b4: "watcom::IF_DEXP",        # IF@DEXP
    0x4f8116: "watcom::exp",
    0x4f8122: "watcom::Strtold",        # __Strtold
    0x4f82b6: "watcom::strtod",
    0x4f8b50: "watcom::setenv",
    0x4f8ce8: "watcom::setenv_",        # __setenv
    0x4f8f38: "watcom::findenv",        # __findenv
    0x4f9482: "watcom::IF_DACOS",       # IF@DACOS
    0x4fb300: "watcom::bsearch",
    0x4fe1d0: "watcom::prtf",           # __prtf
    0x4fe578: "watcom::prtfSpec",       # __prtf's specification
    0x4fe6cc: "watcom::prtfFlags",      # __prtf's flags
    0x4fe734: "watcom::prtfLength",     # __prtf's strnlen
    0x4fe758: "watcom::prtfWideLength", # __prtf's wide strnlen
    0x4fe7c4: "watcom::prtfHex",        # __prtf's %p digits
    0x4fe81e: "watcom::prtfHexEnd",     # the end of __prtf's %p digits, %hf's too
    0x4fe828: "watcom::prtfFixed",      # __prtf's %hf
    0x4fe924: "watcom::prtfFloat",      # [__EFG_printf]
    0x4fe92c: "watcom::prtfZeros",      # __prtf's zero padding
    0x4fe970: "watcom::prtfWideOut",    # __prtf's %ls
    0x4fe9d4: "watcom::prtfConvert",    # __prtf's conversion
    0x4fefdc: "watcom::prtfUpper",      # __prtf's strupr
    0x4ff000: "watcom::file_putc",
    0x4ff014: "watcom::vfprintf",
    0x4ff0ec: "watcom::GetThreadPtr",   # __GetThreadPtr
    0x4ff0f4: "watcom::NullAccess",     # __NullAccessRtn
    0x4ff108: "watcom::NTInit",         # __NTInit
    0x4ff338: "watcom::NTMain",         # the program's start
    0x4ff38c: "watcom::exit_",          # __exit
    0x4ff3d0: "watcom::freeEnvironment",# the environment's copies freed
    0x4ff410: "watcom::callRoutine",    # __FiniRtns' call
    0x4ff41c: "watcom::InitRtns",       # __InitRtns
    0x4ff46c: "watcom::FiniRtns",       # __FiniRtns
    0x4ff4c0: "watcom::STOSB",          # __STOSB
    0x4ff4f7: "watcom::STOSD",          # __STOSD
    0x500f8c: "watcom::set_errno_dos",  # __set_errno_dos
    0x500ff4: "watcom::set_errno_nt",   # __set_errno_nt
    0x502760: "watcom::flush",          # __flush
    0x502880: "watcom::set_errno",      # __set_errno
    0x502890: "watcom::set_EDOM",       # __set_EDOM
    0x5028a4: "watcom::set_ERANGE",     # __set_ERANGE
    0x5028c8: "watcom::set_doserrno",   # __set_doserrno
    0x5028e0: "watcom::lseek",
    0x502960: "watcom::tell",           # _tell
    0x5029c0: "watcom::ioalloc",        # __ioalloc
    0x502a50: "watcom::qread",          # __qread
    0x502af0: "watcom::fgetc",
    0x502bb8: "watcom::filbuf",         # __filbuf
    0x502be8: "watcom::fill_buffer",    # __fill_buffer
    0x508ff4: "watcom::sopen",          # _sopen
    0x50901c: "watcom::sopen_",         # __sopen
    0x509250: "watcom::allocfp",        # __allocfp
    0x509314: "watcom::freefp",         # __freefp
    0x509380: "watcom::chktty",         # __chktty
    0x5093c0: "watcom::getpid",
    0x5094e0: "watcom::tmpdir",         # __tmpdir
    0x5095b0: "watcom::close",          # __close
    0x509640: "watcom::fixed_GetFileAttributesA", # __fixed_GetFileAttributesA
    0x5096c0: "watcom::qwrite",         # __qwrite
    0x5097b0: "watcom::fputc",
    0x50c540: "watcom::setbits",        # __setbits
    0x50c640: "watcom::cgetc",          # __scnf's
    0x50c648: "watcom::uncget",         # __scnf's
    0x50c650: "watcom::scnf",           # __scnf
    0x50c904: "watcom::get_opt",        # __scnf's
    0x50ca08: "watcom::scan_white",     # __scnf's
    0x50ca48: "watcom::scan_char",      # __scnf's
    0x50cb30: "watcom::scan_string",    # __scnf's
    0x50cc88: "watcom::report_scan",    # __scnf's
    0x50cce8: "watcom::makelist",       # __scnf's
    0x50cd2c: "watcom::scan_arb",       # __scnf's
    0x50ce18: "watcom::scan_float",     # __scnf's
    0x50d0f0: "watcom::scan_int",       # __scnf's
    0x50d510: "watcom::radix_value",    # __scnf's
    0x50d538: "watcom::chktrm",         # __scnf's
    0x50ea4e: "watcom::modf",
    0x50ead0: "watcom::getenv",
    0x50eb30: "watcom::getctime",       # __getctime
    0x50eba0: "watcom::mktime",
    0x511cd0: "watcom::fatalWrite",     # __fatal_runtime_error's
    0x511d0c: "watcom::fatal_runtime_error",# __fatal_runtime_error
    0x515300: "watcom::MemAllocator",   # __MemAllocator
    0x5153b0: "watcom::MemFree",        # __MemFree
    0x5154e0: "watcom::LinkUpNewNHeap", # __LinkUpNewNHeap
    0x515558: "watcom::CreateNewNHeap", # __CreateNewNHeap
    0x515614: "watcom::ExpandDGROUP",   # __ExpandDGROUP
    0x515628: "watcom::AdjustAmount",   # __AdjustAmount
    0x515680: "watcom::nmemneed",       # __nmemneed
    0x515690: "watcom::NTRealKey",      # __NTRealKey
    0x5156b8: "watcom::openConsoles",   # the console's handles
    0x515728: "watcom::NTConsoleInput", # __NTConsoleInput
    0x515734: "watcom::NTConsoleOutput", # __NTConsoleOutput
    0x515850: "watcom::flushall",
    0x515858: "watcom::flushall_",      # __flushall
    0x5158a0: "watcom::mainStart",      # the main thread's start
    0x515b34: "watcom::tzWindows",      # __tzset's without TZ
    0x515c2c: "watcom::tzset",          # __tzset
    0x515c90: "watcom::tzNumber",       # __tzset's number
    0x515cbc: "watcom::tzName",         # __tzset's zone
    0x515dd4: "watcom::tzRule",         # __tzset's rule
    0x515ec2: "watcom::tzDaylight",     # __tzset's daylight saving
    0x515fa0: "watcom::brktime",        # __brktime
    0x5160f0: "watcom::leapyear",       # __leapyear
    0x51612c: "watcom::calc_yday",
    0x516224: "watcom::check_order",
    0x516270: "watcom::isindst",        # __isindst
    0x51652c: "watcom::time_less",
    0x51655c: "watcom::math87_err",     # __math87_err
    0x5165d8: "watcom::ZBuf2LD",        # __ZBuf2LD
    0x51663a: "watcom::normalize",      # __ZBuf2LD's
    0x51667b: "watcom::scalePowers",    # __LDScale10x's
    0x5166cf: "watcom::scaleEnd",       # __LDScale10x's way out
    0x5166d8: "watcom::scaleBy",        # __LDScale10x's
    0x516759: "watcom::LDScale10x",     # __LDScale10x
    0x518050: "watcom::mbslen",         # _mbslen
    0x518080: "watcom::mbstowcs",
    0x518290: "watcom::wsetenv",        # __wsetenv
    0x518498: "watcom::wfindenv",       # __wfindenv
    0x5185c0: "watcom::nrealloc",       # _nrealloc
    0x518640: "watcom::memmove",
    0x51a1ba: "watcom::DSQRT",          # __@DSQRT
    0x51e060: "watcom::wctomb_",        # wctomb's core
    0x51e0a0: "watcom::utoa",
    0x51e0ec: "watcom::itoa",
    0x51e130: "watcom::ulltoa",
    0x51e220: "watcom::prtfUtoa",       # __prtf's utoa
    0x51e2b0: "watcom::handlesFull",    # _sopen's: no handle free
    0x51e304: "watcom::allocPOSIXHandle", # __allocPOSIXHandle
    0x51e45c: "watcom::freePOSIXHandle", # __freePOSIXHandle
    0x51e490: "watcom::initPOSIXHandles", # __initPOSIXHandles
    0x51e4f0: "watcom::NTGetFakeHandle", # __NTGetFakeHandle
    0x51e7ec: "watcom::initThreadData", # the main thread's data
    0x51eb00: "watcom::moduleWideName", # the module's wide name
    0x51eba0: "watcom::copyCommandLine", # the command line's copy
    0x51ebf0: "watcom::copyWideCommandLine", # the wide command line's copy
    0x51ec20: "watcom::stackLimits",    # the stack's limits
    0x51f0d0: "watcom::InitExceptionHandling", # __InitExceptionHandling
    0x51f11c: "watcom::DoneExceptionHandler",# __DoneExceptionHandler
    0x522d80: "watcom::fsync",
    0x522de0: "watcom::GetIOMode",      # __GetIOMode
    0x522e38: "watcom::SetIOMode",      # __SetIOMode
    0x522e60: "watcom::getche",
    0x5234d0: "watcom::GetNTAccessAttr", # __GetNTAccessAttr
    0x523504: "watcom::GetNTShareAttr", # __GetNTShareAttr
    0x523560: "watcom::isatty",
    0x523610: "watcom::fullpath",       # _fullpath
    0x523c00: "watcom::mbtowc",
    0x523ca0: "watcom::FDFS",           # __FDFS
    0x523cec: "watcom::U8M",            # __U8M
    0x523d10: "watcom::mbsnicmp",       # _mbsnicmp
    0x524190: "watcom::EnterWVIDEO",    # __EnterWVIDEO
    0x5241c0: "watcom::nheapshrink",    # _nheapshrink
    0x5241fc: "watcom::ReturnMemToSystem", # __ReturnMemToSystem
    0x52425c: "watcom::ReleaseNHeap",   # _nheapshrink's step
    0x524290: "watcom::callMain",       # main's call
    0x5242f0: "watcom::wcstombs",
    0x5243a7: "watcom::reporterror",    # __reporterror
    0x5243b1: "watcom::reportError",    # __reporterror's struct exception
    0x5250c0: "watcom::mbterm",         # _mbterm
    0x525100: "watcom::mbsinc",         # _mbsinc
    0x5252a0: "watcom::create_wide_environment",# __create_wide_environment
    0x525300: "watcom::wcslen",
    0x525320: "watcom::memcpy",
    0x525350: "watcom::wcscpy",
    0x525370: "watcom::towupper",       # the environment's towupper
    0x525380: "watcom::msize",          # _msize
    0x525390: "watcom::nexpand",        # _nexpand
    0x5253cc: "watcom::HeapManager_expand",# __HeapManager_expand
    0x525578: "watcom::nexpandFailed",  # _nexpand's way out
    0x525d59: "watcom::U8D",            # __U8D
    0x525df0: "watcom::calloc",
    0x526030: "watcom::initThread",     # the thread's data's start
    0x52b290: "watcom::putch",
    0x52b5b0: "watcom::mbclen",         # _mbclen
    0x52b5e0: "watcom::mbcicmp",        # _mbcicmp
    0x52b640: "watcom::startHook",      # a start-up hook
    0x52b647: "watcom::printMathError", # the maths' error printed
    0x52b68f: "watcom::callMatherr",    # _matherr's call
    0x52c178: "watcom::wputenv",        # __wputenv
    0x52c300: "watcom::wputenvFind",    # __wputenv's find
    0x52e310: "watcom::mbccpy",         # _mbccpy
    0x52e350: "watcom::mbsupr",         # _mbsupr
    0x52e3a0: "watcom::mbccmp",         # _mbccmp
    0x52e400: "watcom::get_std_stream", # __get_std_stream
    0x52ed30: "watcom::mbsnextc",       # _mbsnextc
    0x52ed70: "watcom::mbctoupper",     # _mbctoupper
    0x52ede0: "watcom::mbvtop",         # _mbvtop
    0x52eec0: "watcom::mbcupper932",    # _mbctoupper's, code page 932
    0x52eee0: "watcom::mbclower",       # _ismbclower's, code page 932
    0x52ef30: "watcom::mbclegal",       # _ismbclegal's
    0x52ef70: "watcom::mbbtrail",       # _ismbbtrail's
    0x533de4: "watcom::RtlUnwindThunk", # RtlUnwind's import
}
GAME_NATIVES.update(WATCOM_NATIVES)

# The menus' text and what the screens do around their items, native: each
# address is the port's function of that name in nfs3hp::frontend --
# src/nfs3hp/native_frontend_text.cpp's the names past the language's lines,
# the checks of a line typed in; native_frontend_screen.cpp's the pointer and
# the screens' art.  tools/game_checks compares them with their generated code.
FRONTEND_NATIVES = {
    0x43d9f0: "frontend::controllerName",
    0x448400: "frontend::displayModeName",
    0x4721f0: "frontend::nameValid",
    0x443630: "frontend::shortNameCharacter",
    0x443660: "frontend::shortNameValid",
    0x443700: "frontend::languageNameValid",
    0x443780: "frontend::languageFileNameValid",
    0x443860: "frontend::lineAllowed",
    0x435230: "frontend::placePointer",
    0x49bfc0: "frontend::setPointer",
    0x440970: "frontend::loadScreenArt",
}
GAME_NATIVES.update(FRONTEND_NATIVES)

# The controls, native: each address is the port's function of that name in
# nfs3hp::controls -- src/nfs3hp/native_controls_input.cpp's the game's side
# of DirectInput (the devices found and opened, the mouse, the keyboard, the
# joysticks read, their force-feedback effects); native_controls_devices.cpp's
# the control sets the game makes for the devices it knows; and
# native_controls_race.cpp's the controls read in a race and kept in
# config.dat; native_controls_pipeline.cpp's the controls read and turned
# into the players' actions every frame of a race; native_controls_queue.cpp's
# the race's queue of inputs, by frame and player, that hands them to the cars
# and to the other machines in a network race, and the edges of the keys and
# actions the race's screens ask about.  tools/game_checks compares
# them with their generated code;
# what one of these files calls of another's it calls by address.
CONTROLS_INPUT_NATIVES = {
    0x434f20: "controls::startDirectInput",
    0x434fd0: "controls::stopDirectInput",
    0x4350a0: "controls::openMouse",
    0x4352d0: "controls::setPointerBounds",
    0x435310: "controls::readMouse",
    0x435600: "controls::findJoysticks",
    0x435690: "controls::describeJoystick",
    0x4358f0: "controls::readJoystick",
    0x435ae0: "controls::readAxisZone",
    0x435b40: "controls::setAxisZone",
    0x435b60: "controls::resetForceFeedback",
    0x435bc0: "controls::setAutocenter",
    0x435c60: "controls::pauseEffects",
    0x435cc0: "controls::continueEffects",
    0x435d20: "controls::actuatorsOn",
    0x435d90: "controls::actuatorsOff",
    0x435df0: "controls::setGain",
    0x435f10: "controls::createEffect",
    0x4361e0: "controls::startEffect",
    0x436230: "controls::stopEffect",
    0x436270: "controls::stopEffects",
    0x4362f0: "controls::downloadEffect",
    0x436310: "controls::unloadEffect",
    0x436330: "controls::updateEffect",
    0x436610: "controls::setCooperativeLevel",
    0x4366e0: "controls::acquireDevice",
    0x436800: "controls::unacquireDevice",
    0x4368c0: "controls::acquireResult",
    0x4368f0: "controls::effectPlaying",
    0x436930: "controls::hideCursor",
    0x436980: "controls::joystickFound",
    0x436b10: "controls::axisProperty",
}
CONTROLS_DEVICES_NATIVES = {
    0x4904d0: "controls::controlsFor",
    0x4904f0: "controls::defaultControls",
    0x4905b0: "controls::setControls",
    0x490610: "controls::controllerNamed",
    0x490640: "controls::knownControllerControls",
    0x4906b0: "controls::sideWinderGamePadControls",
    0x490770: "controls::gravisGripControls",
    0x490830: "controls::wingManForceControls",
    0x4908f0: "controls::combatStickControls",
    0x4909b0: "controls::sideWinderProControls",
    0x490a70: "controls::sideWinderWheelControls",
    0x490b30: "controls::steeringWheelControls",
    0x490c40: "controls::keyboardControls",
    0x490cb0: "controls::gamePadControls",
    0x490e30: "controls::joystickControls",
    0x490fa0: "controls::otherControllerControls",
}
CONTROLS_RACE_NATIVES = {
    0x470bc0: "controls::steadyEngineRumble",
    0x470c60: "controls::startEngineRumble",
    0x470cb0: "controls::silenceEngineRumble",
    0x470d10: "controls::setForceGain",
    0x470d30: "controls::findForceController",
    0x470dc0: "controls::createForceEffects",
    0x470f40: "controls::stopForces",
    0x470f80: "controls::stopForcesAgain",
    0x470fc0: "controls::stopForcesOnceMore",
    0x471000: "controls::stopForcesOrActuatorsOn",
    0x471070: "controls::stopForcesOrActuatorsOff",
    0x4710f0: "controls::forcePlayerCar",
    0x4711e0: "controls::startRaceForces",
    0x471240: "controls::stopRaceEffects",
    0x471270: "controls::crashJolt",
    0x471440: "controls::bumpJolt",
    0x4714f0: "controls::gearChangeKick",
    0x471650: "controls::steeringAmount",
    0x4716e0: "controls::playSteeringSpring",
    0x471860: "controls::playSlideSpring",
    0x4719f0: "controls::playRoadRumble",
    0x471af0: "controls::playEngineRumble",
    0x471dc0: "controls::isForceCar",
    0x471e10: "controls::raceForcesFrame",
    0x471fb0: "controls::allActuatorsOff",
    0x471fd0: "controls::playJolt",
    0x4720a0: "controls::stopEngineRumble",
    0x4720e0: "controls::stopSteeringSpring",
    0x472110: "controls::carSpeedMph",
    0x472b60: "controls::loadControllers",
}
CONTROLS_PIPELINE_NATIVES = {
    0x431580: "controls::clearJoysticks",
    0x4315a0: "controls::readJoysticks",
    0x431690: "controls::pollJoystick",
    0x4316c0: "controls::buttonHeld",
    0x4316f0: "controls::hatPointing",
    0x431740: "controls::axisAmount",
    0x4317d0: "controls::keyHeld",
    0x431800: "controls::controlHeld",
    0x431880: "controls::controlHeldNo",
    0x431885: "controls::controlHeldYes",
    0x43188d: "controls::controlHeldByKind",
    0x4318b0: "controls::noControl",
    0x48f1f0: "controls::bindPlayersControls",
    0x48f4f0: "controls::startRaceControls",
    0x48f550: "controls::readRaceControls",
    0x48f940: "controls::bindControl",
    0x48f970: "controls::bindFunctionKeys",
    0x48fa10: "controls::queueRaceControls",
}
CONTROLS_QUEUE_NATIVES = {
    0x48f6a0: "controls::takeCarControls",
    0x48f6e0: "controls::stepMenuChoice",
    0x48f710: "controls::stepMenuChoiceOfThree",
    0x48f740: "controls::actionPressed",
    0x48f7b0: "controls::functionKeyPressed",
    0x48f820: "controls::pressFunctionKey",
    0x48f860: "controls::functionKeyRepeats",
    0x48f8d0: "controls::keyPressed",
    0x48f920: "controls::firstFunctionKeyHeld",
    0x4c6080: "controls::appendToPacket",
    0x4c60c0: "controls::sendOwnInput",
    0x4c61e0: "controls::receiveInput",
    0x4c6530: "controls::queueInput",
    0x4c66a0: "controls::queueRemoteInput",
    0x4c67a0: "controls::takeQueuedFrame",
    0x4c6840: "controls::takenControls",
    0x4c6870: "controls::repeatMissingInputs",
    0x4c6970: "controls::syncFrame",
    0x4c69a0: "controls::framesTaken",
    0x4c69b0: "controls::framesQueued",
    0x4c69c0: "controls::queueLead",
    0x4c6a10: "controls::noteLag",
    0x4c6a40: "controls::setQueueDelays",
    0x4c6ae0: "controls::queueFull",
}
CONTROLS_GROUPS = (CONTROLS_INPUT_NATIVES, CONTROLS_DEVICES_NATIVES, CONTROLS_RACE_NATIVES,
                   CONTROLS_PIPELINE_NATIVES, CONTROLS_QUEUE_NATIVES)
for group in CONTROLS_GROUPS:
    GAME_NATIVES.update(group)

# EA's platform libraries, native whole, in nfs3hp::ea: native_ea_threads.cpp's
# threads, events, timers and locks; native_ea_memory.cpp's memory manager and
# memmove; native_ea_files.cpp's files, read and written a block at a time;
# native_ea_support.cpp's trace and log, files loaded whole and .viv archives.
# tools/game_checks compares them with their generated code.
EA_THREADS_NATIVES = {
    0x4df6f0: "ea::waitForObjects",
    0x4df730: "ea::threadStart",
    0x4df740: "ea::threadsInit",
    0x4df7a0: "ea::createThread",
    0x4df870: "ea::removeThread",
    0x4df8e0: "ea::sleepMs",
    0x4df8f0: "ea::sleepTicks",
    0x4df930: "ea::isThread",
    0x4df980: "ea::threadPriority",
    0x4dfa30: "ea::setThreadPriority",
    0x4dfaf0: "ea::exitThread",
    0x4dfb00: "ea::createEvent",
    0x4dfb20: "ea::postEvent",
    0x4dfb30: "ea::signalEvent",
    0x4dfb40: "ea::pollEvent",
    0x4dfb70: "ea::waitEvent",
    0x4dfb90: "ea::waitEventTicks",
    0x4dfbf0: "ea::waitForAny",
    0x4dfc00: "ea::closeEvent",
    0x4dfc10: "ea::createManualEvent",
    0x4dfc30: "ea::setEvent",
    0x4dfc40: "ea::resetEvent",
    0x4dfc50: "ea::pulseEvent",
    0x4dfc60: "ea::waitManualEvent",
    0x4dfc80: "ea::setAndCloseEvent",
    0x4dfca0: "ea::lastErrorText",
    0x4dfce0: "ea::setLastError",
    0x4dfcf0: "ea::lastError",
    0x4dfd00: "ea::doNothing",
    0x4dfd80: "ea::xRotation",
    0x4dfde0: "ea::yRotation",
    0x4dfe40: "ea::zRotation",
    0x4dfea0: "ea::axisRotation",
    0x4eb230: "ea::randomFraction",
    0x4eb260: "ea::growLocks",
    0x4eb2e0: "ea::makeLock",
    0x4eb340: "ea::enterLock",
    0x4eb350: "ea::leaveLock",
    0x4eb360: "ea::freeLock",
    0x4f9070: "ea::surfaceLocked",
    0x5221f0: "ea::physicalMemory",
}
EA_MEMORY_NATIVES = {
    0x4e1000: "ea::findFreeBlock",
    0x4e1030: "ea::findLargestFreeBlock",
    0x4e1080: "ea::linkFreeBlock",
    0x4e10e0: "ea::unlinkFreeBlock",
    0x4e1100: "ea::clearMemClasses",
    0x4e1120: "ea::markMemBlock",
    0x4e11e0: "ea::memClassOverhead",
    0x4e11f0: "ea::memBlockOverhead",
    0x4e1230: "ea::creatememclass",
    0x4e14b0: "ea::releasememclass",
    0x4e1500: "ea::getfreememclass",
    0x4e1580: "ea::largestmem",
    0x4e15b0: "ea::largestmemclass",
    0x4e15d0: "ea::memBlockSize",
    0x4e15e0: "ea::memBlockName",
    0x4e1610: "ea::memBlockFlags",
    0x4e1620: "ea::reservememadr",
    0x4e1890: "ea::purgememadr",
    0x4e1990: "ea::purgememclass",
    0x4e19d0: "ea::reservememStdcall",
    0x4e19f0: "ea::purgememStdcall",
    0x4ea4f0: "ea::memmove",
}
EA_FILES_NATIVES = {
    0x4e28c0: "ea::initHandlesDefault",
    0x4e28d0: "ea::openHandle",
    0x4e2ab0: "ea::openHandleErrorsAs",
    0x4e2ae0: "ea::openHandleToWrite",
    0x4e2ba0: "ea::openMapHandle",
    0x4e2da0: "ea::closeHandle",
    0x4e2de0: "ea::readHandle",
    0x4e2e20: "ea::writeHandle",
    0x4e2e60: "ea::seekHandle",
    0x4e2eb0: "ea::isSlash",
    0x4e2ebe: "ea::notSlash",
    0x4e2ed0: "ea::volumeRoot",
    0x4e2fd0: "ea::fullPathName",
    0x4e3110: "ea::logicalDrives",
    0x4e3120: "ea::driveOfPath",
    0x4e3250: "ea::driveLetter",
    0x4e32e0: "ea::diskFreeBytes",
    0x4e3330: "ea::driveNumber",
    0x4e3340: "ea::setDirectory",
    0x4e33a0: "ea::getDirectory",
    0x4e3410: "ea::handlesReady",
    0x4e3430: "ea::initHandles",
    0x4e34c0: "ea::closeAllHandles",
    0x4e3570: "ea::takeHandleSlot",
    0x4e3600: "ea::deleteFile",
    0x4e3620: "ea::openFileHandle",
    0x4e3980: "ea::handleInfo",
    0x4e3ae0: "ea::readHandleBytes",
    0x4e3cc0: "ea::writeHandleBytes",
    0x4e3e60: "ea::seekHandleTo",
    0x4e3f30: "ea::handlePosition",
    0x4e3fe0: "ea::closeHandleSlot",
    0x4e40a0: "ea::lockHandleDrive",
    0x4e40f0: "ea::unlockHandleDrive",
    0x4e4140: "ea::openMemoryHandle",
    0x4e41f0: "ea::handleMemory",
    0x501010: "ea::checkSurfaceUnlocked",
    0x501050: "ea::fileReport",
    0x501110: "ea::virtualAllocPages",
    0x501180: "ea::virtualFreePages",
    0x5011a0: "ea::touchMemoryPages",
    0x501270: "ea::openBlockHandle",
    0x501360: "ea::blockHandleFile",
    0x5013a0: "ea::closeBlockHandle",
    0x5013b0: "ea::readBlockHandle",
    0x501450: "ea::seekBlockHandle",
}
EA_SUPPORT_NATIVES = {
    0x42d510: "ea::gameTrace",
    0x43b830: "ea::openViv",
    0x43b9b0: "ea::closeViv",
    0x43b9d0: "ea::findInViv",
    0x4679f0: "ea::findVivEntry",
    0x4e08f0: "ea::monoPrintf",
    0x4e0980: "ea::monoPrintfAt",
    0x4e0a60: "ea::monoWrite",
    0x4e0c10: "ea::monoScroll",
    0x4e0ca0: "ea::monoClear",
    0x4e0d00: "ea::monoSetTop",
    0x4e0d50: "ea::monoSetAttribute",
    0x4e0d60: "ea::monoSetTab",
    0x4e0d70: "ea::monoPage",
    0x4e0d80: "ea::monoNoCursor",
    0x4e0dee: "ea::seedRandom",
    0x4e0e60: "ea::fileExists",
    0x4e0ec0: "ea::loadFile",
    0x4e0fe0: "ea::loadFileQuietly",
}
EA_NET_NATIVES = {
    0x49ecb0: "ea::dropMachine",
    0x49ecf0: "ea::announceMachineLeft",
    0x49eed0: "ea::endNetworkGame",
    0x49f1c0: "ea::sessionIndex",
    0x49f550: "ea::forgetPendingPlayer",
    0x4a5d10: "ea::sendOverLink",
    0x4a60e0: "ea::sendPacket",
    0x4a6390: "ea::resetNetwork",
    0x4a65e0: "ea::closeNetwork",
    0x4a6650: "ea::machineNumber",
    0x4a6690: "ea::playerMachine",
    0x4f4c50: "ea::netTrace",
    0x4f4f50: "ea::netShutGame",
    0x4f4f90: "ea::netEndGame",
    0x4f4ff0: "ea::netKillPlayer",
    0x4f5040: "ea::netCheckPlayer",
    0x4f5280: "ea::connRestore",
    0x4f5310: "ea::connSend",
    0x4f5330: "ea::connClose",
    0x50dc00: "ea::qnth",
    0x50dc20: "ea::qunlink",
    0x50dde0: "ea::qfree",
    0x50e150: "ea::qremove",
    0x50e1d0: "ea::qlength",
    0x50e220: "ea::qposition",
    0x50e470: "ea::qfindindex",
    0x50ed50: "ea::huffUnpack",
    0x510110: "ea::runUnpack",
    0x510420: "ea::dxtBlock",
    0x510760: "ea::dxtUnpack",
    0x510820: "ea::pairExpand",
    0x510870: "ea::pairUnpack",
    0x5123b0: "ea::layerSend",
    0x512530: "ea::layerCallPacker",
    0x512630: "ea::layerPackFromTop",
    0x5126c0: "ea::layerFree",
    0x512770: "ea::layerSendPacket",
}
EA_FORMATS_NATIVES = {
    0x4ef270: "ea::locateShape",
    0x4f0010: "ea::keyCharacter",
    0x4f0100: "ea::keyFromHook",
    0x4f0120: "ea::inkey",
    0x4f0240: "ea::flushKeys",
    0x4f2560: "ea::unpackQuietly",
    0x4f2620: "ea::unpack",
    0x4f26a0: "ea::unpackSizeQuietly",
    0x4f2710: "ea::unpacksize",
    0x4f2830: "ea::stopLowTimer",
    0x4f29b0: "ea::removeLowTimer",
    0x4f2e20: "ea::loadpackadr",
}
EA_GROUPS = (EA_THREADS_NATIVES, EA_MEMORY_NATIVES, EA_FILES_NATIVES, EA_SUPPORT_NATIVES,
             EA_NET_NATIVES, EA_FORMATS_NATIVES)
for group in EA_GROUPS:
    GAME_NATIVES.update(group)

# The game's data loaded and saved, native whole, in nfs3hp::load:
# native_load_track.cpp's race data files (the track's file cache, paths,
# textures, horizon, sky and weather, the opponents' speeds, the cameras, the
# ghost); native_load_cardb.cpp's cars' database (fedata, car.viv, .FEC,
# carp.txt); native_load_carmodels.cpp's cars' models and specs (.fce, .art,
# carspecs); native_load_saves.cpp's replays, records and config.dat, and
# EA's BIG archives, savefile and shape comments under them.
# tools/game_checks compares them with their generated code.
LOAD_TRACK_NATIVES = {
    0x40e000: "load::loadOpponentSpeeds",
    0x40f230: "load::clearTrackCameras",
    0x40f280: "load::loadTrackCameras",
    0x418d00: "load::swapBigEndian",
    0x418d40: "load::newFileNode",
    0x418d70: "load::freeFileNode",
    0x418dd0: "load::findFileNode",
    0x418e90: "load::insertFileNode",
    0x418f00: "load::newEmptyFileNode",
    0x418f40: "load::freeFileNodes",
    0x418f70: "load::countFileNodes",
    0x418fe0: "load::findCachedFile",
    0x419000: "load::addCachedFile",
    0x419080: "load::newFileCache",
    0x4190e0: "load::freeFileCache",
    0x419120: "load::bigHeaderSize",
    0x4191b0: "load::cachedDataSize",
    0x4191e0: "load::writeBigFile",
    0x419460: "load::packFileCache",
    0x419510: "load::readBigFile",
    0x4195f0: "load::openFileCache",
    0x41d8a0: "load::trackFilePath",
    0x41d8d0: "load::trackFolderPath",
    0x41dbc0: "load::loadTrackTextures",
    0x477a90: "load::saveGhost",
    0x477ac0: "load::writeGhost",
    0x477b00: "load::readGhost",
    0x478340: "load::loadHorizon",
    0x47ac30: "load::loadSky",
    0x4dcd20: "load::loadWeatherShapes",
}
LOAD_CARDB_NATIVES = {
    0x438340: "load::endWithSlash",
    0x438350: "load::folderWritable",
    0x4383f0: "load::firstFreeHexFolder",
    0x4384f0: "load::firstFreePlayerFolder",
    0x4385a0: "load::freePlayerCarFolder",
    0x4386b0: "load::firstSubfolder",
    0x4387e0: "load::nextSubfolder",
    0x438870: "load::firstFile",
    0x438970: "load::nextFile",
    0x4389d0: "load::removeFolder",
    0x438a80: "load::firstCar",
    0x438aa0: "load::nextCar",
    0x438ad0: "load::makeCarId",
    0x438af0: "load::firstCarGlobal",
    0x438b10: "load::nextCarGlobal",
    0x438b90: "load::appendCar",
    0x438bd0: "load::carCount",
    0x438be0: "load::countCars",
    0x438c10: "load::unlinkCar",
    0x438c50: "load::insertCarByName",
    0x438cc0: "load::joinCarLists",
    0x438d20: "load::carListOf",
    0x438d60: "load::takePoliceCars",
    0x438dc0: "load::takeClasslessCars",
    0x438e30: "load::takeOrdinaryCars",
    0x438eb0: "load::sortCars",
    0x439250: "load::carName",
    0x4392b0: "load::carFolder",
    0x439300: "load::carFilePrefix",
    0x439520: "load::carId",
    0x439570: "load::carFlag40",
    0x4395c0: "load::carFlag4",
    0x439610: "load::carHasNumericId",
    0x439620: "load::idIsNumeric",
    0x439660: "load::numericId",
    0x439680: "load::carIsBonus",
    0x4396b0: "load::carIsPolice",
    0x4396e0: "load::carIsFerrariOrMercedes",
    0x439730: "load::carClass",
    0x439990: "load::findCarById",
    0x4399f0: "load::clearCarCache",
    0x439a50: "load::carRecord",
    0x439af0: "load::fecText",
    0x439b20: "load::findCarByName",
    0x439bd0: "load::attachFecTexts",
    0x439c10: "load::bonusCarLocked",
    0x439d20: "load::checkDuplicateCars",
    0x439d90: "load::loadCarList",
    0x43a4f0: "load::carSelectable",
    0x43a580: "load::buildCarClassLists",
    0x43ae60: "load::copyCarAsNew",
    0x43b3f0: "load::loadCarSpecs",
    0x43b4f0: "load::writeCarFec",
    0x43b570: "load::readCarFecBody",
    0x43b630: "load::readCarFec",
    0x43b680: "load::carFecPath",
    0x43b740: "load::loadCarFec",
    0x43ba20: "load::placeCarVivPath",
    0x43ba30: "load::swapBytes",
    0x43ba70: "load::writeBigEndian",
    0x43baa0: "load::appendCarpToViv",
    0x43bbf0: "load::lastCarLoaded",
    0x43bc00: "load::setLastCarId",
    0x43bc40: "load::setLastCarName",
    0x43bc70: "load::setLastCarFlags",
    0x43bc90: "load::markLastCarFlag8",
}
LOAD_CARMODELS_NATIVES = {
    0x49b540: "load::beginCarpReceive",
    0x49b5f0: "load::receiveCarpChunk",
    0x49b800: "load::beginCarpSend",
    0x49c700: "load::readCarColors",
    0x49c880: "load::rgbToHsv",
    0x49ca50: "load::hsvToRgb",
    0x49cc50: "load::tintPixel",
    0x49cd00: "load::scaleTexture",
    0x49cea0: "load::fitTexture",
    0x49d130: "load::loadArtTextures",
    0x49d240: "load::nearestModelColor",
    0x49d320: "load::loadTgaTexture",
    0x49d450: "load::loadCarModel",
    0x4b7a60: "load::loadVivCarModel",
    0x4b7ba0: "load::copyVivCarModel",
    0x4dc3a0: "load::setSpecUsed",
    0x4dc3f0: "load::specUsed",
    0x4dc440: "load::specCarName",
    0x4dc490: "load::findSpec",
    0x4dc500: "load::freeSpecSlot",
    0x4dc5a0: "load::setSpec",
    0x4dc670: "load::defaultSpecs",
    0x4dc710: "load::refreshSpecs",
    0x4dc800: "load::specPath",
    0x4dc8b0: "load::readSpecs",
    0x4dc940: "load::loadPlayerSpecs",
    0x4dca50: "load::savePlayerSpecs",
}
LOAD_SAVES_NATIVES = {
    0x446060: "load::readReplayInfo",
    0x446780: "load::saveGhostAs",
    0x446990: "load::loadKeptReplay",
    0x446a10: "load::saveReplayAs",
    0x4475a0: "load::listReplays",
    0x44f510: "load::loadRecords",
    0x44f5f0: "load::saveRecords",
    0x472230: "load::languageStub",
    0x472250: "load::setLanguage",
    0x472300: "load::setCameraViews",
    0x4723f0: "load::defaultConfig",
    0x472820: "load::saveConfig",
    0x4728e0: "load::fileWriteTime",
    0x472940: "load::fileOlder",
    0x472980: "load::loadConfig",
    0x4bd1e0: "load::saveRaceReplay",
    0x4bd2e0: "load::readRaceReplay",
    0x4e9f00: "load::typeOfBigFile",
    0x4e9fe0: "load::bigHeaderLength",
    0x4ea040: "load::findBigEntry",
    0x4ea290: "load::locateBigEntry",
    0x4eeae0: "load::savefileadr",
    0x4eebb0: "load::savefileadrQuietly",
    0x4f2380: "load::fixedSine",
    0x4f23f0: "load::fixedCosine",
}
LOAD_GROUPS = (LOAD_TRACK_NATIVES, LOAD_CARDB_NATIVES, LOAD_CARMODELS_NATIVES, LOAD_SAVES_NATIVES)
for group in LOAD_GROUPS:
    GAME_NATIVES.update(group)

# What the main menu calls of the game's own, native whole, in nfs3hp::frontend:
# native_frontend_glue.cpp's -- a race started or the game left, the HUD's art,
# the attract mode's movies, the lists' keys -- and native_frontend_draw.cpp's
# -- the menus' 3D and 2D drawing, text and shapes.  tools/game_checks
# compares them with their generated code.
FRONTEND_GLUE_NATIVES = {
    0x43c000: "frontend::bindingIsDigital",
    0x43c040: "frontend::noteDigitalControls",
    0x46c9b0: "frontend::sliderValue",
    0x46cac0: "frontend::sliderSetOnMove",
    0x46cca0: "frontend::sliderSetColour",
    0x4960e0: "frontend::playMovie",
    0x4961e0: "frontend::resetAttractTimer",
    0x496200: "frontend::attractMode",
    0x49a4d0: "frontend::freeNetworkLocks",
    0x49ab60: "frontend::allPlayersReady",
    0x49c080: "frontend::mouseKey",
    0x49c1f0: "frontend::pointerInBox",
    0x4cc970: "frontend::nextKey",
}
FRONTEND_DRAW_NATIVES = {
    0x469520: "frontend::sliderStoredValue",
    0x46afd0: "frontend::twoAxisValue",
    0x4956f0: "frontend::loadQfs",
}
FRONTEND_GROUPS = (FRONTEND_GLUE_NATIVES, FRONTEND_DRAW_NATIVES)
for group in FRONTEND_GROUPS:
    GAME_NATIVES.update(group)

# A car's race state, native whole, in nfs3hp::racestart (native_racestart.cpp):
# put back on the road, its wheels' marks, smoke and sounds, the camera's
# shake, a crash's effects, and the start's grid (TRKnnn\start.dat) with a car
# placed on it.  tools/game_checks compares them with their generated code.
RACESTART_NATIVES = {
    0x428f90: "racestart::respawnCar",
    0x429100: "racestart::resetCar",
    0x429260: "racestart::stuckCheck",
    0x4294c0: "racestart::clearRespawn",
    0x4294e0: "racestart::shakeAmplitude",
    0x4295a0: "racestart::startShake",
    0x4296c0: "racestart::startIdleShake",
    0x429790: "racestart::shakeWave",
    0x4297d0: "racestart::objectContacts",
    0x429880: "racestart::wheelSlip",
    0x4299a0: "racestart::wheelCells",
    0x429b40: "racestart::wheelSurface",
    0x429ba0: "racestart::endSkidMark",
    0x429d20: "racestart::extendSkidMark",
    0x429f10: "racestart::skidSounds",
    0x42a130: "racestart::wheelSpray",
    0x42a280: "racestart::surfaceEffects",
    0x42a480: "racestart::wheelSkid",
    0x42a560: "racestart::wheelEffects",
    0x42a7d0: "racestart::updateShake",
    0x42ac40: "racestart::crashEffects",
    0x42b480: "racestart::respawnEffects",
    0x42b660: "racestart::stopCarEffects",
    0x42b780: "racestart::carEffects",
    0x42b7b0: "racestart::airborneSpin",
    0x42b900: "racestart::updateCar",
    0x42ba00: "racestart::defaultGridPlace",
    0x42bb80: "racestart::loadStartGrid",
    0x42bc70: "racestart::gridPlace",
    0x42bcb0: "racestart::placeOnGrid",
}
GAME_NATIVES.update(RACESTART_NATIVES)

# The race's cars loaded, kept up and in collision, the race's messages and
# its controls' thread (0x42c2a0 to 0x42f550), native whole in nfs3hp::raceload
# (native_raceload.cpp).  tools/game_checks compares them with their generated code.
RACELOAD_NATIVES = {
    0x42c2a0: "raceload::loadCarPerformance",
    0x42c350: "raceload::loadRaceCar",
    0x42c400: "raceload::setCarHandlers",
    0x42c520: "raceload::freeCarShapes",
    0x42c530: "raceload::restartRaceCars",
    0x42c660: "raceload::clearCarSlots",
    0x42c680: "raceload::clearRaceCounts",
    0x42c6d0: "raceload::clearCarList",
    0x42c700: "raceload::loadRaceCars",
    0x42c7f0: "raceload::setTimeStep",
    0x42c830: "raceload::addCarTasks",
    0x42c950: "raceload::openCarViv",
    0x42ca80: "raceload::closeCarViv",
    0x42cab0: "raceload::removeCarTasks",
    0x42cc20: "raceload::countLaps",
    0x42cd00: "raceload::carAxesSpeed",
    0x42cdd0: "raceload::carForwardSpeed",
    0x42ce10: "raceload::carRaceDistance",
    0x42ce70: "raceload::updateRaceCars",
    0x42cff0: "raceload::frameFraction",
    0x42d020: "raceload::placeHeadlights",
    0x42d160: "raceload::placeCarShadow",
    0x42d3e0: "raceload::sortRaceCars",
    0x42d500: "raceload::emptyHook",
    0x42d520: "raceload::emptyHook2",
    0x42d620: "raceload::showMessage",
    0x42d710: "raceload::sendMessage",
    0x42d790: "raceload::openMessages",
    0x42d7f0: "raceload::closeMessages",
    0x42d800: "raceload::clearMessages",
    0x42d810: "raceload::sendTypedMessage",
    0x42d860: "raceload::newMessageLine",
    0x42d8a0: "raceload::freeMessageLine",
    0x42d8b0: "raceload::insertMessageLine",
    0x42d910: "raceload::freeMessageLines",
    0x42d940: "raceload::clearMessageLines",
    0x42d9b0: "raceload::showMessageLines",
    0x42da30: "raceload::appendMessageLine",
    0x42daf0: "raceload::closeMessageLines",
    0x42db70: "raceload::openMessageLines",
    0x42dc20: "raceload::raceControlsThread",
    0x42dc40: "raceload::wakeRaceControls",
    0x42dc60: "raceload::raceControlsTick",
    0x42dcb0: "raceload::startRaceControlsTick",
    0x42dd00: "raceload::stopRaceControlsTick",
    0x42dd60: "raceload::cornerInBox",
    0x42e010: "raceload::cornerInBoxBack",
    0x42e2c0: "raceload::cornerInBox2",
    0x42e4f0: "raceload::cornerInBoxBack2",
    0x42e720: "raceload::clampSpin",
    0x42e820: "raceload::applyImpulse",
    0x42e9c0: "raceload::bounceOffWall",
    0x42ee10: "raceload::hitWall",
    0x42efe0: "raceload::boxHitsWall",
    0x42f280: "raceload::carsCollide",
}
GAME_NATIVES.update(RACELOAD_NATIVES)

# The track's data and a place on it, native whole, in nfs3hp::trackdata
# (native_trackdata.cpp): the nodes, the .ccm and simhts.dat loaded with them,
# the road's quads found under a place, its frame, light and surface, the
# tunnels.  tools/game_checks compares them with their generated code.
TRACKDATA_NATIVES = {
    0x41e180: "trackdata::loadCcmPoints",
    0x41e2c0: "trackdata::freeCcmPoints",
    0x41e2f0: "trackdata::loadSimHeights",
    0x41e340: "trackdata::freeSimHeights",
    0x41e370: "trackdata::openTrackNodes",
    0x41e3b0: "trackdata::closeTrackNodes",
    0x41e3c0: "trackdata::nearestNodeCoarse",
    0x41e420: "trackdata::followNode",
    0x41e470: "trackdata::nearestNode",
    0x41e6a0: "trackdata::quadVertices",
    0x41e720: "trackdata::quadStepForward",
    0x41e770: "trackdata::quadStepBack",
    0x41e7c0: "trackdata::quadToFixed",
    0x41e8d0: "trackdata::quadSet",
    0x41e8f0: "trackdata::nodeSection",
    0x41e940: "trackdata::resetPosition",
    0x41e990: "trackdata::findQuad",
    0x41f090: "trackdata::findQuadNearby",
    0x41f330: "trackdata::findQuadByLinks",
    0x41f500: "trackdata::findQuadFallback",
    0x41f650: "trackdata::locateOnRoad",
    0x41f6e0: "trackdata::updatePosition",
    0x41f870: "trackdata::findMatchingQuad",
    0x41fd60: "trackdata::quadEdgeFlags",
    0x41ff10: "trackdata::sectionItemCount",
    0x41ff20: "trackdata::sectionItem",
    0x41ffa0: "trackdata::setLaneOffsets",
    0x41ffd0: "trackdata::lanePosition",
    0x420230: "trackdata::positionFrame",
    0x4205a0: "trackdata::snapToTrack",
    0x420650: "trackdata::placeOnTrack",
    0x4206f0: "trackdata::positionColour",
    0x4207e0: "trackdata::positionSurface",
    0x420840: "trackdata::surfaceUnder",
    0x420bf0: "trackdata::inTunnel",
    0x420d60: "trackdata::carFacingWay",
    0x4211c0: "trackdata::carSectionKind",
}
GAME_NATIVES.update(TRACKDATA_NATIVES)

# A car's physics step and its direct leaves, native whole, in nfs3hp::physics
# (native_physics.cpp): the step each frame, the full step, the speed in the
# car's axes, the steering's angle, the spin's torque, the dampings.  The
# engine's, the inputs' and the wheels' functions under it are other files'.
# tools/game_checks compares them with their generated code.
PHYSICS_NATIVES = {
    0x4ad250: "physics::clearWheelForces",
    0x4ad2e1: "physics::steerAngle",
    0x4ad672: "physics::sideSlip",
    0x4ad6eb: "physics::groundSpeed",
    0x4ad7a3: "physics::physicsStep",
    0x4ae4c1: "physics::carPhysics",
    0x4ae5c2: "physics::settleLoadShift",
    0x4ae68e: "physics::driveForce",
    0x4ae79b: "physics::holdStill",
    0x4ae85b: "physics::sideGrip",
    0x4ae98b: "physics::handbrakeDrag",
    0x4aea88: "physics::yawDamping",
    0x4aeffc: "physics::idleDamping",
    0x4af140: "physics::coastDamping",
    0x4af254: "physics::spinTorque",
}
GAME_NATIVES.update(PHYSICS_NATIVES)

# A car's handling, native whole, in nfs3hp::handling (native_handling.cpp):
# the physics' entries from outside -- the handling made from the player's
# setup, the automatic gear, the revs matched to the speed, the drive reset,
# a car at rest settled -- and an axle's tyres' forces with what they call.
# tools/game_checks compares them with their generated code.
HANDLING_NATIVES = {
    0x4a78d6: "handling::applyTuning",
    0x4a8bb9: "handling::automaticGear",
    0x4aa4f4: "handling::matchRevsToSpeed",
    0x4aa79f: "handling::resetDriveState",
    0x4aa8e7: "handling::settleAtRest",
    0x4abd9c: "handling::setupGripLoss",
    0x4ac027: "handling::setupSlipLimit",
    0x4ac094: "handling::tyreGrip",
    0x4ac3af: "handling::tyreSlide",
    0x4ac994: "handling::logisticCurve",
    0x4ac9dd: "handling::steeringGrip",
    0x4aca4f: "handling::axleForces",
}
GAME_NATIVES.update(HANDLING_NATIVES)

# A car's engine, gears and driver's inputs, native whole, in
# nfs3hp::drivetrain (native_drivetrain.cpp): the pedals and steering eased
# (or the computer's), the gears changed by hand or by the automatic box,
# the engine's revs and its force at the wheels, the wheels' grip.
# tools/game_checks compares them with their generated code.
DRIVETRAIN_NATIVES = {
    0x4a83d3: "drivetrain::computerDriver",
    0x4a8a99: "drivetrain::engineTorque",
    0x4a90a0: "drivetrain::automaticGears",
    0x4a9769: "drivetrain::gearLimit",
    0x4a97cb: "drivetrain::autoReverse",
    0x4a9bc4: "drivetrain::driverInputs",
    0x4aab41: "drivetrain::wheelGrip",
    0x4aaed1: "drivetrain::engineForce",
}
GAME_NATIVES.update(DRIVETRAIN_NATIVES)

# The race's cameras, native whole, in nfs3hp::carstate (native_carstate.cpp):
# their modes, how each is placed and kept to the track, blended and turned
# into the view.  tools/game_checks compares them with their generated code.
CARSTATE_NATIVES = {
    0x421360: "carstate::idleHook",
    0x421380: "carstate::viewFromCamera",
    0x4213b0: "carstate::viewFromScreenCamera",
    0x421450: "carstate::chooseView",
    0x421480: "carstate::idleHookToo",
    0x421490: "carstate::pickChaseNode",
    0x421570: "carstate::turnAround",
    0x4215c0: "carstate::attachCameras",
    0x421730: "carstate::headTurn",
    0x4217e0: "carstate::cameraNode",
    0x421880: "carstate::cameraTrackPlace",
    0x421900: "carstate::keepOutOfCars",
    0x421b90: "carstate::keepAboveRoad",
    0x421cc0: "carstate::keepOnTrack",
    0x421e80: "carstate::keepFromRightWall",
    0x421f90: "carstate::keepFromLeftWall",
    0x4220a0: "carstate::keepAboveGround",
    0x422290: "carstate::lookAtTarget",
    0x4223c0: "carstate::blendCamera",
    0x422520: "carstate::placeCamera",
    0x422610: "carstate::switchCamera",
    0x4227e0: "carstate::cameraZoom",
    0x422850: "carstate::viewMatrix",
    0x4228e0: "carstate::updateCamera",
    0x4229c0: "carstate::cameraView",
    0x422aa0: "carstate::cockpitShown",
    0x422ba0: "carstate::cameraCar",
    0x422bd0: "carstate::cameraTarget",
    0x422cf0: "carstate::initCameras",
    0x4238b0: "carstate::setCameraMode",
    0x423960: "carstate::setOverrideMode",
    0x423a20: "carstate::nextCycleMode",
    0x423a60: "carstate::buildCameraCycle",
    0x423ab0: "carstate::modeInCycle",
    0x423ae0: "carstate::rememberMode",
    0x423b00: "carstate::cameraMode",
    0x423b50: "carstate::setModeAnew",
    0x423b80: "carstate::settleCamera",
    0x423d90: "carstate::nearestSpot",
    0x423fb0: "carstate::scaledIfSet",
    0x423fe0: "carstate::bitsBroken",
    0x424030: "carstate::dampVector",
    0x4240c0: "carstate::chaseOffset",
    0x424260: "carstate::chaseOffsetLevel",
}
GAME_NATIVES.update(CARSTATE_NATIVES)

# The track, native whole, in nfs3hp::track (native_track.cpp): its .frd
# geometry read into its arena and tinted, its blocks and objects drawn with
# the headlights on them, its ambient sounds, its .col file and textures,
# the views' bookkeeping.  tools/game_checks compares them with their
# generated code (track.inc).
TRACK_NATIVES = {
    0x418cc0: "track::shellOpen",
    0x419660: "track::tintColours",
    0x419720: "track::tintObjectList",
    0x419750: "track::tintTrack",
    0x4197c0: "track::greyColours",
    0x419840: "track::greyObjectList",
    0x419870: "track::greyTrack",
    0x4198e0: "track::readObjectList",
    0x419ab0: "track::readLights",
    0x419b60: "track::newArena",
    0x419b90: "track::freeArena",
    0x419bc0: "track::arenaTake",
    0x419bf0: "track::frdRead",
    0x41a100: "track::loadGeometry",
    0x41a120: "track::projectedTriangle",
    0x41a3e0: "track::shadeVertex",
    0x41a800: "track::setShadeDistances",
    0x41a8d0: "track::viewObjectVertices",
    0x41a9b0: "track::viewTurnedObjectVertices",
    0x41aa60: "track::modelByDistance",
    0x41aaf0: "track::viewOnBlock",
    0x41ab40: "track::lightObjectPolygons",
    0x41b8c0: "track::bracketList",
    0x41bb50: "track::ambientSounds",
    0x41bc20: "track::ambientWanted",
    0x41bc50: "track::pointBehindView",
    0x41bc90: "track::litBoxOutside",
    0x41c1a0: "track::flushBuckets",
    0x41c310: "track::headlightsOnObjects",
    0x41c670: "track::headlightsOnBlock",
    0x41c9f0: "track::blockLights",
    0x41ca40: "track::drawObjectList",
    0x41cdc0: "track::drawTrack",
    0x41d210: "track::ambientSource",
    0x41d310: "track::selectView",
    0x41d350: "track::objectAnimation",
    0x41d510: "track::turnFromAngles",
    0x41d5a0: "track::turnFromPitchYaw",
    0x41d610: "track::setViewDistance",
    0x41d870: "track::nextViewDistance",
    0x41d900: "track::modelsUnlimited",
    0x41da40: "track::freeAt",
    0x41da60: "track::unloadTrack",
    0x41daa0: "track::newPool",
    0x41dae0: "track::newObjectPool",
    0x41db00: "track::freeTrackKinds",
    0x41db10: "track::loadTextures",
    0x41dc60: "track::takeTextures",
    0x41dcc0: "track::loadCollisions",
    0x41de10: "track::freeCollisions",
    0x41de40: "track::clearObjectStates",
    0x41de80: "track::resetTrackViews",
    0x41dea0: "track::resetViewsOuter",
    0x41deb0: "track::resetViews",
    0x41e070: "track::blockInView",
}
GAME_NATIVES.update(TRACK_NATIVES)

# The game's collisions -- car with car, with the track's walls, with its
# objects -- the COLL file, and the drawing code after them (render states,
# the software blend, the polygon lists' drawers, the race's pointer, the
# race's messages), native whole in nfs3hp::collide (native_collide.cpp).
# tools/game_checks compares them with their generated code (collide.inc).
COLLIDE_NATIVES = {
    0x42f550: "collide::findContact",
    0x42f8c0: "collide::contactNormal",
    0x42fe10: "collide::pushCarsApart",
    0x42fee0: "collide::separateCars",
    0x430210: "collide::polygonEdgeNormal",
    0x430350: "collide::clearOfWall",
    0x4304b0: "collide::carAgainstWalls",
    0x430c80: "collide::collideCars",
    0x430cd0: "collide::clearCarList",
    0x430ce0: "collide::objectInReach",
    0x430da0: "collide::carAgainstObjects",
    0x4310c0: "collide::addCarToList",
    0x4311d0: "collide::carCollisions",
    0x431250: "collide::distanceSquared",
    0x431280: "collide::nearestPoint",
    0x431320: "collide::openCollFile",
    0x4313b0: "collide::closeCollFile",
    0x4313d0: "collide::collFileSize",
    0x4313e0: "collide::readCollFile",
    0x431410: "collide::collFileCount",
    0x431420: "collide::collChunk",
    0x431430: "collide::playerControls",
    0x4318c0: "collide::forgetRenderStates",
    0x431aa0: "collide::statesForText",
    0x431af0: "collide::statesForPictures",
    0x431b40: "collide::statesForHud",
    0x431bc0: "collide::statesForScreens",
    0x431c20: "collide::passLayerNothing",
    0x431c30: "collide::passSecondLayer",
    0x431c40: "collide::passFirstLayer",
    0x431c50: "collide::makeBlendTables",
    0x431d50: "collide::blendSpan",
    0x432690: "collide::subdivisionDepth",
    0x433530: "collide::drawTexturedTriangles",
    0x433660: "collide::drawFlatTriangles",
    0x4336e0: "collide::drawTrianglesByZ",
    0x433840: "collide::drawQuadsByZ",
    0x4339d0: "collide::drawFlatTrianglesAgain",
    0x4347b0: "collide::drawColouredTriangles",
    0x434850: "collide::stateRecord",
    0x434950: "collide::drawRecords",
    0x434990: "collide::raceTextFrame",
    0x4349a0: "collide::setMirrored",
    0x4349d0: "collide::drawMirroredRecords",
    0x434a70: "collide::countFrame",
    0x434a80: "collide::raceViewDistance",
    0x434a90: "collide::raceFrameEnd",
    0x434ae0: "collide::raceFrameNothing",
    0x434af0: "collide::raceEndNothing",
    0x434b00: "collide::raceStartDrawing",
    0x434b10: "collide::raceStartNothing",
    0x434b60: "collide::drawPointer",
    0x436c40: "collide::openMessages",
    0x436cf0: "collide::closeMessages",
    0x436d40: "collide::clearAllMessages",
    0x436d60: "collide::clearMessages",
    0x436dc0: "collide::clearMessageLine",
    0x436e10: "collide::addMessageText",
    0x436e90: "collide::addMessageCharacter",
    0x437080: "collide::copyMessageLines",
    0x4371a0: "collide::messagesScrolled",
    0x4371c0: "collide::messagesNothing",
    0x4371d0: "collide::seedHash",
}
GAME_NATIVES.update(COLLIDE_NATIVES)

# The cops' Hot Pursuit, the extra cars and the computer's car's driving
# and physics (0x404d30 to 0x4088f0, but 0x4064f0 and 0x4070c0, which are
# native_frontend_sima.cpp's), native whole in nfs3hp::aidrive
# (native_aidrive.cpp).  tools/game_checks compares them with their
# generated code (aidrive.inc).
AIDRIVE_NATIVES = {
    0x404d30: "aidrive::copStartPlace",
    0x404e40: "aidrive::callRoadblock",
    0x404f80: "aidrive::sirenSwitch",
    0x404ff0: "aidrive::watchRacers",
    0x405220: "aidrive::jumpBehindLeader",
    0x4052e0: "aidrive::clockRacers",
    0x4053b0: "aidrive::startPursuit",
    0x405430: "aidrive::bustCheck",
    0x405630: "aidrive::pursuitStep",
    0x4056e0: "aidrive::dropPursuit",
    0x405740: "aidrive::copDriver",
    0x405a20: "aidrive::nearCar",
    0x405af0: "aidrive::nearHuman",
    0x405c80: "aidrive::humanNearSlice",
    0x405cd0: "aidrive::trafficUnwatched",
    0x405d40: "aidrive::trafficStart",
    0x405dc0: "aidrive::trafficStop",
    0x405e20: "aidrive::extraCarStep",
    0x405e60: "aidrive::setDrivingConstants",
    0x405fb0: "aidrive::drivingNothing",
    0x405fd0: "aidrive::setReverse",
    0x4060d0: "aidrive::reverseCheck",
    0x406120: "aidrive::reverseWhenSlow",
    0x406160: "aidrive::reverseWhenTipped",
    0x4061b0: "aidrive::engineRevs",
    0x4063e0: "aidrive::brakeForce",
    0x4066c0: "aidrive::speedAlongRoad",
    0x406880: "aidrive::laneError",
    0x4068a0: "aidrive::glideAlongRoad",
    0x406980: "aidrive::lookAheadSlice",
    0x406a80: "aidrive::sidewaysToRoad",
    0x406b00: "aidrive::towardTarget",
    0x406b60: "aidrive::towardTargetAhead",
    0x406c00: "aidrive::towardSlice",
    0x406c90: "aidrive::towardSliceSlowing",
    0x406d40: "aidrive::towardLane",
    0x406e50: "aidrive::wayToGo",
    0x4071a0: "aidrive::integrate",
    0x407380: "aidrive::coastAfterFinish",
    0x4076b0: "aidrive::steerOnRoad",
    0x407c30: "aidrive::steerReversing",
    0x407fa0: "aidrive::steerOffRoad",
    0x408520: "aidrive::rideWithTarget",
    0x408600: "aidrive::aiPhysics",
    0x408870: "aidrive::easeLane",
    0x4088f0: "aidrive::aiStep",
}
GAME_NATIVES.update(AIDRIVE_NATIVES)

# The police of a Hot Pursuit race (0x401220..0x404d12: the cops' driver, the
# pursuit, the pull over and the ticket, the roadblocks, the radio) and a few
# racers' rules, native whole in nfs3hp::airace (native_ai_race.cpp).
# tools/game_checks compares them with their generated code (ai_race.inc).
AI_NATIVES = {
    0x401220: "airace::nothingAtStart",
    0x401230: "airace::nothingAtLoad",
    0x401240: "airace::releaseEdge",
    0x4012c0: "airace::setUpCops",
    0x401320: "airace::radioOn",
    0x401330: "airace::radioLevelDown",
    0x401350: "airace::radioLevelUp",
    0x401370: "airace::copTopSpeed",
    0x4013b0: "airace::nearestCopGap",
    0x401430: "airace::setWantedSpeed",
    0x4014b0: "airace::noteSpeeding",
    0x401500: "airace::pulledOver",
    0x401520: "airace::copRadioCall",
    0x401580: "airace::ticketRadio",
    0x4015b0: "airace::stopIfCopNear",
    0x401650: "airace::stopForCop",
    0x401780: "airace::sizeUpTarget",
    0x4018b0: "airace::joinPursuit",
    0x401a20: "airace::leaveCopList",
    0x401a80: "airace::giveUpPursuit",
    0x401b50: "airace::pullOverSpot",
    0x401c20: "airace::planPullOver",
    0x401e20: "airace::endPullOver",
    0x401e40: "airace::copStopsBy",
    0x401fb0: "airace::stopOver",
    0x401ff0: "airace::giveTicket",
    0x402110: "airace::targetStopping",
    0x402420: "airace::chaseSpeed",
    0x4025c0: "airace::pursueTarget",
    0x402750: "airace::votePursuitLane",
    0x4028e0: "airace::sideTowardNode",
    0x402910: "airace::keepSideOfTarget",
    0x4029c0: "airace::placeCopAtStart",
    0x402a00: "airace::placeCop",
    0x402c70: "airace::alwaysOne",
    0x402c80: "airace::carReversing",
    0x402cb0: "airace::copHitByCar",
    0x402d90: "airace::voteRoadLane",
    0x402e10: "airace::spotSpeeder",
    0x402f70: "airace::laneOffRoad",
    0x402fd0: "airace::atOrderNode",
    0x403000: "airace::sideOfOrder",
    0x403030: "airace::takeOrder",
    0x403180: "airace::parkCop",
    0x403270: "airace::drivePatrol",
    0x4032e0: "airace::copDriver",
    0x403830: "airace::giveOrder",
    0x403870: "airace::spreadCops",
    0x403910: "airace::radioStopped",
    0x403950: "airace::radioStop",
    0x403970: "airace::initPursuits",
    0x4039c0: "airace::callRoadblock",
    0x403b20: "airace::callTwoCars",
    0x403d30: "airace::roadblockSiren",
    0x403df0: "airace::watchRoadblock",
    0x403ed0: "airace::watchSecondCar",
    0x403f80: "airace::copsWanted",
    0x404070: "airace::radioJoined",
    0x4040b0: "airace::radioGiveUp",
    0x4040e0: "airace::releaseCops",
    0x4042b0: "airace::dropCop",
    0x404340: "airace::sendCopAhead",
    0x4043a0: "airace::callBackup",
    0x404560: "airace::orderTaken",
    0x404580: "airace::nothingForCop",
    0x404610: "airace::classRule",
    0x404650: "airace::nothingAtEnd",
    0x404660: "airace::nothingAtEndToo",
    0x404690: "airace::setRammingRules",
    0x4047f0: "airace::classValue",
    0x404810: "airace::classValueToo",
    0x404840: "airace::rivalTick",
    0x404a00: "airace::carContact",
    0x404b90: "airace::slowForClass",
    0x404c40: "airace::nothingAtStartToo",
    0x404c50: "airace::catchUpSpeed",
    0x404c80: "airace::findLeader",
}
GAME_NATIVES.update(AI_NATIVES)

# Hot Pursuit's police -- the cops' and the traffic's target speeds, the
# cop's state step, the radio's phrases and messages, the racers' speech
# banks -- native whole in nfs3hp::pursuit (native_pursuit.cpp).
# tools/game_checks compares them with their generated code (pursuit.inc).
PURSUIT_NATIVES = {
    0x408ad0: "pursuit::raceStartHook",
    0x408ae0: "pursuit::raceEndHook",
    0x408b30: "pursuit::roadsideTarget",
    0x408bd0: "pursuit::countLaneTime",
    0x408c50: "pursuit::pulledOverTarget",
    0x408dc0: "pursuit::classSpeedFactor",
    0x408f00: "pursuit::unitFactor",
    0x408f10: "pursuit::radarFactor",
    0x409090: "pursuit::unitFactorToo",
    0x4090a0: "pursuit::leaderFactor",
    0x409290: "pursuit::nodeTargetSpeed",
    0x4092f0: "pursuit::markChased",
    0x409330: "pursuit::targetSpeed",
    0x4094b0: "pursuit::targetSpeedOnly",
    0x4094d0: "pursuit::matchChasedSpeed",
    0x4095d0: "pursuit::scoreCloseCops",
    0x409660: "pursuit::growLaneTimes",
    0x4096b0: "pursuit::scoreLane",
    0x4097c0: "pursuit::scoreChases",
    0x409840: "pursuit::radioChaseCount",
    0x409920: "pursuit::scoreSpeeds",
    0x409a80: "pursuit::alwaysOne",
    0x409a90: "pursuit::takeOverCars",
    0x409b40: "pursuit::startReset",
    0x409b70: "pursuit::checkProgress",
    0x409c00: "pursuit::copStep",
    0x40a010: "pursuit::loadRacerBanks",
    0x40a330: "pursuit::countRadioFrames",
    0x40a380: "pursuit::radioNoise",
    0x40a410: "pursuit::sayPhrase",
    0x40a580: "pursuit::sayCarPhrase",
    0x40a6d0: "pursuit::namePhrase",
    0x40a6f0: "pursuit::namePhrases",
    0x40a910: "pursuit::radioFree",
    0x40a940: "pursuit::radioNothing",
    0x40a950: "pursuit::quietVoice",
    0x40a970: "pursuit::sayVoicePhrase",
    0x40aa20: "pursuit::sayCallSign",
    0x40aa70: "pursuit::sayPhraseOnly",
    0x40aa90: "pursuit::sayUnit",
    0x40ab20: "pursuit::sayDescription",
    0x40aea0: "pursuit::radioNothingToo",
    0x40aeb0: "pursuit::sayLocation",
    0x40b190: "pursuit::saySpeed",
    0x40b220: "pursuit::radioSpotted",
    0x40b330: "pursuit::radioPursuit",
    0x40b480: "pursuit::radioLost",
    0x40b530: "pursuit::radioBackup",
    0x40b670: "pursuit::radioPatrol",
    0x40b720: "pursuit::radioRoadblock",
    0x40b7f0: "pursuit::radioNothingThree",
    0x40b860: "pursuit::radioTicket",
    0x40b910: "pursuit::radioArrest",
    0x40b9c0: "pursuit::radioStopped",
    0x40bac0: "pursuit::radioAmbulance",
    0x40bba0: "pursuit::radioSpikes",
    0x40bc30: "pursuit::radioHelp",
    0x40bca0: "pursuit::radioResume",
    0x40bd00: "pursuit::radioAbort",
    0x40bd80: "pursuit::resetOnTrack",
    0x40bf50: "pursuit::copRestart",
    0x40bfe0: "pursuit::topSpeedFactor",
    0x40bff0: "pursuit::followChased",
    0x40c140: "pursuit::raceEndHookToo",
    0x40c150: "pursuit::raceStartHookToo",
    0x40c160: "pursuit::followCar",
    0x40c210: "pursuit::countdownDone",
    0x40c270: "pursuit::trafficTarget",
    0x40c2f0: "pursuit::trafficStep",
}
GAME_NATIVES.update(PURSUIT_NATIVES)

# The traffic cars' driver and lane planner, the track nodes' lane helpers,
# the tracks' tuning records, the track's animation paths and the sound's
# start-up, native whole in nfs3hp::aiplan (native_aiplan.cpp).
# tools/game_checks compares them with their generated code (aiplan.inc).
AIPLAN_NATIVES = {
    0x40c4f0: "aiplan::leastTrafficPlayer",
    0x40c590: "aiplan::placeTrafficCar",
    0x40c630: "aiplan::reactToTraffic",
    0x40c780: "aiplan::slowForPolice",
    0x40c7f0: "aiplan::countStopped",
    0x40c830: "aiplan::nearPoliceCar",
    0x40c8a0: "aiplan::weighLaneVotes",
    0x40c8d0: "aiplan::voteRoadside",
    0x40c950: "aiplan::atRoadside",
    0x40c9f0: "aiplan::advanceCarAi",
    0x40cb20: "aiplan::clearSensedCars",
    0x40cb50: "aiplan::cacheTrafficSide",
    0x40cb70: "aiplan::resetTrafficCar",
    0x40cb90: "aiplan::clearTrafficRecord",
    0x40cd80: "aiplan::placeOnTrack",
    0x40ce70: "aiplan::brakeToStop",
    0x40cee0: "aiplan::voteCurrentLane",
    0x40cfc0: "aiplan::senseTraffic",
    0x40d1f0: "aiplan::planLaneChange",
    0x40d2e0: "aiplan::edgeLaneCost",
    0x40d3d0: "aiplan::scoreLanesByRoad",
    0x40d720: "aiplan::scoreLanesByObstacles",
    0x40d800: "aiplan::scoreLanesByCars",
    0x40d9f0: "aiplan::edgePenalty",
    0x40da30: "aiplan::sumLaneVotes",
    0x40daf0: "aiplan::steerToLane",
    0x40dd60: "aiplan::beginLanePlanning",
    0x40ddb0: "aiplan::chooseLane",
    0x40de50: "aiplan::startHorn",
    0x40deb0: "aiplan::stopHorn",
    0x40df10: "aiplan::stepTrafficLoading",
    0x40e080: "aiplan::targetSpeedAt",
    0x40e0c0: "aiplan::roadKindAt",
    0x40e0f0: "aiplan::cacheRoadWidth",
    0x40e120: "aiplan::nodeGap",
    0x40e160: "aiplan::nodeOffset",
    0x40e1b0: "aiplan::nodesAheadBySpeed",
    0x40e200: "aiplan::nodeLaneBits",
    0x40e220: "aiplan::nodeHasLane",
    0x40e270: "aiplan::laneBit",
    0x40e2b0: "aiplan::roadSliceIndex",
    0x40e350: "aiplan::carSlices",
    0x40e3c0: "aiplan::fillTrackTuning",
    0x40e410: "aiplan::setupTrackTuning",
    0x40ec40: "aiplan::freeSpeedTable",
    0x40ec70: "aiplan::trafficSide",
    0x40eca0: "aiplan::roadIsWide",
    0x40ed40: "aiplan::trafficSpeedAt",
    0x40ee80: "aiplan::tuningSpeed",
    0x40eed0: "aiplan::tuningDistance",
    0x40ef10: "aiplan::tuningFirst",
    0x40ef70: "aiplan::nodeDirectionDot",
    0x40f000: "aiplan::nodeWithinRange",
    0x40f050: "aiplan::carWithinReach",
    0x40f100: "aiplan::alwaysOne",
    0x40f110: "aiplan::carBoxedIn",
    0x40f330: "aiplan::freeAnimationPaths",
    0x40f370: "aiplan::findFreeAnimation",
    0x40f3b0: "aiplan::startAnimation",
    0x40f430: "aiplan::setAnimationMode",
    0x40f460: "aiplan::stopAnimation",
    0x40f4b0: "aiplan::animationPose",
    0x40f6d0: "aiplan::animationPlace",
    0x40f800: "aiplan::animationProgress",
    0x40f880: "aiplan::animationLastPose",
    0x40f8d0: "aiplan::pathEndPose",
    0x40f940: "aiplan::animationStep",
    0x40fac0: "aiplan::animationPlaying",
    0x40fb10: "aiplan::startSound",
    0x40fcf0: "aiplan::restartSound",
}
GAME_NATIVES.update(AIPLAN_NATIVES)

# The rest of the drawing -- the clipping of a triangle whose corners carry z,
# the polygon lists, the sky, the cars' lights, the effects, the views, the
# HUD, the race's textures and the loading screen -- native whole in
# nfs3hp::render (native_render.cpp), no generated code behind it.  The port's
# changes inside them (mirror, View Distance, alpha intensity, HUD scale and
# editor, cabin fit, widescreen, car detail, loading screen) are the natives'
# own; each tools/apply_*.py leaves these functions out (NATIVE_SITES).
# tools/game_checks/render holds them against their code made afresh, the
# changes made to it as tools/game_checks/generate.py's PORT_CHANGES make them.
RENDER_NATIVES = {
    0x41b9b0: "render::viewPass",
    0x41d620: "render::viewDistances",
    0x41d960: "render::raceViews",
    0x41df10: "render::viewRecord",
    0x432720: "render::subdivideQuad",
    0x433060: "render::quadSubdivided",
    0x4332b0: "render::quadRecords",
    0x433e30: "render::colouredTriangles",
    0x434120: "render::triangleRecords",
    0x434870: "render::polygonDrawers",
    0x472d10: "render::detailSettings",
    0x475b50: "render::solidText",
    0x47a190: "render::sky",
    0x47a1f0: "render::flatSky",
    0x47a880: "render::viewSky",
    0x47b7d0: "render::glareVertices",
    0x47b850: "render::skyDome",
    0x47f650: "render::hudFrame",
    0x4800a0: "render::hudBand",
    0x480910: "render::hudRect",
    0x480fc0: "render::hudPicture",
    0x4812e0: "render::hudDial",
    0x481aa0: "render::hudPanel",
    0x481c50: "render::hudTable",
    0x482620: "render::countdown",
    0x482b00: "render::hud",
    0x487210: "render::hudInset",
    0x4880b0: "render::needles",
    0x488470: "render::barGauges",
    0x489240: "render::cockpit",
    0x48c0e0: "render::segmentMeter",
    0x48c7c0: "render::startLights",
    0x491190: "render::glow",
    0x491bc0: "render::roadLight",
    0x4946e0: "render::lightning",
    0x494cb0: "render::loadingScreen",
    0x49d800: "render::glareColours",
    0x4a4410: "render::bouncers",
    0x4b56e0: "render::transformBuffer",
    0x4b59c0: "render::startDriver",
    0x4b7d30: "render::carTextures",
    0x4b97b0: "render::nightColour",
    0x4b9bc0: "render::carLights",
    0x4bae00: "render::detailedCarLights",
    0x4bb200: "render::mediumCarLights",
    0x4bef50: "render::screenMode",
    0x4bf790: "render::clipByZ",
    0x4c0b20: "render::line",
    0x4c20e0: "render::quadByZ",
    0x4c3ad0: "render::nearTriangle",
    0x4cb670: "render::clearBuffers",
    0x4d0a10: "render::pictureTexture",
    0x4d3f50: "render::squareTexture",
    0x4d41a0: "render::paletteTexture",
    0x4d4580: "render::pieceTexture",
    0x4d4ae0: "render::cabinTextures",
    0x4ddb40: "render::sparks",
    0x4de070: "render::smoke",
    0x4de700: "render::streaks",
}
GAME_NATIVES.update(RENDER_NATIVES)
# The drawing's vertex and polygon loops -- track and object vertices into
# view and onto the screen, the clipper and its callers, the depth buckets,
# the particles, a car's lights and level of detail, the small vector helpers
# -- native whole in nfs3hp::vertices (native_vertices.cpp), each in every
# x87 mode as the game's code rounds it.  tools/game_checks/vertices holds
# them against their code, made afresh; tools/game_checks leaves them out.
VERTICES_NATIVES = {
    0x41a360: "vertices::projectedTriangle",
    0x41a550: "vertices::trackVertices",
    0x41a970: "vertices::objectVerticesColoured",
    0x41aab0: "vertices::groundDistanceOf",
    0x41ae90: "vertices::trackPolygons",
    0x41b300: "vertices::trackRecords",
    0x41b640: "vertices::objectRecords",
    0x41bf30: "vertices::boxOutside",
    0x41e0f0: "vertices::nearestBlock",
    0x433a50: "vertices::triangleList",
    0x433bb0: "vertices::quadList",
    0x434380: "vertices::submitPolygons",
    0x492370: "vertices::lightGlow",
    0x492980: "vertices::headlightBeam",
    0x4968a0: "vertices::groundHeight",
    0x4972f0: "vertices::normalise",
    0x49d9d0: "vertices::objectVerticesNearest",
    0x49da30: "vertices::shadeNormals",
    0x49db70: "vertices::envMapCoords",
    0x49dbf0: "vertices::cullPolygons",
    0x4bb5d0: "vertices::carDetail",
    0x4bf4c0: "vertices::objectVertices",
    0x4c11b0: "vertices::clipTriangle",
    0x4c12e0: "vertices::clipQuad",
    0x4c1aa0: "vertices::clipTriangleByZ",
    0x4c1e20: "vertices::quadMesh",
    0x4c2cd0: "vertices::clipPolygon",
    0x4c7320: "vertices::gatherSorted",
    0x4c7610: "vertices::sortedBuckets",
    0x4c7790: "vertices::depthBuckets",
    0x4c79a0: "vertices::gatherBuckets",
    0x4cb2c0: "vertices::moveParticles",
    0x4cb750: "vertices::drops",
    0x4cbc90: "vertices::particles",
    0x4dbef0: "vertices::objectVerticesList",
    0x4dffb0: "vertices::addVectors",
    0x4e0000: "vertices::subtractVectors",
    0x4e0050: "vertices::scaleVectors",
    0x4e01c0: "vertices::unitVector",
    0x4e01f0: "vertices::dotPush",
    0x4e0210: "vertices::dotProducts",
    0x4e0280: "vertices::multiplyMatrices",
    0x4e03b0: "vertices::rotateVectors",
    0x4e0430: "vertices::transformPoints",
    0x4e06a0: "vertices::vectorLengthOf",
}
GAME_NATIVES.update(VERTICES_NATIVES)

# The game's 2D drawing, rewritten as the port's own C++ (src/nfs3hp/
# port_draw2d.h, nfs3hp::port::draw2d): the screen quad's clip codes,
# rectangles, boxes, panels, frames and lines, a tile's texture over a
# rectangle, the 2D screens' font made and its characters, texts and titles,
# the menus' pointer, the screens started afresh, the 640 by 480 quads scaled
# to the screen -- what the menus, the HUD and the pause screens draw with.
# tools/game_checks holds them against their generated code (draw2d.inc).
PORT_DRAW2D_NATIVES = {
    0x4d5840: "port::draw2d::quadOutcodes",
    0x4d81b0: "port::draw2d::drawShadedRectangle",
    0x4d8550: "port::draw2d::gradientRectangle",
    0x4d8860: "port::draw2d::drawShadedBox",
    0x4d8910: "port::draw2d::filledRectangle",
    0x4d8930: "port::draw2d::frame",
    0x4d89f0: "port::draw2d::drawPanel",
    0x4d8ac0: "port::draw2d::drawFieldBox",
    0x4d8b80: "port::draw2d::drawTileRectangle",
    0x4d8ed0: "port::draw2d::drawLine",
    0x4d9240: "port::draw2d::fontCharacter",
    0x4d9320: "port::draw2d::accentedCharacter",
    0x4d93a0: "port::draw2d::buildScreenFont",
    0x4d95c0: "port::draw2d::drawScreenCharacter",
    0x4d9b90: "port::draw2d::drawTurnedCharacter",
    0x4da120: "port::draw2d::drawTurnedText",
    0x4da1e0: "port::draw2d::drawTitleText",
    0x4da4a0: "port::draw2d::drawScreenText",
    0x4dab90: "port::draw2d::drawPointer",
    0x4daf80: "port::draw2d::startScreens",
    0x4db000: "port::draw2d::drawTexturedQuad",
    0x4db380: "port::draw2d::drawShadedQuad",
}
GAME_NATIVES.update(PORT_DRAW2D_NATIVES)
# The menus' text and the 2D drawing under it, rewritten as the port's own
# (port::text, src/nfs3hp/port_text.cpp): the language's text, the 3D card's
# font and its quads, the text scale, a text placed on the menus' screen and
# on the screen's own, a text wrapped into a box, the menus' key.  None of
# their code is generated; tools/game_checks holds each against it (text.inc).
PORT_TEXT_NATIVES = {
    0x451400: "port::text::toScreenX",
    0x451440: "port::text::toScreenY",
    0x4513c0: "port::text::setTextColour",
    0x451480: "port::text::screenToMenuX",
    0x4514c0: "port::text::screenToMenuY",
    0x451500: "port::text::readMenuMouse",
    0x451530: "port::text::dimBackground",
    0x451590: "port::text::drawTextAt",
    0x451700: "port::text::joystickKey",
    0x451810: "port::text::captureKeys",
    0x451840: "port::text::scanCodeOfKey",
    0x4518c0: "port::text::keyOfScanCode",
    0x451960: "port::text::menuKey",
    0x451b20: "port::text::resetMenuKey",
    0x451b50: "port::text::nextWord",
    0x451cd0: "port::text::drawWrappedText",
    0x451fc0: "port::text::drawTextBox",
    0x451ff0: "port::text::measureTextBox",
    0x452020: "port::text::textWidth",
    0x452100: "port::text::selectFont",
    0x452110: "port::text::menusEnd",
    0x452190: "port::text::drawMenuText",
    0x49bcd0: "port::text::takeQuad",
    0x49bd30: "port::text::queueQuad",
    0x49bdc0: "port::text::unqueueQuad",
    0x49be40: "port::text::flushQuads",
    0x49c000: "port::text::mouseMenuKey",
    0x4beb80: "port::text::viewport",
    0x4d06f0: "port::text::drawCharacter",
    0x4d1000: "port::text::drawText",
    0x4d1340: "port::text::flushFonts",
    0x4d1390: "port::text::setTextScale",
    0x4d1400: "port::text::keepTextScale",
    0x4d1410: "port::text::restoreTextScale",
    0x4d1430: "port::text::measureText",
    0x4d1480: "port::text::textWidthWithOverhang",
    0x4d1540: "port::text::characterWidth",
    0x4d1590: "port::text::characterOverhang",
    0x4d1750: "port::text::loadLanguageText",
    0x4d17b0: "port::text::specialText",
    0x4d1850: "port::text::languageText",
}
GAME_NATIVES.update(PORT_TEXT_NATIVES)
# The game's shapes rewritten (port::, src/nfs3hp/port_shapes.cpp): an FSH
# shape's alpha scanned and its pixels converted into a texture's format, the
# menus' art made textures a tile at a time, found by name and drawn.  No
# generated code behind them; tools/game_checks holds each adapter against
# its code, made afresh (shapes.inc).
PORT_SHAPES_NATIVES = {
    0x4d18e0: "port::shapes::scan4444",
    0x4d1950: "port::shapes::scan8888",
    0x4d19b0: "port::shapes::scanNibbles",
    0x4d1a00: "port::shapes::scanPaletted",
    0x4d1a90: "port::shapes::scan1555",
    0x4d1ad0: "port::shapes::scan565",
    0x4d1b20: "port::shapes::convert4444",
    0x4d1c00: "port::shapes::convert8888",
    0x4d1ce0: "port::shapes::convert8",
    0x4d1db0: "port::shapes::convert565",
    0x4d1e60: "port::shapes::convert1555",
    0x4d1f70: "port::shapes::nibblesTo1555",
    0x4d2050: "port::shapes::nibblesTo565",
    0x4d2150: "port::shapes::nibblesTo4444",
    0x4d21f0: "port::shapes::nibblesTo8888",
    0x4d22b0: "port::shapes::paletteTo1555",
    0x4d23f0: "port::shapes::paletteTo565",
    0x4d2510: "port::shapes::paletteTo4444",
    0x4d2690: "port::shapes::paletteTo8888",
    0x4d27e0: "port::shapes::argb8888To4444",
    0x4d2940: "port::shapes::argb8888To565",
    0x4d2a80: "port::shapes::argb8888To1555",
    0x4d2be0: "port::shapes::rgb565To1555",
    0x4d2d20: "port::shapes::rgb565To4444",
    0x4d2e60: "port::shapes::rgb565To8888",
    0x4d2fb0: "port::shapes::argb1555To565",
    0x4d30f0: "port::shapes::argb1555To4444",
    0x4d3230: "port::shapes::argb1555To8888",
    0x4d37a0: "port::shapes::shapeToPixels",
    0x4d5c40: "port::shapes::resetTextures",
    0x4d5db0: "port::shapes::shapeByName",
    0x4d6070: "port::shapes::shapeTexture",
    0x4d6110: "port::shapes::cutTile",
    0x4d6190: "port::shapes::shape565To1555",
    0x4d6210: "port::shapes::tileRoom",
    0x4d6390: "port::shapes::artShapeTextures",
    0x4d66f0: "port::shapes::shapeSize",
    0x4d70f0: "port::shapes::drawShapeQuad",
    0x4d7160: "port::shapes::drawArtShape",
    0x4d7990: "port::shapes::drawShape",
}
GAME_NATIVES.update(PORT_SHAPES_NATIVES)
# The menus' widgets that hold a value rewritten (port::, src/nfs3hp/port_fields.cpp):
# the fields typed into, the list of lines, the sliders, the car's colours and
# the car records' colours.  No generated code behind them; tools/game_checks
# holds each adapter against its code, made afresh (fields.inc).
PORT_FIELDS_NATIVES = {
    0x439760: "port::fields::carColourCount",
    0x439780: "port::fields::carCustomColour",
    0x439790: "port::fields::carColour",
    0x4397c0: "port::fields::carColourName",
    0x4397d0: "port::fields::setCarColour",
    0x439810: "port::fields::carColourRgb",
    0x4398c0: "port::fields::setCarCustomColour",
    0x439ab0: "port::fields::carText",
    0x43b7f0: "port::fields::saveCarColours",
    0x43dbe0: "port::fields::wheelFromCarColour",
    0x43dd50: "port::fields::wheelForPlayer",
    0x4513e0: "port::fields::setCursorColour",
    0x4515e0: "port::fields::drawFieldText",
    0x452220: "port::fields::drawFieldLine",
    0x4522c0: "port::fields::drawSpacedText",
    0x4569a0: "port::fields::colourListWidth",
    0x456a20: "port::fields::colourListRefresh",
    0x456b50: "port::fields::drawColourLine",
    0x456ce0: "port::fields::drawColourList",
    0x456ea0: "port::fields::colourListShown",
    0x456f10: "port::fields::pointAtColour",
    0x456f50: "port::fields::openColourList",
    0x456f70: "port::fields::closeColourList",
    0x456f90: "port::fields::chooseColour",
    0x457030: "port::fields::colourListKey",
    0x4571e0: "port::fields::layoutColourList",
    0x457250: "port::fields::colourListHidden",
    0x457ae0: "port::fields::editKey",
    0x457c40: "port::fields::drawIntField",
    0x457df0: "port::fields::intRangeMessage",
    0x457e30: "port::fields::intFieldKey",
    0x457f70: "port::fields::layoutIntField",
    0x458010: "port::fields::intFieldShown",
    0x458050: "port::fields::intFieldHidden",
    0x458080: "port::fields::bindIntField",
    0x4580b0: "port::fields::drawStringField",
    0x4581f0: "port::fields::stringFieldKey",
    0x458340: "port::fields::layoutStringField",
    0x4583b0: "port::fields::stringFieldShown",
    0x458410: "port::fields::stringFieldHidden",
    0x458480: "port::fields::drawCharField",
    0x458640: "port::fields::charRangeMessage",
    0x458690: "port::fields::charFieldKey",
    0x4587e0: "port::fields::layoutCharField",
    0x4588a0: "port::fields::charFieldShown",
    0x4588e0: "port::fields::charFieldHidden",
    0x458910: "port::fields::bindCharField",
    0x458940: "port::fields::drawFloatField",
    0x458b20: "port::fields::floatRangeMessage",
    0x458b80: "port::fields::floatFieldKey",
    0x458ce0: "port::fields::layoutFloatField",
    0x458d90: "port::fields::floatFieldShown",
    0x458dd0: "port::fields::floatFieldHidden",
    0x458e00: "port::fields::bindFloatField",
    0x458e30: "port::fields::drawFixedField",
    0x459020: "port::fields::fixedRangeMessage",
    0x459090: "port::fields::fixedFieldKey",
    0x459210: "port::fields::layoutFixedField",
    0x4592b0: "port::fields::fixedFieldShown",
    0x4592f0: "port::fields::fixedFieldHidden",
    0x459320: "port::fields::bindFixedField",
    0x465500: "port::fields::drawListBox",
    0x465880: "port::fields::pointAtListBoxLine",
    0x4658f0: "port::fields::listBoxDown",
    0x465950: "port::fields::listBoxUp",
    0x4659a0: "port::fields::listBoxPartAt",
    0x465a10: "port::fields::listBoxArrowAt",
    0x465a30: "port::fields::listBoxKey",
    0x465ba0: "port::fields::layoutListBox",
    0x465c10: "port::fields::listBoxShown",
    0x465c60: "port::fields::listBoxHidden",
    0x465ca0: "port::fields::listBoxChooseAt",
    0x469080: "port::fields::sliderPartSizes",
    0x4692c0: "port::fields::sliderPartUnderPointer",
    0x469340: "port::fields::sliderThumbPlace",
    0x4693c0: "port::fields::snapToStep",
    0x469480: "port::fields::setSliderValue",
    0x469530: "port::fields::setSliderStep",
    0x469590: "port::fields::dragSliderTo",
    0x469680: "port::fields::drawSlider",
    0x46a5f0: "port::fields::trackSlider",
    0x46a860: "port::fields::sliderKey",
    0x46a980: "port::fields::placeSliderParts",
    0x46ab10: "port::fields::setUpSlider",
    0x46abb0: "port::fields::sliderBox",
    0x46ac40: "port::fields::twoAxisThumbPlace",
    0x46add0: "port::fields::setTwoAxisValue",
    0x46afe0: "port::fields::setTwoAxisStep",
    0x46b040: "port::fields::dragTwoAxisTo",
    0x46b3a0: "port::fields::drawTwoAxis",
    0x46c320: "port::fields::trackTwoAxis",
    0x46c4a0: "port::fields::twoAxisKey",
    0x46c640: "port::fields::placeTwoAxisParts",
    0x46c820: "port::fields::setUpTwoAxis",
    0x46c8b0: "port::fields::twoAxisBox",
    0x46c950: "port::fields::sliderSetValue",
    0x46cae0: "port::fields::sliderDraw",
    0x46cb80: "port::fields::sliderWidgetKey",
    0x46cbe0: "port::fields::sliderWidgetLayout",
    0x46cc10: "port::fields::sliderWidgetHidden",
    0x46cc50: "port::fields::sliderWidgetShown",
    0x472360: "port::fields::refreshCustomColours",
    0x4d11f0: "port::fields::drawCursorText3d",
    0x4d5e40: "port::fields::sliderShape",
    0x4d8570: "port::fields::drawShadedTriangle",
    0x4d8aa0: "port::fields::drawFrameAt",
    0x4da560: "port::fields::drawCursor",
    0x4da8c0: "port::fields::drawCursorText",
}
GAME_NATIVES.update(PORT_FIELDS_NATIVES)

# The menus' 3D object rewritten (port::obj3d, src/nfs3hp/port_obj3d.cpp):
# the obj3d widget's handlers, its loader that reads a car's .viv a part at a
# time, the fade between cars, the object turned and dragged, its rectangle,
# its lights' flares, a car's .viv path, the logo's quad.  No generated code
# behind them; tools/game_checks holds each adapter against its code, made
# afresh (obj3d.inc).
PORT_OBJ3D_NATIVES = {
    0x439360: "port::obj3d::carVivPath",
    0x465eb0: "port::obj3d::flareQuad",
    0x466330: "port::obj3d::lightPlaced",
    0x4663a0: "port::obj3d::headlightGlare",
    0x466580: "port::obj3d::taillightGlare",
    0x466740: "port::obj3d::objectLights",
    0x4667d0: "port::obj3d::objectDragged",
    0x4668b0: "port::obj3d::objectBounds",
    0x466cf0: "port::obj3d::objectDraw",
    0x4675b0: "port::obj3d::objectKeyed",
    0x4675d0: "port::obj3d::objectPassed",
    0x467650: "port::obj3d::objectReleased",
    0x467700: "port::obj3d::objectOpened",
    0x467930: "port::obj3d::objectClosed",
    0x467ac0: "port::obj3d::resetLoader",
    0x467bc0: "port::obj3d::genericVivPath",
    0x467be0: "port::obj3d::genericModelName",
    0x467c00: "port::obj3d::trackVivPath",
    0x467c20: "port::obj3d::trackModelName",
    0x467c40: "port::obj3d::chooseFiles",
    0x467ea0: "port::obj3d::readPrehead",
    0x467ef0: "port::obj3d::readHead",
    0x468010: "port::obj3d::readModel",
    0x468170: "port::obj3d::readTexture",
    0x4682c0: "port::obj3d::readShapes",
    0x468410: "port::obj3d::takeModel",
    0x468580: "port::obj3d::fadeIn",
    0x4685e0: "port::obj3d::fadeOut",
    0x468670: "port::obj3d::wantedChanged",
    0x468860: "port::obj3d::runLoader",
    0x468da0: "port::obj3d::carColour",
    0x468f00: "port::obj3d::makeTexture",
    0x4d79b0: "port::obj3d::drawLogo",
}
GAME_NATIVES.update(PORT_OBJ3D_NATIVES)
# The menus' widgets rewritten (port::buttons, src/nfs3hp/port_buttons.cpp):
# what a screen's text, image, button, cascade, drop list, select list, check
# box, box overlay, speech, rectangle and text box do as they are shown,
# drawn, keyed, laid out and hidden, and what those call of their own -- the
# lists by name, the HUD's art loaded, a network race's ready marks, the
# track's map, a shape drawn turned.  No generated code behind them;
# tools/game_checks holds each against it (buttons.inc).
PORT_BUTTONS_NATIVES = {
    0x43fdd0: "port::buttons::joinListedGame",
    0x440c40: "port::buttons::drawReadyMarks",
    0x440d20: "port::buttons::networkLeft",
    0x442450: "port::buttons::drawLicence",
    0x442b90: "port::buttons::listColours",
    0x442bf0: "port::buttons::listAllowed",
    0x442c50: "port::buttons::listLength",
    0x442c80: "port::buttons::findList",
    0x442d00: "port::buttons::menuEdited",
    0x442d20: "port::buttons::menuRecordEdited",
    0x44ac90: "port::buttons::loadShapes",
    0x44b170: "port::buttons::redrawMenu",
    0x455e30: "port::buttons::textBoxDraw",
    0x456020: "port::buttons::textBoxKey",
    0x456110: "port::buttons::textBoxUp",
    0x456170: "port::buttons::textBoxShown",
    0x4561d0: "port::buttons::textBoxHidden",
    0x456320: "port::buttons::slideTrackMap",
    0x456590: "port::buttons::drawTrackFacts",
    0x456670: "port::buttons::drawGlow",
    0x456800: "port::buttons::trackMapShape",
    0x45fae0: "port::buttons::rectDraw",
    0x45fc90: "port::buttons::speechDraw",
    0x45fca0: "port::buttons::speechShown",
    0x45fcb0: "port::buttons::speechHidden",
    0x45fcd0: "port::buttons::textDraw",
    0x460070: "port::buttons::textUp",
    0x4600b0: "port::buttons::textShown",
    0x460100: "port::buttons::imageDraw",
    0x460410: "port::buttons::imageUp",
    0x460440: "port::buttons::imageShown",
    0x460700: "port::buttons::pulseOn",
    0x460770: "port::buttons::pulseColour",
    0x4607c0: "port::buttons::buttonDraw",
    0x461270: "port::buttons::buttonKey",
    0x4613e0: "port::buttons::layoutMenuItem",
    0x461900: "port::buttons::buttonShown",
    0x461a70: "port::buttons::cascadeLayoutClosed",
    0x461aa0: "port::buttons::cascadeLineText",
    0x461d00: "port::buttons::cascadeDraw",
    0x462320: "port::buttons::cascadePass",
    0x4623a0: "port::buttons::cascadeShown",
    0x4624a0: "port::buttons::cascadeKey",
    0x4628d0: "port::buttons::cascadeUp",
    0x4629c0: "port::buttons::checkboxDraw",
    0x462ad0: "port::buttons::checkboxKey",
    0x462b20: "port::buttons::checkboxShown",
    0x462b30: "port::buttons::checkboxUp",
    0x462b90: "port::buttons::boxOverlayDraw",
    0x462c30: "port::buttons::selectListShown",
    0x462ce0: "port::buttons::selectListHidden",
    0x462d20: "port::buttons::selectListDraw",
    0x462f90: "port::buttons::selectListPass",
    0x462fa0: "port::buttons::selectListKey",
    0x4633d0: "port::buttons::selectListUp",
    0x4634d0: "port::buttons::dropListDraw",
    0x463de0: "port::buttons::dropListPass",
    0x463e60: "port::buttons::dropListShown",
    0x463ff0: "port::buttons::listKey",
    0x464560: "port::buttons::layoutList",
    0x4d6730: "port::buttons::drawTurnedShape",
    0x4d88f0: "port::buttons::drawColouredRectangle",
}
GAME_NATIVES.update(PORT_BUTTONS_NATIVES)

# The menu files rewritten (port::menus, src/nfs3hp/port_menus.cpp): the .MNU
# loader -- a file's lines, the keys looked up in the parser's tables and
# stored into the items' fields by their kinds, the screens a button leads to
# loaded too -- the item lookup's codelink match, a screen's handlers by its
# name, the editor's checks of the files, and the path function the file's
# name is built with.  No generated code behind them; tools/game_checks holds
# each against it (menus.inc).
PORT_MENUS_NATIVES = {
    0x4429c0: "port::menus::codelinkMatches",
    0x442b10: "port::menus::assignHandlers",
    0x448f00: "port::menus::keywordIndex",
    0x448f50: "port::menus::storeString",
    0x448fd0: "port::menus::widgetType",
    0x449020: "port::menus::widgetName",
    0x449060: "port::menus::initItem",
    0x4490e0: "port::menus::sectionType",
    0x449130: "port::menus::parseInt",
    0x449170: "port::menus::parseLong",
    0x4491b0: "port::menus::parseFloat",
    0x449240: "port::menus::setField",
    0x4493b0: "port::menus::readEntry",
    0x449510: "port::menus::loadMenus",
    0x4498a0: "port::menus::checkMenuFile",
    0x449960: "port::menus::checkMenuFiles",
    0x449990: "port::menus::callOnLoad",
    0x4529a0: "port::menus::appendBackslash",
}
GAME_NATIVES.update(PORT_MENUS_NATIVES)

# The menu loop rewritten (port::loop, src/nfs3hp/port_loop.cpp): the screen
# shown and the screens gone through, its items shown, drawn and passed, the
# selection moved by the keys and the mouse, a key handed to the item that has
# it, the race settings kept consistent, the help box, the loop itself, and the
# answers of the dialogs it opens.  No generated code behind them;
# tools/game_checks holds each against it (loop.inc).
PORT_LOOP_NATIVES = {
    0x4499b0: "port::loop::itemUsable",
    0x4499f0: "port::loop::keyAllowed",
    0x449aa0: "port::loop::drawHelp",
    0x449df0: "port::loop::keysTaken",
    0x449e50: "port::loop::selectable",
    0x449ea0: "port::loop::pointAtSelected",
    0x449f30: "port::loop::selectPrevious",
    0x44a0c0: "port::loop::selectNext",
    0x44a240: "port::loop::escapeButton",
    0x44a2d0: "port::loop::handleKey",
    0x44a440: "port::loop::selectedIndex",
    0x44a470: "port::loop::selectItem",
    0x44a4f0: "port::loop::drawItems",
    0x44a690: "port::loop::releaseMouse",
    0x44a6b0: "port::loop::itemUnderMouse",
    0x44a860: "port::loop::itemsShown",
    0x44a8b0: "port::loop::passItems",
    0x44a9e0: "port::loop::hideItems",
    0x44aa30: "port::loop::leaveScreen",
    0x44aad0: "port::loop::resetItems",
    0x44ab40: "port::loop::enterScreen",
    0x44ae60: "port::loop::menuByName",
    0x44aeb0: "port::loop::currentMenu",
    0x44aec0: "port::loop::popMenu",
    0x44af10: "port::loop::pushMenu",
    0x44af50: "port::loop::setHistory",
    0x44af90: "port::loop::player1NameAnswer",
    0x44b040: "port::loop::player2NameAnswer",
    0x44b0f0: "port::loop::quitAnswer",
    0x44b1a0: "port::loop::waitWhileSuspended",
    0x44b230: "port::loop::menuLoop",
    0x44be60: "port::loop::fitRaceSettings",
}
GAME_NATIVES.update(PORT_LOOP_NATIVES)

# The dialogs over the screens rewritten (port::dialogs, src/nfs3hp/port_dialogs.cpp):
# the dialog up and its kind, what it draws -- its box, its lines, its buttons,
# the field typed in, a list's entries, the help box over it -- the pointer on
# its buttons and entries, each kind's keys and its pass (a question, a
# message, a line to type in, a list to tick), a list opened and its entries
# laid out, and the dialog's own keys and pass, which hand an answer to its
# callback.  No generated code behind them; tools/game_checks holds each
# against it (dialogs.inc).  What opens a question, a message and a line to
# type in, and lays out their boxes, is MENU_NATIVES'.
PORT_DIALOGS_NATIVES = {
    0x4438b0: "port::dialogs::dialogShown",
    0x4438c0: "port::dialogs::dialogKind",
    0x4438d0: "port::dialogs::drawBox",
    0x443f40: "port::dialogs::drawLines",
    0x444000: "port::dialogs::drawButtons",
    0x444350: "port::dialogs::drawEditField",
    0x4445d0: "port::dialogs::buttonUnderMouse",
    0x444910: "port::dialogs::questionKey",
    0x4449d0: "port::dialogs::questionPass",
    0x444a60: "port::dialogs::endMessage",
    0x444b20: "port::dialogs::messageKey",
    0x444b60: "port::dialogs::messagePass",
    0x444f50: "port::dialogs::editKey",
    0x4450c0: "port::dialogs::editPass",
    0x445150: "port::dialogs::listFocus",
    0x445160: "port::dialogs::pointAtEntry",
    0x4451d0: "port::dialogs::entryUnderMouse",
    0x4452d0: "port::dialogs::placeEntries",
    0x445380: "port::dialogs::drawList",
    0x445520: "port::dialogs::listKey",
    0x445680: "port::dialogs::listPass",
    0x445750: "port::dialogs::openList",
    0x445ac0: "port::dialogs::drawDialogHelp",
    0x445de0: "port::dialogs::loadDialogArt",
    0x445e60: "port::dialogs::dialogKey",
    0x445f10: "port::dialogs::closeDialog",
    0x445f50: "port::dialogs::dialogPass",
}
GAME_NATIVES.update(PORT_DIALOGS_NATIVES)

# The pan-and-zoom view over the maps, rewritten as the port's own, without the
# x87 (src/nfs3hp/port_view.{h,cpp}, nfs3hp::port::view): the participant
# record's floats, the pan's base, the lap-metric popup and the view's pan,
# grid and keys.  No generated code behind them; tools/game_checks holds each
# against it (view.inc).
PORT_VIEW_NATIVES = {
    0x420ff0: "port::view::sectionAheadIsTwo",
    0x4646e0: "port::view::storeIncremented",
    0x4733c0: "port::view::colourNearest",
    0x474db0: "port::view::edgeTriangles",
    0x475240: "port::view::glyphResource",
    0x464840: "port::view::gridDrawn",
    0x464fd0: "port::view::viewFrame",
    0x4651e0: "port::view::panKey",
    0x469580: "port::view::recordFloat",
    0x46b030: "port::view::recordFloatAt",
    0x46ca70: "port::view::recordFloatByKind",
    0x46cf30: "port::view::metricPopup",
}
GAME_NATIVES.update(PORT_VIEW_NATIVES)

# The race HUD, rewritten as the port's own, without the x87 (src/nfs3hp/port_hud.{h,cpp},
# nfs3hp::port::hud): its gauges, needles and the track's map, a function at a time.  No
# generated code behind them; tools/game_checks holds each against it (hud.inc).
PORT_HUD_NATIVES = {
    0x484040: "port::hud::loadingTextPrepared",
    0x485c20: "port::hud::floatsSwapped",
    0x487350: "port::hud::fourIntegersAppended",
    0x48cb50: "port::hud::pointTransformed",
    0x48cc60: "port::hud::boxDrawn",
    0x48a220: "port::hud::indicatorGeometryUpdated",
    0x4897f0: "port::hud::textAlignedDrawn",
    0x489d70: "port::hud::percentTextDrawn",
    0x489b30: "port::hud::listRowsDrawn",
    0x489c40: "port::hud::timeTextDrawn",
    0x48ea20: "port::hud::viewZoomStepped",
    0x48e790: "port::hud::viewPaneStepped",
    0x484b70: "port::hud::screenScalesSet",
    0x4870c0: "port::hud::rectangleQuadSet",
    0x486f70: "port::hud::stackQuadSet",
    0x485180: "port::hud::labelTextSet",
    0x485c30: "port::hud::pointClipped",
    0x485f20: "port::hud::segmentClippedToRect",
    0x4860b0: "port::hud::segmentCutIntoPieces",
    0x4873e0: "port::hud::strokeClippedDrawn",
    0x485a60: "port::hud::standingsGapMeasured",
    0x486df0: "port::hud::floatsQuadSet",
    0x484070: "port::hud::hudStarted",
    0x48a3a0: "port::hud::rankTextDrawn",
    0x48a660: "port::hud::labelRowsDrawn",
    0x48ae70: "port::hud::standingsDrawn",
    0x48aa40: "port::hud::columnsStarted",
    0x48cdb0: "port::hud::mapOutlineStarted",
    0x48d110: "port::hud::hudViewsSet",
    0x4844c0: "port::hud::hudPaneSet",
    0x484c10: "port::hud::hudTablesStarted",
    0x489e00: "port::hud::indicatorQuadSet",
    0x487620: "port::hud::gaugeQuadClipped",
    0x486340: "port::hud::segmentsMet",
    0x487a30: "port::hud::boxQuadsSet",
    0x48b5f0: "port::hud::markersSet",
    0x48d4c0: "port::hud::mapPointsDrawn",
}
GAME_NATIVES.update(PORT_HUD_NATIVES)

# Functions of the game's that use the x87, rewritten from their code by tools/decomp.py (src/nfs3hp/port_conv.{h,cpp},
# nfs3hp::port::conv); tools/game_checks/conv.inc holds each against the game's code.
PORT_CONV_NATIVES = {
    0x49fe70: "port::conv::fn49fe70",
    0x4a00d0: "port::conv::fn4a00d0",
    0x4a0440: "port::conv::fn4a0440",
    0x4a0980: "port::conv::fn4a0980",
    0x4a0a10: "port::conv::fn4a0a10",
    0x4a0d20: "port::conv::fn4a0d20",
    0x4a0f30: "port::conv::fn4a0f30",
    0x4a1090: "port::conv::fn4a1090",
    0x4a20c0: "port::conv::fn4a20c0",
    0x4a2630: "port::conv::fn4a2630",
    0x4a2830: "port::conv::fn4a2830",
    0x4a2c60: "port::conv::fn4a2c60",
    0x4a2e20: "port::conv::fn4a2e20",
    # @@NATIVES@@
}
GAME_NATIVES.update(PORT_CONV_NATIVES)



# The menus' screens that are native whole, no generated code behind them:
# each address is a handler of native_frontend_main.cpp, in nfs3hp::frontend,
# that the screens' table at 0x555c48 or a button hands to the menu loop.
# The main menu in its three forms -- main, mainsplt, mainmult -- each as it
# opens, as it closes and every pass of the loop; the players' name buttons
# and their dialogs' answers; mainmult's race list and its Back's question.
# And what the main menu calls of the menu loop (native_frontend_loop.cpp)
# and of the dialogs (native_frontend_dialogs.cpp): a screen's items shown,
# the shown screen's name; a question, a message, a line to type in opened,
# and their layout.  tools/frontend_checks compares them with the game's
# code, made afresh (its generate.py); tools/game_checks leaves them out.
def menu_group_natives():
    """tools/frontend_checks/groups/*.txt: the screens' functions by group."""
    from pathlib import Path
    natives = {}
    for path in sorted((Path(__file__).resolve().parent / "tools/frontend_checks/groups").glob("*.txt")):
        for line in path.read_text().splitlines():
            if line and not line.startswith("#"):
                address, name, _ = line.split()
                natives[int(address, 16)] = "frontend::" + name
    return natives


MENU_NATIVES = {
    # The options screens, the cars' and the tracks' (native_frontend_options.cpp, _cars, _tracks).
    0x442aa0: "frontend::optionsOnEnter",
    0x4374b0: "frontend::feedOnEnter",
    0x437590: "frontend::feedOnExit",
    0x437250: "frontend::feedSeparate",
    0x437300: "frontend::feedTimeButtonKey",
    0x437390: "frontend::feedPositionButtonKey",
    0x437420: "frontend::feedSpeedButtonKey",
    0x437350: "frontend::feedTimeListKey",
    0x4373e0: "frontend::feedPositionListKey",
    0x437470: "frontend::feedSpeedListKey",
    0x437e00: "frontend::audioOnEnter",
    0x438070: "frontend::audioOnExit",
    0x438260: "frontend::audioOnFrame",
    0x437a90: "frontend::audioVolumeDraw",
    0x437990: "frontend::audioMenuMusicMoved",
    0x4379b0: "frontend::audioRaceMusicMoved",
    0x4379d0: "frontend::audioEffectsMoved",
    0x437a20: "frontend::audioEngineMoved",
    0x437a70: "frontend::audioSpeechMoved",
    0x448440: "frontend::graphicsOnEnter",
    0x4484e0: "frontend::graphicsOnExit",
    0x448430: "frontend::graphicsOnFrame",
    0x448610: "frontend::advancedOnEnter",
    0x448770: "frontend::advancedOnExit",
    0x4485a0: "frontend::advancedOnFrame",
    0x448570: "frontend::advancedIntensityMoved",
    0x4104b0: "frontend::carMenuMusicBack",
    0x4267a0: "frontend::carSaveFrame",
    0x4267f0: "frontend::trackSaveFrame",
    0x426870: "frontend::carRecord",
    0x426b00: "frontend::trackRecord",
    0x426d60: "frontend::car1OnEnter",
    0x426ef0: "frontend::car2OnEnter",
    0x426e20: "frontend::car1OnFrame",
    0x426f80: "frontend::car2OnFrame",
    0x426ee0: "frontend::car1OnExit",
    0x427040: "frontend::car2OnExit",
    0x426c20: "frontend::car1EditClicked",
    0x426c40: "frontend::car2EditClicked",
    0x426c60: "frontend::car1ColourClicked",
    0x426ce0: "frontend::car2ColourClicked",
    0x427050: "frontend::opponentFitCar",
    0x4270c0: "frontend::opponentFitArt",
    0x4272a0: "frontend::opponentOnEnter",
    0x4272e0: "frontend::opponentOnExit",
    0x427190: "frontend::opponentOnFrame",
    0x4272f0: "frontend::tuningLoad",
    0x427370: "frontend::tuningSave",
    0x4273e0: "frontend::tuningEngineMoved",
    0x427470: "frontend::tuningBrakesMoved",
    0x427500: "frontend::tuningSteeringMoved",
    0x427590: "frontend::tuningGearsMoved",
    0x427620: "frontend::tuningSuspensionMoved",
    0x4276b0: "frontend::tuningAeroMoved",
    0x427740: "frontend::tuningTiresMoved",
    0x4277d0: "frontend::tuningSetSliders",
    0x4279b0: "frontend::tuningRememberSliders",
    0x427ac0: "frontend::tuningSlidersMoved",
    0x427c20: "frontend::tuningBindSliders",
    0x427d00: "frontend::tuningLoadAdvanced",
    0x427d80: "frontend::tuningDefaultsAnswered",
    0x427e10: "frontend::tuningDoneKey",
    0x427ea0: "frontend::tuningCancelKey",
    0x427f40: "frontend::tuningCarListKey",
    0x427fb0: "frontend::tuningDefaultsKey",
    0x428020: "frontend::tuningBindDoneCancel",
    0x428060: "frontend::tuningFitButtons",
    0x4281e0: "frontend::tuningPrepare",
    0x428360: "frontend::tuningOnEnter",
    0x4283b0: "frontend::tuningOnExit",
    0x4dcaf0: "frontend::tuningAdvancedDiffers",
    0x4dcb60: "frontend::tuningCopyAdvanced",
    0x4487e0: "frontend::locationOnEnter",
    0x4487d0: "frontend::locationOnFrame",
    0x4489e0: "frontend::locationOnExit",
    0x456200: "frontend::infoPlaceTrackPlaces",
    0x4568d0: "frontend::infoOnEnter",
    0x456890: "frontend::infoOnFrame",
    0x456990: "frontend::infoOnExit",
    0x4c94e0: "frontend::infoSpeak",
    0x4c9590: "frontend::infoStopSpeaking",
    0x43fcb0: "frontend::multiMenuLeaveAnswer",
    0x4400d0: "frontend::multiMenuOnBack",
    0x440160: "frontend::multiMenuApplyRaceList",
    0x440390: "frontend::multiMenuFollowRaceType",
    0x4403f0: "frontend::multiMenuOnFrame",
    0x440470: "frontend::multiMenuOnEnter",
    0x4405b0: "frontend::multiMenuOnExit",
    0x45cdd0: "frontend::mainMenuPlayerNameAnswer",
    0x45ce60: "frontend::splitMenuPlayer2NameAnswer",
    0x45cef0: "frontend::mainMenuOnPlayerName",
    0x45cf70: "frontend::splitMenuOnPlayer2Name",
    0x45cff0: "frontend::mainMenuOnFrame",
    0x45d050: "frontend::mainMenuOnEnter",
    0x45d2f0: "frontend::mainMenuOnExit",
    0x45d300: "frontend::splitMenuOnFrame",
    0x45d360: "frontend::splitMenuOnEnter",
    0x45d620: "frontend::splitMenuOnExit",
    0x44a960: "frontend::menuShowItems",
    0x44ae00: "frontend::menuCurrentName",
    0x443a90: "frontend::dialogLinesBox",
    0x443c00: "frontend::dialogButtonRowBox",
    0x443d20: "frontend::dialogPlaceButtons",
    0x444570: "frontend::dialogPointAtButton",
    0x4446d0: "frontend::dialogOpenQuestion",
    0x444a80: "frontend::dialogOpenMessage",
    0x444be0: "frontend::dialogMessageById",
    0x444c10: "frontend::dialogEditText",
    0x444c20: "frontend::dialogOpenEdit",
    0x445f00: "frontend::dialogFadedIn",
    # The controls' screens: the function keys', the hardware keys', the
    # controllers' (control1), their dead zones and force feedback.
    0x437600: "frontend::funkeysOnEnter",
    0x4377c0: "frontend::funkeysOnExit",
    0x4377d0: "frontend::hardkeysOnEnter",
    0x43d4d0: "frontend::controlsOnEnter",
    0x43d5f0: "frontend::controlsOnExit",
    0x43d3f0: "frontend::controlsCancelKey",
    0x43d460: "frontend::controlsResetClicked",
    0x43d410: "frontend::controlsResetAnswered",
    0x43d9a0: "frontend::deadZoneFillList",
    0x4432c0: "frontend::deadZoneOnEnter",
    0x443430: "frontend::deadZoneOnExit",
    0x442db0: "frontend::deadZoneCancelKey",
    0x443260: "frontend::deadZoneChooseKey",
    0x443290: "frontend::deadZoneChooseKeyInRace",
    0x443070: "frontend::deadZoneShowController",
    0x442e60: "frontend::deadZoneAxisIndex",
    0x442dd0: "frontend::deadZoneApplyAxis",
    0x442ef0: "frontend::deadZoneAxisMoved",
    0x442ff0: "frontend::deadZoneAxisDraw",
    0x447dd0: "frontend::forceOnEnter",
    0x4481c0: "frontend::forceOnExit",
    0x447b40: "frontend::forceCancelKey",
    0x447b60: "frontend::forceStrengthMoved",
    0x447c40: "frontend::forceRoadEffectsMoved",
    0x447c70: "frontend::forceCollisionMoved",
    0x447ca0: "frontend::forceEngineMoved",
    0x447cd0: "frontend::forceBodyRollMoved",
    0x447d50: "frontend::forceRoadGripMoved",
    0x442a40: "frontend::menuFindItem",
    0x44a660: "frontend::menuShownNow",
    0x4446c0: "frontend::dialogFocusedButton",
}
MENU_NATIVES.update(menu_group_natives())
GAME_NATIVES.update(MENU_NATIVES)

# The game's code that uses MMX, native: each address is the port's function
# of that name in src/nfs3hp/native_mmx.cpp, in nfs3hp::mmx -- the SND
# library's MMX routines and the memmove the game's file reads copy with.
# Only 0x4ea4f0's and 0x4f09c0's emms are left generated.  tools/game_checks
# compares them with their generated code as well.
MIX_NATIVES = {
    0x5033e4: "mix::copySwapped",       # the memmove's copy, xchg edx, eax first
    0x5033e5: "mix::copy",              # its copy: eax to, edx from, ebx bytes
    0x51f5dd: "mix::addMono8",
    0x51f6c7: "mix::addStereo8",
    0x51f7ad: "mix::addMono16",
    0x51f897: "mix::addStereo16",
    0x520830: "mix::outputMono8",
    0x520870: "mix::outputStereo8",
    0x520920: "mix::outputStereo16",
    0x5209a0: "mix::clear",
    0x520a00: "mix::finished",
    0x520a10: "mix::outputMono16",
    0x5270f0: "mix::interpolateMono8",
    0x527170: "mix::interpolateStereo8",
    0x527210: "mix::interpolateMono16",
    0x527290: "mix::interpolateStereo16",
    0x52d1a0: "mix::xaMono",
    0x52d250: "mix::xaStereo",
}
GAME_NATIVES.update(MIX_NATIVES)

# The game asked the processor what it can do and timed it, to choose what to run
# on it.  The port's answers are constants (native_game.cpp): nfs3hp::removed.
REMOVED_NATIVES = {
    0x4f1c70: "removed::processorChecked",
    0x4f1ef0: "removed::processorSpeed",
}
GAME_NATIVES.update(REMOVED_NATIVES)

# EA's SND library, rewritten as the port's own C++ (src/nfs3hp/port_snd.h):
# each address is the game's entry of that name in nfs3hp::port, first those
# of port_snd_voices.cpp -- the voices, their volume, pan and pitch, the 100
# Hz tick, the platform layer over the mixer.  EA's names where the game's
# messages give them.  tools/game_checks compares them with their generated
# code as well; then the register-level natives of the rest of the sound
# (nfs3hp::snd).
SND_NATIVES = {
    0x4e0d88: "port::eaRandom",
    0x4e257c: "port::soundIdle",
    0x4e25c0: "port::iSNDstop",
    0x4e8fd0: "port::SNDstatus",
    0x4e9010: "port::SNDstopall",
    0x4e9420: "port::SNDautovol",
    0x4e9540: "port::setSoundVolume",
    0x4e95fc: "port::SNDvol",
    0x4e9670: "port::SNDstop",
    0x4e9710: "port::setSoundBend",
    0x4e97e4: "port::SNDpitchbend",
    0x4e9850: "port::SNDpos3d",
    0x4e9950: "port::SNDpan",
    0x4e9b20: "port::SNDplaysetdef",
    0x4e9ba0: "port::SNDplay",
    0x4e9bf8: "port::playOptions",
    0x4ff840: "port::voicePicked",
    0x4ff86c: "port::iSNDallocchan",
    0x4ffb64: "port::iSNDfreechan",
    0x4ffc9c: "port::voiceOf",
    0x500604: "port::iSNDmixstop",
    0x500654: "port::setSendVolume",
    0x5006a4: "port::setMixVolume",
    0x500c80: "port::soundTick",
    0x500e20: "port::iSNDenteraudio",
    0x500e7c: "port::iSNDleaveaudio",
    0x500e94: "port::iSNDserveradd100hzclient",
    0x500ef0: "port::iSNDserverremove100hzclient",
    0x5014a0: "port::updateVoiceVolume",
    0x501540: "port::iSNDvol",
    0x501670: "port::soundVolume",
    0x501938: "port::newGroup",
    0x502cb0: "port::updateVoicePitch",
    0x502d30: "port::iSNDplatformdetune",
    0x502df0: "port::iSNDplatform3dpos",
    0x502ef0: "port::iSNDpan",
    0x5222f0: "port::panSplit",
    0x522320: "port::panSurround",
    0x5224b0: "port::panLawPositive",
    0x5224e0: "port::randomOffset",
    0x5229d0: "port::nextTag",
    0x522e90: "port::pitchFactor",
    0x522f10: "port::mixStep",
    0x522f50: "port::setMixStep",
    0x523310: "port::setVoiceSend",
    0x52b1c0: "port::panLaw",
    0x52b230: "port::readBigEndian",
    0x52b500: "port::setSendLevel",
    # port_snd_play.cpp: patches, banks, a sample started on a channel.
    0x4e73d0: "port::SNDmastervol",
    0x4e8bd0: "port::removeBankPatches",
    0x4e8c1c: "port::removeAllBanks",
    0x4e8c28: "port::SNDbankremove",
    0x4e8d20: "port::SNDbankadd",
    0x4e8db0: "port::SNDbankheadersize",
    0x4e8e20: "port::SNDbankheadercopy",
    0x4fb1e0: "port::SNDbankpatpresent",
    0x5000b8: "port::iSNDmixstart",
    0x5017c0: "port::timbreDefaults",
    0x5017d8: "port::resetTimbre",
    0x5018dc: "port::addTimbreDeltas",
    0x501978: "port::iSNDplaytaggedtimbre",
    0x501de8: "port::iSNDplaytaggedpatch",
    0x5021ac: "port::iSNDresolvetaggedpatch",
    0x5022d0: "port::unresolveTaggedPatch",
    0x502330: "port::iSNDextracthdrinfo",
    0x5024a0: "port::iSNDbankremovepat",
    0x502570: "port::bankUsable",
    0x5025b0: "port::iSNDbankalloc",
    0x502620: "port::iSNDdownloadbank",
    0x522510: "port::iSNDplatformplay",
    0x522a80: "port::iSNDplatformresolve",
    0x522d08: "port::platformUnresolve",
    0x523d94: "port::iSNDplatformpacketplay",
    0x523f90: "port::iSNDcheckplayopts",
    # native_snd_stream.cpp: the system tasks, the top-up's coordination, the
    # SND streamer.
    0x4e7520: "port::addSystemTask",
    0x4e75e0: "port::removeSystemTask",
    0x4e7630: "port::runSystemTasks",
    0x4e7720: "port::waitSystemTasks",
    0x4ff660: "port::setTopupFunction",
    0x4ff670: "port::topUpStream",
    0x4ff780: "port::topupDone",
    0x4ff7b0: "port::toppedUp",
    0x4ff7c0: "port::topUpBeforeLoad",
    0x4ff7e0: "port::topupReturn",
    0x4ff7f0: "port::waitTopupStream",
    0x4ff700: "port::waitTopupMain",
    0x4ff76e: "port::waitTopupEvent",
    0x4ff820: "port::loadBlockSize",
    0x4ff830: "port::loadFlags",
    0x5016c0: "port::iSNDclearchunk",
    0x5017b4: "port::platformChunks",
    0x4e7470: "port::SNDstreamautovol",
    0x4e74f0: "port::SNDstreamgetvol",
    0x4e7750: "port::SNDstreamsetup",
    0x4e784c: "port::SNDstreamrestore",
    0x4e78a0: "port::SNDstreamqueue",
    0x4e7a14: "port::SNDstreaminterrupt",
    0x4e7b28: "port::SNDstreampurge",
    0x4e7c58: "port::SNDstreamchunksleft",
    0x4e7ce0: "port::SNDstreambuftime",
    0x4e7dd0: "port::streamToppedUp",
    0x4e7df4: "port::iSNDstreamchecktopup",
    0x4e7e98: "port::iSNDstreamsetbuftime",
    0x4e7f30: "port::parseStreamChunk",
    0x4e7ff8: "port::dropQueuedChunks",
    0x4e8078: "port::queueDataChunk",
    0x4e80cc: "port::iSNDparseheader",
    0x4e8258: "port::queueLoopChunk",
    0x4e82c4: "port::streamSeekChunk",
    0x4e82ec: "port::streamEndChunk",
    0x4e8320: "port::streamEndsSeen",
    0x4e8328: "port::nextStreamChunk",
    0x4e85a8: "port::iSNDstreamservice",
    0x4e8944: "port::resetStreamer",
    0x4e89f8: "port::iSNDstreamstart",
    0x4e8b60: "port::SNDstreamvol",
    # native_snd_cdstream.cpp: the CD stream and its reader thread.
    0x4e4420: "port::initStreamStruct",
    0x4e45d0: "port::initStreamStructSecond",
    0x4e45e0: "port::initStreamStructFirst",
    0x4e45f0: "port::initStream",
    0x4e4780: "port::initStreamSecond",
    0x4e4790: "port::initStreamFirst",
    0x4e4a30: "port::deleteStreamStruct",
    0x4e4b50: "port::deleteStream",
    0x4e4c60: "port::streamCommand",
    0x4e4d70: "port::purgeStreamCommand",
    0x4e4ea0: "port::streamOpen",
    0x4e4ec0: "port::streamOpenQuietly",
    0x4e4ee0: "port::streamOpenNext",
    0x4e4f00: "port::streamOpenNextQuietly",
    0x4e4f20: "port::streamContinue",
    0x4e4f40: "port::streamContinueQuietly",
    0x4e4f60: "port::streamSwitch",
    0x4e4f80: "port::streamSwitchQuietly",
    0x4e4fa0: "port::streamQueueFile",
    0x4e4fc0: "port::streamQueueFileQuietly",
    0x4e4fe0: "port::streamRestart",
    0x4e5000: "port::streamRestartQuietly",
    0x4e5020: "port::streamSeek",
    0x4e5040: "port::streamSeekQuietly",
    0x4e5060: "port::streamReadOn",
    0x4e5080: "port::streamReadOnQuietly",
    0x4e50a0: "port::streamStopAt",
    0x4e50c0: "port::streamStopAtQuietly",
    0x4e53c0: "port::streamReaderThread",
    0x4e6660: "port::passFreedChunks",
    0x4e66e0: "port::roomAhead",
    0x4e6710: "port::roomToEnd",
    0x4e6720: "port::roomAtStart",
    0x4e6740: "port::getStreamChunk",
    0x4e68a0: "port::freeStreamChunk",
    0x4e68c0: "port::streamBytesRead",
    0x4e68e0: "port::readerIdle",
    0x4e6950: "port::readerWaits",
    0x4e69e0: "port::chainCommandBlocks",
    0x4e6a20: "port::freeCommandBlock",
    0x4e6a70: "port::commandBlocksFree",
    0x4e6b30: "port::takeCommandBlock",
    # port_snd_path.cpp: Pathfinder.
    0x4e6be0: "port::pathStep",
    0x4e6d68: "port::pathTask",
    0x4e6d94: "port::pathHook",
    0x4e6dc8: "port::SNDpathinit",
    0x4e6f0c: "port::SNDpathstartstream",
    0x4e705c: "port::SNDpathcontrol",
    0x4e70fc: "port::SNDpathevent",
    0x4e7208: "port::SNDpatheventinterrupt",
    0x4e7388: "port::SNDpathrestore",
    # port_snd_strm.cpp: SNDSTRM (a movie's sound) and its packet player.
    0x503670: "port::mul64",
    0x503760: "port::div64",
    0x523d90: "port::platformPacketSize",
    0x4f3aa4: "port::findStream",
    0x4f3fd0: "port::streamCount",
    0x510a10: "port::findRequest",
    0x4f2f50: "port::SNDSTRM_requeststatus",
    0x4f3a10: "port::streamMemory",
    0x511210: "port::playerMemory",
    0x4f3a60: "port::SNDSTRM_createqueued",
    0x4f3a80: "port::SNDSTRM_requestdata",
    0x4f3a90: "port::SNDSTRM_restore",
    0x4f3ab8: "port::removeRequest",
    0x4f3b44: "port::packetFreed",
    0x4f3b60: "port::framesPlayed",
    0x4f3ba4: "port::iSNDstreamparseheader",
    0x4f3cc8: "port::iSNDstreamparseloop",
    0x4f3d00: "port::iSNDstreamparsedata",
    0x4f3dac: "port::iSNDstreamparseend",
    0x4f3dc4: "port::iSNDstreamparsechunk",
    0x4f3e1c: "port::streamWaits",
    0x4f3ebc: "port::iSNDstreamsystemtask",
    0x4f3fec: "port::iSNDstreamcreate",
    0x4f4258: "port::iSNDstreamrequest",
    0x4f43c0: "port::SNDSTRM_destroy",
    0x4f4440: "port::SNDSTRM_purge",
    0x511228: "port::SNDPKTPLAY_create",
    0x5112d0: "port::SNDPKTPLAY_start",
    0x5114d4: "port::SNDPKTPLAY_submit",
    0x5115b8: "port::SNDPKTPLAY_submitspace",
    0x51162c: "port::framesOutstanding",
    0x511680: "port::SNDPKTPLAY_framesoutstanding",
    0x511718: "port::SNDPKTPLAY_purge",
    0x51181c: "port::SNDPKTPLAY_stop",
    0x5118d0: "port::SNDPKTPLAY_destroy",
    0x5118fc: "port::nextPacket",
    0x511998: "port::packetFramesPlayed",
    0x5119b0: "port::patchDefaults",
    0x5119d8: "port::iSNDpatchtohdr",
    0x511bf0: "port::iSNDplatformcalcdatarate",
    0x511ca0: "port::streamChunksAtOnce",
    0x4f3ab5: "port::streamTaskSkipped",
    0x4f3b31: "port::framesPlayedDone",
    0x4ff80f: "port::waitTopupDone",
    # port_snd_driver.cpp: SNDinit, SNDrestore, the driver, the sound thread.
    0x4e217c: "port::iSNDunloaddll",
    0x4e2228: "port::SNDsetrate",
    0x4e1d1c: "port::leaveAudioLocks",
    0x4e225c: "port::iSNDmessageinit",
    0x4e2450: "port::iSNDinit",
    0x4e253c: "port::iSNDrestore",
    0x4e2754: "port::iSNDverifymode",
    0x4e2630: "port::SNDinit",
    0x4e26b4: "port::SNDrestore",
    0x48ec40: "port::soundThread",
    0x48ec80: "port::setSoundServer",
    # port_timer.cpp: EA's timer (the game's clock): its thread, the multimedia
    # timer's callback, the registration of the functions it runs, its start and stop.
    0x48eb90: "port::timerThread",
    0x48ed00: "port::clearTimerCallbacks",
    0x48ed30: "port::timerTick",
    0x48ee30: "port::addTimerCallback",
    0x48ee90: "port::removeTimerCallback",
    0x48ef50: "port::stopTimer",
    0x48eff0: "port::startTimer",
    # port_snd_fx.cpp: the effects, the reverb's start, the mixer's start.
    0x523200: "port::reverbLength",
    0x52325c: "port::reverbInit",
    0x502ff0: "port::iSNDwinsetfx",
    0x5030ac: "port::setBusFx",
    0x5030c4: "port::busLevelSet",
    0x5030d0: "port::SNDfxlevel",
    0x4e9c70: "port::SNDfxinitbus",
    0x4e9dfc: "port::SNDfxmasterlevel",
    0x500008: "port::iSNDmixreinit",
    0x4ffd84: "port::mixFunctions",
    0x4fff18: "port::iSNDmixinit",
    # port_snd_mixer.cpp: the mix and a channel's ramp, on the native mixer.
    0x500864: "port::mixerServe",
    0x5004c8: "port::mixerRamp",
    # port_snd_mixsetup.cpp: what sets a mixer channel up, MMX set.
    0x5274b0: "port::pcmOnceMono",
    0x527540: "port::pcmOnceStereo",
    0x5276f0: "port::pcmOnceStereo2",
    0x527780: "port::pcmOnceQuad",
    0x5274d0: "port::pcmLoopedMono",
    0x527560: "port::pcmLoopedStereo",
    0x527710: "port::pcmLoopedStereo2",
    0x5277a0: "port::pcmLoopedQuad",
    0x527500: "port::pcmStreamedMono",
    0x527590: "port::pcmStreamedStereo",
    0x527740: "port::pcmStreamedStereo2",
    0x5277d0: "port::pcmStreamedQuad",
    0x52c9a0: "port::pcmOnceSetup",
    0x52ca48: "port::pcmLoopedSetup",
    0x52cb74: "port::pcmStreamedSetup",
    0x529104: "port::xaOnceSetup",
    0x529534: "port::xaIntroSetup",
    0x52a354: "port::xaStereoStreamSetup",
    0x52d160: "port::packetPlayerOf",
    0x52a5b4: "port::xaStereoFileSetup",
    0x51f750: "port::volumeMonoSetup",
    0x51f850: "port::volumeStereoSetup",
    0x5203a0: "port::resampleMonoSetup",
    0x5207e4: "port::resampleStereoSetup",
    0x5202fc: "port::resampleMonoStep",
    0x520740: "port::resampleStereoStep",
    0x521d80: "port::pcmTablesFirst",
    0x521e60: "port::pcmTablesSecond",
    0x521f30: "port::xaFloatTable",
    0x521f7c: "port::xaIntTable",
    0x521fc0: "port::xaTables",
    0x5220a0: "port::lastTables",
    0x5230b8: "port::reverbStart",
    # The game's own sound: its music (native_snd_music.cpp), its effects and
    # the race's sound slots (port_sound_effects.cpp, rewritten as C++ of the port's own).
    0x40feb0: "port::soundExit",
    0x40fed0: "port::deleteMusicCopy",
    0x40ffd0: "port::raceTuneOf",
    0x410030: "port::raceMusicPrepare",
    0x410330: "port::musicStreamMake",
    0x410370: "port::musicStreamMakeOf",
    0x4103a0: "port::musicAfterSetting",
    0x410460: "port::musicMenuSix",
    0x4104f0: "port::musicStart",
    0x4107c0: "port::raceMusicStart",
    0x4109d0: "port::musicStop",
    0x410aa0: "port::musicStreamEnd",
    0x410ad0: "port::banksFree",
    0x410b20: "port::banksRemove",
    0x410b50: "port::bankLoad",
    0x416f60: "port::slotStop",
    0x416fc0: "port::slotEffect",
    0x417080: "port::slotPlay",
    0x4181d0: "port::playMenuEffect",
    0x418280: "port::playCarEffectFive",
    0x4182b0: "port::playEffect",
    0x4184f0: "port::playSpeechEffect",
    0x4185e0: "port::panFromSide",
    0x418610: "port::masterSilent",
    0x418620: "port::masterBack",
    0x4186a0: "port::silenceForMenus",
    0x4186f0: "port::silenceForRace",
    0x418740: "port::busInit",
    0x418770: "port::busUp",
    0x418790: "port::busDown",
    0x4187a0: "port::slotsStop",
    0x4187e0: "port::musicMuteToggle",
    0x4e8f28: "port::SNDangle",
    0x5008a0: "port::convert16",
    0x500910: "port::convertToWords",
    0x500990: "port::convertXaStereo",
    0x500b10: "port::convert8",
    0x500b90: "port::convertXaMono",
    0x52a610: "port::framesPlayedCoded",
    0x52a618: "port::codedToWords",
    0x52a668: "port::codedDecode",
    0x52a790: "port::codedSetup",
    0x52a7e0: "port::codedStreamDecode",
    0x52a970: "port::codedStreamSetup",
    0x52a9c0: "port::codedPacketDecode",
    0x52ab70: "port::codedPacketSetup",
    0x52d370: "port::codedBits",
    0x52d3cc: "port::codedSkip",
    0x52d428: "port::codedPulses",
    0x52d5cc: "port::codedInterpolate",
    0x52d618: "port::codedFilter",
    0x52d6d8: "port::codedSynthesis",
    0x52de94: "port::codedStart",
    0x52df80: "port::codedFrame",
    0x52a1a0: "port::xaStereoStreamDecode",
    0x52a3a0: "port::xaStereoPacketDecode",
    0x529cfc: "port::xaStereoLoopDecode",
    0x529f24: "port::xaStereoIntroDecode",
    0x52a144: "port::xaStereoIntroSetup",
    0x529a50: "port::xaStereoPlayed",
    0x529a54: "port::xaStereoOnceDecode",
    0x529c2c: "port::xaStereoOnceSetup",
    0x529c80: "port::xaStereoKeptCopy",
    0x5297c0: "port::xaPacketDecode",
    0x5299f4: "port::xaPacketSetup",
    0x529384: "port::xaIntroDecode",
    0x529590: "port::xaStreamDecode",
    0x529778: "port::xaStreamSetup",
    0x528f70: "port::xaPlayed",
    0x528f74: "port::xaOnceDecode",
    0x529140: "port::xaKeptCopy",
    0x5291a8: "port::xaLoopDecode",
    0x52c8f0: "port::pcmPlayed",
    0x52c8f4: "port::pcmOnceDecode",
    0x52c9d0: "port::pcmLoopedDecode",
    0x52ca80: "port::pcmStreamedDecode",
    0x52cba0: "port::pcmFileDecode",
    0x52ccb8: "port::pcmFileSetup",
    0x527520: "port::pcmFileMono",
    0x5275b0: "port::pcmFileStereo",
    0x527760: "port::pcmFileStereo2",
    0x5277f0: "port::pcmFileQuad",
    0x4ffce0: "port::bufferPosition",
    0x51f520: "port::panSides",
    0x51f580: "port::volumeMonoPair",
    0x51f5a4: "port::volumeMonoLevels",
    0x51f680: "port::volumeStereoPair",
    0x51f6a4: "port::volumeStereoLevels",
    0x51f774: "port::volumeMonoLevelsMmx",
    0x51f874: "port::volumeStereoLevelsMmx",
    0x51fc38: "port::resample8MonoSetup",
    0x51fb94: "port::resample8MonoStep",
    0x51f930: "port::resample8MonoNeededDown",
    0x51f978: "port::resample8MonoNeededSame",
    0x51f980: "port::resample8MonoNeededUp",
    0x51fa80: "port::resample8MonoSame",
    0x51f9ac: "port::resample8MonoDown",
    0x51fa8c: "port::resample8MonoUp",
    0x52002c: "port::resample8StereoSetup",
    0x51ff88: "port::resample8StereoStep",
    0x51fc80: "port::resample8StereoNeededDown",
    0x51fcc8: "port::resample8StereoNeededSame",
    0x51fcd0: "port::resample8StereoNeededUp",
    0x51fe24: "port::resample8StereoSame",
    0x51fcfc: "port::resample8StereoDown",
    0x51fe30: "port::resample8StereoUp",
    0x520070: "port::resample16MonoNeededDown",
    0x5200b8: "port::resample16MonoNeededSame",
    0x5200c0: "port::resample16MonoNeededUp",
    0x5201d8: "port::resample16MonoSame",
    0x5200ec: "port::resample16MonoDown",
    0x5201e4: "port::resample16MonoUp",
    0x5203f0: "port::resample16StereoNeededDown",
    0x520438: "port::resample16StereoNeededSame",
    0x520440: "port::resample16StereoNeededUp",
    0x5205b0: "port::resample16StereoSame",
    0x52046c: "port::resample16StereoDown",
    0x5205bc: "port::resample16StereoUp",
    0x4c8370: "port::speechInit",
    0x4c8440: "port::speechFree",
    0x4c85a0: "port::speechLine",
    0x415210: "port::scenerySounds",
    0x413bc0: "port::raceBanksLoad",
    0x412d60: "port::raceAnnouncer",
    0x410e30: "port::trackSounds",
    0x4c8d10: "port::coDriverTick",
    0x417390: "port::carEngine",
    0x411e60: "port::copCarSound",
    0x4117a0: "port::playerCarSound",
    0x412220: "port::copSourcesPick",
    0x411410: "port::raceSoundTick",
    0x415050: "port::zoneSound",
    0x413750: "port::voiceBankLoad",
    0x4138e0: "port::carBankLoad",
    0x4124d0: "port::placeAhead",
    0x412620: "port::placeAcross",
    0x4127c0: "port::carRadio",
    0x412880: "port::playerCopRadio",
    0x412be0: "port::copLineFor",
    0x4121b0: "port::copRemove",
    0x4127b0: "port::carFlagTest",
    0x4129a0: "port::raceValue",
    0x4129b0: "port::soundNothing",
    0x414800: "port::slotsEmpty",
    0x4136e0: "port::raceBankLoad",
    0x4129c0: "port::raceSoundInit",
    0x412b50: "port::raceSoundEnd",
    0x411620: "port::sourceReset",
    0x411680: "port::sourcesReset",
    0x411750: "port::copChaseSet",
    0x411c50: "port::copSiren",
    0x411ca0: "port::copSirenStop",
    0x411cb0: "port::copEngine",
    0x411da0: "port::copEngineStop",
    0x411db0: "port::copDistance",
    0x411e20: "port::copsCloseness",
    0x410c40: "port::listenerMove",
    0x410cb0: "port::sourceDistance",
    0x410d30: "port::sourceAngle",
    0x410dc0: "port::carAngle",
    0x4188a0: "port::copBeepTick",
    0x418930: "port::copBeepStart",
    0x418c30: "port::copEffect",
    0x418c60: "port::musicEventResume",
    0x418990: "port::chaseLevel",
    0x4c9240: "snd::descriptionLoad",
    0x4c9340: "port::descriptionSpeak",
    0x4c9380: "port::descriptionTick",
    0x4c9490: "port::descriptionStop",
    0x4c95e0: "port::spokenStatus",
    0x4c99f0: "port::spokenStatus",
    0x4c95f0: "port::descriptionStart",
    0x4c9860: "port::descriptionService",
    0x4c9890: "port::raceVoiceLoad",
    0x4c9990: "port::carLineSpeak",
    0x4c99d0: "port::spokenStop",
    0x4c9a00: "port::raceVoiceFree",
    0x4c86d0: "port::speechTick",
    0x4c8810: "port::radioTick",
    0x4c89b0: "port::radioStart",
    0x4c8ac0: "port::speechSaid",
    0x4c8b30: "port::copRadio",
    0x4c8ca0: "port::queueSpeechLine",
    0x4c8f90: "port::queueRaceLine",
    0x4c9010: "port::raceEventLine",
    0x4c9090: "port::gapLine",
    0x4c91a0: "port::lapTimeLine",
    0x414830: "port::engineSoundsStart",
    0x414e10: "port::farEffectAim",
    0x414ef0: "port::farEffectPlay",
    0x414f60: "port::farEffectTick",
    0x414f90: "port::crowdEffect",
    0x415010: "port::carEffectFifteen",
    0x414940: "port::crashPitch",
    0x4149e0: "port::crashEffect",
    0x414d70: "port::scrapeEffect",
    0x416d90: "port::carKnock",
    0x417b80: "port::noiseByDistance",
    0x417c00: "port::playerNoise",
    0x417c70: "port::playerNoiseStop",
    0x417ca0: "port::sirenIsOn",
    0x417cb0: "port::sirenFirstTone",
    0x417db0: "port::sirenSecondTone",
    0x417e00: "port::sirenSteadyTone",
    0x417d00: "port::sirenFirstDone",
    0x417e60: "port::sirenSecondDone",
    0x417eb0: "port::sirenStart",
    0x417f60: "port::sirenStop",
    0x417fd0: "port::sirenTick",
    0x4179d0: "port::noiseBySpeed",
}
# EA SND's mixer code that never runs on the port: a stub each that stops the
# game (src/nfs3hp/native_snd_dead.inc, native_snd_dead.cpp), no code generated.
for _line in open(__import__("os").path.join(__import__("os").path.dirname(__import__("os").path.abspath(__file__)),
                                     "src/nfs3hp/native_snd_dead.inc")).read().splitlines():
    if _line.startswith("DEAD("):
        SND_NATIVES[int(_line[5:-1], 16)] = "snd::dead_%s" % _line[5:-1]
GAME_NATIVES.update(SND_NATIVES)

# The graphics code that never runs on the port's driver, voodoo2a: a stub each
# that stops the game (src/nfs3hp/native_graphics_dead.inc,
# native_graphics_dead.cpp), no code generated.
GRAPHICS_DEAD = {}
for _line in open(__import__("os").path.join(__import__("os").path.dirname(__import__("os").path.abspath(__file__)),
                                     "src/nfs3hp/native_graphics_dead.inc")).read().splitlines():
    if _line.startswith("DEAD("):
        GRAPHICS_DEAD[int(_line[5:-1], 16)] = "graphics::dead_%s" % _line[5:-1]
GAME_NATIVES.update(GRAPHICS_DEAD)

# The code nothing can reach -- no call, jump, pointer or address of it
# anywhere that runs, proved by tools/audit_dead.py: a stub each that stops
# the game (src/nfs3hp/native_dead.inc, native_dead.cpp), no code generated.
GAME_DEAD = {}
for _line in open(__import__("os").path.join(__import__("os").path.dirname(__import__("os").path.abspath(__file__)),
                                     "src/nfs3hp/native_dead.inc")).read().splitlines():
    if _line.startswith("DEAD("):
        _address = _line[5:_line.index(")")]
        GAME_DEAD[int(_address, 16)] = "dead::stub_%s" % _address
GAME_NATIVES.update(GAME_DEAD)

# Functions decompiled one at a time (src/nfs3hp/native_decompiled.cpp, and
# native_graphics.cpp for the graphics'): each
# address of src/nfs3hp/decompiled.json is the function of its name there, in
# nfs3hp::decompiled; tools/game_checks holds each against the game's code on
# made-up values of the globals the list names for it.
import json as _json
import os as _os


def decompiled_list():
    path = _os.path.join(_os.path.dirname(_os.path.abspath(__file__)), "src", "nfs3hp", "decompiled.json")
    with open(path, encoding="utf-8") as f:
        return _json.load(f)


def decompiled_name(entry):
    """An entry's function as the code calls it: in nfs3hp::decompiled, or in
    the namespace the entry names (graphics: native_graphics.cpp)."""
    return "%s::%s" % (entry.get("namespace", "decompiled"), entry["name"])


DECOMPILED_NATIVES = {int(entry["address"], 16): decompiled_name(entry) for entry in decompiled_list()}
GAME_NATIVES.update(DECOMPILED_NATIVES)

# eacsnd.dll's data inside its code (tools/sound_checks/generate.py makes it
# the same way).
EACSND_DATA_SEGMENTS = [(0xa35749, 0xa3575f), (0xa372bf, 0xa372dd)]

# eacsnd.dll is native: each export is the port's function of that name in
# src/nfs3hp/native_sound.cpp (the driver) and port_snd_isnd.cpp (the rest), and
# none of its code is generated.  tools/sound_checks generates it all the same,
# to compare the natives with.
EACSND_NATIVES = {
    "_iSNDdllversion@0": "iSNDdllversion",
    "_iSNDdirectsetfunctions@32": "iSNDdirectsetfunctions",
    "_iSNDdirectcaps@4": "iSNDdirectcaps",
    "_iSNDdirectstart@8": "iSNDdirectstart",
    "_iSNDdirectstop@0": "iSNDdirectstop",
    "_iSNDdirectrecordstart@16": "iSNDdirectrecordstart",
    "_iSNDdirectrecordpacket@4": "iSNDdirectrecordpacket",
    "_iSNDdirectrecordstop@0": "iSNDdirectrecordstop",
    "_iSNDdirectserve@0": "iSNDdirectserve",
    "_iSNDdirectvol@8": "iSNDdirectvol",
    "_iSNDdirectpos3d@12": "iSNDdirectpos3d",
    "_iSNDdirectrate@8": "iSNDdirectrate",
    "_iSNDdirectcreate3dbuf@28": "iSNDdirectcreate3dbuf",
    "_iSNDdirectremovebuf@4": "iSNDdirectremovebuf",
    "_iSNDdirectplay3d@28": "iSNDdirectplay3d",
    "_iSNDdirectstopbuf@4": "iSNDdirectstopbuf",
}

# voodoo2a.dll, the Voodoo2 driver behind THRASH: data in its code, and two
# functions that are a bare ret after another (THRASH_flushwindow, THRASH_idle),
# for the disassembly to find its way through it.
VOODOO2A_DATA_SEGMENTS = [(0xa83204, 0xa83210), (0xA86833, 0xA868A2)]
VOODOO2A_KNOWN_SUBROUTINES = [0xA847D0, 0xA84810]

# voodoo2a is the port's own: each export is the port's function of that name
# in src/nfs3hp/port_thrash.cpp, and none of its code is generated.  Of its own
# functions only one was handed out to be called: sub_a84360, which its
# THRASH_setvideomode gave the game's window thread to run on message 0x465.
# The port's driver hands it to nobody; the address stays bound all the same.
VOODOO2A_NATIVES = {
    "_THRASH_about@0": "THRASH_about",
    "_THRASH_clearwindow@0": "THRASH_clearwindow",
    "_THRASH_clip@16": "THRASH_clip",
    "_THRASH_drawline@8": "THRASH_drawline",
    "_THRASH_drawlinemesh@12": "THRASH_drawlinemesh",
    "_THRASH_drawlinestrip@8": "THRASH_drawlinestrip",
    "_THRASH_drawpoint@4": "THRASH_drawpoint",
    "_THRASH_drawpointmesh@12": "THRASH_drawpointmesh",
    "_THRASH_drawpointstrip@8": "THRASH_drawpointstrip",
    "_THRASH_drawquad@16": "THRASH_drawquad",
    "_THRASH_drawquadmesh@12": "THRASH_drawquadmesh",
    "_THRASH_drawtri@12": "THRASH_drawtri",
    "_THRASH_drawtrifan@8": "THRASH_drawtrifan",
    "_THRASH_drawtrimesh@12": "THRASH_drawtrimesh",
    "_THRASH_drawtristrip@8": "THRASH_drawtristrip",
    "_THRASH_flushwindow@0": "THRASH_flushwindow",
    "_THRASH_idle@0": "THRASH_idle",
    "_THRASH_init@0": "THRASH_init",
    "_THRASH_is@0": "THRASH_is",
    "_THRASH_lockwindow@0": "THRASH_lockwindow",
    "_THRASH_pageflip@0": "THRASH_pageflip",
    "_THRASH_readrect@20": "THRASH_readrect",
    "_THRASH_restore@0": "THRASH_restore",
    "_THRASH_selectdisplay@4": "THRASH_selectdisplay",
    "_THRASH_setstate@8": "THRASH_setstate",
    "_THRASH_settexture@4": "THRASH_settexture",
    "_THRASH_setvideomode@12": "THRASH_setvideomode",
    "_THRASH_sync@4": "THRASH_sync",
    "_THRASH_talloc@20": "THRASH_talloc",
    "_THRASH_treset@0": "THRASH_treset",
    "_THRASH_tupdate@12": "THRASH_tupdate",
    "_THRASH_unlockwindow@4": "THRASH_unlockwindow",
    "_THRASH_window@4": "THRASH_window",
    "_THRASH_writerect@20": "THRASH_writerect",
}
VOODOO2A_CALLBACKS = {0xa84360: "THRASH_modemessage"}

if __name__ == '__main__':
    application = disassembler.disassemble('nfs3hp', 'nfs3hp/nfs3.exe', DATA_SEGMENTS,
                                           KNOWN_SUBROUTINES, SPLIT_INSTRUCTIONS,
                                           THREAD_ROUTINES, THREAD_SEGMENTS, MERGE_ROUTINES)
    eacsnd = disassembler.disassemble('eacsnd', 'nfs3hp/eacsnd.dll', type=dll.DLL,
                                      data_segments=EACSND_DATA_SEGMENTS, rebase_after=application)
    eacsnd.native('nfs3hp', EACSND_NATIVES)
    # softtria.dll, the software renderer, is not on the port: the game loads
    # one renderer, the one the registry's "Thrash Driver" names (sub_4b5720),
    # unless a command-line option names another, and the port sets voodoo2 and
    # gives no options.  So none of it is built -- no code, no data, no library,
    # and LoadLibrary("softtria.dll") would fail as it does with the file
    # missing.  It is read only for where voodoo2a goes after it.
    softtria = dll.DLL('softtria', 'nfs3hp/softtria.dll', eacsnd.sections[-1][1] + eacsnd.sections[-1][2])
    voodoo2a = disassembler.disassemble('voodoo2a', 'nfs3hp/voodoo2a.dll', type=dll.DLL,
                                        data_segments=VOODOO2A_DATA_SEGMENTS,
                                        known_subroutines=VOODOO2A_KNOWN_SUBROUTINES, rebase_after=softtria)
    voodoo2a.native('nfs3hp', VOODOO2A_NATIVES, VOODOO2A_CALLBACKS)
    application.write(THREAD_SEGMENTS, skip_instructions=SKIP_INSTRUCTIONS, dlls=[eacsnd, voodoo2a],
                      natives=GAME_NATIVES)
    from pathlib import Path
    for stale in Path('src/nfs3hp/disassembly').glob('softtria.*'):
        stale.unlink()  # what an earlier run wrote of softtria

    # Reapply the reviewed port heap budget after regenerating the original code.
    from tools.apply_heap_budget import apply
    apply(Path(__file__).resolve().parent)

    # tools/apply_glide_output.py is deliberately NOT applied.  Advertising a
    # 32-bit output depth blacked out every race on device while leaving the
    # resolution list still reading 16-bit -- the table is rewritten after the
    # game has already built that list, and read again by something later.
    # Confirmed by bisection: reverting these two files alone brought the
    # picture back.  The tool is kept for whoever picks the work up again; its
    # half for voodoo2a, whose code is no longer generated, would now go into
    # THRASH_about and measureModes in src/nfs3hp/port_thrash.cpp.





    # Car detail: every car's texture at the player's size, a larger transform
    # buffer and wheel spin kept per car, so split screen and the mirror match.
    from tools.apply_car_detail import apply as apply_car_detail
    apply_car_detail(Path(__file__).resolve().parent)

    # View Distance: Full by default and 500 in every view, split screen and
    # night included; the other settings stay the game's own.
    from tools.apply_track_detail import apply as apply_track_detail
    apply_track_detail(Path(__file__).resolve().parent)

    # The rear-view mirror at the main view's distances and car detail.
    from tools.apply_mirror_detail import apply as apply_mirror_detail
    apply_mirror_detail(Path(__file__).resolve().parent)

    # Race state: which of the two on-screen layouts belongs on the screen.
    from tools.apply_race_state import apply as apply_race_state
    apply_race_state(Path(__file__).resolve().parent)

    # HUD scale: the parts of a HUD element keep their proportions on a wide
    # screen instead of growing with its width.
    from tools.apply_hud_scale import apply as apply_hud_scale
    apply_hud_scale(Path(__file__).resolve().parent)

    # Cabin fit: the in-car dashboard and wheel keep their shape on a wide screen.
    from tools.apply_cabin_fit import apply as apply_cabin_fit
    apply_cabin_fit(Path(__file__).resolve().parent)

    # HUD editor: an element is framed, moved and checked for room as it is
    # drawn, and written back to the layout as laid out.
    from tools.apply_hud_editor import apply as apply_hud_editor
    apply_hud_editor(Path(__file__).resolve().parent)

    # Loading screen: 4:3 in the middle of a wide screen, as the Modern Patch
    # shows it.
    from tools.apply_loading_screen import apply as apply_loading_screen
    apply_loading_screen(Path(__file__).resolve().parent)

    from tools.apply_movie_tap import apply as apply_movie_tap
    apply_movie_tap(Path(__file__).resolve().parent)

    # Alpha intensity: the Advanced Graphics slider, which the original applies
    # on its Direct3D driver only.
    from tools.apply_alpha_intensity import apply as apply_alpha_intensity
    apply_alpha_intensity(Path(__file__).resolve().parent)

    # Widescreen: a 16:9 screen size in the list, Hor+ cameras, and the
    # interface fitted to the screen it is drawn on.
    from tools.apply_widescreen import apply as apply_widescreen
    apply_widescreen(Path(__file__).resolve().parent)

    # View Angle's third choice, 105 degrees, as the Modern Patch has it.
    from tools.apply_view_angle import apply as apply_view_angle
    apply_view_angle(Path(__file__).resolve().parent)



    # Force feedback: player one's car keeps its effects when the on-screen
    # controls are bound as keys, which leaves no axis on the pad to claim them.
    # tools/apply_force_feedback_car.py's change is in sub_4710f0's native now
    # (native_controls_race.cpp's forcePlayerCar).

    # Full colour: the Screen Size list shows the depth a race is drawn in (the
    # Voodoo2 driver's taking the game's 32-bit textures is port_thrash.cpp's).
    from tools.apply_full_colour import apply as apply_full_colour
    apply_full_colour(Path(__file__).resolve().parent)


    # Hooks that only watch the game's code (the [VIEW] arena peak, the AI's
    # lane votes, the comm library's traces); the vertex loops that were
    # entered here are VERTICES_NATIVES.
    from tools.apply_native_vertices import apply as apply_native_vertices
    apply_native_vertices(Path(__file__).resolve().parent)


    # The loading native: RefPack, the FSH textures' alpha scans, an FCE's
    # polygon records and a track's FRD.
    from tools.apply_native_loading import apply as apply_native_loading
    apply_native_loading(Path(__file__).resolve().parent)

    # The specs file's text native (sub_4b22d0), which nothing in the game calls.
    from tools.apply_native_specs_file import apply as apply_native_specs_file
    apply_native_specs_file(Path(__file__).resolve().parent)

    # The movies native: the MAD player, decoder and display (decomp/mad), after
    # apply_movie_tap's MovieSession.
    from tools.apply_native_movie import apply as apply_native_movie
    apply_native_movie(Path(__file__).resolve().parent)

    # The menus decompiled a screen at a time, the generated code kept behind
    # them: first the main menu.
