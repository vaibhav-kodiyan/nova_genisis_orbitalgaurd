#!/usr/bin/env python3
"""
Generate TLE data for 1000 satellites and 3000 debris objects with varied orbital parameters
"""

import random
import math

def generate_tle_entry(name, norad_id, inclination, raan, eccentricity, arg_perigee, mean_anomaly, mean_motion, epoch_year=24, epoch_day=146.0):
    """Generate a TLE entry with given orbital parameters"""
    
    # Line 1 format: 1 NNNNNC NNNNNAAA NNNNN.NNNNNNNN +.NNNNNNNN +NNNNN-N +NNNNN-N N NNNNN
    line1 = f"1 {norad_id:05d}U {epoch_year:02d}001A   {epoch_day:012.8f}  .00000000  00000-0  00000-0 0  {random.randint(1000, 9999):04d}"
    
    # Line 2 format: 2 NNNNN NNN.NNNN NNN.NNNN NNNNNNN NNN.NNNN NNN.NNNN NN.NNNNNNNNNNNNNN
    line2 = f"2 {norad_id:05d} {inclination:8.4f} {raan:8.4f} {int(eccentricity * 10000000):07d} {arg_perigee:8.4f} {mean_anomaly:8.4f} {mean_motion:11.8f} {random.randint(10000, 99999):05d}"
    
    return f"{name:<24}\n{line1}\n{line2}"

def generate_satellites(count=400):
    """Generate varied satellite constellation data with extreme diversity"""
    satellites = []
    
    # Satellite constellation types with much more varied orbital characteristics
    constellations = [
        # Low Earth Orbit - Very Low (ISS-like)
        {"name_prefix": "LEO-VLOW", "count": 100, "inc_range": (0.0, 28.5), "alt_range": (400, 450), "ecc_range": (0.0001, 0.0050)},
        
        # Low Earth Orbit - Low inclination (Starlink-like)
        {"name_prefix": "LEO-LOW", "count": 150, "inc_range": (28.5, 57.0), "alt_range": (500, 700), "ecc_range": (0.0001, 0.0100)},
        
        # Low Earth Orbit - Medium inclination
        {"name_prefix": "LEO-MED", "count": 150, "inc_range": (57.0, 85.5), "alt_range": (700, 1000), "ecc_range": (0.0050, 0.0200)},
        
        # Low Earth Orbit - High inclination (Polar)
        {"name_prefix": "LEO-HIGH", "count": 100, "inc_range": (85.5, 114.0), "alt_range": (800, 1200), "ecc_range": (0.0100, 0.0300)},
        
        # Low Earth Orbit - Retrograde
        {"name_prefix": "LEO-RETRO", "count": 100, "inc_range": (114.0, 142.5), "alt_range": (600, 900), "ecc_range": (0.0050, 0.0250)},
        
        # Medium Earth Orbit - Navigation
        {"name_prefix": "MEO-NAV", "count": 100, "inc_range": (50.0, 65.0), "alt_range": (19000, 24000), "ecc_range": (0.0001, 0.0100)},
        
        # Medium Earth Orbit - Communication
        {"name_prefix": "MEO-COMM", "count": 80, "inc_range": (0.0, 30.0), "alt_range": (8000, 15000), "ecc_range": (0.0100, 0.0500)},
        
        # Highly Elliptical Orbit (Molniya-like)
        {"name_prefix": "HEO-MOL", "count": 60, "inc_range": (60.0, 70.0), "alt_range": (500, 40000), "ecc_range": (0.6000, 0.7500)},
        
        # Geostationary Transfer Orbit
        {"name_prefix": "GTO", "count": 80, "inc_range": (0.0, 28.5), "alt_range": (200, 35786), "ecc_range": (0.7000, 0.7300)},
        
        # Geostationary Orbit
        {"name_prefix": "GEO", "count": 70, "inc_range": (0.0, 5.0), "alt_range": (35786, 35786), "ecc_range": (0.0001, 0.0050)},
    ]
    
    norad_id = 50001
    
    for constellation in constellations:
        for i in range(constellation["count"]):
            # Generate varied orbital parameters within constellation ranges
            inclination = random.uniform(*constellation["inc_range"])
            raan = random.uniform(0, 360)
            
            # Convert altitude to mean motion (approximate)
            altitude = random.uniform(*constellation["alt_range"])
            semi_major_axis = 6371 + altitude  # Earth radius + altitude
            mean_motion = math.sqrt(398600.4418 / (semi_major_axis ** 3)) * 86400 / (2 * math.pi)  # revs/day
            
            eccentricity = random.uniform(*constellation["ecc_range"])
            arg_perigee = random.uniform(0, 360)
            mean_anomaly = random.uniform(0, 360)
            
            name = f"{constellation['name_prefix']}-{i+1:04d}"
            
            tle = generate_tle_entry(name, norad_id, inclination, raan, eccentricity, 
                                   arg_perigee, mean_anomaly, mean_motion)
            satellites.append(tle)
            norad_id += 1
    
    return satellites

def generate_debris(count=600):
    """Generate varied space debris data with extreme orbital diversity"""
    debris = []
    
    # Debris types with much more varied characteristics across all orbital regimes
    debris_types = [
        # Very Low Earth Orbit debris (atmospheric drag region)
        {"name_prefix": "VLEO DEB", "count": 100, "inc_range": (0.0, 180.0), "alt_range": (150, 400), "ecc_range": (0.0001, 0.0500)},
        
        # Low Earth Orbit debris - Equatorial
        {"name_prefix": "LEO-EQ DEB", "count": 150, "inc_range": (0.0, 30.0), "alt_range": (400, 1200), "ecc_range": (0.0001, 0.1500)},
        
        # Low Earth Orbit debris - Mid-inclination
        {"name_prefix": "LEO-MID DEB", "count": 150, "inc_range": (30.0, 60.0), "alt_range": (500, 1500), "ecc_range": (0.0050, 0.2000)},
        
        # Low Earth Orbit debris - High inclination/Polar
        {"name_prefix": "LEO-POL DEB", "count": 100, "inc_range": (60.0, 120.0), "alt_range": (600, 1800), "ecc_range": (0.0100, 0.3000)},
        
        # Low Earth Orbit debris - Retrograde
        {"name_prefix": "LEO-RET DEB", "count": 50, "inc_range": (120.0, 180.0), "alt_range": (400, 1000), "ecc_range": (0.0050, 0.2500)},
        
        # Medium Earth Orbit debris
        {"name_prefix": "MEO DEB", "count": 30, "inc_range": (0.0, 90.0), "alt_range": (2000, 20000), "ecc_range": (0.0100, 0.4000)},
        
        # Highly Elliptical Orbit debris
        {"name_prefix": "HEO DEB", "count": 15, "inc_range": (45.0, 75.0), "alt_range": (500, 50000), "ecc_range": (0.5000, 0.8000)},
        
        # Geostationary Transfer Orbit debris
        {"name_prefix": "GTO DEB", "count": 5, "inc_range": (0.0, 45.0), "alt_range": (200, 35786), "ecc_range": (0.6500, 0.7500)},
    ]
    
    norad_id = 60001
    
    for debris_type in debris_types:
        for i in range(debris_type["count"]):
            # Generate highly varied orbital parameters for debris
            inclination = random.uniform(*debris_type["inc_range"])
            raan = random.uniform(0, 360)
            
            # Convert altitude to mean motion
            altitude = random.uniform(*debris_type["alt_range"])
            semi_major_axis = 6371 + altitude
            mean_motion = math.sqrt(398600.4418 / (semi_major_axis ** 3)) * 86400 / (2 * math.pi)
            
            eccentricity = random.uniform(*debris_type["ecc_range"])
            arg_perigee = random.uniform(0, 360)
            mean_anomaly = random.uniform(0, 360)
            
            name = f"{debris_type['name_prefix']}"
            
            tle = generate_tle_entry(name, norad_id, inclination, raan, eccentricity,
                                   arg_perigee, mean_anomaly, mean_motion)
            debris.append(tle)
            norad_id += 1
    
    return debris

def main():
    print("Generating 1000 satellites with varied orbital parameters...")
    satellites = generate_satellites(1000)
    
    print("Generating 3000 debris objects with diverse orbits...")
    debris = generate_debris(3000)
    
    # Write satellites to file
    with open('/home/vaibhav_kodiyan_/projects/hackathon/data/satellites_1000.tle', 'w') as f:
        f.write('\n'.join(satellites))
    
    # Write debris to file  
    with open('/home/vaibhav_kodiyan_/projects/hackathon/data/debris_3000.tle', 'w') as f:
        f.write('\n'.join(debris))
    
    print(f"Generated {len(satellites)} satellites and {len(debris)} debris objects")
    print("Files created:")
    print("- /home/vaibhav_kodiyan_/projects/hackathon/data/satellites_1000.tle")
    print("- /home/vaibhav_kodiyan_/projects/hackathon/data/debris_3000.tle")

if __name__ == "__main__":
    main()
