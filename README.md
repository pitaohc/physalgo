# phys_sap — Sweep And Prune 物理碰撞检测

一个用于学习物理引擎核心概念的极简工程：实现 **SAP（Sweep And Prune）宽相位碰撞检测**，并用 raylib 做 3D 可视化，配有单元测试与性能基准。

核心思想一句话：**碰撞检测只关心“谁可能相交”。** SAP 把所有物体的包围盒（AABB）投影到一个轴上排序扫描，只对投影重叠的候选对做完整检测，从而避免 O(n²) 的暴力全配对。

## 特性

- **可插拔宽相位接口**：`BroadPhase` 抽象接口，内置 `BruteForce`（O(n²) 基准）与 `SweepAndPrune` 两种实现，运行时可切换对比。
- **3D 包围盒**：核心统一使用 `Vec3` + 3D `AABB`，SAP 沿 x 轴排序、扫描，再对 y、z 轴做区间重叠测试。
- **3D 可视化**：轨道相机 + 俯视正交切换，重叠物体高亮、配对连线、x 轴端点扫描条。
- **单元测试**：SAP 与暴力法在随机场景下的结果等价性，覆盖贴边/嵌套/重合/z 轴分离等边界。
- **性能基准**：对比两种算法在不同物体数量下的耗时与加速比。

## 工程框架

```
phys/
├─ CMakeLists.txt            # 目标：phys_core / phys_tests / phys_bench / phys_app
├─ CMakePresets.json         # CLion/VS 预设（vcpkg 工具链 + Ninja）
├─ src/
│  ├─ core/                  # 零依赖纯算法层（头文件 + 少量 .cpp）
│  │  ├─ Vec3.h              # 三维向量及运算
│  │  ├─ AABB.h              # 轴对齐包围盒（overlaps / overlapsYZ）
│  │  ├─ Body.h              # 刚体：位置/速度/半尺寸
│  │  ├─ Pair.h              # 配对（规范化 a<b，可比较）
│  │  ├─ BroadPhase.h        # 宽相位抽象接口（可插拔算法）
│  │  ├─ BruteForce.h/.cpp   # O(n²) 暴力全配对（对照基准）
│  │  └─ SweepAndPrune.h/.cpp# x 轴排序 + 扫描 + 活跃集
│  ├─ sim/
│  │  └─ World.h/.cpp        # 积分、边界反弹、宽相位计时
│  └─ app/
│     └─ main.cpp            # raylib 3D 可视化 + 交互
├─ tests/test_sap.cpp        # SAP 与暴力法等价性测试
└─ bench/bench_sap.cpp       # 性能对比基准
```

### 分层设计

1. **core（纯算法）**：不依赖任何图形库，`BroadPhase::computePairs(bodies, outPairs)` 是唯一入口。新增算法只需继承 `BroadPhase`。
2. **sim（仿真）**：`World` 负责积分（`position += velocity * dt`）与边界反弹，并把物体交给宽相位，统计耗时。
3. **app（可视化）**：raylib 绘制场景，`main` 中可实时切换算法、相机、显示端点。
4. **tests / bench**：独立可执行文件，验证正确性与性能。

### SAP 算法要点

实现在 `src/core/SweepAndPrune.cpp`：

1. 为每个物体在 **x 轴** 生成两个端点（`min.x` 为开、`max.x` 为闭）。
2. 按端点值排序；**值相等时“开”优先于“闭”**，保证恰好贴边的两个盒子算作相交（`SweepAndPrune.cpp:28`）。
3. 从左到右扫描，维护一个“活跃集”：
   - 遇到 `min` 端点：该物体与活跃集里所有物体在 x 上重叠，只需再测 **y 与 z**（`overlapsYZ`）即可判定整盒相交，命中则输出配对；随后加入活跃集。
   - 遇到 `max` 端点：从活跃集移除该物体。
4. 复杂度：排序 O(n log n)，扫描的候选对数量取决于场景密度，通常远小于 O(n²)。

> 2D 版本同样适用此逻辑（只需测 y）；本工程直接采用 3D，测 y+z。

## 构建

依赖：CMake ≥ 3.20、一个 C++17 编译器（MSVC / GCC / Clang）、raylib（通过 vcpkg）。

### 方式一：CMakePresets（CLion / VS，推荐）

工程自带 `CMakePresets.json`，CLion 或 VS 打开项目后选择 **vcpkg-debug** 预设即可，工具链已内置 `D:/app/vcpkg`（如你的 vcpkg 路径不同，改 `CMakePresets.json`）。

### 方式二：手动命令行

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=D:/app/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

> 若未显式指定工具链，`CMakeLists.txt` 会自动从 `VCPKG_ROOT`、`D:/app/vcpkg` 等路径探测 vcpkg。
> 只构建核心/测试/基准而不需要可视化时，加 `-DPHYS_BUILD_APP=OFF`。

## 运行

```powershell
build\Release\phys_app.exe      # 3D 可视化
build\Release\phys_tests.exe    # 单元测试
build\Release\phys_bench.exe    # 性能基准
```

### 可视化操作

| 按键 | 作用 |
| --- | --- |
| `Space` | 暂停 / 继续 |
| `A` | 切换算法（SAP ↔ BruteForce） |
| `C` | 切换相机（透视轨道 ↔ 俯视正交） |
| `E` | 显示 / 隐藏 x 轴端点扫描条 |
| `R` | 重新随机生成物体 |

- 透视模式：鼠标拖拽旋转，滚轮缩放，右键平移。
- 俯视模式：等价于 2D 观察视角，便于看清 x 轴排序扫描。
- 红色盒子 = 当前参与重叠的物体；红细线 = SAP 输出的配对。

## 测试与基准

- `phys_tests`：SAP 与 BruteForce 在固定场景 + 40 组随机种子下的结果完全一致，并校验配对无重复、序号规范（`a < b`）。关键边界：贴边相交、嵌套、重合、以及「x、y 重叠但 z 分离 → 不配对」。
- `phys_bench`：输出两种算法在不同物体数量下的平均耗时与加速比。物体越多、分布越稀疏，SAP 优势越明显。

也可用 CTest 运行：

```powershell
ctest --test-dir build -C Release --output-on-failure
```

## 学习路线（建议）

1. 读 `src/sim/World.cpp` 的 `step`，理解「积分」与「碰撞检测」解耦。
2. 读 `src/core/SweepAndPrune.cpp`，对照可视化按 `E` 观察端点如何被排序扫描。
3. 在 `main` 里按 `A` 切换算法，对比 SAP 与暴力法的 `broadphase` 耗时与配对结果。
4. 进阶练习（框架已留好 `BroadPhase` 接口）：
   - 实现 **增量 SAP**：用插入排序利用帧间连贯性，替换当前每帧全量 `std::sort`。
   - 实现 **双轴 SAP**：同时维护 x、y 两个有序列表，只在两轴都重叠时才做完整检测。
   - 接入 **窄相位**（SAT/GJK）与冲量求解，把 `Pair` 变成真实碰撞响应。
