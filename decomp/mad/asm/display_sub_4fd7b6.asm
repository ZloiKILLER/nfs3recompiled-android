==== sub_4fd7b6 .. 4fd892
004fd7b6  56                       push esi
004fd7b7  57                       push edi
004fd7b8  89c6                     mov esi, eax
004fd7ba  89d7                     mov edi, edx
004fd7bc  890dd8765600             mov dword ptr [0x5676d8], ecx
004fd7c2  55                       push ebp
004fd7c3  8b06                     mov eax, dword ptr [esi]
004fd7c5  8b17                     mov edx, dword ptr [edi]
004fd7c7  01d0                     add eax, edx
004fd7c9  83c604                   add esi, 4
004fd7cc  d1e8                     shr eax, 1
004fd7ce  83c704                   add edi, 4
004fd7d1  257f7f7f7f               and eax, 0x7f7f7f7f
004fd7d6  31d2                     xor edx, edx
004fd7d8  89c1                     mov ecx, eax
004fd7da  89c5                     mov ebp, eax
004fd7dc  c1e908                   shr ecx, 8
004fd7df  88c2                     mov dl, al
004fd7e1  c1ed10                   shr ebp, 0x10
004fd7e4  81e1ff000000             and ecx, 0xff
004fd7ea  c1e818                   shr eax, 0x18
004fd7ed  8b149518629f00           mov edx, dword ptr [edx*4 + 0x9f6218]
004fd7f4  81e5ff000000             and ebp, 0xff
004fd7fa  8b0c8d18609f00           mov ecx, dword ptr [ecx*4 + 0x9f6018]
004fd801  01ca                     add edx, ecx
004fd803  8b0485185e9f00           mov eax, dword ptr [eax*4 + 0x9f5e18]
004fd80a  01d0                     add eax, edx
004fd80c  8b2cad185e9f00           mov ebp, dword ptr [ebp*4 + 0x9f5e18]
004fd813  01d5                     add ebp, edx
004fd815  89c2                     mov edx, eax
004fd817  c1ea17                   shr edx, 0x17
004fd81a  89c1                     mov ecx, eax
004fd81c  c1e90b                   shr ecx, 0xb
004fd81f  25ff010000               and eax, 0x1ff
004fd824  8b149550599f00           mov edx, dword ptr [edx*4 + 0x9f5950]
004fd82b  81e1ff010000             and ecx, 0x1ff
004fd831  8b0485b04f9f00           mov eax, dword ptr [eax*4 + 0x9f4fb0]
004fd838  83c308                   add ebx, 8
004fd83b  8b0c8d38559f00           mov ecx, dword ptr [ecx*4 + 0x9f5538]
004fd842  01d0                     add eax, edx
004fd844  01c8                     add eax, ecx
004fd846  89ea                     mov edx, ebp
004fd848  c1ea17                   shr edx, 0x17
004fd84b  8943f8                   mov dword ptr [ebx - 8], eax
004fd84e  89e8                     mov eax, ebp
004fd850  81e5ff010000             and ebp, 0x1ff
004fd856  c1e80b                   shr eax, 0xb
004fd859  8b149550599f00           mov edx, dword ptr [edx*4 + 0x9f5950]
004fd860  25ff010000               and eax, 0x1ff
004fd865  8b2cadb04f9f00           mov ebp, dword ptr [ebp*4 + 0x9f4fb0]
004fd86c  01ea                     add edx, ebp
004fd86e  8b0dd8765600             mov ecx, dword ptr [0x5676d8]
004fd874  8b048538559f00           mov eax, dword ptr [eax*4 + 0x9f5538]
004fd87b  01d0                     add eax, edx
004fd87d  49                       dec ecx
004fd87e  890dd8765600             mov dword ptr [0x5676d8], ecx
004fd884  90                       nop 
004fd885  8943fc                   mov dword ptr [ebx - 4], eax
004fd888  0f8535ffffff             jne 0x4fd7c3
004fd88e  5d                       pop ebp
004fd88f  5f                       pop edi
004fd890  5e                       pop esi
004fd891  c3                       ret 

