#include "http.h"

class QThreadEx : public QThread
{
public:
    static void msleep(unsigned long ms)
    {
        QThread::msleep(ms);
    }
};

static void msleep(int ms)
{
    QThreadEx::msleep(ms);
}

Http::Http(QObject *parent) :
    QObject(parent),
    m_pSocketThread(NULL),
    m_pNetworkProxy(NULL)
{
    clear();
    init();
}

Http::~Http()
{
    m_loop.exit(1);
    close();
    endSocketThread();
}

Http::Http(const QString &strUrl, QObject *parent) : QObject(parent), m_pSocketThread(NULL), m_pNetworkProxy(NULL)
{
    clear();
    init();
    setUrl(strUrl);
}


Http::Http(const QUrl &Url, QObject *parent) : QObject(parent), m_pSocketThread(NULL), m_pNetworkProxy(NULL)
{
    clear();
    init();
    setUrl(Url);
}

void Http::endSocketThread()
{
    if(m_pSocketThread != NULL)
    {
        m_pSocketThread->quit();
        m_pSocketThread->wait();

        delete m_pSocketThread;
        m_pSocketThread = NULL;
    }
}

void Http::clear()
{
    m_pReply = NULL;
    m_NetworkError = QNetworkReply::NoError;
    m_nResponse = 0;
    m_bReadTimeOutms = false;
    m_strContentType.clear();
    m_nSize = 0;
    m_sslErrors.clear();
}

void Http::init()
{
    endSocketThread();

    m_pSocketThread = new QThread(this);
    moveToThread(m_pSocketThread);
    m_pSocketThread->start(QThread::HighestPriority);
}

QUrl Http::url() const
{
    return m_url;
}

void Http::setUrl(const QString &strUrl)
{
    m_url = QUrl::fromEncoded(strUrl.toUtf8());
}

void Http::setUrl(const QUrl &Url)
{
    m_url = Url;
}
