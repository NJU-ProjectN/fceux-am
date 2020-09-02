# FCEUX

Nintendo Entertainment System模拟器, 可以正确运行大多数ROM.
移植自 https://github.com/TASVideos/fceux ,
git commit版本为`ed4f5d0000e17b6ae88c4e93e2f9e0695dbceac0`.

内置一些游戏ROM, 见`src/roms/rom`目录.

可通过`mainargs`选择运行的游戏, 如:
```
make ARCH=native run mainargs=mario
```

操作方式:

* U — SELECT
* I — START
* J — A键
* K — B键
* W/S/A/D — UP/DOWN/LEFT/RIGHT

需要正确的IOE (绘图、定时).
