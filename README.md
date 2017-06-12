# GBZ80 Object Dumper

Version 0.1a (2017-06-12)

Purpose: To dump a GBZ80-compiled object into disassembled instructions.

## Setup

On Windows, you will need Cygwin with `gcc-core`, `git`, and `make`.  For Linux and Mac OS X, make sure you have `gcc`, `git`, and `make` defined in your environment.  Then clone the repository:

    git clone git://github.com/pikalaxalt/gbz80-none-any-objdump

If using SSH instead of HTTPS:

    git clone git@github.com:pikalaxalt/gbz80-none-any-objdump.git

## Building

Navigate to the directory to which you cloned the repository, and run `make`.

## Running

Usage: `gbz80-none-any-objdump [filename] [start] [size]`

`filename` is the full (absolute or relative) path to the binary you wish to dump.\
`start` is the start address.  It must be in one of the standard bases (decimal, hexadecimal, binary).\
`size` is the number of bytes to dump.  It must be in one of the standard bases.  The actual size dumped can be more than specified (i.e. a multi-byte opcode extends beyond the specified size) or less than specified (i.e. EOF is reached before the specified size is dumped).

## Reading the output

The output is a plaintext file with fixed-size columns.  Here's an example output:

    00000005 (00:0005) jp $00c3             (c3 c3 00)
    00000008 (00:0008) jp $2acc             (c3 cc 2a)
    0000000b (00:000b) rst $08              (cf)
    0000000c (00:000c) sub h                (94)
    0000000d (00:000d) xor [hl]             (ae)
    0000000e (00:000e) ld c, b              (48)
    0000000f (00:000f) ld d, b              (50)
    00000010 (00:0010) jp $2c71             (c3 71 2c)

Column 1: Absolute offset (hex)\
Column 2: (Bank:Addr) - offset in GameBoy address format (hex)\
Column 3: Disassembled instruction, in [RGBDS](https://github.com/rednex/rgbds) syntax\
Column 4: In parentheses, the sequence of raw bytes dumped.  Note that 16-bit arguments are stored little-endian in ROM.

## To-Do

Future work will include additional functionality present in other flavors of objdump, including flags to set the output style, error on invalid opcodes, etc..
