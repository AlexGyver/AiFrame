#pragma once
#include <GyverDBFile.h>
#include <LittleFS.h>
GyverDBFile db(&LittleFS, "settings.db");

enum kk : size_t {
    wifi_ssid,
    wifi_pass,
    poll_key,
    gen_query,
    gen_negative,
    auto_gen,
    auto_prd,
};

void db_init() {
    LittleFS.begin();
    db.begin();
    db.init(kk::wifi_ssid, "");
    db.init(kk::wifi_pass, "");
    db.init(kk::poll_key, "");
    db.init(kk::gen_query, "");
    db.init(kk::gen_negative, "");
    db.init(kk::auto_gen, 0);
    db.init(kk::auto_prd, 600);
}
