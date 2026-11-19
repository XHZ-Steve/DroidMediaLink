# Windows preview8 透明窗口修复

日期：2026-11-19。

## 现场

在用户实际 preview7 进程确认桌面歌词整窗白底，字幕继续绘制。背景关闭、opacity=1、shadow/glow关闭；HWND为layered、alpha255、无系统backdrop。已保留本地截图和原生状态。非尺寸变化的重绘与关闭重开均不能恢复。进程未设置 QT_D3D_NO_FLIP 等渲染覆盖变量。

## 修改

D3D11/D3D12下，在首个原生窗口创建前使用 Qt Windows 平台提供的 QT_QPA_DISABLE_REDIRECTION_SURFACE，避免GDI重定向表面与透明DirectComposition视觉层并存。保留用户显式覆盖；软件/自定义scenegraph和OpenGL/Vulkan不启用此路径。后设原生位被Windows以ERROR87拒绝，相关临时guard没有纳入交付。

默认程序仅使用Qt Quick界面；启动失败的QMessageBox改为原生MessageBoxW，避免在关闭重定向表面的D3D模式中依赖Qt raster窗口。未通过缩放、延时隐藏、禁用歌词或动画规避。

Qt实现参考：https://github.com/qt/qtbase/blob/v6.11.1/src/plugins/platforms/windows/qwindowswindow.cpp 。现象和修复效果支持该合成路径解释；不把具体驱动内部故障推断成已证实事实。

## 验证

- Qt 6.11.1 Release构建通过；6组CTest通过，54.83秒。
- 合成测试不再调用会强制渲染的QQuickWindow::grabWindow；背景改用原生Win32窗口，避免额外Qt RHI或raster窗口改变被测条件。
- 七种静态状态：首次、重开、穿透、恢复输入、再次重开、半透明、恢复透明度，边缘采样全部通过；穿透另验证WindowFromPoint未命中歌词窗口。
- 软件渲染对照通过，原生exStyle保持80088/800a8，不误启用NoRedirection。
- 保留原有隐藏时释放歌词场景和离屏资源的策略，媒体控制与网络路径未改动。

本地证据位于 artifacts/white-background-investigation/。真实用户截图只保留本地，不提交到仓库或上传。

## 真实运行复核

已退出旧 preview7 并运行打包后的 preview8，自动连接原配对手机，歌曲、歌手和29秒暂停位置保持一致；通过实际窗口截屏确认桌面歌词原文及译文透明合成，白色底板消失。用户现场截图只存于本地 live-white-desktop.png 与 live-fixed-desktop.png。当前观察不替代长期偶发回归，后续若再现仍继续记录。
