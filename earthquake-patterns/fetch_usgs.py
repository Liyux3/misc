import requests, pandas as pd, numpy as np, os

USGS_API = "https://earthquake.usgs.gov/fdsnws/event/1/query"
DATA_DIR = "data"

def fetch_earthquakes(start, end, min_mag=4.0):
    params = {'format': 'geojson', 'starttime': start, 'endtime': end, 'minmagnitude': min_mag, 'limit': 10000}
    resp = requests.get(USGS_API, params=params, timeout=30)
    resp.raise_for_status(); return resp.json()

def generate_sample_data(n=2000):
    np.random.seed(42)
    zones = [
        {'name': 'Ring of Fire - Japan', 'lat': 36, 'lon': 140, 'w': 0.25},
        {'name': 'Ring of Fire - Chile', 'lat': -33, 'lon': -71, 'w': 0.15},
        {'name': 'Ring of Fire - Alaska', 'lat': 61, 'lon': -150, 'w': 0.12},
        {'name': 'Ring of Fire - Indonesia', 'lat': -5, 'lon': 120, 'w': 0.20},
        {'name': 'Mediterranean', 'lat': 37, 'lon': 22, 'w': 0.08},
        {'name': 'Himalayan', 'lat': 28, 'lon': 85, 'w': 0.10},
        {'name': 'Mid-Atlantic Ridge', 'lat': 15, 'lon': -45, 'w': 0.10},
    ]
    records = []; dates = pd.date_range('2019-01-01', '2021-12-31', periods=n)
    for i in range(n):
        w = [z['w'] for z in zones]; z = zones[np.random.choice(len(zones), p=w)]
        records.append({'time': dates[i], 'latitude': round(z['lat']+np.random.normal(0,3),4),
            'longitude': round(z['lon']+np.random.normal(0,4),4),
            'depth': round(abs(np.random.exponential(30)),1),
            'magnitude': round(min(4.0+np.random.exponential(0.5),9.0),1),
            'place': z['name'], 'type': 'earthquake'})
    return pd.DataFrame(records)

def save_data():
    os.makedirs(DATA_DIR, exist_ok=True)
    df = generate_sample_data()
    df.to_csv(os.path.join(DATA_DIR, "earthquakes.csv"), index=False)
    return df

if __name__ == "__main__":
    df = save_data(); print(df['place'].value_counts())
