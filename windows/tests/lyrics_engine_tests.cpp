#include "Bridge.h"
#include "LyricTimeline.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QEventLoop>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QUuid>
#include <QBuffer>
#include <QImage>
#include <cstdio>

class LoopbackProviderNetwork:public QNetworkAccessManager {
public:quint16 port=0;
protected:QNetworkReply *createRequest(Operation operation,const QNetworkRequest &request,QIODevice *data)override{
 QNetworkRequest local(request);QUrl url("http://127.0.0.1:"+QString::number(port)+"/"+request.url().host()+request.url().path());url.setQuery(request.url().query());local.setUrl(url);return QNetworkAccessManager::createRequest(operation,local,data);
}
};
class LyricsEngineTests {
public:
 static int run(){
  QTemporaryDir files;QSettings saved;saved.setValue("preferences/lyrics/online",false);saved.setValue("preferences/lyrics/autoMatchMigration",true);saved.sync();Bridge bridge;
  bridge.setSetting("home/windowBackgroundOpacity",.45);
  if(bridge.setting("home/windowBackgroundOpacity",1.).toDouble()!=.45)return 122;
  bridge.setSetting("home/artworkScale",1.25);
  if(bridge.setting("home/artworkScale",1.).toDouble()!=1.25)return 127;
  const auto savedOrder=bridge.setting("lyrics/providerPriority");
  bridge.m_settings.setValue("preferences/lyrics/preferredProvider","netease");
  bridge.m_settings.setValue("preferences/lyrics/phoneSource","only");
  bridge.setSetting("lyrics/providerPriority","qq,phone,netease,kugou,lrclib");
  if(bridge.lyricProviderOrder().mid(0,3)!=QStringList{"qq","phone","netease"})return 123;
  bridge.setSetting("lyrics/providerPriority",savedOrder);
  const auto metadata=QJsonObject{{"title","TEST ONLY lyric engine A"},{"artist","TEST fixture"},{"album","TEST"},{"durationMs",0},{"state","paused"}};
  bridge.applyMedia(metadata);const auto unknownDurationKey=bridge.m_lyricKey;
  bridge.applyMedia({{"durationMs",30000}});if(unknownDurationKey==bridge.m_lyricKey||bridge.m_lyricKey!=bridge.lyricKey())return 1;
  const auto oldTitle=bridge.title();bridge.setDragging(true);bridge.applyMedia({{"title","TEST deferred drag title"},{"positionMs",1234}});bridge.applyMedia({{"volume",3},{"maxVolume",10}});
  if(bridge.title()!=oldTitle||bridge.m_progress.isActive()||bridge.m_lyricClock.isActive())return 85;
  bridge.setDragging(false);if(bridge.title()!="TEST deferred drag title"||bridge.volume()!=.3||!bridge.m_deferredMedia.isEmpty())return 86;
  bridge.applyMedia(metadata);bridge.applyMedia({{"durationMs",30000}});
  static QJsonObject nativePayload;
  bridge.m_nativeUpdate=[](const char* json){nativePayload=QJsonDocument::fromJson(json).object();return 0;};
  auto artwork=[](QColor color){QImage image(12,12,QImage::Format_RGB32);image.fill(color);QByteArray bytes;QBuffer output(&bytes);output.open(QIODevice::WriteOnly);image.save(&output,"PNG");return QString::fromLatin1(bytes.toBase64());};
  bridge.applyMedia({{"artwork",artwork(Qt::red)}});const auto firstRevision=nativePayload["artworkRevision"].toString();
  const auto nativePath=nativePayload["artwork"].toString();
  if(firstRevision.isEmpty()||nativePath.contains('/')||nativePath.contains('?')||!QFile::exists(nativePath))return 70;
  bridge.applyMedia({{"positionMs",100}});if(nativePayload["artworkRevision"]!=firstRevision)return 71;
  bridge.applyMedia({{"artwork",artwork(Qt::blue)}});if(nativePayload["artworkRevision"]==firstRevision||nativePayload["artworkRevision"].toString().isEmpty())return 72;
  bridge.applyMedia({{"artwork",""}});if(!nativePayload["artwork"].toString().isEmpty()||!nativePayload["artworkRevision"].toString().isEmpty())return 73;
  bridge.m_nativeUpdate=nullptr;
  QImage highPhone(1600,1600,QImage::Format_RGB32);highPhone.fill(Qt::cyan);QByteArray highBytes;QBuffer highOutput(&highBytes);highOutput.open(QIODevice::WriteOnly);highPhone.save(&highOutput,"PNG");
  bridge.applyMedia({{"artwork",QString::fromLatin1(highBytes.toBase64())}});if(bridge.m_phonePixels!=QSize(1600,1600)||QImage(QUrl(bridge.artwork()).toLocalFile()).size()!=QSize(1600,1600))return 87;
  auto originalCover=bridge.artwork();bridge.m_settings.setValue("preferences/cover/mode","high");bridge.m_coverRequestKey=CoverArtTrack{bridge.title(),bridge.artist(),bridge.album(),bridge.duration()}.key()+"1200high";
  const auto enhancedPath=bridge.cachePath()+"/covers/TEST-enhanced.png";QImage enhancedImage(2000,2000,QImage::Format_RGB32);enhancedImage.fill(Qt::cyan);enhancedImage.save(enhancedPath);const auto thumbPath=enhancedPath+".smtc.jpg";highPhone.scaled(512,512).save(thumbPath,"JPEG",90);bridge.m_nativeUpdate=[](const char* json){nativePayload=QJsonDocument::fromJson(json).object();return 0;};bridge.applyEnhancedCover({enhancedPath,"test","test",QSize(2000,2000),"TEST enhanced digest",false,thumbPath});if(nativePayload["artwork"].toString()!=QDir::toNativeSeparators(thumbPath))return 90;const auto enhancedCover=bridge.artwork();
  bridge.applyMedia({{"artwork",QString::fromLatin1(highBytes.toBase64())},{"positionMs",1234}});if(bridge.artwork()!=enhancedCover||bridge.artwork()==originalCover)return 88;
  QImage latePhone(2048,2048,QImage::Format_RGB32);latePhone.fill(Qt::cyan);QByteArray lateBytes;QBuffer lateOutput(&lateBytes);lateOutput.open(QIODevice::WriteOnly);latePhone.save(&lateOutput,"PNG");bridge.applyMedia({{"artwork",QString::fromLatin1(lateBytes.toBase64())}});if(bridge.artwork()==enhancedCover||bridge.m_phonePixels!=QSize(2048,2048))return 91;originalCover=bridge.artwork();
  bridge.m_settings.setValue("preferences/cover/mode","off");bridge.enhanceCover();if(bridge.artwork()!=originalCover)return 89;bridge.m_nativeUpdate=nullptr;
  bridge.m_publishedCoverActive=true;bridge.m_receivedArtworkDigest.clear();bridge.applyMedia({{"artwork",""}});if(bridge.artwork()!=originalCover)return 92;
  bridge.applyMedia({{"sessionId","TEST new publisher"},{"artwork",artwork(Qt::red)}});if(bridge.m_phonePixels!=QSize(12,12)||bridge.m_publishedCoverActive)return 93;

  auto write=[&](QString name,QString text){QFile file(files.path()+"/"+name);if(!file.open(QIODevice::WriteOnly))return QUrl();file.write(text.toUtf8());file.close();return QUrl::fromLocalFile(file.fileName());};
  const auto first=write("first.lrc","[00:01.00]TEST FIRST\n[00:05.00]TEST SECOND");
  bridge.importLyrics(first);if(bridge.lyrics().size()!=2)return 2;
  bridge.m_position=1500;bridge.updateLyricClock();if(bridge.currentLine()!=0)return 3;
  bridge.m_position=6500;bridge.updateLyricClock();if(bridge.currentLine()!=1)return 4;
  bridge.m_position=0;bridge.updateLyricClock();if(bridge.currentLine()!=-1)return 5;
  // Exercise a real in-flight HTTP reply, then supersede it with a manual import.
  // The local fixture contains explicitly marked TEST lyrics and never leaves loopback.
  QTcpServer server;if(!server.listen(QHostAddress::LocalHost,0))return 6;
  QObject::connect(&server,&QTcpServer::newConnection,&server,[&]{auto socket=server.nextPendingConnection();QObject::connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);QTimer::singleShot(120,socket,[socket]{if(socket->state()!=QAbstractSocket::ConnectedState)return;const auto bytes=QJsonDocument(QJsonObject{{"syncedLyrics","[00:01]STALE TEST RESPONSE"}}).toJson(QJsonDocument::Compact);socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "+QByteArray::number(bytes.size())+"\r\nConnection: close\r\n\r\n"+bytes);socket->disconnectFromHost();});});
  bridge.requestLyrics(QUrl("http://127.0.0.1:"+QString::number(server.serverPort())+"/test"),bridge.lyricKey(),bridge.m_lyricGeneration,false);
  const auto replacement=write("replacement.lrc","[00:00]MANUAL TEST WINS");
  QEventLoop wait;QTimer::singleShot(30,&wait,[&]{bridge.importLyrics(replacement);});QTimer::singleShot(250,&wait,&QEventLoop::quit);wait.exec();
  if(bridge.lyrics().size()!=1||bridge.lyrics().first().toMap().value("text").toString()!="MANUAL TEST WINS")return 7;
  bridge.applyMedia({{"durationMs",31000}});if(bridge.lyrics().size()!=1||bridge.lyricSource()!="manual")return 8;
  bridge.clearLyricsCache();if(bridge.lyrics().size()!=1||bridge.lyricSource()!="manual")return 9;
  // All cache and mapping material is isolated under the test app identity.
  const auto directory=bridge.cachePath();if(!directory.contains(QCoreApplication::applicationName()))return 10;
  QDir cache(directory);for(const auto &name:cache.entryList({"*.lrc"},QDir::Files))cache.remove(name);
  // The real provider chain runs through a loopback transport: first source
  // fails, QQ supplies a version/artist/duration-verified TEST lyric document.
  QTcpServer chainServer;if(!chainServer.listen(QHostAddress::LocalHost,0))return 13;
  QStringList requests;QObject::connect(&chainServer,&QTcpServer::newConnection,&chainServer,[&]{auto socket=chainServer.nextPendingConnection();QObject::connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);QObject::connect(socket,&QTcpSocket::readyRead,socket,[&,socket]{const auto bytes=socket->readAll();if(bytes.isEmpty())return;const auto path=QString::fromUtf8(bytes.split(' ').value(1));requests<<path;QJsonObject response;bool failed=false;
   if(path.contains("music.163.com")){response={{"code",503}};failed=true;}
   else if(path.contains("client_search_cp")){const QJsonObject song{{"songmid","testfallback"},{"songname","TEST fallback"},{"singer",QJsonArray{QJsonObject{{"name","TEST fixture"}}}},{"albumname","TEST"},{"interval",30}};response={{"data",QJsonObject{{"song",QJsonObject{{"list",QJsonArray{song}}}}}}};}
   else response={{"lyric",QString::fromLatin1(QByteArray("[00:00]TEST FALLBACK FIRST\n[00:05]TEST FALLBACK NEXT").toBase64())},{"trans",QString::fromLatin1(QByteArray("[00:00]TEST translated first\n[00:05]TEST translated next").toBase64())}};
   const auto body=QJsonDocument(response).toJson(QJsonDocument::Compact);socket->write(QByteArray(failed?"HTTP/1.1 503 Service Unavailable\r\n":"HTTP/1.1 200 OK\r\n")+"Content-Type: application/json\r\nContent-Length: "+QByteArray::number(body.size())+"\r\nConnection: close\r\n\r\n"+body);socket->disconnectFromHost();});});
  LoopbackProviderNetwork providerNetwork;providerNetwork.port=chainServer.serverPort();Bridge fallback;
  delete fallback.m_lyricProviders;fallback.m_lyricProviders=new LyricProviders(&providerNetwork,&fallback);
  fallback.m_settings.setValue("preferences/lyrics/online",true);fallback.m_settings.setValue("preferences/lyrics/autoMatch",true);fallback.m_settings.setValue("preferences/lyrics/preferCurrentApp",false);fallback.m_settings.setValue("preferences/lyrics/preferredProvider","auto");fallback.m_settings.setValue("preferences/lyrics/providerPriority","netease,qq,kugou,lrclib");
  fallback.applyMedia({{"title","TEST fallback"},{"artist","TEST fixture"},{"album","TEST"},{"durationMs",30000}});
  QEventLoop chainWait;QObject::connect(&fallback,&Bridge::lyricsChanged,&chainWait,[&]{if(!fallback.lyricsLoading()&&!fallback.lyrics().isEmpty())chainWait.quit();});QTimer::singleShot(5000,&chainWait,&QEventLoop::quit);chainWait.exec();
  if(fallback.lyricProvider()!="qq"||fallback.lyrics().size()!=2||requests.size()!=3||!requests.first().contains("music.163.com"))return 14;
  fallback.fetchLyrics();if(fallback.lyricSource()!="cache"||fallback.lyricProvider()!="qq"||fallback.lyrics().size()!=2)return 15;
  qInfo()<<"PASS: actual provider chain survives first-source HTTP failure, selects next trusted source, and replays rich cache";
  fallback.m_settings.setValue("preferences/lyrics/crossSourceTranslation",true);
  fallback.applyMedia({{"phoneLyrics","[00:00]<00:00>TEST FALLBACK FIRST\n[00:05]TEST FALLBACK NEXT"},{"phoneTranslation",""}});
  QEventLoop translatedWait;QObject::connect(&fallback,&Bridge::lyricsChanged,&translatedWait,[&]{if(!fallback.lyrics().isEmpty()&&!fallback.lyrics().first().toMap()["translation"].toString().isEmpty())translatedWait.quit();});QTimer::singleShot(5000,&translatedWait,&QEventLoop::quit);translatedWait.exec();
  if(fallback.lyricProvider()!="phone"||fallback.lyrics().first().toMap()["translation"]!="TEST translated first"||fallback.lyrics().first().toMap()["words"].toList().isEmpty())return 84;
  qInfo()<<"PASS: online translation supplements phone word lyrics through the real provider chain";
  Bridge phone;phone.m_settings.setValue("preferences/lyrics/online",false);phone.m_settings.setValue("preferences/lyrics/crossSourceTranslation",true);
  phone.applyMedia({{"title","TEST PHONE SOURCE"},{"artist","TEST fixture"},{"durationMs",30000},{"phoneLyrics","[00:01]TEST first\n[00:05]TEST second\n[00:09]TEST third"},{"phoneTranslation","[00:01]Translated first\n[00:05]Translated second\n[00:09]Translated third"}});
  if(phone.lyricProvider()!="phone"||phone.lyrics().size()!=3||phone.lyrics()[1].toMap()["translation"]!="Translated second")return 80;
  auto manual=LyricTimeline::parse("[00:01]TEST first\n[00:05]TEST second\n[00:09]TEST third");
  phone.acceptLyricDocument(LyricDocument{manual,"local","test","lrc",100},true);
  if(phone.lyricSource()!="manual"||phone.lyricProvider()!="local"||phone.lyrics()[0].toMap()["translation"]!="Translated first")return 81;
  phone.applyMedia({{"phoneLyrics","[00:01]TEST first\n[00:05]TEST second\n[00:09]TEST third"},{"phoneTranslation","[00:01]Replacement translation\n[00:05]Translated second\n[00:09]Translated third"}});
  if(phone.lyricProvider()!="local"||phone.lyrics()[0].toMap()["translation"]!="Translated first")return 82;
  phone.applyMedia({{"title","TEST next phone track"},{"phoneLyrics",""},{"phoneTranslation",""}});if(!phone.phoneLyricRows().isEmpty()||!phone.lyrics().isEmpty())return 83;
  qInfo()<<"PASS: phone source automatically loads, supplements manual originals without overwriting translation, and clears on track change";
  if(QCoreApplication::arguments().contains("--live")){
   bridge.m_settings.setValue("preferences/lyrics/online",true);bridge.m_settings.setValue("preferences/lyrics/autoMatch",true);
   bridge.setSetting("lyrics/providerPriority","lrclib,phone,netease,qq,kugou");
   bridge.applyMedia({{"title","The Scientist"},{"artist","Coldplay"},{"album","A Rush of Blood to the Head"},{"durationMs",309000}});
   QEventLoop network;QObject::connect(&bridge,&Bridge::lyricsChanged,&network,[&]{if(!bridge.lyricsLoading())network.quit();});QTimer::singleShot(25000,&network,&QEventLoop::quit);if(bridge.lyrics().isEmpty())network.exec();
   if(bridge.lyrics().isEmpty()||bridge.lyricSource()!="lrclib")return 11;
   const auto count=bridge.lyrics().size();bridge.fetchLyrics();if(bridge.lyricSource()!="cache"||bridge.lyrics().size()!=count)return 12;
   qInfo()<<"PASS live LRCLIB automatic match and cached replay; rows:"<<count;
   cache.remove(bridge.lyricKey()+".lrc");
  }
  qInfo()<<"PASS: late duration invalidation, seek/backward clock, abort/stale HTTP reply, durable manual mapping and cache priority";
  {
   Bridge cover;cover.m_settings.setValue("preferences/cover/mode","high");
   for(auto id:QStringList{"netease","qq","musicbrainz"})cover.m_settings.setValue("preferences/cover/providers/"+id+"/enabled",false);
   cover.m_media={{"title","TEST deferred artwork"},{"artist","TEST artist"},{"durationMs",0}};
   cover.enhanceCover();if(!cover.m_coverRequestKey.isEmpty())return 94;
   cover.m_media["durationMs"]=200000;cover.enhanceCover();auto key=cover.m_coverRequestKey;if(key.isEmpty())return 95;
   cover.m_phonePixels=QSize(363,363);cover.enhanceCover();if(cover.m_coverRequestKey==key)return 96;
   cover.m_coverTestStates["netease"]={{"status","error"}};cover.m_coverRequestKey.clear();cover.enhanceCover();if(cover.m_coverRetry.isActive())return 97;
   cover.m_coverSource.clear();cover.m_enhancedPixels=QSize(2000,2000);if(!cover.coverSource().contains("363"))return 99;
   cover.m_media["durationMs"]=0;const auto artwork=cover.artwork();cover.testCoverProvider("qq");if(cover.m_coverTestStates["qq"].value("status")!="incomplete"||cover.artwork()!=artwork)return 98;
  }
  {
   Bridge volume;volume.m_online=true;volume.m_media={{"commands",QJsonArray{"volume"}},{"volume",2},{"maxVolume",10}};
   auto reply=[&](QString id,bool ok,int level){volume.receive(QString::fromUtf8(QJsonDocument(QJsonObject{{"protocolVersion",1},{"messageId",QUuid::createUuid().toString()},{"timestamp",QDateTime::currentMSecsSinceEpoch()},{"type","command_result"},{"payload",QJsonObject{{"requestId",id},{"success",ok},{"volume",level},{"maxVolume",10}}}}).toJson()));};
   volume.command("volume",.4);const auto first=volume.m_volumeRequest;
   volume.command("volume",.6);volume.command("volume",.8);
   if(first.isEmpty()||volume.m_pending.size()!=1||volume.requestedVolume()!=.8)return 100;
   reply(first,true,4);const auto second=volume.m_volumeRequest;
   if(second.isEmpty()||second==first||volume.m_pending.size()!=1||volume.requestedVolume()!=.8)return 101;
   reply(first,true,1);if(volume.volume()!=.4)return 102;
   reply(second,false,5);if(volume.volumePending()||volume.volume()!=.5)return 103;
  }
  {
   bridge.m_nativeUpdate=[](const char* json){nativePayload=QJsonDocument::fromJson(json).object();return 0;};
   bridge.m_online=true;bridge.setUiVisible(false);bridge.applyMedia({{"state","playing"},{"positionMs",1000},{"durationMs",30000},{"speed",1}});
   auto publishedProgress=[&]{return nativePayload["positionMs"].toInteger()>=1500;};
   QEventLoop wait;
   QObject::connect(&bridge.m_smtcClock,&QTimer::timeout,&wait,[&]{if(publishedProgress())wait.quit();});
   // The existing clock can tick just after playback restarts. Wait for a
   // published position that crossed the threshold, regardless of timer phase.
   QTimer::singleShot(4000,&wait,&QEventLoop::quit);
   if(!publishedProgress())wait.exec();
   if(!publishedProgress()||bridge.m_progress.isActive()){
    qCritical()<<"Hidden SMTC timeline:"<<nativePayload["positionMs"]<<"UI timer active:"<<bridge.m_progress.isActive();
    return 104;
   }
   bridge.m_online=false;bridge.m_nativeUpdate=nullptr;
  }
  {
   QTemporaryDir parent;Bridge cache;
   QFile manual(cache.cachePath()+"/TEST.manual.lrc");if(!manual.open(QIODevice::WriteOnly))return 105;manual.write("[00:00]TEST manual");manual.close();
   cache.setCacheDirectory(QUrl::fromLocalFile(parent.path()));const auto root=cache.cachePath();
   if(!root.endsWith("/DroidMediaLink-cache")||!QFile::exists(root+"/TEST.manual.lrc"))return 106;
   QFile cover(root+"/covers/TEST.png");if(!cover.open(QIODevice::WriteOnly))return 107;cover.write(QByteArray(128,'x'));cover.close();
   QFile lyric(root+"/TEST.lyrics.json");if(!lyric.open(QIODevice::WriteOnly))return 108;lyric.write(QByteArray(64,'x'));lyric.close();
   cache.refreshCacheUsage();QEventLoop wait;QTimer::singleShot(800,&wait,&QEventLoop::quit);wait.exec();
   if(cache.cacheUsage()["coverBytes"].toLongLong()!=128||cache.cacheUsage()["lyricBytes"].toLongLong()!=64)return 109;
   cache.clearCoverArtCache();cache.clearLyricsCache();if(QFile::exists(cover.fileName())||QFile::exists(lyric.fileName())||!QFile::exists(manual.fileName())||!QFile::exists(root+"/TEST.manual.lrc"))return 110;
   cache.resetSettings();if(cache.cachePath()==root||!QFile::exists(cache.cachePath()+"/TEST.manual.lrc"))return 111;
  }
  {
   Bridge style;style.setAppearance("global","glassBlur",3.2);style.setAppearanceInherited("popup",false);style.setAppearance("popup","glassBlur",.7);style.setSetting("light",true);
   if(!style.saveAppearancePreset("TEST optical"))return 112;
   style.setAppearance("global","glassBlur",5.);style.setAppearance("popup","glassBlur",5.);style.setSetting("light",false);style.applyAppearancePreset("TEST optical");
   if(style.appearanceConfig("global")["glassBlur"].toDouble()!=3.2||style.appearanceConfig("popup")["glassBlur"].toDouble()!=.7||style.appearanceInherited("popup")||!style.setting("light",false).toBool())return 113;
   style.renameAppearancePreset("TEST optical","TEST renamed");auto has=[&](QString name){for(const auto &entry:style.appearancePresets())if(entry.toMap()["name"]==name)return true;return false;};if(has("TEST optical")||!has("TEST renamed"))return 114;
   const auto path=QUrl::fromLocalFile(files.path()+"/style.json");style.exportSettings(path);style.deleteAppearancePreset("TEST renamed");if(has("TEST renamed"))return 115;style.importSettings(path);if(!has("TEST renamed"))return 116;
   style.applyAppearancePreset("TEST renamed");if(style.appearanceConfig("popup")["glassBlur"].toDouble()!=.7)return 117;
   if(style.saveAppearancePreset("balanced")||style.saveAppearancePreset(QString()))return 118;
   for(int i=0;i<49;i++)if(!style.saveAppearancePreset("TEST limit "+QString::number(i)))return 119;
   if(style.saveAppearancePreset("TEST overflow")||style.appearancePresets().size()!=55)return 120;
   style.importSettings(path);if(style.appearancePresets().size()!=55)return 121;
   const QStringList expected{"dark-balanced","dark-bright","dark-immersive","dark-cinema","light-soft"};
   const auto presets=style.appearancePresets();
   for(qsizetype i=0;i<expected.size();++i){
    const auto row=presets[i].toMap();
    if(row["name"]!=expected[i]||!row["builtIn"].toBool())return 128;
   }
   style.applyAppearancePreset("dark-cinema");
   if(style.setting("light").toBool()||style.appearanceConfig("background")["backgroundStrength"].toDouble()!=.1||!style.appearanceInherited("popup"))return 129;
   style.applyAppearancePreset("light-soft");
   if(!style.setting("light").toBool()||style.appearanceConfig("global")["glassRadius"].toInt()!=24)return 130;
   style.setSetting("home/artworkRadius",42);style.setSetting("light",false);
   style.exportSettings(path);style.applyAppearancePreset("dark-balanced");style.importSettings(path);
   if(style.setting("home/artworkRadius").toInt()!=42||style.setting("light").toBool())return 131;
  }
  qInfo()<<"PASS: deferred cover metadata, late phone dimensions, and independent non-destructive cover diagnostics";
  return 0;
 }
};
int main(int argc,char **argv){
 QCoreApplication app(argc,argv);
 QTemporaryDir config;
 app.setOrganizationName("DmlTests");
 app.setApplicationName("lyrics-engine-"+QString::number(QCoreApplication::applicationPid()));
 QSettings::setDefaultFormat(QSettings::IniFormat);
 QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,config.path());
 qputenv("QT_FORCE_STDERR_LOGGING","1");
 const int result=LyricsEngineTests::run();
 if(result)std::fprintf(stderr,"lyrics_engine_tests: assertion %d failed\n",result);
 return result;
}
