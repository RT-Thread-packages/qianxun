# qxwz_application
## 1、介绍
- 本软件包是集成千寻位置差分sdk的集成软件包，通过该软件包，可以快速集成sdk，实现千寻高精度定位。


### 1.1 目录结构

`qxwz_application` 软件包目录结构如下所示：

``` 
qxwz
├───docs 
│   └───figures                                            // 文档使用图片
│   │   api.md                                             // API 使用说明
│   │   README.md                                          // 文档结构说明  
│   │   samples.md                                         // 软件包示例
│   │   差分数据嵌入式SDK开发指南_V2.0.4.pdf                 // 使用说明
│   └───version.md                                         // 版本
├───qxwz_sdk                                               // 集成源代码
├───qxwz_lib                                               // 差分SDK库
├───samples                                                // 示例代码
│       qxwz_application.c                                 // 软件包应用示例代码
│   LICENSE                                                // 软件包许可证
└───README.md                                              // 软件包使用说明
```

### 1.2 许可证

`qxwz_application` package 遵循 Apache 2.0 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread 3.0+
- 需要网络系统（wifi或4G/3G/2G网络）与千寻服务器交互获取差分数据
- 需要读写串口，读取GPS芯片输出数据
- 需要文件系统，保存高精度定位结果


## 2、获取软件包

使用 `qxwz` 软件包需要在 BSP 目录下使用 menuconfig 命令打开 Env 配置界面，在 `RT-Thread online packages → IoT - internet of things`  中选择 QXWZ 软件包，操作界面如下图所示：

![menuconfig 中开启 QXWZ 支持](figures/qx.png)


选择合适的配置项后，使用 `pkgs --update` 命令下载软件包并添加到工程中即可。

## 3、使用 qxwz_application

* 如何从零开始使用，请参考 [用户开发指南](docs/差分数据嵌入式SDK开发指南_V2.0.4.pdf)。
* 完整的 API 文档，请参考 [API 手册](docs/api.md)。
* 详细的示例介绍，请参考 [示例文档](docs/samples.md) 。
* 更多**详细介绍文档**位于 [`/docs`](/docs) 文件夹下，**使用软件包进行开发前请务必查看**。

## 4、注意事项

- 正确填写千寻的账号信息

    如果账号信息填写错误，将无法正确连接到千寻服务器。


## 5、联系方式 & 感谢

* 维护：RT-Thread 开发团队
* 主页：https://github.com/RT-Thread-packages/qianxun

