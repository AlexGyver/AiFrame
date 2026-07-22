#pragma once
#include "Kandinsky/Kandinsky.h"

#include "config.h"
Kandinsky gen;
bool gen_flag = 0;

void generate() {
    gen_flag = 1;
}

void gen_tick() {
    gen.tick();

    if (gen_flag) {
        gen_flag = 0;

        gen.setScale(DISP_SCALE);
        gen.generate(
            db[kk::gen_query],
            DISP_WIDTH * DISP_SCALE,
            DISP_HEIGHT * DISP_SCALE,
            Text(gen.styles).getSub(db[kk::gen_style], ';'),
            db[kk::gen_negative]);
    }
}