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
 struct State {
  HWND backdrop=nullptr;HBRUSH brush=nullptr;QJsonObject result;int phase=0;
  ~State(){if(backdrop)DestroyWindow(backdrop);UnregisterClassW(L"DmlTransparencyBackdrop",GetModuleHandleW(nullptr));if(brush)DeleteObject(brush);}
 };
 auto state=std::make_shared<State>();
 state->result={{"qtVersion",QString::fromLatin1(qVersion())},{"persistentGraphics",desktop->isPersistentGraphics()},{"persistentSceneGraph",desktop->isPersistentSceneGraph()},{"graphicsApi",int(desktop->rendererInterface()->graphicsApi())}};
 state->brush=CreateSolidBrush(RGB(18,115,75));
 WNDCLASSW backgroundClass{};backgroundClass.lpfnWndProc=DefWindowProcW;backgroundClass.hInstance=GetModuleHandleW(nullptr);backgroundClass.lpszClassName=L"DmlTransparencyBackdrop";backgroundClass.hbrBackground=state->brush;
 RegisterClassW(&backgroundClass);
 desktop->setProperty("requestedVisible",true);
 QMetaObject::invokeMethod(desktop,"clampPosition");
 RECT bounds{};GetWindowRect(reinterpret_cast<HWND>(desktop->winId()),&bounds);
 state->backdrop=CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_TOPMOST,L"DmlTransparencyBackdrop",L"Transparency test",WS_POPUP|WS_VISIBLE,bounds.left,bounds.top,bounds.right-bounds.left,bounds.bottom-bounds.top,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
 desktop->setProperty("requestedVisible",true);desktop->show();desktop->raise();
 auto timer=new QTimer(&app);timer->setInterval(500);
 QObject::connect(timer,&QTimer::timeout,&app,[desktop,path,&app,state,timer]{
  const auto hwnd=reinterpret_cast<HWND>(desktop->winId());
  const auto capture=desktop->screen()->grabWindow(0,desktop->x(),desktop->y(),desktop->width(),desktop->height());
  const auto image=capture.toImage();
  const auto pixel=image.isNull()?QColor():image.pixelColor(qMin(2,image.width()-1),qMin(2,image.height()-1));
  const QStringList phases{"firstShow","reopen","clickThrough","inputRestored","secondReopen","halfOpacity","opacityRestored"};
  const QString phase=phases[state->phase];
  // Do not call QQuickWindow::grabWindow: it forces a render and can hide
  // presentation failures. The raster backdrop also avoids an extra RHI window.
  int transparentSamples=0,matchedSamples=0;
  const int margin=qMax(3,qRound(4*desktop->devicePixelRatio()));
  for(int y=qMax(margin,24);y<image.height()-qMax(margin,24);y+=16)for(int x: {margin,image.width()-margin-1}){
   ++transparentSamples;const auto actual=image.pixelColor(x,y);
   if(qAbs(actual.red()-18)<8&&qAbs(actual.green()-115)<8&&qAbs(actual.blue()-75)<8)++matchedSamples;
  }
  const auto api=desktop->rendererInterface()->graphicsApi();const bool d3d=api==QSGRendererInterface::Direct3D11||api==QSGRendererInterface::Direct3D12;
  const bool nativeComposition=!d3d||(GetWindowLongPtrW(hwnd,GWL_EXSTYLE)&WS_EX_NOREDIRECTIONBITMAP);
  RECT actualBounds{};GetWindowRect(hwnd,&actualBounds);POINT pointer{actualBounds.left+actualBounds.right,actualBounds.top+actualBounds.bottom};pointer.x/=2;pointer.y/=2;
  const bool inputPasses=state->phase!=2||WindowFromPoint(pointer)!=hwnd;
  const bool transparent=nativeComposition&&inputPasses&&desktop->isVisible()&&transparentSamples>0&&matchedSamples>=transparentSamples*.98;
  state->result[phase]=QJsonObject{{"transparent",transparent},{"inputPasses",inputPasses},{"transparentSamples",transparentSamples},{"matchedSamples",matchedSamples},{"sample",pixel.name(QColor::HexArgb)},
   {"alphaBufferSize",desktop->format().alphaBufferSize()},{"requestedAlphaBufferSize",desktop->requestedFormat().alphaBufferSize()},
   {"nativeExStyle",QString::number(GetWindowLongPtrW(hwnd,GWL_EXSTYLE),16)},{"clearColor",desktop->color().name(QColor::HexArgb)}};
  const auto prefix=QFileInfo(path).absolutePath();QDir().mkpath(prefix);
  capture.save(prefix+"/desktop-transparency-"+phase+".png");
  if(state->phase==0){desktop->hide();QTimer::singleShot(100,&app,[desktop]{desktop->show();desktop->raise();});}
  else if(state->phase==1)desktop->setFlag(Qt::WindowTransparentForInput,true);
  else if(state->phase==2)desktop->setFlag(Qt::WindowTransparentForInput,false);
  else if(state->phase==3){desktop->hide();QTimer::singleShot(100,&app,[desktop]{desktop->show();desktop->raise();});}
  else if(state->phase==4)desktop->setOpacity(.5);
  else if(state->phase==5)desktop->setOpacity(1.);
  else {
   QFile file(path);if(file.open(QIODevice::WriteOnly))file.write(QJsonDocument(state->result).toJson());
   bool passed=true;for(const auto &name:phases)passed=passed&&state->result[name].toObject()["transparent"].toBool();
   timer->stop();timer->deleteLater();app.exit(passed?0:9);return;
  }
  ++state->phase;
 });
 timer->start();
}
