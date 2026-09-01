#include "cpu/exec/helper.h"

/* Conditional jumps.  Each condition code is emitted for both the 8-bit
 * relative form (0x70..0x7f) and the 32-bit relative form (0x0f 0x80..0x8f).
 * The condition codes are listed in the order of the i386 manual.          */

#define DATA_BYTE 1

#define JCC_NAME "jo"
#define JCC_COND (cpu.eflags.OF)
#define instr jcc_jo
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jno"
#define JCC_COND (!cpu.eflags.OF)
#define instr jcc_jno
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jb"
#define JCC_COND (cpu.eflags.CF)
#define instr jcc_jb
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jae"
#define JCC_COND (!cpu.eflags.CF)
#define instr jcc_jae
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "je"
#define JCC_COND (cpu.eflags.ZF)
#define instr jcc_je
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jne"
#define JCC_COND (!cpu.eflags.ZF)
#define instr jcc_jne
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jbe"
#define JCC_COND (cpu.eflags.CF || cpu.eflags.ZF)
#define instr jcc_jbe
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "ja"
#define JCC_COND (!cpu.eflags.CF && !cpu.eflags.ZF)
#define instr jcc_ja
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "js"
#define JCC_COND (cpu.eflags.SF)
#define instr jcc_js
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jns"
#define JCC_COND (!cpu.eflags.SF)
#define instr jcc_jns
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jp"
#define JCC_COND (cpu.eflags.PF)
#define instr jcc_jp
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jnp"
#define JCC_COND (!cpu.eflags.PF)
#define instr jcc_jnp
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jl"
#define JCC_COND (cpu.eflags.SF != cpu.eflags.OF)
#define instr jcc_jl
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jge"
#define JCC_COND (cpu.eflags.SF == cpu.eflags.OF)
#define instr jcc_jge
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jle"
#define JCC_COND (cpu.eflags.ZF || (cpu.eflags.SF != cpu.eflags.OF))
#define instr jcc_jle
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jg"
#define JCC_COND (!cpu.eflags.ZF && (cpu.eflags.SF == cpu.eflags.OF))
#define instr jcc_jg
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#undef DATA_BYTE


/* 32-bit relative forms (0x0f 0x80..0x8f). */
#define DATA_BYTE 4

#define JCC_NAME "jo"
#define JCC_COND (cpu.eflags.OF)
#define instr jcc32_jo
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jno"
#define JCC_COND (!cpu.eflags.OF)
#define instr jcc32_jno
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jb"
#define JCC_COND (cpu.eflags.CF)
#define instr jcc32_jb
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jae"
#define JCC_COND (!cpu.eflags.CF)
#define instr jcc32_jae
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "je"
#define JCC_COND (cpu.eflags.ZF)
#define instr jcc32_je
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jne"
#define JCC_COND (!cpu.eflags.ZF)
#define instr jcc32_jne
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jbe"
#define JCC_COND (cpu.eflags.CF || cpu.eflags.ZF)
#define instr jcc32_jbe
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "ja"
#define JCC_COND (!cpu.eflags.CF && !cpu.eflags.ZF)
#define instr jcc32_ja
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "js"
#define JCC_COND (cpu.eflags.SF)
#define instr jcc32_js
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jns"
#define JCC_COND (!cpu.eflags.SF)
#define instr jcc32_jns
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jp"
#define JCC_COND (cpu.eflags.PF)
#define instr jcc32_jp
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jnp"
#define JCC_COND (!cpu.eflags.PF)
#define instr jcc32_jnp
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jl"
#define JCC_COND (cpu.eflags.SF != cpu.eflags.OF)
#define instr jcc32_jl
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jge"
#define JCC_COND (cpu.eflags.SF == cpu.eflags.OF)
#define instr jcc32_jge
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jle"
#define JCC_COND (cpu.eflags.ZF || (cpu.eflags.SF != cpu.eflags.OF))
#define instr jcc32_jle
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#define JCC_NAME "jg"
#define JCC_COND (!cpu.eflags.ZF && (cpu.eflags.SF == cpu.eflags.OF))
#define instr jcc32_jg
#include "jcc-template.h"
#undef JCC_COND
#undef JCC_NAME
#undef instr

#undef DATA_BYTE
