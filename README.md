# 智能生产排产系统 | Intelligent Production Scheduling System

这是一个基于遗传算法（Genetic Algorithm, GA）的智能生产排产优化系统。项目旨在解决具有多种复杂约束的生产调度问题，核心目标是在满足所有约束条件的前提下，**最小化总生产周期**。

This is an intelligent production scheduling system based on the Genetic Algorithm (GA). The project aims to solve complex scheduling problems with multiple constraints, with the primary goal of **minimizing the total production duration** while satisfying all operational rules.

---

## ✨ 项目功能 (Features)

- **核心优化算法**: 采用遗传算法，擅长在庞大的搜索空间中寻找高质量的近似最优解。
- **复杂约束处理**: 能够处理每日产能上限、生产优先级、特殊工艺要求、砖型多样性、偶数生产等多种复杂约束。
- **模块化代码设计**: C++ 核心代码结构清晰，分为数据结构、文件处理、算法核心和主控模块，易于维护和扩展。
- **参数化配置**: 所有关键参数（如种群大小、变异率、约束惩罚权重）均在 `Constants.h` 中集中管理，方便快速调整和优化。
- **自动化结果可视化**: 配备 Python 脚本，可自动读取 C++ 程序的输出结果，并生成以下可视化图表：
    1.  适应度收敛曲线
    2.  生产天数收敛曲线
    3.  生产计划热图
- **清晰的结果输出**: 最终生成详细的 `production_plan.csv` 排产表和 `progress_log.csv` 迭代过程记录。

---

## 📂 项目结构 (Project Structure)

```
.
├── results/                # 输出目录，存放所有结果文件
│   ├── production_plan.csv
│   ├── progress_log.csv
│   └── (*.png) ...
├── Constants.h             # 全局常量与参数配置文件
├── DataStructures.h        # 核心数据结构定义 (砖块, 生产计划个体)
├── FileUtils.h             # 文件读写工具模块
├── GeneticAlgorithm.h      # 遗传算法核心逻辑模块
├── main.cpp                # 程序主入口与总控制器
├── visualize.py            # Python 数据可视化脚本
├── data_input_converted.csv # 生产需求输入数据
└── README.md               # 本文档
```

### 文件功能简介

- **`main.cpp`**: 程序的入口，负责组织整个优化流程，包括数据读取、算法初始化、迭代循环和结果输出。
- **`GeneticAlgorithm.h`**: 项目的“大脑”，实现了遗传算法的所有核心操作，如适应度计算、选择、交叉和变异。
- **`DataStructures.h`**: 定义了项目的数据蓝图，如 `Brick`（砖块属性）和 `Individual`（单个排产方案）。
- **`FileUtils.h`**: 封装了所有与文件系统交互的操作，负责读取输入CSV和写入结果CSV。
- **`Constants.h`**: 项目的“控制面板”，集中定义了所有可调参数，使得调整算法行为无需修改核心代码。
- **`visualize.py`**: 用于对优化结果进行可视化分析。它会读取 `results` 目录下的CSV文件，并生成直观的图表。
- **`data_input_converted.csv`**: 定义了所有需要生产的砖块及其属性（需求量、重量、优先级等）的输入文件。

---

## 🚀 使用方法 (How to Use)

### 1. 环境准备 (Prerequisites)

- **C++ 环境**:
    - 一个支持 C++17 标准的编译器 (例如: g++, Clang, MSVC)。推荐使用 g++。
    - （推荐）VS Code with C/C++ Extension Pack for a better experience.
- **Python 环境**:
    - Python 3.8+ (本项目在 3.11 版本下开发测试)
    - 安装必要的 Python 库:
      ```bash
      pip install pandas matplotlib seaborn
      ```

### 2. 运行流程 (Execution Steps)

#### **第一步：运行 C++ 优化程序**

1.  **准备输入数据**: 确保 `data_input_converted.csv` 文件位于项目根目录，并包含正确的生产需求数据。
2.  **编译**: 打开终端，使用以下命令编译 C++ 代码：
    ```bash
    g++ main.cpp -o scheduler -std=c++17 -O3
    ```
    *   `-o scheduler` 指定输出的可执行文件名为 `scheduler`。
    *   `-std=c++17` 指定使用 C++17 标准（`<filesystem>` 需要）。
    *   `-O3` 是一个优化选项，可以提升运行速度。

3.  **运行**: 执行编译好的程序：
    ```bash
    ./scheduler
    ```
    程序将开始运行遗传算法。你会在终端看到每一代的优化进度。运行结束后，会自动在项目根目录下创建一个 `results` 文件夹，并存入 `production_plan.csv` 和 `progress_log.csv` 两个文件。

#### **第二步：运行 Python 可视化脚本**

1.  **执行脚本**: 待 C++ 程序运行完毕后，在终端中运行 Python 脚本：
    ```bash
    python visualize.py
    ```
2.  **查看结果**: 脚本会自动读取 `results` 文件夹中的数据，并生成三张分析图表（.png格式）保存在同一文件夹下。

---

## 📊 预期输出 (Expected Output)

成功运行后，`results` 文件夹将包含以下文件：

- **`production_plan.csv`**: 最终的、最优的生产排产计划表。
- **`progress_log.csv`**: 记录了遗传算法每一次迭代的最佳适应度和实际天数，用于分析收敛过程。
- **`1_fitness_convergence.png`**: 适应度随迭代次数变化的曲线图，展示了算法的收敛情况。
- **`2_days_convergence.png`**: 实际生产天数随迭代次数变化的曲线图，直观展示生产周期的优化过程。
- **`3_production_heatmap.png`**: 生产计划的二维热图，清晰地展示了在哪个日期生产了哪种砖块以及数量的多少。

---

## 💡 核心算法思路 (Core Algorithm)

本项目的解决方案基于遗传算法，其基本思想模拟了生物进化过程：

- **个体 (Individual)**: 代表一个完整的生产计划方案。
- **适应度函数 (Fitness Function)**: 评价一个生产计划优劣的指标。其计算公式为 `Fitness = 实际生产天数 + Σ(违反约束的惩罚值)`。适应度越低，代表方案越优。
- **进化过程**:
    1.  **选择 (Selection)**: 采用“锦标赛选择”，优先选择适应度高的个体进入下一代。
    2.  **交叉 (Crossover)**: 交换两个父代个体的部分计划，产生新的子代，以期结合两者的优点。
    3.  **变异 (Mutation)**: 对个体的计划进行微小的随机扰动，为种群引入多样性，防止算法过早陷入局部最优。

通过成千上万次的迭代，种群会不断进化，最终收敛到一个满足所有约束且生产周期接近最短的优秀解决方案。

---

