#include <QCoreApplication>
#include <QDebug>
#include <QPointer>
#include <QThreadPool>
#include <QWebSocket>
#include <QtConcurrent>

#include <iostream>

class echo_client : public QObject
{
    Q_OBJECT
public:
    explicit echo_client(const QUrl& url, QObject* parent = nullptr)
    : QObject(parent)
    {
        connect(&ws_, &QWebSocket::connected, this, &echo_client::on_connected);
        ws_.open(url);
    }

    void send(const QString& message)
    {
        ws_.sendTextMessage(message);
    }

private Q_SLOTS:
    void on_connected()
    {
        qDebug() << "Connected to server";
        connect(&ws_,
                &QWebSocket::textMessageReceived,
                this,
                &echo_client::on_text_message_received);
    }

    void on_text_message_received(const QString& message)
    {
        qDebug() << "received: " << message;
    }

private:
    QWebSocket ws_;
};

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);

    echo_client client(QUrl{"ws://localhost:9001"});

    QThread* cin_thread = QThread::create(
      [&a, &client]()
      {
          std::string text;
          while(std::cin >> text) // technically its unsafe, practically this
                                  // is the only std::cin used.
          {
              if(text == "exit" || text == "stop")
              {
                  QMetaObject::invokeMethod(
                    &a,
                    [text, &client]
                    {
                        QCoreApplication::quit(); // thread-safe
                    },
                    Qt::QueuedConnection);
                  return; // exit thread func
              }
              QMetaObject::invokeMethod(
                &a,
                [text, &client] { client.send(QString{text.data()}); },
                Qt::QueuedConnection);
          }
      });

    cin_thread->start();
    auto exec_result = a.exec();
    cin_thread->wait();

    return exec_result;
}

#include "qt_client.moc"