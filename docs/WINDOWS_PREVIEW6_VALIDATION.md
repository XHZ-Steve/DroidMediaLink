# Windows preview6 验证记录

日期：2026-11-16。未发布或覆盖 GitHub Release。

本轮调整：桌面歌词原文/译文组居中、间距偏移与防抖调整、时间偏移快捷按钮；歌词预滚动与缓入缓出长句滚动；子窗口原生拖动结束时释放 QML 鼠标抓取并延后恢复媒体刷新；歌词样式快照隔离无关设置更新；音量请求合并及匹配回执；缓存目录、用量与清理；逐项悬停重置；后台暂停氛围动画；使用 Qt Quick Controls BusyIndicator 显示更新检查；中日韩标点字体回退。

- Qt 6.11.1 / MinGW Release 构建成功。
- CTest 六组测试全部通过，47.39 秒。覆盖音量排队/迟到回执/失败回执、缓存迁移/清理/手动歌词保留、隐藏 UI 的 SMTC 时间轴刷新、歌词预定位与滚动曲线、设置数值防抖。
- 资源与协议检查通过；git diff --check 通过。
- 实际启动 Windows 应用，捕获主页、桌面歌词与缓存设置截图，日志未报告 QML 加载错误。截图使用隔离的固定媒体/歌词夹具，不能视为当前手机会话或高清封面匹配证据。
- 只读 Luna 代码审查后修正了恢复默认设置时封面服务目录未同步的问题；设置导入本来通过 setSetting 路由，能够同步。

本地证据：artifacts/lyrics-cache-tests.log、lyrics-cache-build.log、lyrics-cache-review/ 内的截图与运行日志。

## 尚待验证

- Windows Computer Use 运行时两种入口均报“failed to write kernel assets: 系统找不到指定的路径”，因此没有完成真实鼠标连续拖动的交互回归；不能凭代码检查声称该项已实测。
- SMTC 系统面板能否正确读取真实手机进度，仍列在 OPEN_ISSUES.md。自动测试仅证明发布端在隐藏 UI 后持续更新。
- 尚未实测 GPU 百分比；可见歌词和子窗口仍需要渲染，不承诺所有后台场景均为 0%。
