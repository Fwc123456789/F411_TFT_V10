# 贡献指南 (Contributing Guide)

感谢你对本项目的关注！本文档将指导你如何为项目做出贡献。

## 📋 在你开始之前

阅读主项目 [README.md](./README.md) 以了解项目的整体情况。

## 🚀 贡献类型

我们欢迎以下几种贡献方式：

### 1. 📝 文档改进
- 修复拼写错误或语法问题
- 补充缺失的说明或示例
- 添加更清晰的图表或流程图
- 翻译为其他语言

### 2. 🐛 Bug报告与修复
- 发现并报告bug（提交Issue）
- 提供详细的复现步骤
- 提交修复代码（Pull Request）

### 3. ✨ 功能增强
- 新的传感器支持（如BMP280、DHT22等）
- 存储功能（SD卡接口）
- 显示增强（多屏显示、OLED等）
- 通讯增强（蓝牙、LoRa等）

### 4. 🔧 代码优化
- 性能改进（降低CPU占用率）
- 内存优化
- 代码重构与规范化
- 编译选项优化

### 5. 🧪 测试用例
- 单元测试
- 集成测试
- 硬件测试报告

---

## 💻 本地开发环境设置

### 前置要求
- Keil MDK-ARM v5.x 或 STM32CubeIDE
- Git
- 基本的C语言和STM32开发知识

### 克隆仓库
```bash
git clone https://github.com/yourusername/F411_TFT_V10.git
cd F411_TFT_V10
```

### 设置开发环境
```bash
# Windows 用户
# 1. 安装 Keil MDK-ARM 或 STM32CubeIDE
# 2. 打开工程：MDK-ARM/F411_TFT_V2.uvprojx
# 3. 编译测试

# Linux/Mac 用户（如使用 STM32CubeIDE）
# 1. 安装 STM32CubeIDE
# 2. 导入工程
# 3. 构建项目
```

---

## 🔄 提交流程

### 第一步：创建Issue（如果还没有的话）

在提交代码之前，请先创建一个Issue来讨论你的想法：

1. 点击 [New Issue](../../issues/new)
2. 选择适当的Issue模板：
   - 🐛 **Bug Report** - 报告bug
   - ✨ **Feature Request** - 功能建议
   - 📝 **Documentation** - 文档改进
   - 🤔 **Question** - 提问
3. 填写详细信息

#### Bug Report 模板
```markdown
## 问题描述
简要描述你遇到的问题

## 复现步骤
1. ...
2. ...
3. ...

## 预期结果
应该发生什么

## 实际结果
实际上发生了什么

## 环境信息
- 开发板型号：
- IDE版本：
- 操作系统：
- 其他相关信息：

## 日志或截图
如果可能，附加相关日志或截图
```

#### Feature Request 模板
```markdown
## 功能描述
描述你想添加的新功能

## 使用场景
这个功能在什么情况下会用到

## 建议的实现方案
如果你有想法，请描述

## 其他信息
任何其他相关信息
```

### 第二步：Fork仓库

点击GitHub页面右上角的 **Fork** 按钮，创建自己的仓库副本。

### 第三步：创建特性分支

```bash
# 更新本地仓库
git fetch origin
git checkout main

# 创建新分支（分支名应该能反映功能内容）
git checkout -b feature/add-sd-card-support
# 或
git checkout -b fix/dht11-timeout-issue
# 或
git checkout -b docs/update-readme
```

**分支命名规范：**
- `feature/xxx` - 新功能
- `fix/xxx` - bug修复
- `docs/xxx` - 文档更新
- `refactor/xxx` - 代码重构
- `perf/xxx` - 性能优化
- `test/xxx` - 测试相关

### 第四步：进行修改

在你的本地分支上进行修改。

**代码规范：**
```c
// 1. 遵循现有的代码风格
// 2. 使用有意义的变量名
// 3. 添加适当的注释
// 4. 一个commit解决一个问题

// 示例：好的提交注释
// feature: Add SD card support
// - Initialize SDIO interface
// - Implement file read/write functions
// - Add error handling for card detection

// 避免：
// fixed bugs
// updates
// 中文混用等
```

**注释要求：**
```c
// 函数注释
/**
 * @brief 功能简述
 * @param param1 参数1说明
 * @param param2 参数2说明
 * @return 返回值说明
 * 
 * @note 使用说明或注意事项
 * @example 使用示例
 */
void MyFunction(int param1, char* param2);

// 重要代码块注释
// 说明这段代码的目的和关键逻辑
```

### 第五步：提交更改

```bash
# 查看修改
git status
git diff

# 暂存更改
git add .
# 或指定文件
git add Chen_Drives/TFT.c

# 提交更改（使用清晰的提交信息）
git commit -m "feature: Add temperature unit conversion

- Add Celsius to Fahrenheit conversion
- Add conversion function to DHT11 module
- Update display to show selected unit
- Closes #42"
```

**提交信息规范（Conventional Commits）：**
```
<type>(<scope>): <subject>

<body>

<footer>
```

类型：
- `feat:` 新功能
- `fix:` bug修复
- `docs:` 文档
- `style:` 代码风格（不改变功能）
- `refactor:` 代码重构
- `perf:` 性能改进
- `test:` 测试
- `chore:` 构建过程、依赖管理等

示例：
```
fix(DHT11): Fix timeout issue in sensor reading

- Increase timeout threshold from 100ms to 150ms
- Add retry mechanism for failed reads
- Improve error handling

Fixes #38
```

### 第六步：推送到fork

```bash
git push origin feature/add-sd-card-support
```

### 第七步：创建Pull Request

1. 访问你fork的仓库页面
2. 点击 **Compare & pull request** 按钮
3. 确保：
   - 基础分支是主项目的 `main`
   - 你的分支包含你的修改
4. 填写Pull Request模板：

```markdown
## 描述
简要描述你所做的改变

## 相关Issue
修复 #42
相关 #43

## 修改类型
- [ ] Bug修复
- [ ] 新功能
- [ ] 破坏性改变（breaking change）
- [ ] 文档更新

## 如何测试？
描述测试步骤：
1. ...
2. ...

## 检查清单
- [ ] 我已阅读 CONTRIBUTING.md
- [ ] 代码遵循项目的代码风格
- [ ] 我已进行自我审查
- [ ] 我添加了必要的注释
- [ ] 代码在本地测试通过
- [ ] 没有新的警告信息
- [ ] 我没有添加新的依赖项（除非必要）
```

### 第八步：等待审查与反馈

维护者会审查你的代码。可能会：
- 请求修改
- 提出问题
- 合并你的PR

**保持耐心** 😊 - 审查可能需要几天时间。

---

## 📐 代码规范

### C语言代码风格

```c
// 文件头注释
/**
 * @file dht11.c
 * @author Chen Ying
 * @brief DHT11 temperature and humidity sensor driver
 * @version 1.0
 * @date 2026-04-29
 */

// 包含头文件
#include "stm32f4xx_hal.h"
#include "dht11.h"

// 宏定义
#define DHT11_MAX_RETRIES    3
#define DHT11_TIMEOUT_MS     20

// 类型定义
typedef struct {
    uint8_t temperature;
    uint8_t humidity;
    uint32_t last_read_time;
} DHT11_Data_t;

// 全局变量
static DHT11_Data_t g_dht11_data = {0};

// 函数实现
/**
 * @brief Read temperature and humidity from DHT11
 * @return true if successful, false otherwise
 */
bool DHT11_Read(void) {
    uint8_t retries = DHT11_MAX_RETRIES;
    
    // 循环直到成功或重试次数用完
    while(retries--) {
        // 实现逻辑
        if(DHT11_Read_Sensor()) {
            return true;
        }
    }
    
    return false;
}
```

### 缩进和格式

```c
// 使用4个空格缩进（不要用Tab）
if (condition) {
    // 4空格缩进
    DoSomething();
} else {
    // 另一选项
    DoOtherThing();
}

// 函数大括号风格（K&R风格）
void MyFunction(void) {
    // 函数体
}

// 长行折行
uint32_t very_long_function_name(uint8_t param1, 
                                  uint8_t param2, 
                                  uint8_t param3);
```

### 命名规范

```c
// 常量：全大写 + 下划线
#define MAX_BUFFER_SIZE     256
#define DHT11_TIMEOUT_MS    20
static const uint8_t DEFAULT_VALUE = 10;

// 类型定义：PascalCase + _t 后缀
typedef struct {
    uint8_t temp;
    uint8_t humidity;
} DHT11_Data_t;

typedef enum {
    STATE_IDLE,
    STATE_READING,
    STATE_DONE
} DHT11_State_t;

// 函数：snake_case（小写+下划线）
void dht11_init(void);
uint8_t dht11_read_temperature(void);
bool dht11_is_ready(void);

// 全局变量：g_ 前缀 + snake_case
static uint8_t g_dht11_temp = 0;
static uint16_t g_adc_value = 0;

// 局部变量：snake_case
int local_counter = 0;
uint8_t sensor_data = 0;
```

### 注释规范

```c
// 单行注释用 //
// 这是一条单行注释

/*
 * 多行注释用 /* ... */
 * 每行开头加 *
 * 更易阅读
 */

// 函数说明用Doxygen格式
/**
 * @brief 简要说明
 * @param[in] param1 输入参数说明
 * @param[out] param2 输出参数说明
 * @return 返回值说明
 * @retval true 操作成功
 * @retval false 操作失败
 * 
 * @note 额外说明
 * @warning 警告信息
 * @see 相关函数
 */
bool some_function(uint8_t *param1, uint8_t *param2);

// TODO/FIXME注释
// TODO: Implement SD card read function
// FIXME: DHT11 sometimes fails to read on first try
// HACK: Temporary fix for SPI timing issue
```

---

## 🧪 测试

在提交PR前，请确保：

### 1. 本地编译测试
```bash
# Keil IDE中
Build → Rebuild All  (检查无编译错误)
```

### 2. 硬件测试
- 在实际硬件上测试你的修改
- 记录测试结果
- 如果可能，提供测试日志

### 3. 功能验证
```
测试清单示例：
- [ ] DHT11正常读取数据
- [ ] TFT屏幕正常显示
- [ ] 按键响应正确
- [ ] WiFi连接稳定
- [ ] MQTT数据上传成功
```

---

## 📚 项目结构理解

```
F411_TFT_V10/
├── Chen_Drives/          # 自定义驱动
│   ├── TFT.c/h          # 屏幕驱动（关键模块）
│   ├── DHT11.c/h        # 温湿度传感器
│   ├── Ds18b20.c/h      # 温度传感器
│   └── Key.c/h          # 按键处理
├── Core/
│   ├── Inc/             # STM32配置头文件
│   └── Src/             # STM32配置源文件 + main.c
├── MDK-ARM/             # Keil工程文件
└── Drivers/             # HAL库文件
```

**修改影响范围：**
- 修改 `TFT.c` → 影响所有显示功能
- 修改 `DHT11.c` → 只影响温湿度采集
- 修改 `main.c` → 影响菜单系统和主逻辑
- 修改 `Core/Src/*.c` → 可能影响硬件初始化

---

## 🎯 优先级高的改进方向

我们特别欢迎以下方向的贡献：

### P0 - 关键改进
- [ ] ADC配置为DMA模式（解决当前阻塞式采集）
- [ ] 完善MQTT错误处理和超时检测
- [ ] 添加单元测试框架

### P1 - 重要功能
- [ ] SD卡数据存储支持
- [ ] 数据日志功能
- [ ] 本地数据查询
- [ ] 多传感器支持

### P2 - 增强功能
- [ ] OLED副屏显示
- [ ] 蓝牙连接
- [ ] 语音提示
- [ ] 多语言支持

### P3 - 优化项目
- [ ] 代码重构与规范化
- [ ] 性能优化
- [ ] 文档完善
- [ ] 示例代码

---

## ❓ 常见问题

### Q: 我想开始贡献，但不知道从何开始？
**A:** 
1. 查看 [Issues](../../issues) 列表
2. 找到标记为 `good-first-issue` 的任务
3. 在Issue下留言说你想要参与
4. 按照上面的流程提交PR

### Q: 我的PR被拒绝了，怎么办？
**A:** 这是正常的！请：
1. 阅读维护者的反馈意见
2. 根据建议进行修改
3. 推送新的提交
4. 回复维护者

### Q: 提交PR后多久会有反馈？
**A:** 通常在 3-7 天内。紧急情况下可能更快。

### Q: 我可以提交多个PR吗？
**A:** 完全可以！但请：
- 每个PR专注于一个功能/问题
- 不要提交过多的PR而没有等待反馈
- 确保质量高于数量

### Q: 如何获得写入权限？
**A:** 
1. 提交3个以上高质量的PR
2. 表现出对项目的理解和承诺
3. 维护者会邀请你成为贡献者

---

## 📖 有用的资源

- [Git教程](https://git-scm.com/book/zh/v2)
- [Conventional Commits](https://www.conventionalcommits.org/zh-hans/)
- [STM32 HAL Reference Manual](https://www.st.com/resource/en/user_manual/dm00122015-stm32cubef4-hal-user-manual-stmicroelectronics.pdf)
- [GitHub Pull Request Flow](https://guides.github.com/introduction/flow/)

---

## 💬 联系我们

- 在Issue中讨论
- 提交Pull Request
- 发送邮件至 [your-email@example.com]

---

**感谢你的贡献！** 🎉

本项目因你而变得更好！
