# FCEUX

Nintendo Entertainment System模拟器, 可以正确运行大多数ROM.
移植自 https://github.com/TASVideos/fceux ,
git commit版本为`ed4f5d0000e17b6ae88c4e93e2f9e0695dbceac0`.

本项目将其移植到[AM](https://github.com/NJU-ProjectN/abstract-machine)环境中,
完整程度不同的IOE可以支持不同的功能:
* 只有时钟: 可通过注释`src/config.h`中的`HAS_GUI`宏来运行字符模式
* 添加键盘: 可在字符模式下操作
* 添加绘图: 可运行图形模式
* 添加声音: 可播放游戏音效

## 运行方式

将游戏ROM放置在`nes/rom/`目录下, 并命名为`xxx.nes`, 如`nes/rom/mario.nes`.
然后可通过`mainargs`选择运行的游戏, 如:
```
make ARCH=native run mainargs=mario
```

## 操作方式

* U — SELECT
* I — START
* J — A键
* K — B键
* W/S/A/D — UP/DOWN/LEFT/RIGHT
* Q — 退出
