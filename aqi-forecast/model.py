import numpy as np, pandas as pd, os
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
from sklearn.model_selection import TimeSeriesSplit
from data_loader import save_sample_data
from features import build_features, get_feature_columns

def train_test_split_temporal(df, ratio=0.2):
    n = int(len(df) * (1 - ratio))
    return df.iloc[:n].copy(), df.iloc[n:].copy()

def train_model(train_df, cols):
    m = RandomForestRegressor(n_estimators=100, max_depth=10, min_samples_split=5, random_state=42, n_jobs=-1)
    m.fit(train_df[cols].values, train_df['aqi'].values); return m

def evaluate_model(model, test_df, cols):
    X = test_df[cols].values; y = test_df['aqi'].values
    pred = model.predict(X)
    tp = np.array([t.predict(X) for t in model.estimators_])
    metrics = {'mae': mean_absolute_error(y, pred), 'rmse': np.sqrt(mean_squared_error(y, pred)), 'r2': r2_score(y, pred)}
    print(f"MAE: {metrics['mae']:.1f}, RMSE: {metrics['rmse']:.1f}, R2: {metrics['r2']:.3f}")
    r = test_df[['date', 'aqi']].copy()
    r['predicted'] = pred; r['lower_80'] = np.percentile(tp, 10, axis=0); r['upper_80'] = np.percentile(tp, 90, axis=0)
    return r, metrics

def cross_validate(df, cols):
    X, y = df[cols].values, df['aqi'].values
    for fold, (ti, vi) in enumerate(TimeSeriesSplit(n_splits=5).split(X)):
        m = RandomForestRegressor(n_estimators=100, max_depth=10, random_state=42, n_jobs=-1)
        m.fit(X[ti], y[ti])
        print(f"  fold {fold+1}: MAE = {mean_absolute_error(y[vi], m.predict(X[vi])):.1f}")

if __name__ == "__main__":
    if not os.path.exists("data/aqi_weather.csv"): save_sample_data()
    df = pd.read_csv("data/aqi_weather.csv", parse_dates=['date'])
    df = build_features(df); cols = get_feature_columns(df)
    train, test = train_test_split_temporal(df)
    print("cv:"); cross_validate(train, cols)
    model = train_model(train, cols)
    evaluate_model(model, test, cols)
