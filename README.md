### 硬件
 - 天问asrpro
 - esp8266模块
![在这里插入图片描述](https://img-blog.csdnimg.cn/22b97c74c0404aca8f2e0a76f03385e0.png)
![在这里插入图片描述](https://img-blog.csdnimg.cn/49f8329ef81e439aa07be9deae4b7652.png)

### 接线方式
asrpro     esp8266

PA2			D7

PA3			D8

GND        GND

3v3			3v3


### 串口通信
如实现一个开机指令
可以编辑发送POWER_ON% 命令以%结尾
同时通过接受返回值来判断是否成功
ASR图形代码：
![在这里插入图片描述](https://img-blog.csdnimg.cn/14cf2d86840b43cc9b1b468cd2125801.png)
	
esp8266 接受端代码示例：
![在这里插入图片描述](https://img-blog.csdnimg.cn/53baa04d0b8a4f4d9378c128416e7b2a.png)

### 源码示例

天问asr
`asr/asr.hd`

### 配合BLINKER开关机
`https://github.com/jayxtt999/blinker_esp8266_pc_power`
