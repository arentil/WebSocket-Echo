#include <QCoreApplication>
#include <QDebug>
#include <QWebSocket>
#include <QWebSocketServer>

///
/// Run exe with qt cmd
///

class echo_server : public QObject
{
    Q_OBJECT
public:
    echo_server(const quint16 port, QObject* parent = nullptr)
    : QObject(parent),
      server_("WebSocket Echo Server", QWebSocketServer::NonSecureMode, this)
    {
        if(server_.listen(QHostAddress::Any, port))
        {
            qDebug() << "Listening on port " << port;
            connect(&server_,
                    &QWebSocketServer::newConnection,
                    this,
                    &echo_server::on_new_connection);
        }
    }

private Q_SLOTS:
    void on_new_connection()
    {
        QWebSocket* socket = server_.nextPendingConnection(); // socket can be saved for later usage
        connect(socket, &QWebSocket::textMessageReceived, this, &echo_server::on_message);
        connect(socket, &QWebSocket::disconnected, this, &echo_server::on_disconnected);
    }

    void on_message(const QString& message) const
    {
        QWebSocket* client = qobject_cast<QWebSocket*>(sender());
        client->sendTextMessage(message);
    }

    void on_disconnected()
    {
        QWebSocket* client = qobject_cast<QWebSocket*>(sender());
        client->deleteLater();
    }

private:
    QWebSocketServer server_;
};

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    echo_server echo_server(9001);

    return a.exec();
}

#include "qt_server.moc"