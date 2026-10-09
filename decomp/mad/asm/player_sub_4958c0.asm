==== sub_4958c0 .. 495920
004958c0  53                       push ebx
004958c1  51                       push ecx
004958c2  52                       push edx
004958c3  56                       push esi
004958c4  57                       push edi
004958c5  55                       push ebp
004958c6  89e5                     mov ebp, esp
004958c8  8b0d54f37900             mov ecx, dword ptr [0x79f354]
004958ce  89c2                     mov edx, eax
004958d0  89c8                     mov eax, ecx
004958d2  8b1ccd94f27900           mov ebx, dword ptr [ecx*8 + 0x79f294]
004958d9  85c0                     test eax, eax
004958db  7e20                     jle 0x4958fd
004958dd  8d34c500000000           lea esi, [eax*8]
004958e4  3b9688f27900             cmp edx, dword ptr [esi + 0x79f288]
004958ea  7f11                     jg 0x4958fd
004958ec  8dbe90f27900             lea edi, [esi + 0x79f290]
004958f2  8db688f27900             lea esi, [esi + 0x79f288]
004958f8  48                       dec eax
004958f9  a5                       movsd dword ptr es:[edi], dword ptr [esi]
004958fa  a5                       movsd dword ptr es:[edi], dword ptr [esi]
004958fb  ebdc                     jmp 0x4958d9
004958fd  8914c590f27900           mov dword ptr [eax*8 + 0x79f290], edx
00495904  41                       inc ecx
00495905  891cc594f27900           mov dword ptr [eax*8 + 0x79f294], ebx
0049590c  890d54f37900             mov dword ptr [0x79f354], ecx
00495912  5d                       pop ebp
00495913  5f                       pop edi
00495914  5e                       pop esi
00495915  5a                       pop edx
00495916  59                       pop ecx
00495917  5b                       pop ebx
00495918  c3                       ret 
00495919  8d8000000000             lea eax, [eax]
0049591f  90                       nop 

