==== sub_510cbd .. 510e48
00510cbd  53                       push ebx
00510cbe  51                       push ecx
00510cbf  56                       push esi
00510cc0  57                       push edi
00510cc1  55                       push ebp
00510cc2  50                       push eax
00510cc3  c1e202                   shl edx, 2
00510cc6  8915e0cd5600             mov dword ptr [0x56cde0], edx
00510ccc  be2ccf5600               mov esi, 0x56cf2c
00510cd1  c705e4cd56000cce5600     mov dword ptr [0x56cde4], 0x56ce0c
00510cdb  e870fdffff               call 0x510a50
00510ce0  be4ccf5600               mov esi, 0x56cf4c
00510ce5  c705e4cd560010ce5600     mov dword ptr [0x56cde4], 0x56ce10
00510cef  e85cfdffff               call 0x510a50
00510cf4  be6ccf5600               mov esi, 0x56cf6c
00510cf9  c705e4cd560014ce5600     mov dword ptr [0x56cde4], 0x56ce14
00510d03  e848fdffff               call 0x510a50
00510d08  be8ccf5600               mov esi, 0x56cf8c
00510d0d  c705e4cd560018ce5600     mov dword ptr [0x56cde4], 0x56ce18
00510d17  e834fdffff               call 0x510a50
00510d1c  beaccf5600               mov esi, 0x56cfac
00510d21  c705e4cd56001cce5600     mov dword ptr [0x56cde4], 0x56ce1c
00510d2b  e820fdffff               call 0x510a50
00510d30  becccf5600               mov esi, 0x56cfcc
00510d35  c705e4cd560020ce5600     mov dword ptr [0x56cde4], 0x56ce20
00510d3f  e80cfdffff               call 0x510a50
00510d44  beeccf5600               mov esi, 0x56cfec
00510d49  c705e4cd560024ce5600     mov dword ptr [0x56cde4], 0x56ce24
00510d53  e8f8fcffff               call 0x510a50
00510d58  be0cd05600               mov esi, 0x56d00c
00510d5d  c705e4cd560028ce5600     mov dword ptr [0x56cde4], 0x56ce28
00510d67  e8e4fcffff               call 0x510a50
00510d6c  58                       pop eax
00510d6d  be0cce5600               mov esi, 0x56ce0c
00510d72  a3e4cd5600               mov dword ptr [0x56cde4], eax
00510d77  e837feffff               call 0x510bb3
00510d7c  a1e4cd5600               mov eax, dword ptr [0x56cde4]
00510d81  8b15e0cd5600             mov edx, dword ptr [0x56cde0]
00510d87  01d0                     add eax, edx
00510d89  be30ce5600               mov esi, 0x56ce30
00510d8e  a3e4cd5600               mov dword ptr [0x56cde4], eax
00510d93  e81bfeffff               call 0x510bb3
00510d98  a1e4cd5600               mov eax, dword ptr [0x56cde4]
00510d9d  8b15e0cd5600             mov edx, dword ptr [0x56cde0]
00510da3  01d0                     add eax, edx
00510da5  be54ce5600               mov esi, 0x56ce54
00510daa  a3e4cd5600               mov dword ptr [0x56cde4], eax
00510daf  e8fffdffff               call 0x510bb3
00510db4  a1e4cd5600               mov eax, dword ptr [0x56cde4]
00510db9  8b15e0cd5600             mov edx, dword ptr [0x56cde0]
00510dbf  01d0                     add eax, edx
00510dc1  be78ce5600               mov esi, 0x56ce78
00510dc6  a3e4cd5600               mov dword ptr [0x56cde4], eax
00510dcb  e8e3fdffff               call 0x510bb3
00510dd0  a1e4cd5600               mov eax, dword ptr [0x56cde4]
00510dd5  8b15e0cd5600             mov edx, dword ptr [0x56cde0]
00510ddb  01d0                     add eax, edx
00510ddd  be9cce5600               mov esi, 0x56ce9c
00510de2  a3e4cd5600               mov dword ptr [0x56cde4], eax
00510de7  e8c7fdffff               call 0x510bb3
00510dec  a1e4cd5600               mov eax, dword ptr [0x56cde4]
00510df1  8b15e0cd5600             mov edx, dword ptr [0x56cde0]
00510df7  01d0                     add eax, edx
00510df9  bec0ce5600               mov esi, 0x56cec0
00510dfe  a3e4cd5600               mov dword ptr [0x56cde4], eax
00510e03  e8abfdffff               call 0x510bb3
00510e08  a1e4cd5600               mov eax, dword ptr [0x56cde4]
00510e0d  8b15e0cd5600             mov edx, dword ptr [0x56cde0]
00510e13  01d0                     add eax, edx
00510e15  bee4ce5600               mov esi, 0x56cee4
00510e1a  a3e4cd5600               mov dword ptr [0x56cde4], eax
00510e1f  e88ffdffff               call 0x510bb3
00510e24  a1e4cd5600               mov eax, dword ptr [0x56cde4]
00510e29  8b15e0cd5600             mov edx, dword ptr [0x56cde0]
00510e2f  01d0                     add eax, edx
00510e31  be08cf5600               mov esi, 0x56cf08
00510e36  a3e4cd5600               mov dword ptr [0x56cde4], eax
00510e3b  e873fdffff               call 0x510bb3
00510e40  5d                       pop ebp
00510e41  5f                       pop edi
00510e42  5e                       pop esi
00510e43  59                       pop ecx
00510e44  5b                       pop ebx
00510e45  c3                       ret 
00510e46  0000                     add byte ptr [eax], al

