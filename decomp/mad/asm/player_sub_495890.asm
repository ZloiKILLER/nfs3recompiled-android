==== sub_495890 .. 4958c0
00495890  52                       push edx
00495891  55                       push ebp
00495892  89e5                     mov ebp, esp
00495894  8b1558f37900             mov edx, dword ptr [0x79f358]
0049589a  a154f37900               mov eax, dword ptr [0x79f354]
0049589f  39d0                     cmp eax, edx
004958a1  7505                     jne 0x4958a8
004958a3  31d0                     xor eax, edx
004958a5  5d                       pop ebp
004958a6  5a                       pop edx
004958a7  c3                       ret 
004958a8  8b04c594f27900           mov eax, dword ptr [eax*8 + 0x79f294]
004958af  5d                       pop ebp
004958b0  5a                       pop edx
004958b1  c3                       ret 
004958b2  8d8000000000             lea eax, [eax]
004958b8  8d9200000000             lea edx, [edx]
004958be  8bc0                     mov eax, eax

