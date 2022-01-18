import numpy as np, pandas as pd

def compute_timeseries(df, freq='W'):
    df = df[df['cluster']>=0].copy(); df['period'] = df['time'].dt.to_period(freq)
    c = df.groupby(['cluster','period']).size().reset_index(name='count')
    c['period_start'] = c['period'].dt.start_time; return c

def detect_anomalies(counts_df, window=8, threshold=2.0):
    results = []
    for cid in counts_df['cluster'].unique():
        cdf = counts_df[counts_df['cluster']==cid].sort_values('period_start').copy()
        if len(cdf) < window: continue
        cdf['roll_mean'] = cdf['count'].rolling(window, min_periods=3).mean()
        cdf['roll_std'] = cdf['count'].rolling(window, min_periods=3).std()
        cdf['z'] = (cdf['count'] - cdf['roll_mean']) / (cdf['roll_std'] + 1e-6)
        cdf['anomaly'] = cdf['z'].abs() > threshold
        results.append(cdf)
    return pd.concat(results, ignore_index=True) if results else pd.DataFrame()

def risk_scores(stats):
    r = stats.copy()
    r['risk'] = (r['count']/r['count'].max()*0.4 + r['max_mag']/r['max_mag'].max()*0.6).round(3)
    return r.sort_values('risk', ascending=False)
