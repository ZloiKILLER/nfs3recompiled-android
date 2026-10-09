==== sub_4fd6ef .. 4fd7b6
004fd6ef  51                       push ecx
004fd6f0  56                       push esi
004fd6f1  891dd8765600             mov dword ptr [0x5676d8], ebx
004fd6f7  89c6                     mov esi, eax
004fd6f9  57                       push edi
004fd6fa  55                       push ebp
004fd6fb  8b06                     mov eax, dword ptr [esi]
004fd6fd  31db                     xor ebx, ebx
004fd6ff  88c3                     mov bl, al
004fd701  89c7                     mov edi, eax
004fd703  c1ef18                   shr edi, 0x18
004fd706  2500ff0000               and eax, 0xff00
004fd70b  c1e808                   shr eax, 8
004fd70e  8b2c9d18629f00           mov ebp, dword ptr [ebx*4 + 0x9f6218]
004fd715  8b3cbd185e9f00           mov edi, dword ptr [edi*4 + 0x9f5e18]
004fd71c  83c604                   add esi, 4
004fd71f  8b048518609f00           mov eax, dword ptr [eax*4 + 0x9f6018]
004fd726  01ef                     add edi, ebp
004fd728  01c5                     add ebp, eax
004fd72a  01f8                     add eax, edi
004fd72c  89c1                     mov ecx, eax
004fd72e  89c7                     mov edi, eax
004fd730  c1e817                   shr eax, 0x17
004fd733  81e100f80f00             and ecx, 0xff800
004fd739  c1e90b                   shr ecx, 0xb
004fd73c  81e7ff010000             and edi, 0x1ff
004fd742  8b048550599f00           mov eax, dword ptr [eax*4 + 0x9f5950]
004fd749  8a5efe                   mov bl, byte ptr [esi - 2]
004fd74c  8b0c8d38559f00           mov ecx, dword ptr [ecx*4 + 0x9f5538]
004fd753  8b3cbdb04f9f00           mov edi, dword ptr [edi*4 + 0x9f4fb0]
004fd75a  01c8                     add eax, ecx
004fd75c  8b1c9d185e9f00           mov ebx, dword ptr [ebx*4 + 0x9f5e18]
004fd763  01f8                     add eax, edi
004fd765  01eb                     add ebx, ebp
004fd767  8902                     mov dword ptr [edx], eax
004fd769  89d8                     mov eax, ebx
004fd76b  89df                     mov edi, ebx
004fd76d  81e300f80f00             and ebx, 0xff800
004fd773  c1e817                   shr eax, 0x17
004fd776  81e7ff010000             and edi, 0x1ff
004fd77c  c1eb0b                   shr ebx, 0xb
004fd77f  8b0dd8765600             mov ecx, dword ptr [0x5676d8]
004fd785  8b048550599f00           mov eax, dword ptr [eax*4 + 0x9f5950]
004fd78c  8b3cbdb04f9f00           mov edi, dword ptr [edi*4 + 0x9f4fb0]
004fd793  8b1c9d38559f00           mov ebx, dword ptr [ebx*4 + 0x9f5538]
004fd79a  01f8                     add eax, edi
004fd79c  01d8                     add eax, ebx
004fd79e  49                       dec ecx
004fd79f  890dd8765600             mov dword ptr [0x5676d8], ecx
004fd7a5  894204                   mov dword ptr [edx + 4], eax
004fd7a8  8d5208                   lea edx, [edx + 8]
004fd7ab  0f854affffff             jne 0x4fd6fb
004fd7b1  5d                       pop ebp
004fd7b2  5f                       pop edi
004fd7b3  5e                       pop esi
004fd7b4  59                       pop ecx
004fd7b5  c3                       ret 

