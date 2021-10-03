import numpy as np, pandas as pd, os
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_error
from data_loader import save_sample_data
from features import build_features, get_feature_columns

def train_test_split_temporal(df, ratio=0.2):
    n = int(len(df) * (1 - ratio))
    return df.iloc[:n].copy(), df.iloc[n:].copy()

if __name__ == "__main__":
    if not os.path.exists("data/aqi_weather.csv"): save_sample_data()
    df = pd.read_csv("data/aqi_weather.csv", parse_dates=['date'])
    df = build_features(df); cols = get_feature_columns(df)
    train, test = train_test_split_temporal(df)
    m = RandomForestRegressor(n_estimators=50, max_depth=8, random_state=42)
    m.fit(train[cols], train['aqi'])
    print(f"MAE: {mean_absolute_error(test['aqi'], m.predict(test[cols])):.1f}")
