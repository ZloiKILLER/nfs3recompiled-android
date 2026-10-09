==== sub_4fd8ee .. 4fd98c
004fd8ee  56                       push esi
004fd8ef  57                       push edi
004fd8f0  55                       push ebp
004fd8f1  890dd8765600             mov dword ptr [0x5676d8], ecx
004fd8f7  8b30                     mov esi, dword ptr [eax]
004fd8f9  8b3a                     mov edi, dword ptr [edx]
004fd8fb  01f7                     add edi, esi
004fd8fd  83c004                   add eax, 4
004fd900  d1ef                     shr edi, 1
004fd902  83c204                   add edx, 4
004fd905  81e77f7f7f7f             and edi, 0x7f7f7f7f
004fd90b  893ddc765600             mov dword ptr [0x5676dc], edi
004fd911  8b30                     mov esi, dword ptr [eax]
004fd913  8b3a                     mov edi, dword ptr [edx]
004fd915  01f7                     add edi, esi
004fd917  8b35dc765600             mov esi, dword ptr [0x5676dc]
004fd91d  d1ef                     shr edi, 1
004fd91f  83c308                   add ebx, 8
004fd922  81e77f7f7f7f             and edi, 0x7f7f7f7f
004fd928  89f1                     mov ecx, esi
004fd92a  893ddc765600             mov dword ptr [0x5676dc], edi
004fd930  89fd                     mov ebp, edi
004fd932  c1e108                   shl ecx, 8
004fd935  81e77f7f0000             and edi, 0x7f7f
004fd93b  c1ed08                   shr ebp, 8
004fd93e  81e10000007f             and ecx, 0x7f000000
004fd944  81e500007f00             and ebp, 0x7f0000
004fd94a  09cf                     or edi, ecx
004fd94c  09ef                     or edi, ebp
004fd94e  83c004                   add eax, 4
004fd951  01f7                     add edi, esi
004fd953  83c204                   add edx, 4
004fd956  d1ef                     shr edi, 1
004fd958  81e67f7f007f             and esi, 0x7f007f7f
004fd95e  89fd                     mov ebp, edi
004fd960  81e77f7f7f00             and edi, 0x7f7f7f
004fd966  c1ed08                   shr ebp, 8
004fd969  09cf                     or edi, ecx
004fd96b  81e500007f00             and ebp, 0x7f0000
004fd971  897bfc                   mov dword ptr [ebx - 4], edi
004fd974  09ee                     or esi, ebp
004fd976  8b0dd8765600             mov ecx, dword ptr [0x5676d8]
004fd97c  8973f8                   mov dword ptr [ebx - 8], esi
004fd97f  49                       dec ecx
004fd980  890dd8765600             mov dword ptr [0x5676d8], ecx
004fd986  7589                     jne 0x4fd911
004fd988  5d                       pop ebp
004fd989  5f                       pop edi
004fd98a  5e                       pop esi
004fd98b  c3                       ret 

