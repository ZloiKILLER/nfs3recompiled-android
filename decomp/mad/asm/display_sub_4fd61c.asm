==== sub_4fd61c .. 4fd6ef
004fd61c  51                       push ecx
004fd61d  56                       push esi
004fd61e  891dd8765600             mov dword ptr [0x5676d8], ebx
004fd624  89c6                     mov esi, eax
004fd626  57                       push edi
004fd627  55                       push ebp
004fd628  8b06                     mov eax, dword ptr [esi]
004fd62a  31db                     xor ebx, ebx
004fd62c  88c3                     mov bl, al
004fd62e  89c7                     mov edi, eax
004fd630  c1ef18                   shr edi, 0x18
004fd633  2500ff0000               and eax, 0xff00
004fd638  c1e808                   shr eax, 8
004fd63b  8b2c9d18629f00           mov ebp, dword ptr [ebx*4 + 0x9f6218]
004fd642  8b3cbd185e9f00           mov edi, dword ptr [edi*4 + 0x9f5e18]
004fd649  83c604                   add esi, 4
004fd64c  8b048518609f00           mov eax, dword ptr [eax*4 + 0x9f6018]
004fd653  01ef                     add edi, ebp
004fd655  01c5                     add ebp, eax
004fd657  01f8                     add eax, edi
004fd659  89c1                     mov ecx, eax
004fd65b  89c7                     mov edi, eax
004fd65d  c1e817                   shr eax, 0x17
004fd660  81e100f80f00             and ecx, 0xff800
004fd666  c1e90b                   shr ecx, 0xb
004fd669  81e7ff010000             and edi, 0x1ff
004fd66f  8b048550599f00           mov eax, dword ptr [eax*4 + 0x9f5950]
004fd676  8a5efe                   mov bl, byte ptr [esi - 2]
004fd679  8b0c8d38559f00           mov ecx, dword ptr [ecx*4 + 0x9f5538]
004fd680  8b3cbdb04f9f00           mov edi, dword ptr [edi*4 + 0x9f4fb0]
004fd687  01c8                     add eax, ecx
004fd689  8b1c9d185e9f00           mov ebx, dword ptr [ebx*4 + 0x9f5e18]
004fd690  01f8                     add eax, edi
004fd692  01eb                     add ebx, ebp
004fd694  89c5                     mov ebp, eax
004fd696  89d8                     mov eax, ebx
004fd698  89df                     mov edi, ebx
004fd69a  81e300f80f00             and ebx, 0xff800
004fd6a0  c1e817                   shr eax, 0x17
004fd6a3  81e7ff010000             and edi, 0x1ff
004fd6a9  c1eb0b                   shr ebx, 0xb
004fd6ac  8b0dd8765600             mov ecx, dword ptr [0x5676d8]
004fd6b2  8b048550599f00           mov eax, dword ptr [eax*4 + 0x9f5950]
004fd6b9  8b3cbdb04f9f00           mov edi, dword ptr [edi*4 + 0x9f4fb0]
004fd6c0  8b1c9d38559f00           mov ebx, dword ptr [ebx*4 + 0x9f5538]
004fd6c7  01f8                     add eax, edi
004fd6c9  01d8                     add eax, ebx
004fd6cb  81e5ffff0000             and ebp, 0xffff
004fd6d1  250000ffff               and eax, 0xffff0000
004fd6d6  09e8                     or eax, ebp
004fd6d8  49                       dec ecx
004fd6d9  890dd8765600             mov dword ptr [0x5676d8], ecx
004fd6df  8902                     mov dword ptr [edx], eax
004fd6e1  8d5204                   lea edx, [edx + 4]
004fd6e4  0f853effffff             jne 0x4fd628
004fd6ea  5d                       pop ebp
004fd6eb  5f                       pop edi
004fd6ec  5e                       pop esi
004fd6ed  59                       pop ecx
004fd6ee  c3                       ret 

