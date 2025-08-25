import { describe, it, expect, beforeEach, vi } from 'vitest';
import { jsonDataAdapter } from '../data/jsonSource';
import { validateCoordinatesJson, validateConjunctionsJson } from '../types/tle';

// Mock fetch for testing
globalThis.fetch = vi.fn();

describe('JSON Data Adapter Smoke Tests', () => {
  beforeEach(() => {
    vi.clearAllMocks();
  });

  it('should load and validate coordinates JSON', async () => {
    const mockCoordinates = {
      timestamp_minutes: 1440,
      satellites: [
        {
          name: "TEST_SAT",
          position_km: [7000, 0, 0],
          velocity_km_s: [0, 7.5, 0]
        }
      ]
    };

    (fetch as any).mockResolvedValueOnce({
      ok: true,
      json: () => Promise.resolve(mockCoordinates)
    });

    const result = await jsonDataAdapter.loadCoordinatesFromUrl('/test-coordinates.json');
    
    expect(result).toEqual(mockCoordinates);
    expect(validateCoordinatesJson(result)).toBe(true);
  });

  it('should load and validate conjunctions JSON', async () => {
    const mockConjunctions = {
      timestamp_minutes: 1440,
      conjunction_pairs: [
        {
          satellite_a: "SAT_A",
          satellite_b: "SAT_B", 
          time_minutes: 720,
          distance_km: 5.0,
          relative_velocity_km_s: 10.0
        }
      ]
    };

    (fetch as any).mockResolvedValueOnce({
      ok: true,
      json: () => Promise.resolve(mockConjunctions)
    });

    const result = await jsonDataAdapter.loadConjunctionsFromUrl('/test-conjunctions.json');
    
    expect(result).toEqual(mockConjunctions);
    expect(validateConjunctionsJson(result)).toBe(true);
  });

  it('should convert coordinates to tracks format', async () => {
    const coordinates = {
      timestamp_minutes: 1440,
      satellites: [
        {
          name: "TEST_SAT",
          position_km: [7000, 0, 0] as [number, number, number],
          velocity_km_s: [0, 7.5, 0] as [number, number, number]
        }
      ]
    };

    const tracks = jsonDataAdapter.coordinatesToTracks(coordinates);
    
    expect(tracks).toHaveLength(1);
    expect(tracks[0]).toMatchObject({
      id: "satellite_0",
      kind: "satellite",
      states: expect.arrayContaining([
        expect.objectContaining({
          t: expect.any(Number),
          r: [7000, 0, 0],
          v: [0, 7.5, 0]
        })
      ])
    });
  });

  it('should convert conjunctions to analysis results', async () => {
    const conjunctions = {
      timestamp_minutes: 1440,
      conjunction_pairs: [
        {
          satellite_a: "SAT_A",
          satellite_b: "SAT_B",
          time_minutes: 720,
          distance_km: 2.0, // High risk distance
          relative_velocity_km_s: 10.0
        }
      ]
    };

    const result = jsonDataAdapter.conjunctionsToAnalysisResult(conjunctions);
    
    expect(result.encounters).toHaveLength(1);
    expect(result.encounters[0]).toMatchObject({
      aId: "SAT_A",
      bId: "SAT_B",
      missMeters: 2000, // converted to meters
      relSpeedMps: 10000, // converted to m/s
      severity: "Medium",
    });
  });

  it('should handle TLE parsing for compatibility', () => {
    const tleText = `ISS (ZARYA)
1 25544U 98067A   23001.00000000  .00000000  00000-0  00000-0 0  0000
2 25544  51.6400   0.0000 0000000   0.0000   0.0000 15.50000000000000`;

    const result = jsonDataAdapter.parseTle(tleText, 'satellite');
    
    expect(result).toHaveLength(1);
    expect(result[0]).toMatchObject({
      name: "ISS (ZARYA)",
      line1: expect.stringContaining("1 25544U"),
      line2: expect.stringContaining("2 25544"),
      kind: "satellite"
    });
  });

  it('should reject invalid JSON schemas', async () => {
    const invalidCoordinates = {
      invalid: "data"
    };

    (fetch as any).mockResolvedValueOnce({
      ok: true,
      json: () => Promise.resolve(invalidCoordinates)
    });

    await expect(
      jsonDataAdapter.loadCoordinatesFromUrl('/invalid.json')
    ).rejects.toThrow('Invalid coordinates JSON schema');
  });
});
