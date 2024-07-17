#pragma once
#include "ServerBase.h"

#define GS_CLIENT_TOUT 1500

namespace ghttp {

template <typename server_t, typename client_t>
class Server : public ServerBase {
   public:
    Server(uint16_t port) : server(port) {}

    // запустить
    void begin() {
        server.begin();
    }

    // вызывать в loop
    void tick(HeadersCollector* collector = nullptr) {
        client_t client = server.accept();
        if (client) {
            client.setTimeout(GS_CLIENT_TOUT);
            handleRequest(client, collector);
        }
    }

    server_t server;

   private:
};

}