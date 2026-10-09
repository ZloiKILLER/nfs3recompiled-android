==== sub_4df2b0 .. 4df310
004df2b0  53                       push ebx
004df2b1  51                       push ecx
004df2b2  52                       push edx
004df2b3  56                       push esi
004df2b4  57                       push edi
004df2b5  55                       push ebp
004df2b6  89e5                     mov ebp, esp
004df2b8  89c3                     mov ebx, eax
004df2ba  833dd4a28c0002           cmp dword ptr [0x8ca2d4], 2
004df2c1  7405                     je 0x4df2c8
004df2c3  e8b8fbffff               call 0x4dee80
004df2c8  833dd4a28c0002           cmp dword ptr [0x8ca2d4], 2
004df2cf  752e                     jne 0x4df2ff
004df2d1  e84aba0000               call 0x4ead20
004df2d6  8b35cca28c00             mov esi, dword ptr [0x8ca2cc]
004df2dc  56                       push esi
004df2dd  8b3dc4a28c00             mov edi, dword ptr [0x8ca2c4]
004df2e3  8b0dd0a28c00             mov ecx, dword ptr [0x8ca2d0]
004df2e9  57                       push edi
004df2ea  8b15dca28c00             mov edx, dword ptr [0x8ca2dc]
004df2f0  a1e0a28c00               mov eax, dword ptr [0x8ca2e0]
004df2f5  e896e90100               call 0x4fdc90
004df2fa  e891ba0000               call 0x4ead90
004df2ff  5d                       pop ebp
004df300  5f                       pop edi
004df301  5e                       pop esi
004df302  5a                       pop edx
004df303  59                       pop ecx
004df304  5b                       pop ebx
004df305  c3                       ret 
004df306  8d8000000000             lea eax, [eax]
004df30c  8d542200                 lea edx, [edx]

