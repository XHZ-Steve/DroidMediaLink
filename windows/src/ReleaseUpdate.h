#pragma once
#include <QVersionNumber>
#include <QUrl>
#include <QNetworkProxyFactory>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonArray>
namespace ReleaseUpdate {
class SystemProxyFactory final:public QNetworkProxyFactory {
public:QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query)override{return QNetworkProxyFactory::systemProxyForQuery(query);}
};
inline bool newer(QString remote,QString current){
 static const QRegularExpression syntax("^v?[0-9]+(\\.[0-9]+){0,3}$");
 if(!syntax.match(remote).hasMatch()||!syntax.match(current).hasMatch())return false;
 if(remote.startsWith('v'))remote.remove(0,1);if(current.startsWith('v'))current.remove(0,1);
 return QVersionNumber::compare(QVersionNumber::fromString(remote).normalized(),QVersionNumber::fromString(current).normalized())>0;
}
inline QJsonObject asset(const QJsonObject &release){
 if(release.value("draft").toBool()||release.value("prerelease").toBool())return {};
 for(const auto &value:release.value("assets").toArray()){
  auto item=value.toObject();const auto name=item.value("name").toString();
  const auto url=QUrl(item.value("browser_download_url").toString());
  if(name.startsWith("DroidMediaLink-")&&name.endsWith("-windows-x64-setup.exe")&&
     url.scheme()=="https"&&url.host()=="github.com"&&url.path().startsWith("/XHZ-Steve/DroidMusicLink/releases/download/")&&
     QRegularExpression("^sha256:[0-9a-fA-F]{64}$").match(item.value("digest").toString()).hasMatch()&&
     item.value("size").toInteger()>0&&item.value("size").toInteger()<=160*1024*1024)return item;
 }
 return {};
}
}
