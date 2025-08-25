// TLE JSON data types for orbital debris tracking system
export interface TLEJsonRecord {
  id: string;                    // unique sat/debris id
  name?: string;                 // optional common name
  epoch: string;                 // ISO timestamp
  stateECI?: Array<{ t: string; x: number; y: number; z: number; vx: number; vy: number; vz: number }>; // optional trajectory samples
  tle?: { line1: string; line2: string }; // optional original TLE
  kind: "satellite" | "debris"; // body type
}

export interface TLEDataset { 
  records: TLEJsonRecord[]; 
  timestamp_minutes?: number; // optional metadata
}

// Coordinates JSON structure (from C++ backend)
export interface CoordinatesJson {
  timestamp_minutes: number;
  satellites: Array<{
    name: string;
    position_km: [number, number, number];
    velocity_km_s: [number, number, number];
  }>;
}

// Conjunctions JSON structure (from C++ backend)
export interface ConjunctionsJson {
  timestamp_minutes: number;
  conjunction_pairs: Array<{
    satellite_a: string;
    satellite_b: string;
    time_minutes: number;
    distance_km: number;
    relative_velocity_km_s: number;
  }>;
}

// Runtime validation helpers
export function validateTLEDataset(data: any): data is TLEDataset {
  if (!data || typeof data !== 'object') return false;
  if (!Array.isArray(data.records)) return false;
  
  return data.records.every((record: any) => 
    typeof record.id === 'string' &&
    typeof record.epoch === 'string' &&
    (record.kind === 'satellite' || record.kind === 'debris')
  );
}

export function validateCoordinatesJson(data: any): data is CoordinatesJson {
  if (!data || typeof data !== 'object') return false;
  if (typeof data.timestamp_minutes !== 'number') return false;
  if (!Array.isArray(data.satellites)) return false;
  
  return data.satellites.every((sat: any) =>
    typeof sat.name === 'string' &&
    Array.isArray(sat.position_km) && sat.position_km.length === 3 &&
    Array.isArray(sat.velocity_km_s) && sat.velocity_km_s.length === 3
  );
}

export function validateConjunctionsJson(data: any): data is ConjunctionsJson {
  if (!data || typeof data !== 'object') return false;
  if (typeof data.timestamp_minutes !== 'number') return false;
  if (!Array.isArray(data.conjunction_pairs)) return false;
  
  return data.conjunction_pairs.every((pair: any) =>
    typeof pair.satellite_a === 'string' &&
    typeof pair.satellite_b === 'string' &&
    typeof pair.time_minutes === 'number' &&
    typeof pair.distance_km === 'number'
  );
}
