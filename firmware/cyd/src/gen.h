#pragma once
#include "Pollinations/Pollinations.h"

#include "config.h"
#include "db.h"
#include "led.h"

Pollinations gen;
bool gen_flag = 0;
bool gen_image_ready = false;

typedef std::function<void()> GenCallback;
static GenCallback _gen_start_cb = nullptr;
static GenCallback _gen_end_cb = nullptr;

void gen_on_start(GenCallback cb) { _gen_start_cb = cb; }
void gen_on_end(GenCallback cb)   { _gen_end_cb = cb; }

void generate() {
    gen_flag = 1;
}

void gen_tick() {
    if (gen_flag) {
        gen_flag = 0;
        if (_gen_start_cb) _gen_start_cb();
        led_generating();

        gen.setScale(DISP_SCALE);
        bool ok = gen.generate(
            db[kk::gen_query],
            DISP_WIDTH,
            DISP_HEIGHT,
            db[kk::gen_negative]);

        if (ok) {
            gen_image_ready = true;
            led_ok();
        } else {
            led_error();
        }

        if (_gen_end_cb) _gen_end_cb();
    }
}
