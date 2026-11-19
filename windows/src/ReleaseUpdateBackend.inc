#include "ReleaseUpdate.h"
#include <QProcess>
#include <QSaveFile>
#include <memory>
void Bridge::cancelUpdate(){if(m_updateReply)m_updateReply->abort();}
void Bridge::checkUpdates(){
 if(m_updateBusy)return;m_updateAsset={};m_updateBusy=true;m_updateProgress=-1;emit updateChanged();
 QNetworkRequest request(QUrl("https://api.github.com/repos/XHZ-Steve/DroidMediaLink/releases/latest"));
 request.setRawHeader("User-Agent",("DroidMediaLink/"+QCoreApplication::applicationVersion()).toUtf8());request.setRawHeader("Accept","application/vnd.github+json");request.setTransferTimeout(15000);
 auto reply=m_updateNetwork.get(request);m_updateReply=reply;
 QTimer::singleShot(20000,reply,[reply]{if(!reply->isFinished()){reply->setProperty("updateTimedOut",true);reply->abort();}});
 connect(reply,&QNetworkReply::downloadProgress,reply,[reply](qint64 n,qint64){if(n>1024*1024)reply->abort();});
 connect(reply,&QNetworkReply::finished,this,[this,reply]{
  m_updateBusy=false;m_updateReply=nullptr;emit updateChanged();
  if(reply->error()==QNetworkReply::NoError){
   const auto release=QJsonDocument::fromJson(reply->readAll()).object();const auto tag=release.value("tag_name").toString();qInfo()<<"Update version check"<<tag<<QCoreApplication::applicationVersion();
   if(!ReleaseUpdate::newer(tag,QCoreApplication::applicationVersion()))emit notification(tag.isEmpty()?tr("No release available or network unavailable"):tr("You are up to date"));
   else {m_updateAsset=ReleaseUpdate::asset(release);if(m_updateAsset.isEmpty())emit notification(tr("No verified Windows installer is available for this release"));else emit updateAvailable(tag,release.value("html_url").toString());}
  }else if(reply->error()!=QNetworkReply::OperationCanceledError||reply->property("updateTimedOut").toBool())emit notification(tr("No release available or network unavailable"));
  reply->deleteLater();
 });
}
void Bridge::installUpdate(){
 if(m_updateBusy||m_updateAsset.isEmpty())return;
 const auto asset=m_updateAsset;const auto directory=QStandardPaths::writableLocation(QStandardPaths::CacheLocation)+"/updates";QDir().mkpath(directory);
 const auto path=directory+"/DroidMediaLink-update.exe";
 auto file=std::make_shared<QSaveFile>(path);if(!file->open(QIODevice::WriteOnly)){emit notification(tr("Unable to save the update"));return;}
 auto hash=std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);auto received=std::make_shared<qint64>(0);auto writeOk=std::make_shared<bool>(true);
 QNetworkRequest request(QUrl(asset.value("browser_download_url").toString()));request.setTransferTimeout(20000);
 auto reply=m_updateNetwork.get(request);m_updateReply=reply;m_updateBusy=true;m_updateProgress=0;emit updateChanged();
 auto drain=[reply,file,hash,received,writeOk,asset]{while(reply->bytesAvailable()>0){const auto bytes=reply->read(65536);*received+=bytes.size();if(*received>asset.value("size").toInteger()||file->write(bytes)!=bytes.size()){*writeOk=false;reply->abort();return;}hash->addData(bytes);}};
 connect(reply,&QNetworkReply::readyRead,this,drain);
 connect(reply,&QNetworkReply::downloadProgress,this,[this](qint64 n,qint64 total){m_updateProgress=total>0?qBound(0.,double(n)/total,1.):0;emit updateChanged();});
 connect(reply,&QNetworkReply::finished,this,[this,reply,file,hash,received,writeOk,asset,path,drain]{
  drain();m_updateBusy=false;m_updateReply=nullptr;emit updateChanged();
  const bool valid=reply->error()==QNetworkReply::NoError&&*writeOk&&*received==asset.value("size").toInteger()&&hash->result().toHex()==asset.value("digest").toString().mid(7).toLatin1().toLower();
  if(valid&&file->commit()){
   // The installer owns replacement after this process exits. Never execute
   // downloaded data before HTTPS metadata size and SHA-256 both agree.
   const QStringList args{"/SP-","/SILENT","/DMLUPDATE=1","/CLOSEAPPLICATIONS","/RESTARTAPPLICATIONS","/DIR="+QDir::toNativeSeparators(QCoreApplication::applicationDirPath())};
   if(QProcess::startDetached(path,args))QCoreApplication::quit();else emit notification(tr("Unable to start the installer"));
  }else {file->cancelWriting();if(reply->error()!=QNetworkReply::OperationCanceledError)emit notification(tr("Update download or verification failed"));}
  reply->deleteLater();
 });
}
