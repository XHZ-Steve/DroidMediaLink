<div align="center">

# DroidMediaLink

**Your Android music, playback controls and synchronized lyrics on Windows.**

[Download v1.1](https://github.com/XHZ-Steve/DroidMusicLink/releases/tag/v1.1) · [简体中文](README.md) · [Get started](#get-started) · [Report an issue](https://github.com/XHZ-Steve/DroidMusicLink/issues)

</div>

Keep music playing on your phone while Windows displays the current track, artwork, position and lyrics. Control your existing Android player through desktop controls, system media keys or a floating player over your local network. Audio and headphones stay connected to your phone.

![Windows artwork ambience, synchronized lyrics and capsule playback controls](docs/screenshots/windows-current.png)

<details>
<summary>Watch the demo and see the Android app</summary>

![Windows app in motion](docs/screenshots/windows-demo.gif)

<img src="docs/screenshots/android-current.png" width="320" alt="Android connection service and current media" />

Real-device captures taken at different times; some appearance details may differ from the current version.

</details>

## Features

| | Available functionality |
| :--- | :--- |
| **Media synchronization** | Track, artist, album, artwork, state and position. Play, pause, skip, seek and change phone media volume. Favorite controls appear only when the player exposes the capability. |
| **Windows integration** | SMTC metadata and artwork, system media keys, tray menu, global shortcuts, transparent floating player and always-on-top desktop lyrics. |
| **Automatic lyrics** | Local cache, multiple online providers, player-published phone lyrics and local imports. Line/word timing, translation, romanization, cross-source translation supplementation and saved manual selections. |
| **Lyric presentation** | Smooth focus scrolling, word highlighting, long-line scrolling, loading fades and layout transitions. Shared media clock with independent desktop and main-window typography and layout. |
| **High-resolution artwork** | Phone artwork appears immediately, followed by asynchronous Windows enhancement. MusicBrainz / Cover Art Archive and experimental NetEase and QQ sources, draggable source priority, fetch diagnostics, caching and fallback. |
| **Immersive appearance** | Cover-derived flowing colors, adjustable fluid fill lighting, light/dark themes, optional Liquid Glass, capsule progress, water-glass playback buttons and per-layout artwork/title/lyrics positioning. |
| **Devices and preferences** | LAN discovery, matching-number phone authorization, saved pairing, revocation and reconnection. Two-level settings, global search, live preview, presets, section resets and import/export. |
| **Updates** | Check GitHub Releases on either device, compare versions, confirm download and verify the package. Windows starts the installer; Android opens the system installation flow. |

Android listens passively and does not periodically fetch online artwork. Cover enhancement runs asynchronously on Windows. Hidden or dragged windows reduce unnecessary rendering independently of media communication.

## Downloads

| Platform | Package | Usage |
| :--- | :--- | :--- |
| Windows x64 | [Installer EXE](https://github.com/XHZ-Steve/DroidMusicLink/releases/download/v1.1/DroidMediaLink-1.1.0-windows-x64-setup.exe) | Install and launch |
| Windows x64 | [Portable ZIP](https://github.com/XHZ-Steve/DroidMusicLink/releases/download/v1.1/DroidMediaLink-1.1.0-windows-x64.zip) | Extract everything; run `DroidMediaLink.exe` |
| Android 8.0+ | [APK](https://github.com/XHZ-Steve/DroidMusicLink/releases/download/v1.1/DroidMediaLink-1.1.0-android-test-signed.apk) | Install and complete permission onboarding |
| Integrity | [SHA256SUMS.txt](https://github.com/XHZ-Steve/DroidMusicLink/releases/download/v1.1/SHA256SUMS.txt) | Verify downloaded files |

Windows 11 x64 is the primary target. Windows packages are not Authenticode-signed; the APK uses development signing. Android updates require the same signing identity and system installation permission. Windows closes during update installation.

## Get started

1. **Install both apps.** Complete the Android permission wizard, grant notification access and configure background/battery permissions as needed. Enable its connection service.
2. **Use the same LAN.** Reveal the Windows settings entry at the top right, open Devices and connection, then discover and select your phone.
3. **Approve pairing.** Compare the numbers on both screens and confirm on Android. Subsequent connections can reconnect automatically without address or certificate entry.
4. **Play music on Android.** Windows synchronizes media and searches for lyrics automatically. Use the tray and settings to configure desktop lyrics, floating playback and appearance.

New installations use the maintainer's current optical-glass, fluid-lighting and lyric preferences. Existing settings remain intact. Defaults contain no device identities, addresses, credentials or window coordinates.

## Compatibility and limitations

- Players must expose Android MediaSession. Favorites, seeking, idle playback commands and complete phone lyrics depend on player capabilities.
- Online lyrics/artwork can fail or change availability. Local imports and phone artwork remain available; media control does not depend on these lookups.
- NetEase and QQ artwork adapters are experimental. Images and lyrics remain the property of their respective rights holders; see [artwork sources and limits](docs/high-resolution-covers.md).
- No audio streaming, music downloads, mirroring or traditional music library. Device communication is authenticated and encrypted. Do not expose the phone's service port publicly.
- Updates require confirmation and GitHub SHA-256 verification. Android controls final installation. Failed download/verification does not replace the installed application or its settings.

## Development

**Windows:** C++20, Qt Quick/QML and Win32/C++/WinRT. **Android:** Kotlin, Jetpack Compose and Material 3.

[Build and package](docs/build-and-release.md) · [Architecture](docs/architecture.md) · [Protocol](protocol/README.md) · [Testing](docs/testing.md) · [Changelog](CHANGELOG.md) · [Privacy](docs/privacy.md) · [Known limitations](docs/known-limitations.md)

Built with open-source projects including [qwindowkit](https://github.com/stdware/qwindowkit), [liquidglass](https://github.com/shayann07/liquidglass), [AndroidLiquidGlass / Backdrop](https://github.com/Kyant0/AndroidLiquidGlass) and Lucide. See [third-party notices](THIRD_PARTY_NOTICES.md) and [LICENSE](LICENSE).

Product name: **DroidMediaLink**. Repository name: **DroidMusicLink**.
