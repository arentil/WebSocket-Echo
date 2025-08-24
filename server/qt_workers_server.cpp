#include <QCoreApplication>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QThreadPool>
#include <QtConcurrent>
#include <QPointer>
#include <QDebug>

#include <functional>

///
/// Run exe with qt cmd
///

class echo_server : public QObject
{
    Q_OBJECT

    using on_message_handler_type = std::move_only_function<void(QWebSocket*, const QString&)>;

public:
    echo_server(const quint16 port, QObject* parent = nullptr)
    : QObject(parent), server_("WebSocket Echo Server", QWebSocketServer::NonSecureMode, this)
    {
        if (server_.listen(QHostAddress::Any, port))
        {
            qDebug() << "Listening on port " << port;
            connect(&server_, &QWebSocketServer::newConnection, this, &echo_server::on_new_connection);
        }
    }

    void on_message_handler(on_message_handler_type handler)
    {
        on_message_handler_ = std::move(handler);
    }

private Q_SLOTS:
    void on_new_connection()
    {
        QWebSocket *socket = server_.nextPendingConnection(); // socket can be saved for later usage
        connect(socket, &QWebSocket::textMessageReceived, this, &echo_server::on_message);
        connect(socket, &QWebSocket::disconnected, this, &echo_server::on_disconnected);
    }

    void on_message(const QString& message)
    {
        QWebSocket* client = qobject_cast<QWebSocket*>(sender());
        if (on_message_handler_)
            on_message_handler_(client, message);
    }

    void on_disconnected()
    {
        QWebSocket *client = qobject_cast<QWebSocket *>(sender());
        client->deleteLater();
    }

private:
    QWebSocketServer server_;
    on_message_handler_type on_message_handler_;
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QThreadPool pool;
    pool.setMaxThreadCount(QThread::idealThreadCount());

    echo_server echo_server(9001);
    echo_server.on_message_handler([&pool](QWebSocket* socket, const QString& message){
        QPointer<QWebSocket> ws = socket;
        QtConcurrent::run(&pool, [ws, message](){
            if (!ws) return;
            // do some heavy logic
            QMetaObject::invokeMethod(ws.data(), [ws, message]{
                    ws->sendTextMessage(message);
                }, Qt::QueuedConnection);
        });
    });


    return a.exec();
}

#include "qt_workers_server.moc"