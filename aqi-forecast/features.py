import pandas as pd
import numpy as np

def build_features(df, target='aqi'):
    df = df.copy()
    df['date'] = pd.to_datetime(df['date'])
    df['month'] = df['date'].dt.month
    df['weekday'] = df['date'].dt.weekday
    df['month_sin'] = np.sin(2 * np.pi * df['month'] / 12)
    df['month_cos'] = np.cos(2 * np.pi * df['month'] / 12)
    for lag in [1, 2, 3, 7]:
        df[f'{target}_lag{lag}'] = df[target].shift(lag)
    for w in [3, 7, 14]:
        df[f'{target}_roll_mean{w}'] = df[target].rolling(w).mean()
        df[f'{target}_roll_std{w}'] = df[target].rolling(w).std()
    df['north_wind'] = np.cos(np.radians(df['wind_direction']))
    df['stagnation'] = (1 / (df['wind_speed'] + 0.5)) * df['humidity']
    df['rain_flag'] = (df['rainfall'] > 1.0).astype(int)
    df = df.dropna().reset_index(drop=True)
    return df

def get_feature_columns(df):
    return [c for c in df.columns if c not in ['date', 'aqi', 'day_of_year']]
