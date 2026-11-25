# logkit

#### 介绍
常用工具函数，以 **C++14 动态库**（`liblogkit.so`）形式提供。

#### 目录结构
```
logkit/
├── clear.sh                   # 清理构建目录（build/、build-*/、logkit_*/）
├── cmake/toolkitConfig.cmake  # 安装期包配置模板（CONFIG 模式，包名 toolkit）
├── compile.sh                  # 一键配置 + 编译 + 安装（固定 -j3）
├── demo/                       # 示例程序（与 src/ 同级，独立工程，消费安装包）
│   ├── CMakeLists.txt          # find_package(toolkit) + toolkit::logkit
│   └── tlLog_demo.cpp
└── src/                        # CMake 源根：实现 + 公开头文件 + 测试
    ├── CMakeLists.txt          # 构建脚本（CMake 3.16+）
    ├── logkit/                 # 实现 + 公开头文件（安装到 <prefix>/include/toolkit/）
    │   ├── tl*.cc / tl*.h      # KLayout tl 源码（日志、字符串、流、计时、正则…）
    │   └── atomic/             # 原子操作/自旋锁（spinlock.h 依赖同目录的 atomic.h）
    └── test/                   # 单元测试（GoogleTest + CTest）
        ├── CMakeLists.txt
        └── tl*_test.cpp
```

#### 安装教程

编译：

```bash
./compile.sh                          # 等价于：
                                      # cmake -S src -B build -DCMAKE_BUILD_TYPE=Release
                                      # cmake --build build -j3
                                      # cmake --install build --prefix "$PWD/toolkit_0_1_0"
```

`compile.sh` 最后一步把产物装到**与脚本同级的 `toolkit_<ver>/`**：版本默认 `0.1.0`，
点号换成下划线（即 `toolkit_0_1_0/`）：

```
toolkit_0_1_0/
├── include/toolkit/tlLog.h          # 唯一公开头文件
├── include/toolkit/detail/*.h       # 实现细节（tlLog.h 自己会引，勿直接使用）
├── include/toolkit/detail/atomic/*.h
└── lib/{liblogkit.so, cmake/toolkit/*.cmake}
```

对外暴露的只有两样：**`include/toolkit/tlLog.h`** 和 **`lib/liblogkit.so`**，消费端只写
`#include <toolkit/tlLog.h>`。其它头文件并没有消失——`tlLog.h` 本身 include 了
`tlCommon.h / tlString.h / tlThreads.h / tlObjectCollection.h`（再连带约 10 个），
它们编译时不可或缺，但属于实现细节，统一收到 `include/toolkit/detail/` 下，并且安装时
tlLog.h 里的 `#include "tlXxx.h"` 会被重写成 `#include "detail/tlXxx.h"`。

```bash
./compile.sh                                  # 默认：装到 ./toolkit_0_1_0/
./compile.sh --ver 1.2.3                      # 改版本：装到 ./toolkit_1_2_3/
./compile.sh --path /tmp/out                  # 直接导出到 /tmp/out/
./compile.sh --ver 1.2.3 /path/to/toolchain/third_party   # 装到 toolchain
PREFIX=/tmp/tk ./compile.sh                   # 等价于 --path（完整前缀，不拼版本）
```

清理构建产物与安装输出：

```bash
./clear.sh              # 默认清理 build/、build-*/、logkit_*/ 与 toolkit*/
./clear.sh build-cli    # 只删指定目录（必须在仓库内）
```

构建产物：`build/liblogkit.so`（`project()` 没声明版本，所以不生成 `.so.0` / `.so.0.1.0`）。

想让别的工程通过 toolchain 直接 `find_package(toolkit)` 时，再额外装到 toolchain：

```bash
./compile.sh /home/hz/toolchain/third_party   # 或：cmake --install build --prefix /home/hz/toolchain/third_party/toolkit_0_1_0
```

安装时会在 `<prefix>/lib/cmake/toolkit/` 生成 `toolkitConfig.cmake` 和 `toolkitTargets*.cmake`，
也就是标准的 **CONFIG** 模式包。**包名是 `toolkit`，每个 kit 是一个 `toolkit::<kit>` 目标**，
目前只有一个 kit，产物是 `liblogkit.so`：

```cmake
find_package(toolkit REQUIRED)                      # 或 COMPONENTS logkit
find_package(toolkit REQUIRED COMPONENTS logkit)    # 显式要求 logkit 组件
target_link_libraries(your_target PRIVATE toolkit::logkit)
```

后续新增 kit（比如 `xkit`）时，只需再导出 `toolkit::xkit` 并在
`cmake/toolkitConfig.cmake` 里补一行 `set(toolkit_xkit_FOUND TRUE)`，
已有的 `toolkit::logkit` 使用方无需改动。

zlib 是以 `PRIVATE` 链进 `liblogkit.so` 的，不会写进导出目标的接口，所以消费端无需
`find_package(ZLIB)`。没有 `toolkitConfigVersion.cmake`，所以 `find_package(toolkit 0.1 ...)`
这类带版本的请求不支持。

工具链侧需要把 `toolkit_ROOT` 指向 `<toolchain>/third_party/toolkit_0_1_0`
并加进 `CMAKE_PREFIX_PATH`，用了这个 toolchain 的工程就能直接写：

```cmake
find_package(toolkit REQUIRED)
target_link_libraries(your_target PRIVATE toolkit::logkit)
```

运行时需要把装好的 `lib` 目录加进 `LD_LIBRARY_PATH`：

```bash
export LD_LIBRARY_PATH=/home/hz/toolchain/third_party/toolkit_0_1_0/lib:$LD_LIBRARY_PATH
```

#### 使用说明

> **命名空间**：库的命名空间是 **`toolkit::logkit`**（与 CMake 包/目标 `toolkit` /
> `toolkit::logkit` 一致）。**不再提供 `tl` 别名**——原来 KLayout 风格的 `tl::xxx`
> 写法已全部改为 `toolkit::logkit::xxx`，老代码需要同步改名。

1. 普通使用：链接动态库并包含头文件（`tlLog.h` 是公开头文件之一）。

```cpp
#include <toolkit/tlLog.h>

int main() {
    toolkit::logkit::info << "hello" << 42;      // 换行由语句结束自动补上
    toolkit::logkit::warn << "careful";          // stdout，行首带 "[warn]" 标签
    toolkit::logkit::error << "boom";            // stderr，行首带 "[error]" 标签
    return 0;
}
```

> 不要再 `#include <toolkit/tlString.h>` / `tlTimer.h` 这类内部头——它们现在在
> `<prefix>/include/toolkit/detail/` 下，属于实现细节。如果确实需要某个模块的能力，
> 应把它提升为公开头（见下方「约定」）。

`tlGLog.h`（与 `tlLog.h` 一起装在 `include/toolkit/`）提供 glog 风格的宏，它们是通道的别名
（不是 glog 封装，库不依赖 glog），并且**自动带上时间、文件名和行号**：

```cpp
#include <toolkit/tlGLog.h>

LOGI << "starting phase " << 3;        // -> toolkit::logkit::info  （stdout，始终可见）
LOGW << "value out of range: " << v;   // -> toolkit::logkit::warn  （stdout）
LOGE << "cannot open " << path;        // -> toolkit::logkit::error （stderr）
LOGF << "unrecoverable error";         // -> toolkit::logkit::fatal （stderr，随后 abort ()）
```

输出形如——**每行以 `[level]` 开头，宏还会接上 `[HH:MM:SS.mmm 文件:行号]`**
（时间/位置由宏用 `__FILE__`/`__LINE__` 提供）：

```
[info][12:34:56.789 main.cpp:42] starting phase 3
[warn][12:34:56.790 main.cpp:43] value out of range: 7
[error][12:34:56.791 main.cpp:44] cannot open x.txt
[fatal][12:34:56.792 main.cpp:45] unrecoverable error
```

直接写 `toolkit::logkit::info << ...`（不经宏）时只有 `[info] 消息`，没有时间/位置部分。

`LOGI` 走 `info` 通道：**始终可见**（verbosity 0 起）。原始 `toolkit::logkit::log` 通道仍从级别 10 起输出，
初始级别可用环境变量 `KLAYOUT_VERBOSITY=<n>` 覆盖（默认 10）。

`LOGF` **不是可捕获的异常**：消息刷出后进程直接 `abort()`（退出码 134）。
直接写 `toolkit::logkit::info << ...`（不经宏）时没有时间/位置前缀——因为调用点位置只能由宏提供。

#### 打印自定义类型

所有被 `<<` 的值都通过 `toolkit::logkit::to_string (t)` 转成文本，默认实现就是调用
`t.to_string ()`。因此扩展有两种方式：

1. **类型自带 `to_string()` 成员** —— 直接可用，无需任何额外代码：

```cpp
struct Money {
    int cents;
    std::string to_string () const
    {
        return toolkit::logkit::to_string (cents / 100) + "." +
               toolkit::logkit::to_string (cents % 100) + " EUR";
    }
};

LOGI << "money: " << Money { 1234 };      // money: 12.34 EUR
```

2. **无法修改的类型**（`QString`、`std::pair`、第三方类） —— 特化 `to_string`：

```cpp
namespace toolkit { namespace logkit {
template <> std::string to_string (const QString &s)
{
    return s.toUtf8 ().constData ();
}
} }

LOGI << "qstring: " << qs;                 // qstring: <内容>
```

> 特化必须**在使用之前**（同一 TU 内）可见。`demo/tlLog_demo.cpp` 里自有类型和
> “改不了的类型”都用这种**特化（非成员函数）**方式演示（示例里的 `QString` 是模拟类型，
> 库本身不依赖 Qt）。

2. 其它 CMake 工程引用（安装到 toolchain 后，无需额外参数）：

```cmake
find_package(toolkit REQUIRED)                     # 核心，无第三方依赖
target_link_libraries(your_target PRIVATE toolkit::logkit)
```

3. 本地调试时也可以直接把子目录加进来：

```cmake
add_subdirectory(path/to/logkit)
target_link_libraries(your_target PRIVATE logkit)
```

#### 单元测试

测试用 toolchain 里的 GoogleTest（`find_package(GTest REQUIRED)`），由 CTest 驱动，
随 `./compile.sh` 一起编译（没有开关，测试总是构建）。

```bash
ctest --test-dir build --output-on-failure   # 跑全部用例
ctest --test-dir build -R Log                # 只跑匹配的用例（如 Log.*）
ctest --test-dir build -N                    # 只列出用例，不跑
```

新增用例：在 `src/test/` 下新建 `*_test.cpp` 即可（`src/test/CMakeLists.txt` 用
`file(GLOB ... CONFIGURE_DEPENDS)` 自动发现，不用改列表）；每个 `TEST()` 都会被
`gtest_discover_tests()` 单独注册成一条 CTest 用例。

#### 示例程序

`demo/tlLog_demo.cpp` 演示 `tlGLog.h` 的 glog 风格宏（`LOGI`/`LOGW`/`LOGE`）。它在仓库根 `demo/`（与 `src/` 同级），
并且是一个**独立工程**：不链接源码树里的目标，而是用装好的 `toolkit_0_1_0/`
（`find_package(toolkit)` + `#include <toolkit/tlGLog.h>` + `toolkit::logkit`），
所以它同时就是安装树的冒烟测试。`./compile.sh` 在安装之后会自动把它编出来：

```bash
./compile.sh
./build/demo/logkit_log_demo                          # 跑 demo
KLAYOUT_VERBOSITY=20 ./build/demo/logkit_log_demo     # 提高初始日志级别
```

手动对着某个已安装前缀验证：

```bash
cmake -S demo -B build/demo -DCMAKE_PREFIX_PATH=$PWD/toolkit_0_1_0
cmake --build build/demo -j3
./build/demo/logkit_log_demo
```

链接到安装树里的 `.so` 时 CMake 会把该目录写进 RUNPATH，所以跑 demo 不需要
`LD_LIBRARY_PATH`。

注意：一次输出语句的**换行是自动补的**（临时 `ChannelProxy` 析构时），链尾再写
`toolkit::logkit::endl` 会多出一个空行；`toolkit::logkit::endl` 只当语句内分隔符用，要取消自动换行用 `toolkit::logkit::noendl`。

#### 约定

- 命名：**包名 `toolkit`，kit 目标 `toolkit::<kit>`**；当前只有一个 kit → `toolkit::logkit`
  （SHARED，产物 `liblogkit.so`，无第三方依赖；zlib 为 `PRIVATE`）。
- 安装形态：`lib/liblogkit.so` + `include/toolkit/` + `lib/cmake/toolkit/`（CONFIG 模式包）。
  `find_package(toolkit)` 不需要 toolchain 侧的任何 Find 模块，只要前缀能被
  `toolkit_ROOT` / `CMAKE_PREFIX_PATH` 找到即可。
- **公开头文件只有 `tlLog.h`**：`src/CMakeLists.txt` 把它单独装到 `include/toolkit/`，
  并把它的 `#include "tlXxx.h"` 重写成 `#include "detail/tlXxx.h"`；其余头全部装到
  `include/toolkit/detail/`。想把另一个头（如 `tlTimer.h`）也公开，只需在安装规则里
  把它从“排剩的头文件”里摘出来、单独装一份到 `include/toolkit/`；若它自己也 include 了
  兄弟头，同样要重写路径（或者直接也放到 `detail/` 下靠相对 include 解决）。
- `tlGLog.h` 现在作为内部头装在 `detail/` 下，且**不再有 glog 的 CMake 目标**（库内部不再
  依赖 glog）；该头目前是注释状态，真正启用时用它的工程需自备 glog 与 Qt（`QString`）。
- 标准：C++14（`CMAKE_CXX_EXTENSIONS OFF`，即 `-std=c++14`）。
- 符号可见性：不做特殊处理（默认可见/导出），`src/logkit/` 下只有手写头文件，
  没有 `logkit_export.h` 之类的生成头，公开 API 也不需要加导出宏。
- 新增模块：实现和头文件都放 `src/logkit/`，实现加入 `CMakeLists.txt` 的源码列表。

