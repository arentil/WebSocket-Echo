#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Report a failure
void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver resolver_;
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;

public:
    explicit session(net::io_context& ioc)
    : resolver_(net::make_strand(ioc)),
      ws_(net::make_strand(ioc))
    {
    }

    void run(char const* host, char const* port)
    {
        host_ = host;

        resolver_.async_resolve(
          host,
          port,
          beast::bind_front_handler(&session::on_resolve, shared_from_this()));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        // Set the timeout for the operation
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(ws_).async_connect(
          results,
          beast::bind_front_handler(&session::on_connect, shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
    {
        if(ec)
            return fail(ec, "connect");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws_).expires_never();

        // Set suggested timeout settings for the websocket
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(websocket::stream_base::decorator(
          [](websocket::request_type& req)
          {
              req.set(http::field::user_agent,
                      std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
          }));

        // Perform the websocket handshake
        ws_.async_handshake(host_,
                            "/",
                            beast::bind_front_handler(&session::on_handshake, shared_from_this()));
    }

    void on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

       std::cout << "Connected to server" << std::endl;

        ws_.async_read(
          buffer_,
          beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void write(const std::string& text)
    {
        ws_.async_write(net::buffer(text),
                        beast::bind_front_handler(&session::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
        {
            if(ec == net::error::operation_aborted || ec == websocket::error::closed)
            {
                // if client closed or peer(server) closed
                return;
            }
            else
                return fail(ec, "read");
        }

        std::cout << "received: \"" << beast::make_printable(buffer_.data()) << "\"" << std::endl;
        buffer_.clear();

        ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void close()
    {
        ws_.async_close(websocket::close_code::normal,
                        beast::bind_front_handler(&session::on_close, shared_from_this()));
    }

    void on_close(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "close");
    }
};

int main(int argc, char** argv)
{
    net::io_context ioc;

    auto session_ptr = std::make_shared<session>(ioc);
    session_ptr->run("localhost", "9001");

    std::thread cin_thread(
      [session_ptr, &ioc]()
      {
          std::string text;
          while(std::cin
                >> text) // technically its unsafe, practically this is the only std::cin used.
          {
              if(ioc.stopped() || text == "exit" || text == "stop")
              {
                  boost::asio::post(ioc.get_executor(),
                                    [session_ptr]()
                                    {
                                        if(std::weak_ptr<session> weak_session_ptr = session_ptr;
                                           !weak_session_ptr.expired())
                                            weak_session_ptr.lock()->close();
                                    });
                  return; // exit thread func
              }

              boost::asio::post(ioc.get_executor(),
                                [session_ptr, text]()
                                {
                                    if(std::weak_ptr<session> weak_session_ptr = session_ptr;
                                       !weak_session_ptr.expired())
                                        weak_session_ptr.lock()->write(text);
                                });
          }
      });

    ioc.run();
    cin_thread.join();

    return EXIT_SUCCESS;
}