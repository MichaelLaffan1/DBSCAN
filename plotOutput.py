import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv("dbscan/results.txt", header=None, delim_whitespace=True)
try:
    df.columns = ['x', 'y', 'cluster_id', 'thread_id']
    sns.set_style('darkgrid')
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    sns.scatterplot(data=df, x='x', y='y', hue='cluster_id', palette='tab10', marker='o', ax=axes[0])
    axes[0].set_title('Scatterplot by Cluster ID')

    value_counts = df['cluster_id'].value_counts()

    for value, count in value_counts.items():
        print(f"Cluster: {value}, Count: {count}")

    sns.scatterplot(data=df, x='x', y='y', hue='thread_id', palette='tab10', marker='o', ax=axes[1])
    axes[1].set_title('Scatterplot by Thread ID')

    plt.tight_layout()
    plt.show()

except:
    df.columns = ['x', 'y', 'cluster_id']
    sns.set_style('darkgrid')
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))

    sns.scatterplot(data=df, x='x', y='y', hue='cluster_id', palette='tab10', marker='o', ax=axes[0])
    axes[0].set_title('Scatterplot by Cluster ID')

    plt.tight_layout()
    plt.show()
