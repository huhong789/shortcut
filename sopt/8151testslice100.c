asm (
".section	.text /*[SLICE_EXTRA] comes with 00000000*/\n"
".globl _section1 /*[SLICE_EXTRA] comes with 00000000*/\n"
"_section1: /*[SLICE_EXTRA] comes with 00000000*/\n"
"pushfd\n"
"and eax, 0xffff\n"
"or eax, 0x0\n"
"popfd\n"
"pushfd\n"
"and eax, 0xffff\n"
"or eax, 0x0\n"
"popfd\n"
"test eax, eax\n"
"jle jump_diverge\n"
"mov eax, dword ptr [0x8d34d9c]\n"
"pushfd\n"
"and eax, 0xffff\n"
"or eax, 0x0\n"
"popfd\n"
"cmp eax, 0x2\n"
"jle jump_diverge\n"
"mov eax, dword ptr [0x8d34d98]\n"
"pushfd\n"
"and eax, 0xffff\n"
"or eax, 0x0\n"
"popfd\n"
"cmp eax, 0x2\n"
"jle jump_diverge\n"
"mov edx, dword ptr [0x8d34d9c]\n"
"pushfd\n"
"and edx, 0xffff\n"
"or edx, 0x0\n"
"popfd\n"
"mov dword ptr [0x8589214], edx\n"
"mov eax, dword ptr [0x8d34d9c]\n"
"pushfd\n"
"and eax, 0xffff\n"
"or eax, 0x0\n"
"popfd\n"
"pushfd\n"
"and eax, 0xffff\n"
"or eax, 0x0\n"
"popfd\n"
"test eax, eax\n"
"jle jump_diverge\n"
"mov eax, dword ptr [0x8d34d98]\n"
"pushfd\n"
"and eax, 0xffff\n"
"or eax, 0x0\n"
"popfd\n"
"pushfd\n"
"and eax, 0xffff\n"
"or eax, 0x0\n"
"popfd\n"
"test eax, eax\n"
"jle jump_diverge\n"
"call ioctl_recheck\n"
"mov ecx, dword ptr [0xbfffee78]\n"
"mov edi, dword ptr [0xbfffee84]\n"
"mov ebp, dword ptr [0xbfffee80]\n"
"mov dword ptr [0xbfffeee0], ecx\n"
"mov ecx, dword ptr [0xbfffee7c]\n"
"mov dword ptr [0xbfffeeec], edi\n"
"movzx edi, byte ptr [0xbfffee88]\n"
"mov dword ptr [0xbfffeee8], ebp\n"
"mov dword ptr [0xbfffeee4], ecx\n"
"pushfd\n"
"and edi, 0xff\n"
"or edi, 0x0\n"
"popfd\n"
"mov ecx, 20\n"
"mov ecx, edi\n"
"mov byte ptr [0xbfffeef0], cl\n"
"mov ecx, ebp\n"
"and ecx, 0x100f\n"
"mov dword ptr [0xbfffef14], ecx\n"
"mov dword ptr [0xbfffef14], 259\n"
"mov dword ptr [0xbfffef14], ecx\n"
"mov dword ptr [0xbfffef18], ecx\n"
"mov ecx, dword ptr [0xbfffee89]\n"
"mov dword ptr [0xbfffeef1], ecx\n"
"mov ecx, dword ptr [0xbfffee8d]\n"
"mov dword ptr [0xbfffeef5], ecx\n"
"mov ecx, dword ptr [0xbfffee91]\n"
"mov dword ptr [0xbfffeef9], ecx\n"
"mov ecx, dword ptr [0xbfffee95]\n"
"mov dword ptr [0xbfffeefd], ecx\n"
"movzx ecx, word ptr [0xbfffee99]\n"
"movzx edx, byte ptr [0xbfffee9b]\n"
"mov word ptr [0xbfffef01], cx\n"
"mov byte ptr [0xbfffef03], dl\n"
"mov eax, dword ptr [0xbfffeee4]\n"
"and eax, 0x1800\n"
"cmp eax, 0x1800\n"
"setnz al\n"
"movzx eax, al\n"
"pushfd\n"
"and eax, 0xff\n"
"or eax, 0x0\n"
"popfd\n"
"pushfd\n"
"and eax, 0xff\n"
"or eax, 0x0\n"
"popfd\n"
"test eax, eax\n"
"jz jump_diverge\n"
);
