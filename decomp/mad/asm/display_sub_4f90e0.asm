==== sub_4f90e0 .. 4f9120
004f90e0  83ec70                   sub esp, 0x70
004f90e3  833d2850560000           cmp dword ptr [0x565028], 0
004f90ea  741c                     je 0x4f9108
004f90ec  833dcc7d560000           cmp dword ptr [0x567dcc], 0
004f90f3  741c                     je 0x4f9111
004f90f5  89e0                     mov eax, esp
004f90f7  e8d4b5ffff               call 0x4f46d0
004f90fc  e88f1cffff               call 0x4ead90
004f9101  89e0                     mov eax, esp
004f9103  e8f8b5ffff               call 0x4f4700
004f9108  b801000000               mov eax, 1
004f910d  83c470                   add esp, 0x70
004f9110  c3                       ret 
004f9111  b8d0389f00               mov eax, 0x9f38d0
004f9116  e81542ffff               call 0x4ed330
004f911b  83c470                   add esp, 0x70
004f911e  c3                       ret 
004f911f  90                       nop 

