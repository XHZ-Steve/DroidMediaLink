# 应用更新 · v1.1

Windows「高级 → 更新」与 Android「设置 → 应用」均可手动检查 GitHub 最新正式 Release。不进行后台轮询。

- Windows 更新请求使用独立网络管理器并遵循系统代理，手机局域网通信仍明确绕过代理。
- 稳定数字版本按段比较：`v1.1` 等于 `1.1.0`，`1.10` 高于 `1.9.9`；忽略草稿和预发布，不降级。
- 确认后才下载本仓库 Release 中对应的 Windows x64 安装 EXE 或 Android APK；安装包必须有 GitHub API 提供的 SHA-256 digest，且不超过 160 MiB。
- 下载流式写入临时文件，校验实际长度与 SHA-256；失败不执行、不提交临时包。取消、超时与离线均保留现有安装与设置。
- Windows 验证后启动 Inno 安装程序，退出当前进程，在当前应用目录更新并于成功后重新启动。便携目录经此方式更新后也会包含安装器的卸载信息；若要保持完全解压式部署，可手动下载便携 ZIP。
- Android 另外核对 APK 包名和版本，通过仅授权 updates 缓存目录的 FileProvider 交给系统安装界面；首次可能要求「允许安装未知应用」。系统负责签名一致性和最终安装确认，普通应用不能静默安装。开发签名必须在以后版本中保持一致。
- GitHub digest 用于完整性验证，不等同于 Windows Authenticode 签名。Windows 包仍未签名，APK 仍使用已有开发签名，发布说明必须明确标注。

实现参考官方 [GitHub Release asset API](https://docs.github.com/en/rest/releases/assets)、[Android FileProvider](https://developer.android.com/reference/androidx/core/content/FileProvider)、[Android 安装授权](https://developer.android.com/reference/android/content/pm/PackageManager#canRequestPackageInstalls()) 与 [Inno Run](https://jrsoftware.org/ishelp/topic_runsection.htm)。

## 本次验证

- Windows：构建与六组 CTest 通过；覆盖等价版本、较旧版本、多位版本、非法/预发布标记、合法/非法来源、预发布资产过滤，以及实际本地 HTTP 坏包下载后 SHA-256 拒绝。
- Android：testDebugUnitTest、lintDebug、assembleDebug 通过；版本比较包含上述版本场景与数值溢出。Lint 为 0 错误，25 警告（含原有 KTX/风格提示）。
- 两种语言资源与通信协议校验通过；Windows 实际请求 GitHub v1.0 元数据并正确判断当前 1.1.0 无需降级，界面显示已是最新版本。
- 没有连接 ADB 设备，未宣称 Android 实机安装流程已验证；未人为发布虚假的更高版本来触发生产安装。正式 Release 上传后核对线上元数据、资产摘要与下载完整性。
