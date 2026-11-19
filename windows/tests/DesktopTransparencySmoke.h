#pragma once
#include <QQuickWindow>
#include <QTimer>
#include <QScreen>
#include <QSGRendererInterface>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <memory>
#include <windows.h>

// Screen pixels exercise DWM composition; grabWindow() only sees the Qt render target.
inline void startDesktopTransparencySmoke(QQuickWindow *desktop,const QString &path,QCoreApplication &app){
 struct State { QQuickWindow backdrop;QJsonObject result;int phase=0; };
 auto state=std::make_shared<State>();
 state->result={{"qtVersion",QString::fromLatin1(qVersion())},{"persistentGraphics",desktop->isPersistentGraphics()},{"persistentSceneGraph",desktop->isPersistentSceneGraph()},{"graphicsApi",int(desktop->rendererInterface()->graphicsApi())}};
 state->backdrop.setFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
 state->backdrop.setColor(QColor("#12734b"));
 desktop->setProperty("requestedVisible",true);
 QMetaObject::invokeMethod(desktop,"clampPosition");
 state->backdrop.setGeometry(desktop->geometry());
 state->backdrop.show();
 desktop->setProperty("requestedVisible",true);desktop->show();desktop->raise();
 auto timer=new QTimer(&app);timer->setInterval(500);
 QObject::connect(timer,&QTimer::timeout,&app,[desktop,path,&app,state,timer]{
  const auto hwnd=reinterpret_cast<HWND>(desktop->winId());
  const auto capture=desktop->screen()->grabWindow(0,desktop->x(),desktop->y(),desktop->width(),desktop->height());
  const auto image=capture.toImage();
  const auto pixel=image.isNull()?QColor():image.pixelColor(qMin(2,image.width()-1),qMin(2,image.height()-1));
  const QString phase=state->phase==0?"firstShow":state->phase==1?"reopen":"clickThrough";
  const auto rendered=desktop->grabWindow();
  int transparentSamples=0,matchedSamples=0;
  if(!rendered.isNull()&&rendered.size()==image.size())for(int y=2;y<image.height();y+=16)for(int x=2;x<image.width();x+=16){
   if(rendered.pixelColor(x,y).alpha()>4)continue;++transparentSamples;const auto actual=image.pixelColor(x,y);
   if(qAbs(actual.red()-18)<8&&qAbs(actual.green()-115)<8&&qAbs(actual.blue()-75)<8)++matchedSamples;
  }
  const bool transparent=transparentSamples>0&&matchedSamples>=transparentSamples*.98;
  state->result[phase]=QJsonObject{{"transparent",transparent},{"transparentSamples",transparentSamples},{"matchedSamples",matchedSamples},{"sample",pixel.name(QColor::HexArgb)},
   {"alphaBufferSize",desktop->format().alphaBufferSize()},{"requestedAlphaBufferSize",desktop->requestedFormat().alphaBufferSize()},
   {"nativeExStyle",QString::number(GetWindowLongPtrW(hwnd,GWL_EXSTYLE),16)}};
  const auto prefix=QFileInfo(path).absolutePath();QDir().mkpath(prefix);
  capture.save(prefix+"/desktop-transparency-"+phase+".png");
  if(state->phase==0){desktop->hide();QTimer::singleShot(100,&app,[desktop]{desktop->show();desktop->raise();});}
  else if(state->phase==1)desktop->setFlag(Qt::WindowTransparentForInput,true);
  else {
   QFile file(path);if(file.open(QIODevice::WriteOnly))file.write(QJsonDocument(state->result).toJson());
   const bool passed=state->result["firstShow"].toObject()["transparent"].toBool()&&state->result["reopen"].toObject()["transparent"].toBool()&&transparent;
   timer->stop();timer->deleteLater();app.exit(passed?0:9);return;
  }
  ++state->phase;
 });
 timer->start();
}
