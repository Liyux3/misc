import numpy as np, pandas as pd
from sklearn.cluster import DBSCAN

def cluster_earthquakes(df, eps_km=300, min_samples=10):
    df = df.copy()
    df['cluster'] = DBSCAN(eps=eps_km/111.0, min_samples=min_samples).fit_predict(df[['latitude','longitude']].values)
    nc = len(set(df['cluster'])) - (1 if -1 in df['cluster'].values else 0)
    print(f"{nc} clusters, {(df['cluster']==-1).sum()} noise"); return df

def get_cluster_stats(df):
    s = df[df['cluster']>=0].groupby('cluster').agg({'latitude':'mean','longitude':'mean','magnitude':['mean','max','count'],'depth':'mean'}).round(2)
    s.columns = ['lat','lon','avg_mag','max_mag','count','avg_depth']
    return s.sort_values('count', ascending=False)

def haversine_km(lat1, lon1, lat2, lon2):
    R=6371; dlat=np.radians(lat2-lat1); dlon=np.radians(lon2-lon1)
    a=np.sin(dlat/2)**2+np.cos(np.radians(lat1))*np.cos(np.radians(lat2))*np.sin(dlon/2)**2
    return R*2*np.arctan2(np.sqrt(a), np.sqrt(1-a))
