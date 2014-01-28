/*
 * Network Class For D2RM.
 *
 */

#ifndef HTTP_H
#define HTTP_H

#include <QObject>
#include <QtNetwork>
#include <QList>
#include <QDateTime>
#include <QEventLoop>
#include <QThread>

class Http : public QObject
{
    Q_OBJECT
public:
    explicit Http(QObject *parent = 0);
    explicit Http(const QString &strUrl, QObject *parent = 0);
    explicit Http(const QUrl &Url, QObject *parent = 0);
    ~Http();

    QUrl url() const;
    void setUrl(const QString &strUrl);
    void setUrl(const QUrl &Url);
    virtual void close();

    QByteArray readAll();

    QNetworkReply::NetworkError error() const;
    QString errorString(QNetworkReply::NetworkError networkerror = QNetworkReply::NoError) const;
    int response() const;
    QString responseString() const;

    bool isFinished() const;
    bool isRunning() const;
    bool isThreadRunning() const;

    bool waitForConnect(int nTimeOutms, QNetworkAccessManager *manager);
    bool waitForData(int nTimeOutms);

    QString getContentType() const;
    QDateTime getLastModified() const;

    QNetworkReply *getReply();

    void endSocketThread();

    void wait() const;

signals:
    void signalReadTimeout();

public slots:
    void slotOpen(void *pReturnSuccess, void *pLoop, qint64 offset = 0);
    void slotRead(void *pResultBytesRead, void *pLoop, void *data, qint64 maxlen);
    void slotReadArray(void *pLoop, QByteArray *data, qint64 maxlen);

private slots:
    void slotError(QNetworkReply::NetworkError);
    void slotSslErrors(QList<QSslError>);
    void slotWaitTimeout();

protected:
    void clear();
    void init();

    QEventLoop m_loop;

    QNetworkReply *m_pReply;
    QUrl m_url;
    QThread *m_pSocketThread;
    const QNetworkProxy *m_pNetworkProxy;
    QNetworkReply::NetworkError m_NetworkError;
    QList<QSslError> m_sslErrors;
    int m_nResponse;
    QString m_strResponse;
    bool m_bReadTimeOutms;
    qint64 m_nSize;

    QString m_strContentType;
    QDateTime m_LastModified;
};

#endif // HTTP_H
