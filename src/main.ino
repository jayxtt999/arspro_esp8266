#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFiClient.h>
// #include <ESP8266WebServer.h>
// #include <ElegantOTA.h>
#include <SoftwareSerial.h>
#include <ESP8266Ping.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);
WiFiClient tcpclient;
// ESP8266WebServer server(80);//暂时关闭OTA

// const char *wifihostname = "ESP_WITH_TW"; //暂不开启自动配网
// const char *jobname = "tianwen";

const char *WIFI_SSID = ""; // 填写你的WIFI名称及密码
const char *WIFI_PWD = "";
#define PCIP "192.168.6.100"         // 地址一：电脑ip地址，这里存在用命令开关机电脑，注意在路由器上绑定ip，此处主要是用于ping一下，再次查询真实开关机状态
#define TCP_SERVER_IP "192.168.6.28" // 地址一：控制电脑开关的继电器esp01s模块ip地址，注意在路由器上绑定ip
#define TCP_SERVER_PORT 6000

#define MAX_COMMAND_LENGTH 10
char command[MAX_COMMAND_LENGTH] = ""; // 存储接收到的命令字符串
int commandLength = 0;                 // 当前接收到的命令长度

SoftwareSerial MySerial(13, 15); // 定义软串口对象，RX=13, TX=15

char chBuffer[64];
int p = 0;
const unsigned long TIMEOUT = 10000; // 设置超时时间为 10 秒

void setup()
{
    // 初始化串口和软串口
    Serial.begin(9600);
    MySerial.begin(115200);
    //  连接Wi-Fi
    wificonnect();
    // 初始化时间客户端、HTTP服务器和OTA
    // timeClient.begin();
    /*    server.on("/", []()
                  { server.send(200, "text/plain", "Hi! I am ESP8266."); });
        ElegantOTA.begin(&server);
        server.begin();
        Serial.println("HTTP server started");
    */
    Serial.println("Ready");
}

void loop()
{

    // 处理串口指令和HTTP客户端请求
    // SCmd.readSerial();
    // server.handleClient();//暂时关闭OTA

    if (MySerial.available())
    {
        int commandLength = 0; // 当前接收到的命令长度
        while (MySerial.available() > 0)
        {
            char c = MySerial.read();
            if (c == '%')
            {
                break;
            }
            if (commandLength >= MAX_COMMAND_LENGTH - 1)
            {
                break;
            }
            command[commandLength++] = c;
        }
        // command[commandLength] = '\0'; // 末尾添加"\0"作为字符串结束标志
        if (commandLength > 0)
        {
            String c = String(command);
            c.trim();
            Serial.println("Received command: " + c); // 输出接收到的命令
            commandLength = 0;                        // 清空命令长度变量
            memset(command, 0, sizeof(command));
            executeCommand(c); // 执行命令
        }
    }
}

/* 判断命令名称并执行对应的方法 */
void executeCommand(String command)
{
    if (command == "POWER_ON")
    { // 重启命令
        PowerOn();
    }
    else if (command == "POWER_OFF")
    { // 启动命令
        PowerOff();
    }
    else if (command == "SayHello")
    { // 停止命令
        SayHello();
    }
    else
    { // 未知命令
        Serial.println("Unknown command: " + command);
    }
}

// 回调函数：打开电源
void PowerOn()
{

    if (Ping.ping(PCIP))
    {
        // 如果能ping通目标PC，则记录一条日志消息并返回
        Serial.println("pc is on...");
        MySerial.write("success%");
    }
    else
    {
        Serial.println("PowerOn Start");
        SendTcpData("POWER\r"); // 这里的POWER是与用于与esp01s通信，我在esp01s启用了一个tcp服务
    }
}

void PowerOff()
{

    if (Ping.ping(PCIP))
    {
        Serial.println("PowerOff Start");
        SendTcpData("POWER\r"); // 这里的POWER是与用于与esp01s通信，我在esp01s启用了一个tcp服务
    }
    else
    {
        Serial.println("success");
        Serial.println("pc is off...");
        MySerial.write("success%");
    }
}

void SayHello()
{
    Serial.print("Hello ");
    MySerial.write("success%");
}

void SendTcpData(char *msg)
{
    Serial.print("connecting to tcp server");
    Serial.print(TCP_SERVER_IP);
    Serial.print(":");
    Serial.println(TCP_SERVER_PORT);

    if (!tcpclient.connect(TCP_SERVER_IP, TCP_SERVER_PORT))
    {
        Serial.println("connection failed"); // 如果连接失败，则打印连接失败信息，并返回
        MySerial.write("error%");
        return;
    }
    /* 如果连接成功，则发送一个字符串到TCP服务器 */
    Serial.println("Connected to TCPServer, sending data to server");
    if (!tcpclient.connected())
    {
        Serial.println("connection failed"); // 如果连接失败，则打印连接失败信息，并返回
        MySerial.write("error%");
        return;
    }
    tcpclient.println(msg);

    /* 等待TCP服务器返回消息 */
    Serial.println("waiting for receive from remote Tcpserver...");
    String response = "";
    unsigned long startTime = millis();
    while (tcpclient.connected())
    {
        while (tcpclient.available())
        {
            char ch = tcpclient.read();
            response += ch;
            if (response.endsWith("\r\n"))
            {
                // 完整地读取到了 HTTP 响应的头部，退出循环
                break;
            }
        }
        if (millis() - startTime > TIMEOUT)
        {
            // 如果超时，退出循环
            Serial.println("time out");
            break;
        }
    }

    // 处理接收到的数据（注意去除开始/结尾的空白字符）
    response.trim();
    Serial.print("response:");
    Serial.println(response);
    if (response == "Success")
    {
        // 如果收到有效响应，则记录一条日志消息并返回
        Serial.println("success");
        MySerial.write("success%");
    }
    else
    {
        // 否则，记录一条错误消息并返回
        Serial.println("error");
        MySerial.write("error%");
    }

    /* 接受到服务器返回的消息后关闭TCP连接 */
    Serial.println("closing connection");
}

// 连接Wi-Fi网络
void wificonnect()
{
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("wificonnect!!!");
    delay(500);
}