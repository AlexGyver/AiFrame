#include <Arduino.h>
#include <GyverHTTP.h>
#include <DNSServer.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

ghttp::Server<WiFiServer, WiFiClient> server(80);
DNSServer dns;

const char html_p[] PROGMEM = R"raw(
<!DOCTYPE html>
<html lang="en">
<body>
    <h1>Hello!</h1>
</body>
</html>
)raw";

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_AP);
    WiFi.softAP("AP ESP");
    Serial.print("AP: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
    dns.start(53, "*", WiFi.softAPIP());

    server.onRequest([](ghttp::ServerBase::Request req) {
        server.sendFile_P(html_p, "text/html");
    });
}

void loop() {
    server.tick();
    dns.processNextRequest();
}