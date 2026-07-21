#pragma once
#include <GyverDBFile.h>
#include <LittleFS.h>

// New file name: old "settings.db" from Kandinsky has shifted key indices
// (string where bool is expected) and can crash on save/update.
GyverDBFile db(&LittleFS, "/settings_poll.db");

// Start at 100 so indices never collide with old Kandinsky schema (0..8)
enum kk : size_t {
    wifi_ssid = 100,
    wifi_pass,
    poll_key,
    gen_query,
    gen_negative,
    auto_gen,
    auto_prd,
};

void db_init() {
    LittleFS.begin();

    // Drop legacy Kandinsky DB if present (wrong schema)
    if (LittleFS.exists("/settings.db")) {
        LittleFS.remove("/settings.db");
    }
    if (LittleFS.exists("settings.db")) {
        LittleFS.remove("settings.db");
    }

    db.begin();
    db.init(kk::wifi_ssid, "");
    db.init(kk::wifi_pass, "");
    db.init(kk::poll_key, "");
    db.init(kk::gen_query, "");
    db.init(kk::gen_negative, "");
    db.init(kk::auto_gen, 0);
    db.init(kk::auto_prd, 600);
}
