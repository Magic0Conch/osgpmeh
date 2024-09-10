# osgb模型简化工具
## 分支说明
release分支为正式出包时的代码，无调试信息。但打包包体较小。
main分支为调试分支，有调试信息，可以断点调试，但包体较大。


## 命令使用范例
简化范例，0.3表示单次简化模板比例，比例越低简化程度越高。3表示迭代次数，数值越高简化程度越高。
```bash
./osgpmesh 0.3 3 E:\Data\gaunglianda\input\zhibei1.osgb E:\Data\gaunglianda\output\zhibei1_3_3.osg
```
工具还提供预览功能，预览文件格式如下：
```bash
 ./osgpmesh E:\Data\gaunglianda\output\zhibei1_3_1.osgb
```
进入预览窗口后，按下w(必须英文输入法)可以看到线框模式，连续按s可以看到三角形数量等信息。
