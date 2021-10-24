import pandas as pd, numpy as np, matplotlib.pyplot as plt, matplotlib.dates as mdates, os
from data_loader import save_sample_data
from features import build_features, get_feature_columns
from model import train_test_split_temporal, train_model, evaluate_model

def aqi_color(v):
    if v <= 50: return '#00E400'
    elif v <= 100: return '#FFFF00'
    elif v <= 150: return '#FF7E00'
    elif v <= 200: return '#FF0000'
    return '#7E0023'

def plot_dashboard(results, raw, metrics, path="output"):
    os.makedirs(path, exist_ok=True)
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle('AQI Forecast Dashboard', fontsize=14, fontweight='bold', y=0.98)
    ax = axes[0, 0]; last = results.tail(60); d = pd.to_datetime(last['date'])
    ax.plot(d, last['aqi'], 'ko-', ms=3, lw=1, label='Actual')
    ax.plot(d, last['predicted'], 'b-', lw=1.5, label='Predicted')
    ax.fill_between(d, last['lower_80'], last['upper_80'], alpha=0.2, color='blue', label='80% CI')
    ax.set_title(f'Forecast vs Actual (MAE={metrics["mae"]:.1f})')
    ax.legend(fontsize=8); ax.set_ylabel('AQI'); ax.xaxis.set_major_formatter(mdates.DateFormatter('%b'))
    ax = axes[0, 1]; val = round(raw['aqi'].iloc[-1])
    ax.add_patch(plt.Circle((0.5, 0.5), 0.35, color=aqi_color(val), alpha=0.8))
    ax.text(0.5, 0.5, str(val), ha='center', va='center', fontsize=48, fontweight='bold', color='white')
    ax.text(0.5, 0.12, 'Current AQI', ha='center', fontsize=12)
    ax.set_xlim(0,1); ax.set_ylim(0,1); ax.set_aspect('equal'); ax.axis('off')
    ax = axes[1, 0]; rdf = raw.copy(); rdf['month'] = pd.to_datetime(rdf['date']).dt.month
    md = [rdf[rdf['month']==m]['aqi'].values for m in range(1,13)]
    bp = ax.boxplot(md, tick_labels=['J','F','M','A','M','J','J','A','S','O','N','D'], patch_artist=True)
    cm = ['#FFD700']*2 + ['#90EE90']*6 + ['#FFD700']*4
    for p, c in zip(bp['boxes'], cm): p.set_facecolor(c)
    ax.set_title('Monthly AQI'); ax.set_ylabel('AQI')
    ax = axes[1, 1]
    sc = ax.scatter(raw['wind_speed'], raw['aqi'], c=raw['temperature'], cmap='RdYlBu_r', alpha=0.5, s=15)
    plt.colorbar(sc, ax=ax, label='Temp (C)'); ax.set_xlabel('Wind Speed (m/s)'); ax.set_ylabel('AQI')
    ax.set_title('Wind Speed vs AQI')
    plt.tight_layout(); plt.savefig(f'{path}/dashboard.png', dpi=150, bbox_inches='tight'); plt.close()

if __name__ == "__main__":
    if not os.path.exists("data/aqi_weather.csv"): save_sample_data()
    df = pd.read_csv("data/aqi_weather.csv", parse_dates=['date'])
    feat = build_features(df); cols = get_feature_columns(feat)
    train, test = train_test_split_temporal(feat)
    model = train_model(train, cols)
    r, m = evaluate_model(model, test, cols)
    plot_dashboard(r, df, m); print("saved")
