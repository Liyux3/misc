import pandas as pd
import numpy as np

def build_features(df, target='aqi'):
    df = df.copy()
    df['date'] = pd.to_datetime(df['date'])
    df['month'] = df['date'].dt.month
    df['month_sin'] = np.sin(2 * np.pi * df['month'] / 12)
    df['month_cos'] = np.cos(2 * np.pi * df['month'] / 12)
    for lag in [1, 2, 3, 7]:
        df[f'{target}_lag{lag}'] = df[target].shift(lag)
    for w in [3, 7]:
        df[f'{target}_roll_mean{w}'] = df[target].rolling(w).mean()
    df = df.dropna().reset_index(drop=True)
    return df

def get_feature_columns(df):
    return [c for c in df.columns if c not in ['date', 'aqi']]
