import os, pandas as pd
from fetch_usgs import save_data
from spatial_cluster import cluster_earthquakes, get_cluster_stats
from temporal_anomaly import compute_timeseries, detect_anomalies
from globe_viz import plot_map, plot_timeline

if not os.path.exists("data/earthquakes.csv"): save_data()
df = pd.read_csv("data/earthquakes.csv", parse_dates=['time'])
df = cluster_earthquakes(df, eps_km=500, min_samples=15)
print(get_cluster_stats(df))
ts = compute_timeseries(df); adf = detect_anomalies(ts)
print(f"\n{adf['anomaly'].sum() if len(adf) else 0} anomalous weeks")
plot_map(df); plot_timeline(adf); print("done")
