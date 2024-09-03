num = 512

txt = """
#pragma once
#define _FM_EXP(x) x
#define _FM_CONCAT(x, y) _FM_CONCAT_IMPL(x, y)
#define _FM_CONCAT_IMPL(x, y) x##y

#define _FM_FOR_NARG(...) _FM_FOR_NARG_(__VA_ARGS__, _FM_FOR_RSEQ_N())
#define _FM_FOR_NARG_(...) _FM_EXP(_FM_FOR_ARG_N(__VA_ARGS__))

#define _FM_FOR_ARG_N("""

for i in range(1, num):
    txt += f"_{i}, "

txt += """N, ...) N
#define _FM_FOR_RSEQ_N() """

for i in range(0, num):
    txt += f"{num - i - 1}, "

txt += '\n'

for i in range(0, num):
    txt += f"\n#define _FM_FOR_{i + 1}(f, N, p, x, ...) f(N, {i}, p, x)"
    if (i): txt += f" _FM_EXP(_FM_FOR_{i}(f, N, p, __VA_ARGS__))"

txt += """

#define _FM_FOR_(N, f, p, ...) _FM_EXP(_FM_CONCAT(_FM_FOR_, N)(f, N, p, __VA_ARGS__))
#define FOR_MACRO(f, p, ...) _FM_FOR_(_FM_FOR_NARG(__VA_ARGS__), f, p, __VA_ARGS__)
"""

with open('FOR_MACRO.h', 'w') as f:
    f.write(txt)