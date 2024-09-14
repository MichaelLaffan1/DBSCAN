import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv("points.txt", header=None)
df.columns = ['x', 'y', 'cluster_id']
sns.set_style('darkgrid')
sns.scatterplot(data=df, x='x', y='y', hue='cluster_id', palette='tab10', marker='o')
plt.show()
