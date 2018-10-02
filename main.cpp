#include <QCoreApplication>
#include <QGuiApplication>
#include <QWebEngineUrlRequestJob>
#include <QWebEngineUrlSchemeHandler>
#include <QtWebEngine>
#include <QDebug>
#include <QBuffer>


class StaticPageSchemeReply : public QIODevice {
    Q_OBJECT

public:

    explicit StaticPageSchemeReply(QWebEngineUrlRequestJob *job,
                                   QObject                 *parent = nullptr)
        : QIODevice(parent)
    {
        m_job    = job;
        m_loaded = false;
        open(QIODevice::ReadOnly);
        m_buffer.open(QIODevice::ReadWrite);
    }

    qint64 bytesAvailable() const
    {
        return m_buffer.bytesAvailable();
    }

    qint64 readData(char *data, qint64 maxSize)
    {
        qDebug() << "readData";
        loadPage();
        return m_buffer.read(data, maxSize);
    }

    qint64 writeData(const char *data, qint64 len)
    {
        Q_UNUSED(data);
        Q_UNUSED(len);
        return 0;
    }

private Q_SLOTS:

    void loadPage()
    {
        qDebug() << "loadPage";

        if (m_loaded)
        {
            return;
        }

        QTextStream stream(&m_buffer);
        stream.setCodec("UTF-8");
        stream << getPage();

        stream.flush();
        m_buffer.reset();
        m_loaded = true;
    }

private:

    QString getPage()
    {
        static QString page;

        page.append(QString(
                        "<html><body><head><style>h1 {background-color: silver;}</style></head><h1>Hello World!</h1></body></html>"));
        qDebug() << "Text: " << page;
        return page;
    }

    bool m_loaded;
    QBuffer m_buffer;
    QWebEngineUrlRequestJob *m_job;
};


class StaticPageSchemeHandler : public QWebEngineUrlSchemeHandler {
public:

    explicit StaticPageSchemeHandler(QObject *parent = nullptr) {}

    void requestStarted(QWebEngineUrlRequestJob *job)
    {
        if (handleRequest(job)) {
            return;
        }

        qDebug() << "Url: " << job->requestUrl().url();
        QStringList knownPages;
        knownPages << "bummer" << "doc";

        if (knownPages.contains(job->requestUrl().scheme())) {
            qDebug() << "Scheme: " << job->requestUrl().scheme();
            job->reply(QByteArrayLiteral("text/html"),
                       new StaticPageSchemeReply(job));
        }
        else job->fail(QWebEngineUrlRequestJob::UrlInvalid);
    }

private:

    bool handleRequest(QWebEngineUrlRequestJob *job)
    {
        return false;
    }
};


class SchemeHandler : public QWebEngineUrlSchemeHandler {
    Q_OBJECT

public:

    SchemeHandler(QObject *parent = nullptr)
        : QWebEngineUrlSchemeHandler(parent)
    {}

    void requestStarted(QWebEngineUrlRequestJob *job)
    {
        QUrl newUrl("http://doc.qt.io");

        newUrl.setPath(job->requestUrl().path());
        qDebug() << "Redirecting" << job->requestUrl() << "to" << newUrl;
        job->redirect(newUrl);
    }
};


class CustomSchemeApp : public QGuiApplication {
    Q_OBJECT

public:

    CustomSchemeApp(int& argc, char **argv) : QGuiApplication(argc, argv) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setOrganizationName("testproject");
        QCoreApplication::setApplicationName("customscheme");
        QCoreApplication::setApplicationVersion("0.1");

        QtWebEngine::initialize();
        m_engine.rootContext()->setContextProperty("dekkoapp", this);
        QQuickWebEngineProfile::defaultProfile()->installUrlSchemeHandler(
            "help",
            &m_handler);
        QQuickWebEngineProfile::defaultProfile()->installUrlSchemeHandler(
            "bummer",
            &m_sphandler);
        m_engine.load(QUrl(QStringLiteral("qrc:/weby.qml")));
    }

    ~CustomSchemeApp() {}

private:

    QQmlApplicationEngine m_engine;
    SchemeHandler m_handler;
    StaticPageSchemeHandler m_sphandler;
};


int main(int argc, char *argv[])
{
    CustomSchemeApp app(argc, argv);

    return app.exec();
}

#include "main.moc"
