#include <Arduino.h>
#include <GyverHTTP.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#define WIFI_SSID ""
#define WIFI_PASS ""

ghttp::Server<WiFiServer, WiFiClient> server(80);

const char html_p[] PROGMEM = R"raw(
<!DOCTYPE html>
<html lang="en">

<body>
    <button onclick="url()">url</button>
    <button onclick="json()">json</button>
    <button onclick="answer()">answer</button>
    <button onclick="answer_headers()">answer+headers</button>
    <button onclick="file_headers()">file+headers</button>
    <input type="file">
    <button onclick="file()">send file</button>
</body>

<script>
    async function url() {
        const res = await fetch('/qs?kek=pek&lol=kek', {
            method: 'POST',
        });
    }
    async function answer() {
        const res = await fetch('/answer', {
            method: 'POST',
        });
        console.log(...res.headers);
        console.log(await res.text());
    }
    async function answer_headers() {
        const res = await fetch('/answer_headers', {
            method: 'POST',
        });
        console.log(...res.headers);
        console.log(await res.text());
    }
    async function file_headers() {
        const res = await fetch('/file_headers', {
            method: 'POST',
        });
        console.log(...res.headers);
        console.log(await res.text());
    }
    async function json() {
        const res = await fetch('/json', {
            method: 'POST',
            body: JSON.stringify({ test: 123 }),
        });
    }
    async function file() {
        let input = document.querySelector('input[type="file"]');
        let data = new FormData();
        data.append('upload', input.files[0], 'upload')

        const res = await fetch('/upload', {
            method: 'POST',
            body: data,
        });
    }
</script>

</html>
)raw";

void setup() {
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected");
    Serial.println(WiFi.localIP());

    server.begin();

    server.onRequest([](ghttp::ServerBase::Request req) {
        // URL
        Serial.println(req.method());
        Serial.println(req.url());
        Serial.println(req.path());
        Serial.println(req.param("kek"));
        Serial.println(req.param("lol"));

        // BODY
        Serial.println(req.body());
        // req.body().writeTo(Serial);
        // req.body().writeTo(file);
        // req.body().stream.readBytes(buf, req.length());

        // RESPONSE
        if (req.url() == "/") {
            // большие текстовые PROGMEM "файлы" эффективнее отсылать через sendFile
            // отправка идёт сильно бысрее, чем отправка в send как текст
            server.sendFile_P(html_p, "text/html");

            // server.sendFile((uint8_t*)"hello text!", 11);
            // File f = LittleFS.open("lorem.txt", "r");
            // server.sendFile(f);
        } else if (req.url() == "/answer") {
            // chunked
            server.send("hello");
            server.send(", ");
            server.send("WORLD");

            // single
            // server.sendSingle("HELLO, WORLD");
        } else if (req.url() == "/answer_headers") {
            // добавить свои хэдеры к send
            ghttp::ServerBase::Headers headers(200);
            headers.add("kek-header", "kek value");
            headers.add("another-header", "jello!");
            server.beginResponse(headers);

            server.send("this is ");
            server.send("answer");
        } else if (req.url() == "/file_headers") {
            // добавить свои хэдеры к файлу
            ghttp::ServerBase::Headers headers(200);
            headers.add("file-header", "abcdef");
            server.beginResponse(headers);

            char file[] = "hello!";
            server.sendFile((uint8_t*)file, strlen(file));
        } else {
            server.send(200);
        }
    });
}

void loop() {
    server.tick();
}