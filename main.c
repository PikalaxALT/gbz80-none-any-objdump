#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;

const char *regs16[4] = {"bc", "de", "hl", "sp"};
const char *regs16_2[4] = {"bc", "de", "hli", "hld"};
const char *regs16_3[4] = {"bc", "de", "hl", "af"};
const char *regs8[8] = {"b", "c", "d", "e", "h", "l", "[hl]", "a"};
const char *flags[4] = {"nz", "z", "nc", "c"};
const char *arith[8] = {"add", "adc", "sub", "sbc", "and", "xor", "or", "cp"};
const char *bitops[8] = {"rlc", "rrc", "rl", "rr", "sla", "sra", "swap", "srl"};
const char *bitops2[3] = {"bit", "res", "set"};

typedef struct {
    u8 bank;
    u16 addr;
} gbaddr_t;

gbaddr_t inttogbaddr(long addr) {
    gbaddr_t output;
    output.bank = addr >> 14;
    output.addr = addr & 0x3fff;
    if (output.bank)
        output.addr |= 0x4000;
    return output;
}

#define FILE_READ_ERROR(size) {\
    fprintf(stderr, "error: failed to read %d bytes\n", size);\
    exit(EIO);\
}

int get_opcode(char *dest, FILE *file) {
    u8 arg8;
    u16 arg16;
    u8 opcode;
    int size;
#define READ(amt) {\
    if (fread(&arg##amt, sizeof arg##amt, 1, file) != sizeof arg##amt) \
        FILE_READ_ERROR((amt) / 8); \
    size += sizeof arg##amt;}
#define READ8 READ(8)
#define READ16 READ(16)
    size = 1;
    if (fread(&opcode, sizeof opcode, 1, file) != sizeof opcode)
        FILE_READ_ERROR(1);
    switch (opcode) {
        case 0x00:
            strcpy(dest, "nop");
            break;
        case 0x01:
        case 0x11:
        case 0x21:
        case 0x31:
            READ16
            sprintf(dest, "ld %s, $%04x", regs16[(opcode - 0x01) >> 4], arg16);
            break;
        case 0x02:
        case 0x12:
        case 0x22:
        case 0x32:
            sprintf(dest, "ld [%s], a", regs16_2[(opcode - 0x02) >> 4]);
            break;
        case 0x03:
        case 0x13:
        case 0x23:
        case 0x33:
            sprintf(dest, "inc %s", regs16[(opcode - 0x03) >> 4]);
            break;
        case 0x04:
        case 0x0c:
        case 0x14:
        case 0x1c:
        case 0x24:
        case 0x2c:
        case 0x34:
        case 0x3c:
            sprintf(dest, "inc %s", regs8[(opcode - 0x04) >> 3]);
            break;
        case 0x05:
        case 0x0d:
        case 0x15:
        case 0x1d:
        case 0x25:
        case 0x2d:
        case 0x35:
        case 0x3d:
            sprintf(dest, "dec %s", regs8[(opcode - 0x05) >> 3]);
            break;
        case 0x06:
        case 0x0e:
        case 0x16:
        case 0x1e:
        case 0x26:
        case 0x2e:
        case 0x36:
        case 0x3e:
            READ8
            sprintf(dest, "ld %s, $%02x", regs8[(opcode - 0x06) >> 3], arg8);
            break;
        case 0x07:
            strcpy(dest, "rlca");
            break;
        case 0x08:
            READ16
            sprintf(dest, "ld [$%04x], sp", arg16);
            break;
        case 0x09:
        case 0x19:
        case 0x29:
        case 0x39:
            sprintf(dest, "add hl, %s", regs16[(opcode - 0x09) >> 4]);
            break;
        case 0x0a:
        case 0x1a:
        case 0x2a:
        case 0x3a:
            sprintf(dest, "ld a, [%s]", regs16_2[(opcode - 0x0a) >> 4]);
            break;
        case 0x0b:
        case 0x1b:
        case 0x2b:
        case 0x3b:
            sprintf(dest, "dec %s", regs16[(opcode - 0x0b) >> 4]);
            break;
        case 0x0f:
            strcpy(dest, "rrca");
            break;
        case 0x10:
            strcpy(dest, "stop");
            break;
        case 0x17:
            strcpy(dest, "rla");
            break;
        case 0x18:
            READ8
            arg16 = inttogbaddr(ftell(file)).addr + (s8)arg8;
            sprintf(dest, "jr $%04x", arg16);
            break;
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38:
            READ8
            arg16 = inttogbaddr(ftell(file)).addr + (s8)arg8;
            sprintf(dest, "jr %s, $%04x", flags[(opcode - 0x20) >> 3], arg16);
            break;
        case 0x1f:
            strcpy(dest, "rra");
            break;
        case 0x27:
            strcpy(dest, "daa");
            break;
        case 0x2f:
            strcpy(dest, "cpl");
            break;
        case 0x37:
            strcpy(dest, "scf");
            break;
        case 0x3f:
            strcpy(dest, "ccf");
            break;
        case 0x76:
            strcpy(dest, "halt");
            break;
        case 0x40 ... 0x75:
        case 0x77 ... 0x7f:
            sprintf(dest, "ld %s, %s", regs8[(opcode - 0x40) >> 3], regs8[opcode & 0x07]);
            break;
        case 0x80 ... 0xbf:
            sprintf(dest, "%s %s", arith[(opcode - 0x80) >> 3], regs8[opcode & 0x07]);
            break;
        case 0xc0:
        case 0xc8:
        case 0xd0:
        case 0xd8:
            sprintf(dest, "ret %s", flags[(opcode - 0xc0) >> 3]);
            break;
        case 0xc1:
        case 0xd1:
        case 0xe1:
        case 0xf1:
            sprintf(dest, "pop %s", regs16_3[(opcode - 0xc1) >> 4]);
            break;
        case 0xc2:
        case 0xca:
        case 0xd2:
        case 0xda:
            READ16
            sprintf(dest, "jp %s, $%04x", flags[(opcode - 0xc2) >> 3], arg16);
            break;
        case 0xc3:
            READ16
            sprintf(dest, "jp $%04x", arg16);
            break;
        case 0xc4:
        case 0xcc:
        case 0xd4:
        case 0xdc:
            READ16
            sprintf(dest, "call %s, $%04x", flags[(opcode - 0xc4) >> 3], arg16);
            break;
        case 0xc5:
        case 0xd5:
        case 0xe5:
        case 0xf5:
            sprintf(dest, "push %s", regs16_3[(opcode - 0xc5) >> 4]);
            break;
        case 0xc6:
        case 0xce:
        case 0xd6:
        case 0xde:
        case 0xe6:
        case 0xee:
        case 0xf6:
        case 0xfe:
            READ8
            sprintf(dest, "%s $%02x", arith[(opcode - 0xc6) >> 3], arg8);
            break;
        case 0xc7:
        case 0xcf:
        case 0xd7:
        case 0xdf:
        case 0xe7:
        case 0xef:
        case 0xf7:
        case 0xff:
            sprintf(dest, "rst $%02x", opcode - 0xc7);
            break;
        case 0xc9:
            strcpy(dest, "ret");
            break;
        case 0xcd:
            READ16
            sprintf(dest, "call $%04x", arg16);
            break;
        case 0xd9:
            strcpy(dest, "reti");
            break;
        case 0xe0:
            READ8
            sprintf(dest, "ld [$ff%02x], a", arg8);
            break;
        case 0xe2:
            strcpy(dest, "ld [$ff00+c], a");
            break;
        case 0xe8:
            READ8
            sprintf(dest, "add sp, $%02x", (s8)arg8);
            break;
        case 0xe9:
            strcpy(dest, "jp hl");
            break;
        case 0xea:
            READ16
            sprintf(dest, "ld [$%04x], a", arg16);
            break;
        case 0xf0:
            READ8
            sprintf(dest, "ld a, [$ff%02x]", arg8);
            break;
        case 0xf3:
            strcpy(dest, "di");
            break;
        case 0xf8:
            READ8
            sprintf(dest, "ld hl, sp+$%02x", arg8);
            break;
        case 0xf9:
            strcpy(dest, "ld sp, hl");
            break;
        case 0xfa:
            READ16
            sprintf(dest, "ld a, [$%04x]", arg16);
            break;
        case 0xfb:
            strcpy(dest, "ei");
            break;
        case 0xcb:
            READ8
            if (arg8 < 0x40)
                sprintf(dest, "%s %s", bitops[arg8 >> 3], regs8[arg8 & 0x07]);
            else
                sprintf(dest, "%s %d, %s", bitops2[(arg8 - 0x40) >> 6], (arg8 >> 3) & 0x07, regs8[arg8 & 0x07]);
            break;
        default:
            sprintf(dest, "--invalid $%02x--", opcode);
            break;
    }
#undef READ8
#undef READ16
#undef READ
    return size;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("usage: gbz80-none-any-objdump [filename] [start] [size]\n");
        return 1;
    }
    long start = strtol(argv[2], NULL, 0);
    long length = strtol(argv[3], NULL, 0);
    char buffer[20];
    char buff2[3];
    int size;
    long curpos;
    gbaddr_t gbaddr;
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        fprintf(stderr, "error: file not found: %s\n", argv[1]);
        return ENOENT;
    }
    fseek(file, start, SEEK_SET);
    while (!feof(file) && (curpos = ftell(file)) < start + length) {
        gbaddr = inttogbaddr(curpos);
        memset(buffer, 0, sizeof buffer);
        memset(buff2, 0, sizeof buff2);
        size = get_opcode(buffer, file);
        if (size == 0)
            return 2;
        fseek(file, -size, SEEK_CUR);
        if (fread(buff2, 1, (size_t)size, file) != size)
            FILE_READ_ERROR(size);
        printf("%08zx (%02x:%04x) %-20s (", curpos, gbaddr.bank, gbaddr.addr, buffer);
        for (int i=0; i<size; i++) {
            printf("%02x", (u8)buff2[i]);
            if (i != size - 1)
                printf(" ");
        }
        printf(")\n");
    }
    return 0;
}
