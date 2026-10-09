==== sub_4f3600 .. 4f3a10
004f3600  56                       push esi
004f3601  57                       push edi
004f3602  55                       push ebp
004f3603  83ec08                   sub esp, 8
004f3606  89c5                     mov ebp, eax
004f3608  89d6                     mov esi, edx
004f360a  89df                     mov edi, ebx
004f360c  51                       push ecx
004f360d  8b1540259f00             mov edx, dword ptr [0x9f2540]
004f3613  d1ff                     sar edi, 1
004f3615  85d2                     test edx, edx
004f3617  0f843c020000             je 0x4f3859
004f361d  8a253f259f00             mov ah, byte ptr [0x9f253f]
004f3623  f6c4c0                   test ah, 0xc0
004f3626  0f8536020000             jne 0x4f3862
004f362c  b802000000               mov eax, 2
004f3631  31db                     xor ebx, ebx
004f3633  e818fdffff               call 0x4f3350
004f3638  895c2408                 mov dword ptr [esp + 8], ebx
004f363c  e807d80100               call 0x510e48
004f3641  83f801                   cmp eax, 1
004f3644  0f8598020000             jne 0x4f38e2
004f364a  ba10000000               mov edx, 0x10
004f364f  b838219f00               mov eax, 0x9f2138
004f3654  e887fdffff               call 0x4f33e0
004f3659  f644240802               test byte ptr [esp + 8], 2
004f365e  0f85a6020000             jne 0x4f390a
004f3664  e8dfd70100               call 0x510e48
004f3669  83f801                   cmp eax, 1
004f366c  0f8584020000             jne 0x4f38f6
004f3672  ba10000000               mov edx, 0x10
004f3677  b858219f00               mov eax, 0x9f2158
004f367c  e85ffdffff               call 0x4f33e0
004f3681  f644240804               test byte ptr [esp + 8], 4
004f3686  0f85b3020000             jne 0x4f393f
004f368c  e8b7d70100               call 0x510e48
004f3691  83f801                   cmp eax, 1
004f3694  0f8591020000             jne 0x4f392b
004f369a  ba10000000               mov edx, 0x10
004f369f  b838239f00               mov eax, 0x9f2338
004f36a4  e837fdffff               call 0x4f33e0
004f36a9  f644240808               test byte ptr [esp + 8], 8
004f36ae  0f85c4020000             jne 0x4f3978
004f36b4  e88fd70100               call 0x510e48
004f36b9  83f801                   cmp eax, 1
004f36bc  0f85a2020000             jne 0x4f3964
004f36c2  ba10000000               mov edx, 0x10
004f36c7  b858239f00               mov eax, 0x9f2358
004f36cc  e80ffdffff               call 0x4f33e0
004f36d1  f644240810               test byte ptr [esp + 8], 0x10
004f36d6  0f85d8020000             jne 0x4f39b4
004f36dc  e867d70100               call 0x510e48
004f36e1  83f801                   cmp eax, 1
004f36e4  0f85b6020000             jne 0x4f39a0
004f36ea  ba08000000               mov edx, 8
004f36ef  b8380e9f00               mov eax, 0x9f0e38
004f36f4  e8e7fcffff               call 0x4f33e0
004f36f9  f644240820               test byte ptr [esp + 8], 0x20
004f36fe  0f85e5020000             jne 0x4f39e9
004f3704  e83fd70100               call 0x510e48
004f3709  83f801                   cmp eax, 1
004f370c  0f85c3020000             jne 0x4f39d5
004f3712  ba08000000               mov edx, 8
004f3717  b8380f9f00               mov eax, 0x9f0f38
004f371c  e8bffcffff               call 0x4f33e0
004f3721  ba380e9f00               mov edx, 0x9f0e38
004f3726  b838219f00               mov eax, 0x9f2138
004f372b  89f3                     mov ebx, esi
004f372d  e8c6d80100               call 0x510ff8
004f3732  c1e702                   shl edi, 2
004f3735  ba380e9f00               mov edx, 0x9f0e38
004f373a  01fe                     add esi, edi
004f373c  b878219f00               mov eax, 0x9f2178
004f3741  89f3                     mov ebx, esi
004f3743  e8b0d80100               call 0x510ff8
004f3748  ba580e9f00               mov edx, 0x9f0e58
004f374d  01fe                     add esi, edi
004f374f  b8b8219f00               mov eax, 0x9f21b8
004f3754  89f3                     mov ebx, esi
004f3756  e89dd80100               call 0x510ff8
004f375b  ba580e9f00               mov edx, 0x9f0e58
004f3760  01fe                     add esi, edi
004f3762  b8f8219f00               mov eax, 0x9f21f8
004f3767  89f3                     mov ebx, esi
004f3769  e88ad80100               call 0x510ff8
004f376e  ba780e9f00               mov edx, 0x9f0e78
004f3773  01fe                     add esi, edi
004f3775  b838229f00               mov eax, 0x9f2238
004f377a  89f3                     mov ebx, esi
004f377c  e877d80100               call 0x510ff8
004f3781  ba780e9f00               mov edx, 0x9f0e78
004f3786  01fe                     add esi, edi
004f3788  b878229f00               mov eax, 0x9f2278
004f378d  89f3                     mov ebx, esi
004f378f  e864d80100               call 0x510ff8
004f3794  ba980e9f00               mov edx, 0x9f0e98
004f3799  01fe                     add esi, edi
004f379b  b8b8229f00               mov eax, 0x9f22b8
004f37a0  89f3                     mov ebx, esi
004f37a2  e851d80100               call 0x510ff8
004f37a7  01fe                     add esi, edi
004f37a9  59                       pop ecx
004f37aa  ba980e9f00               mov edx, 0x9f0e98
004f37af  b8f8229f00               mov eax, 0x9f22f8
004f37b4  89f3                     mov ebx, esi
004f37b6  e83dd80100               call 0x510ff8
004f37bb  bab80e9f00               mov edx, 0x9f0eb8
004f37c0  01fe                     add esi, edi
004f37c2  b838239f00               mov eax, 0x9f2338
004f37c7  89f3                     mov ebx, esi
004f37c9  e82ad80100               call 0x510ff8
004f37ce  bab80e9f00               mov edx, 0x9f0eb8
004f37d3  01fe                     add esi, edi
004f37d5  b878239f00               mov eax, 0x9f2378
004f37da  89f3                     mov ebx, esi
004f37dc  e817d80100               call 0x510ff8
004f37e1  bad80e9f00               mov edx, 0x9f0ed8
004f37e6  01fe                     add esi, edi
004f37e8  b8b8239f00               mov eax, 0x9f23b8
004f37ed  89f3                     mov ebx, esi
004f37ef  e804d80100               call 0x510ff8
004f37f4  bad80e9f00               mov edx, 0x9f0ed8
004f37f9  01fe                     add esi, edi
004f37fb  b8f8239f00               mov eax, 0x9f23f8
004f3800  89f3                     mov ebx, esi
004f3802  e8f1d70100               call 0x510ff8
004f3807  baf80e9f00               mov edx, 0x9f0ef8
004f380c  01fe                     add esi, edi
004f380e  b838249f00               mov eax, 0x9f2438
004f3813  89f3                     mov ebx, esi
004f3815  e8ded70100               call 0x510ff8
004f381a  baf80e9f00               mov edx, 0x9f0ef8
004f381f  01fe                     add esi, edi
004f3821  b878249f00               mov eax, 0x9f2478
004f3826  89f3                     mov ebx, esi
004f3828  e8cbd70100               call 0x510ff8
004f382d  ba180f9f00               mov edx, 0x9f0f18
004f3832  01fe                     add esi, edi
004f3834  b8b8249f00               mov eax, 0x9f24b8
004f3839  89f3                     mov ebx, esi
004f383b  e8b8d70100               call 0x510ff8
004f3840  ba180f9f00               mov edx, 0x9f0f18
004f3845  b8f8249f00               mov eax, 0x9f24f8
004f384a  8d1c3e                   lea ebx, [esi + edi]
004f384d  e8a6d70100               call 0x510ff8
004f3852  83c408                   add esp, 8
004f3855  5d                       pop ebp
004f3856  5f                       pop edi
004f3857  5e                       pop esi
004f3858  c3                       ret 
004f3859  89542408                 mov dword ptr [esp + 8], edx
004f385d  e9dafdffff               jmp 0x4f363c
004f3862  f6c480                   test ah, 0x80
004f3865  7463                     je 0x4f38ca
004f3867  b801000000               mov eax, 1
004f386c  b9ff030000               mov ecx, 0x3ff
004f3871  e8dafaffff               call 0x4f3350
004f3876  894c2408                 mov dword ptr [esp + 8], ecx
004f387a  e831fbffff               call 0x4f33b0
004f387f  89c2                     mov edx, eax
004f3881  e82afbffff               call 0x4f33b0
004f3886  89c1                     mov ecx, eax
004f3888  0fafcf                   imul ecx, edi
004f388b  89d0                     mov eax, edx
004f388d  d1f8                     sar eax, 1
004f388f  83e201                   and edx, 1
004f3892  01c8                     add eax, ecx
004f3894  89542404                 mov dword ptr [esp + 4], edx
004f3898  c1e002                   shl eax, 2
004f389b  8a742408                 mov dh, byte ptr [esp + 8]
004f389f  01c5                     add ebp, eax
004f38a1  f6c601                   test dh, 1
004f38a4  0f8492fdffff             je 0x4f363c
004f38aa  e801fbffff               call 0x4f33b0
004f38af  b938219f00               mov ecx, 0x9f2138
004f38b4  83e840                   sub eax, 0x40
004f38b7  8b542404                 mov edx, dword ptr [esp + 4]
004f38bb  50                       push eax
004f38bc  89fb                     mov ebx, edi
004f38be  89e8                     mov eax, ebp
004f38c0  e85bfbffff               call 0x4f3420
004f38c5  e98ffdffff               jmp 0x4f3659
004f38ca  a13c259f00               mov eax, dword ptr [0x9f253c]
004f38cf  c1e818                   shr eax, 0x18
004f38d2  89442408                 mov dword ptr [esp + 8], eax
004f38d6  b808000000               mov eax, 8
004f38db  e870faffff               call 0x4f3350
004f38e0  eb98                     jmp 0x4f387a
004f38e2  ba10000000               mov edx, 0x10
004f38e7  b838219f00               mov eax, 0x9f2138
004f38ec  e8ccd30100               call 0x510cbd
004f38f1  e963fdffff               jmp 0x4f3659
004f38f6  ba10000000               mov edx, 0x10
004f38fb  b858219f00               mov eax, 0x9f2158
004f3900  e8b8d30100               call 0x510cbd
004f3905  e977fdffff               jmp 0x4f3681
004f390a  e8a1faffff               call 0x4f33b0
004f390f  b958219f00               mov ecx, 0x9f2158
004f3914  83e840                   sub eax, 0x40
004f3917  8b542404                 mov edx, dword ptr [esp + 4]
004f391b  50                       push eax
004f391c  89fb                     mov ebx, edi
004f391e  8d4510                   lea eax, [ebp + 0x10]
004f3921  e8fafaffff               call 0x4f3420
004f3926  e956fdffff               jmp 0x4f3681
004f392b  ba10000000               mov edx, 0x10
004f3930  b838239f00               mov eax, 0x9f2338
004f3935  e883d30100               call 0x510cbd
004f393a  e96afdffff               jmp 0x4f36a9
004f393f  e86cfaffff               call 0x4f33b0
004f3944  83e840                   sub eax, 0x40
004f3947  b938239f00               mov ecx, 0x9f2338
004f394c  50                       push eax
004f394d  89f8                     mov eax, edi
004f394f  8b542408                 mov edx, dword ptr [esp + 8]
004f3953  c1e005                   shl eax, 5
004f3956  89fb                     mov ebx, edi
004f3958  01e8                     add eax, ebp
004f395a  e8c1faffff               call 0x4f3420
004f395f  e945fdffff               jmp 0x4f36a9
004f3964  ba10000000               mov edx, 0x10
004f3969  b858239f00               mov eax, 0x9f2358
004f396e  e84ad30100               call 0x510cbd
004f3973  e959fdffff               jmp 0x4f36d1
004f3978  e833faffff               call 0x4f33b0
004f397d  83e840                   sub eax, 0x40
004f3980  50                       push eax
004f3981  89f8                     mov eax, edi
004f3983  b958239f00               mov ecx, 0x9f2358
004f3988  c1e005                   shl eax, 5
004f398b  8b542408                 mov edx, dword ptr [esp + 8]
004f398f  01e8                     add eax, ebp
004f3991  89fb                     mov ebx, edi
004f3993  83c010                   add eax, 0x10
004f3996  e885faffff               call 0x4f3420
004f399b  e931fdffff               jmp 0x4f36d1
004f39a0  ba08000000               mov edx, 8
004f39a5  b8380e9f00               mov eax, 0x9f0e38
004f39aa  e80ed30100               call 0x510cbd
004f39af  e945fdffff               jmp 0x4f36f9
004f39b4  e8f7f9ffff               call 0x4f33b0
004f39b9  b9380e9f00               mov ecx, 0x9f0e38
004f39be  83e840                   sub eax, 0x40
004f39c1  ba01000000               mov edx, 1
004f39c6  50                       push eax
004f39c7  89fb                     mov ebx, edi
004f39c9  89e8                     mov eax, ebp
004f39cb  e820fbffff               call 0x4f34f0
004f39d0  e924fdffff               jmp 0x4f36f9
004f39d5  ba08000000               mov edx, 8
004f39da  b8380f9f00               mov eax, 0x9f0f38
004f39df  e8d9d20100               call 0x510cbd
004f39e4  e938fdffff               jmp 0x4f3721
004f39e9  e8c2f9ffff               call 0x4f33b0
004f39ee  b9380f9f00               mov ecx, 0x9f0f38
004f39f3  83e840                   sub eax, 0x40
004f39f6  89fb                     mov ebx, edi
004f39f8  50                       push eax
004f39f9  31d2                     xor edx, edx
004f39fb  89e8                     mov eax, ebp
004f39fd  e8eefaffff               call 0x4f34f0
004f3a02  e91afdffff               jmp 0x4f3721
004f3a07  0000                     add byte ptr [eax], al
004f3a09  0000                     add byte ptr [eax], al
004f3a0b  0000                     add byte ptr [eax], al
004f3a0d  0000                     add byte ptr [eax], al

