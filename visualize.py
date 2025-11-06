import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os
import shutil
import warnings

warnings.filterwarnings("ignore")
# --- 配置 ---
# 设置matplotlib支持中文显示
plt.rcParams["font.sans-serif"] = ["WenQuanYi Micro Hei"]  # Linux
plt.rcParams["axes.unicode_minus"] = False  # 解决负号'-'显示为方块的问题

# 定义输入和输出路径
RESULTS_DIR = "results"
PLAN_CSV = os.path.join(RESULTS_DIR, "production_plan.csv")
PROGRESS_CSV = os.path.join(RESULTS_DIR, "progress_log.csv")


def plot_fitness_convergence(df):
    """绘制适应度随迭代次数的变化曲线"""
    print("Generating fitness convergence plot...")
    plt.figure(figsize=(12, 7))
    plt.plot(df["Generation"], df["BestFitness"], marker=".", linestyle="-", color="b")
    plt.title("适应度收敛曲线 (Fitness Convergence)", fontsize=16)
    plt.xlabel("迭代次数 (Generation)", fontsize=12)
    plt.ylabel("最佳适应度 (Best Fitness Score)", fontsize=12)
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.tight_layout()
    plt.savefig(os.path.join(RESULTS_DIR, "1_fitness_convergence.png"), dpi=300)
    plt.close()
    print("... Done.")


def plot_days_vs_fitness(df):
    """绘制实际天数随适应度的变化"""
    print("Generating days vs. fitness plot...")
    plt.figure(figsize=(12, 7))
    # 使用散点图更合适，因为适应度可能在同一天数下有波动
    plt.scatter(df["Generation"], df["ActualDays"], alpha=0.6, color="g")
    plt.title("实际天数收敛曲线 (Actual Days)", fontsize=16)
    plt.xlabel("迭代次数 (Generation)", fontsize=12)
    plt.ylabel("实际生产天数 (Actual Days)", fontsize=12)
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)
    plt.tight_layout()
    plt.savefig(os.path.join(RESULTS_DIR, "2_days_convergence.png"), dpi=300)
    plt.close()
    print("... Done.")


def plot_production_heatmap(filepath):
    """绘制生产计划的热图"""
    print("Generating production plan heatmap...")
    # 读取生产计划，可能需要处理编码问题
    try:
        df = pd.read_csv(filepath, encoding="utf-8-sig")
    except Exception as e:
        print(f"Error reading {filepath} with utf-8-sig, trying gbk... Error: {e}")
        df = pd.read_csv(filepath, encoding="gbk")

    # 将'砖型'列设置为索引
    df.set_index("砖型", inplace=True)

    # 提取生产数据部分（从第10列开始，索引为9）
    # C++输出格式为 "序号,砖型,砖数,重量,,,,,,," 后接日期
    production_data = df.iloc[:, 9:].copy()

    # 清理列名中的多余逗号
    production_data.columns = [str(col).strip(",") for col in production_data.columns]

    # 将所有非数值（空值）填充为0，并转换为浮点数
    production_data.fillna(0, inplace=True)
    production_data = production_data.astype(float)

    # 过滤掉全为0的行和列，使热图更紧凑
    production_data = production_data.loc[(production_data != 0).any(axis=1)]
    production_data = production_data.loc[:, (production_data != 0).any(axis=0)]

    if production_data.empty:
        print("Warning: No production data found to plot heatmap.")
        return

    # 绘制热图
    plt.figure(figsize=(20, 12))
    ax = sns.heatmap(
        production_data,
        cmap="viridis",  # 使用视觉效果好的色谱
        linewidths=0.5,
        annot=False,  # 如果格子太多，不建议显示数字
        cbar_kws={"label": "生产数量 (Production Quantity)"},
    )
    ax.set_title("生产计划热图 (Production Plan Heatmap)", fontsize=20, pad=20)
    ax.set_xlabel("生产日期 (Date)", fontsize=15)
    ax.set_ylabel("砖型 (Brick Type)", fontsize=15)

    # 优化X轴标签显示，防止重叠
    num_cols = len(production_data.columns)
    tick_spacing = max(1, num_cols // 20)  # 每隔N个显示一个标签
    ax.set_xticks(ax.get_xticks()[::tick_spacing])
    plt.xticks(rotation=45, ha="right")

    plt.tight_layout()
    plt.savefig(os.path.join(RESULTS_DIR, "3_production_heatmap.png"), dpi=300)
    plt.close()
    print("... Done.")


def main():
    """主执行函数"""
    if not os.path.exists(RESULTS_DIR):
        print(
            f"Error: Directory '{RESULTS_DIR}' not found. Please run the C++ program first."
        )
        return

    if not os.path.exists(PROGRESS_CSV) or not os.path.exists(PLAN_CSV):
        print("Error: Required CSV files not found in 'results' directory.")
        return

    # 读取迭代数据
    progress_df = pd.read_csv(PROGRESS_CSV)

    # 生成图表
    plot_fitness_convergence(progress_df)
    plot_days_vs_fitness(progress_df)
    plot_production_heatmap(PLAN_CSV)

    print("\nAll plots have been generated and saved in the 'results' directory.")


if __name__ == "__main__":
    main()
