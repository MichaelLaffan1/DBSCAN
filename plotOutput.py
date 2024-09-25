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

    # count of each unique ids
    value_counts = df['cluster_id'].value_counts()

    num_unique_clusters = value_counts.shape[0]

    non_zero_clusters = value_counts.drop(0)  # exclude cluster 0
    cluster_with_max_count = non_zero_clusters.idxmax()  # cluster_id w/ the highest count
    max_count = non_zero_clusters.max()  # count of the cluster w/ max count

    cluster_0_count = value_counts.get(0, 0)  # ount of cluster 0 (or 0 if not present)

    print(f"unique cluster_ids: {num_unique_clusters-1}")
    print(f"largest cluster count: {cluster_with_max_count}, Count: {max_count}")
    print(f"Noise count: {cluster_0_count}")

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
