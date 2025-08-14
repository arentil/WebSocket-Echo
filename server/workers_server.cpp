#include "App.h"

#include <iostream>
#include <unordered_map>
#include <thread>

struct PerSocketData
{
    PerSocketData() { id = id_counter++; }
    static int id_counter;
    int id = 0;
};
int PerSocketData::id_counter = 0;

int main() 
{
    uWS::App app;
    uWS::Loop* loop = uWS::Loop::get();
    app.ws<PerSocketData>("/*", {
        .compression = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR | uWS::DEDICATED_DECOMPRESSOR),
        .maxPayloadLength = 100 * 1024 * 1024,
        .idleTimeout = 120, // time in seconds until connection is closed
        .maxBackpressure = 100 * 1024 * 1024,
        .closeOnBackpressureLimit = false,
        .resetIdleTimeoutOnSend = true,
        .sendPingsAutomatically = true,
        .upgrade = nullptr,
        .open = [loop](auto *ws) 
        {
            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
            ws->send("Open connection");
            
            std::thread t1{[loop, ws](){
                loop->defer([ws](){
                    ws->send("Threaded send");
                });
            }};
            t1.detach();
        },
        .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {

            std::cout << "Received: \"" << message << "\", socket id: " << ws->getUserData()->id << std::endl;
            ws->send(message, opCode, false);
        },
        .dropped = [](auto *ws, std::string_view /*message*/, uWS::OpCode /*opCode*/) {
            /* A message was dropped due to set maxBackpressure and closeOnBackpressureLimit limit */
        },
        .drain = [](auto */*ws*/) {
            /* Check ws->getBufferedAmount() here */
        },
        .ping = [](auto */*ws*/, std::string_view) {
            /* Not implemented yet */
        },
        .pong = [](auto */*ws*/, std::string_view) {
            /* Not implemented yet */
        },
        .close = [](auto *ws, int /*code*/, std::string_view message) {
            std::cout << "Closing socket: " << ws->getUserData()->id << " " << message << std::endl;
            /* You may access ws->getUserData() here */
        }
    }).listen(9001, [](auto *listen_socket) {
        if (listen_socket) {
            std::cout << "Listening on port " << 9001 << std::endl;
        }
    }).run();
}