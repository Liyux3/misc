import pandas as pd
import numpy as np
import os

DATA_DIR = "data"

def generate_sample_data(n_days=365):
    np.random.seed(42)
    dates = pd.date_range('2020-01-01', periods=n_days, freq='D')
    doy = dates.dayofyear
    seasonal = 80 + 40 * np.cos(2 * np.pi * (doy - 30) / 365)
    aqi = np.clip(seasonal + np.random.normal(0, 20, n_days), 20, 300).round(0).astype(int)
    temperature = 14 + 12 * np.sin(2 * np.pi * (doy - 100) / 365) + np.random.normal(0, 3, n_days)
    humidity = 65 + 15 * np.sin(2 * np.pi * (doy - 200) / 365) + np.random.normal(0, 10, n_days)
    wind_speed = np.abs(3.5 + np.random.normal(0, 2, n_days))
    wind_dir = np.where((dates.month >= 10) | (dates.month <= 3),
        np.random.normal(330, 40, n_days) % 360, np.random.normal(150, 50, n_days) % 360)
    rainfall = np.where((dates.month >= 6) & (dates.month <= 9),
        np.random.exponential(8, n_days), np.random.exponential(1.5, n_days))
    return pd.DataFrame({
        'date': dates, 'aqi': aqi, 'temperature': np.round(temperature, 1),
        'humidity': np.round(humidity, 1), 'wind_speed': np.round(wind_speed, 1),
        'wind_direction': np.round(wind_dir, 0), 'rainfall': np.round(rainfall, 1),
    })

def save_sample_data():
    os.makedirs(DATA_DIR, exist_ok=True)
    df = generate_sample_data(730)
    df.to_csv(os.path.join(DATA_DIR, "aqi_weather.csv"), index=False)
    print(f"saved {len(df)} rows"); return df

if __name__ == "__main__":
    df = save_sample_data(); print(df.head())
