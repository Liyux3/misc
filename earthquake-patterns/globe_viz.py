import numpy as np, pandas as pd, matplotlib.pyplot as plt, os

def plot_map(df, path="output"):
    os.makedirs(path, exist_ok=True)
    fig, ax = plt.subplots(figsize=(16,8))
    noise = df[df['cluster']==-1]
    if len(noise): ax.scatter(noise['longitude'], noise['latitude'], c='lightgray', s=3, alpha=0.3)
    cmap = plt.cm.Set1
    for i, cid in enumerate(sorted(df[df['cluster']>=0]['cluster'].unique())):
        cd = df[df['cluster']==cid]
        ax.scatter(cd['longitude'], cd['latitude'], c=[cmap(i%9)], s=cd['magnitude']**2, alpha=0.5, label=f'{cid} (n={len(cd)})')
    ax.set_xlabel('Longitude'); ax.set_ylabel('Latitude'); ax.set_title('Earthquake Clusters')
    ax.legend(fontsize=7, loc='lower left', ncol=2); ax.set_xlim(-180,180); ax.set_ylim(-90,90); ax.grid(alpha=0.3)
    plt.tight_layout(); plt.savefig(f'{path}/earthquake_map.png', dpi=150); plt.close()

def plot_timeline(adf, path="output"):
    os.makedirs(path, exist_ok=True)
    clusters = sorted(adf['cluster'].unique()); n = min(len(clusters),6)
    fig, axes = plt.subplots(n,1,figsize=(14,3*n),sharex=True)
    if n==1: axes=[axes]
    for i, cid in enumerate(clusters[:n]):
        ax=axes[i]; cd=adf[adf['cluster']==cid].sort_values('period_start')
        ax.bar(cd['period_start'], cd['count'], width=5, color='steelblue', alpha=0.7)
        am=cd[cd['anomaly']]
        if len(am): ax.scatter(am['period_start'], am['count'], c='red', s=50, zorder=5, marker='*')
        ax.set_ylabel(f'C{cid}'); ax.grid(alpha=0.2)
    axes[-1].set_xlabel('Date')
    fig.suptitle('Weekly frequency per cluster (* = anomaly)')
    plt.tight_layout(); plt.savefig(f'{path}/cluster_timeline.png', dpi=150); plt.close()
