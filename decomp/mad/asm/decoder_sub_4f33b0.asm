==== sub_4f33b0 .. 4f33e0
004f33b0  52                       push edx
004f33b1  a13c259f00               mov eax, dword ptr [0x9f253c]
004f33b6  c1e81a                   shr eax, 0x1a
004f33b9  8b148538109f00           mov edx, dword ptr [eax*4 + 0x9f1038]
004f33c0  89d0                     mov eax, edx
004f33c2  25ff000000               and eax, 0xff
004f33c7  e884ffffff               call 0x4f3350
004f33cc  89d0                     mov eax, edx
004f33ce  c1f816                   sar eax, 0x16
004f33d1  5a                       pop edx
004f33d2  c3                       ret 
004f33d3  8d8000000000             lea eax, [eax]
004f33d9  8d9200000000             lea edx, [edx]
004f33df  90                       nop 

