#ifndef __JCC_H__
#define __JCC_H__

/* 8-bit relative (0x70..0x7f) */
make_helper(jcc_jo_b);
make_helper(jcc_jno_b);
make_helper(jcc_jb_b);
make_helper(jcc_jae_b);
make_helper(jcc_je_b);
make_helper(jcc_jne_b);
make_helper(jcc_jbe_b);
make_helper(jcc_ja_b);
make_helper(jcc_js_b);
make_helper(jcc_jns_b);
make_helper(jcc_jp_b);
make_helper(jcc_jnp_b);
make_helper(jcc_jl_b);
make_helper(jcc_jge_b);
make_helper(jcc_jle_b);
make_helper(jcc_jg_b);

/* 32-bit relative (0x0f 0x80..0x8f) */
make_helper(jcc32_jo_l);
make_helper(jcc32_jno_l);
make_helper(jcc32_jb_l);
make_helper(jcc32_jae_l);
make_helper(jcc32_je_l);
make_helper(jcc32_jne_l);
make_helper(jcc32_jbe_l);
make_helper(jcc32_ja_l);
make_helper(jcc32_js_l);
make_helper(jcc32_jns_l);
make_helper(jcc32_jp_l);
make_helper(jcc32_jnp_l);
make_helper(jcc32_jl_l);
make_helper(jcc32_jge_l);
make_helper(jcc32_jle_l);
make_helper(jcc32_jg_l);

#endif
