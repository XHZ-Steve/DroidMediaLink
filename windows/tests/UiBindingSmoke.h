#pragma once
#include "../src/Bridge.h"
#include "VisualTree.h"
#include <QQuickWindow>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QDebug>
#include <cmath>
#include <memory>

inline void startUiBindingSmoke(QQuickWindow *window,Bridge &bridge,const QString &path,QCoreApplication &app){
 window->setProperty("page",2);
 QTimer::singleShot(700,window,[=,&bridge,&app]{
  bool passed=true;QJsonObject checks;
  for(const auto &key:QStringList{"light","motion","showLyrics"}){
   const auto controls=visualItemsNamed(window,"setting-"+key);auto control=controls.isEmpty()?nullptr:controls.first();if(!control){passed=false;checks[key]="control not found";continue;}
   const bool original=control->property("checked").toBool();
   const bool invoked=QMetaObject::invokeMethod(control,"click");
   const auto value=control->property("checked").toBool(),stored=bridge.setting(key).toBool(),root=window->property(key.toLatin1().constData()).toBool();
   const bool valid=invoked&&value!=original&&stored!=original&&root!=original;passed=passed&&valid;checks[key]=QJsonObject{{"found",true},{"invoked",invoked},{"original",original},{"checked",value},{"stored",stored},{"root",root},{"passed",valid}};
   QMetaObject::invokeMethod(control,"click");
  }
  const auto original=bridge.setting("glass",.16);bridge.setSetting("glass",.3);passed=passed&&qAbs(window->property("glass").toDouble()-.3)<.001;bridge.setSetting("glass",original);
  checks["glassRoot"]=window->property("glass").toDouble();QFile report(path);if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument(QJsonObject{{"checks",checks},{"nativeQtButtonClicks",3},{"settingsAndRootBindings",passed}}).toJson());
  app.exit(passed?0:12);
 });
}

inline void startSettingsDrawerSmoke(QQuickWindow *window,const QString &directory,QCoreApplication &app){
 const auto gears=visualItemsNamed(window,"settingsGear"),panels=visualItemsNamed(window,"settingsPage");
 const auto covers=visualItemsNamed(window,"heroArtwork"),titles=visualItemsNamed(window,"trackMetadata");
 if(gears.isEmpty()||panels.isEmpty()||covers.isEmpty()||titles.isEmpty()){app.exit(14);return;}
 QDir().mkpath(directory);
 auto gear=gears.first(),panel=panels.first(),cover=covers.first(),title=titles.first();
 QTimer::singleShot(1200,window,[=,&app]{
  struct State{bool falling=false,entering=false,exiting=false,closing=false;QElapsedTimer elapsed;};
  auto state=std::make_shared<State>();state->elapsed.start();
  const auto coverTop=cover->mapToScene(QPointF()).y(),titleTop=title->mapToScene(QPointF()).y();
  if(!QMetaObject::invokeMethod(gear,"click")){app.exit(14);return;}
  auto timer=new QTimer(window);timer->setTimerType(Qt::PreciseTimer);
  QObject::connect(timer,&QTimer::timeout,window,[=,&app]{
   const double progress=window->property("stageProgress").toDouble();
   const auto page=window->property("page").toInt();
   if(state->elapsed.elapsed()>6500){qWarning()<<"Settings drawer timed out"<<page<<progress;app.exit(14);return;}
   const bool panelInMotion=panel->y()>55&&panel->y()<window->height()-12;
   if(!state->closing){
    if(progress>.12&&progress<.32){
     const auto coverFall=cover->mapToScene(QPointF()).y()-coverTop;
     const auto titleFall=title->mapToScene(QPointF()).y()-titleTop;
     state->falling|=coverFall>0&&std::abs(coverFall-titleFall)>1&&panel->y()>=window->height();
    }
    if(page==2&&panelInMotion)state->entering=true;
    if(progress==1&&page==2&&!visualItemsNamed(window,"setting-language").isEmpty()){
     window->grabWindow().save(directory+"/drawer-open.png");
     state->closing=QMetaObject::invokeMethod(gear,"click");
    }
   }else{
    if(page==0&&panelInMotion&&!state->exiting){state->exiting=true;window->grabWindow().save(directory+"/drawer-exit.png");}
    if(progress==0&&page==0&&!panel->isVisible()){
     timer->stop();
     qInfo()<<"Settings drawer: staggered fall"<<state->falling<<"entry"<<state->entering<<"exit"<<state->exiting;
     app.exit(state->falling&&state->entering&&state->exiting?0:14);
    }
   }
  });
  timer->start(12);
 });
}

// Explicit live-device test: click the real Qt control, observe remote state,
// and restore the original playback state before closing the test application.
inline void startTransportSmoke(QQuickWindow *window,Bridge &bridge,const QString &directory,QCoreApplication &app){
 struct State {int stage=0,replies=0;bool original=false,failed=false;qint64 originalPosition=0;QElapsedTimer clock;};
 auto state=std::make_shared<State>();state->clock.start();QDir().mkpath(directory);window->setProperty("page",0);
 auto timer=new QTimer(window);timer->setInterval(100);
 QObject::connect(&bridge,&Bridge::commandFinished,window,[state](QString,bool success,QString){++state->replies;state->failed=state->failed||!success;});
 QObject::connect(timer,&QTimer::timeout,window,[=,&bridge,&app]{
  auto click=[&]{for(auto item:visualItemsNamed(window,"primaryTransport"))if(item->isVisible()&&item->isEnabled())return QMetaObject::invokeMethod(item,"click");return false;};
  if(state->clock.elapsed()>15000||state->failed){timer->stop();if(state->stage)bridge.command(state->original?"play":"pause");QFile report(directory+"/transport-report.json");if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument(QJsonObject{{"passed",false},{"stage",state->stage},{"online",bridge.online()},{"replies",state->replies}}).toJson());QTimer::singleShot(400,&app,[&app]{app.exit(13);});return;}
  const auto covers=visualItemsNamed(window,"heroArtwork");const bool ready=bridge.artwork().isEmpty()||(!covers.isEmpty()&&covers.first()->property("ready").toBool());
  if(state->stage==0&&state->clock.elapsed()>1800&&ready&&bridge.online()&&bridge.commands().contains("play")&&bridge.commands().contains("pause")){
   state->original=bridge.playing();state->originalPosition=bridge.position();if(state->original)window->grabWindow().save(directory+"/home-playing.png");state->stage=1;if(!click())state->failed=true;
  }else if(state->stage==1&&bridge.playing()!=state->original&&state->replies>=1){
   window->grabWindow().save(directory+(bridge.playing()?"/home-playing.png":"/home-paused.png"));state->stage=2;if(!click())state->failed=true;
  }else if(state->stage==2&&bridge.playing()==state->original&&state->replies>=2){
   timer->stop();if(!state->original&&bridge.commands().contains("seek"))bridge.command("seek",state->originalPosition);
   QFile report(directory+"/transport-report.json");if(report.open(QIODevice::WriteOnly))report.write(QJsonDocument(QJsonObject{{"actualQtControlClicks",2},{"authenticatedRemoteStateChanged",true},{"originalPlayStateRestored",true},{"initiallyPlaying",state->original},{"passed",!state->failed}}).toJson());
   QTimer::singleShot(400,&app,[&app]{app.exit(0);});
  }
 });timer->start();
}
