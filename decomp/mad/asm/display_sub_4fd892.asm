==== sub_4fd892 .. 4fd8ee
004fd892  51                       push ecx
004fd893  56                       push esi
004fd894  57                       push edi
004fd895  55                       push ebp
004fd896  8b30                     mov esi, dword ptr [eax]
004fd898  8b7804                   mov edi, dword ptr [eax + 4]
004fd89b  89f1                     mov ecx, esi
004fd89d  89fd                     mov ebp, edi
004fd89f  c1e108                   shl ecx, 8
004fd8a2  81e77f7f0000             and edi, 0x7f7f
004fd8a8  c1ed08                   shr ebp, 8
004fd8ab  81e10000007f             and ecx, 0x7f000000
004fd8b1  81e500007f00             and ebp, 0x7f0000
004fd8b7  09cf                     or edi, ecx
004fd8b9  09ef                     or edi, ebp
004fd8bb  83c004                   add eax, 4
004fd8be  01f7                     add edi, esi
004fd8c0  83c208                   add edx, 8
004fd8c3  d1ef                     shr edi, 1
004fd8c5  81e67f7f007f             and esi, 0x7f007f7f
004fd8cb  89fd                     mov ebp, edi
004fd8cd  81e77f7f7f00             and edi, 0x7f7f7f
004fd8d3  c1ed08                   shr ebp, 8
004fd8d6  09cf                     or edi, ecx
004fd8d8  81e500007f00             and ebp, 0x7f0000
004fd8de  897afc                   mov dword ptr [edx - 4], edi
004fd8e1  09ee                     or esi, ebp
004fd8e3  4b                       dec ebx
004fd8e4  8972f8                   mov dword ptr [edx - 8], esi
004fd8e7  75ad                     jne 0x4fd896
004fd8e9  5d                       pop ebp
004fd8ea  5f                       pop edi
004fd8eb  5e                       pop esi
004fd8ec  59                       pop ecx
004fd8ed  c3                       ret 

