#include "UpdateChecker.h"

UpdateChecker::UpdateChecker(QNetworkAccessManager *network_manager, const QString &base_url, const QString &client_uuid, quint32 version, QObject *parent) :
    QObject(parent)
{
    net_manager = network_manager;
    url = base_url;
    current_version = version;

    QUrl req_url = QUrl(QString(url).append("latest.release"));
    req_url.addQueryItem("uuid", client_uuid);
    QNetworkRequest req = QNetworkRequest(req_url);
    req.setPriority(QNetworkRequest::LowPriority);
    net_reply = net_manager->get(req);
    connect(net_reply, SIGNAL(finished()), this, SLOT(checkDone()));
}

void UpdateChecker::checkDone()
{
    disconnect(net_reply, SIGNAL(finished()), this, SLOT(checkDone()));
    if(net_reply->error() == QNetworkReply::NoError)
    {
        QString raw = QString(net_reply->readAll());
        net_reply->deleteLater();
        bool ok;
        quint32 ver = raw.section(',', 0, 0).toUInt(&ok, 16);
        if(ok == true && ver > current_version)
        {
            emit updateAvailable(raw.section(',', 1, 1), raw.section(',', 2));

            QNetworkRequest req = QNetworkRequest(QUrl(QString(url).append("notes.release")));
            req.setPriority(QNetworkRequest::LowPriority);
            net_reply = net_manager->get(req);
            connect(net_reply, SIGNAL(finished()), this, SLOT(notesDone()));
        }
        else
        {
            net_reply = 0;
        }
    }
}

void UpdateChecker::notesDone()
{
    disconnect(net_reply, SIGNAL(finished()), this, SLOT(notesDone()));
    if(net_reply->error() == QNetworkReply::NoError)
    {
        emit updateNotes(QString(net_reply->readAll()));
        net_reply->deleteLater();
        net_reply = 0;
    }
}
