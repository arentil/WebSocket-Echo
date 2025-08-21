#include "App.h"

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <queue>
#include <thread>
#include <unordered_map>

struct PerSocketData
{
    PerSocketData()
    {
        id = id_counter++;
    }
    static int id_counter;
    int id = 0;
};
int PerSocketData::id_counter = 0;

class ThreadPool
{
public:
    explicit ThreadPool(size_t n) : stop(false)
    {
        for(size_t i = 0; i < n; i++)
            workers.emplace_back([this] { workerLoop(); });
    }

    ~ThreadPool()
    {
        {
            std::unique_lock lk(m);
            stop = true;
        }
        cv.notify_all();
        for(auto& t : workers)
            t.join();
    }

    template <class F> void post(F&& f)
    {
        {
            std::unique_lock lk(m);
            q.emplace(std::move(f));
        }
        cv.notify_one();
    }

private:
    void workerLoop()
    {
        for(;;)
        {
            std::function<void()> job;
            {
                std::unique_lock lk(m);
                cv.wait(lk, [this] { return !q.empty() || stop; });
                if(stop && q.empty())
                    return;
                job = std::move(q.front());
                q.pop();
            }
            job();
        }
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> q;
    std::mutex m;
    std::condition_variable cv;
    bool stop;
};

int main()
{
    uWS::App app;

    ThreadPool t_pool(std::thread::hardware_concurrency() - 1);

    uWS::Loop* loop = uWS::Loop::get();

    app
      .ws<PerSocketData>(
        "/*",
        {.compression
         = uWS::CompressOptions(uWS::DEDICATED_COMPRESSOR | uWS::DEDICATED_DECOMPRESSOR),
         .maxPayloadLength = 100 * 1024 * 1024,
         .idleTimeout = 120, // time in seconds until connection is closed
         .maxBackpressure = 100 * 1024 * 1024,
         .closeOnBackpressureLimit = false,
         .resetIdleTimeoutOnSend = true,
         .sendPingsAutomatically = true,
         .upgrade = nullptr,
         .open =
           [loop, &app, &t_pool](auto* ws)
         {
             ws->send("Hello from server!");

             // subscribe this WS to topic
             const auto ws_id_str = std::to_string(ws->getUserData()->id);
             ws->subscribe("user:" + ws_id_str);

             t_pool.post(
               [loop, &app, ws_id_str]()
               {
                   // do some heavy logic...

                   // publish on App()'s thread
                   loop->defer(
                     [&app, ws_id_str]()
                     {
                         app.publish(
                           "user:" + ws_id_str,
                           "This is a message published using workers thread, "
                           "however publish itself has been executed in App's loop "
                           "using uWS::Loop::defer since its the only thread-safe function.",
                           uWS::OpCode::TEXT);
                     });
               });
         },
         .message =
           [loop, &app, &t_pool](auto* ws, std::string_view message, uWS::OpCode opCode)
         {
             const auto ws_id_str = std::to_string(ws->getUserData()->id);
             t_pool.post(
               [loop, &app, ws_id_str, message = std::string{message}]()
               {
                   // do some heavy logic...

                   // publish on App()'s thread
                   loop->defer([&app, ws_id_str, message]()
                               { app.publish("user:" + ws_id_str, message, uWS::OpCode::TEXT); });
               });
         },
         .dropped =
           [](auto* ws, std::string_view /*message*/, uWS::OpCode /*opCode*/)
         {
             /* A message was dropped due to set maxBackpressure and closeOnBackpressureLimit limit */
         },
         .drain =
           [](auto* /*ws*/)
         {
             /* Check ws->getBufferedAmount() here */
         },
         .ping =
           [](auto* /*ws*/, std::string_view)
         {
             /* Not implemented yet */
         },
         .pong =
           [](auto* /*ws*/, std::string_view)
         {
             /* Not implemented yet */
         },
         .close =
           [](auto* ws, int /*code*/, std::string_view message)
         {
             /* You may access ws->getUserData() here */
             // unsubscribe socket from room?
         }})
      .listen(9001,
              [](auto* listen_socket)
              {
                  if(listen_socket)
                  {
                      std::cout << "Listening on port " << 9001 << std::endl;
                  }
              })
      .run();

    return 0;
}