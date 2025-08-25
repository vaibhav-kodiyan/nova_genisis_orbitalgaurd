import type { CoordinatesJson, ConjunctionsJson } from "../types/tle";
import { validateCoordinatesJson, validateConjunctionsJson } from "../types/tle";
import { Track, StateECI, AnalysisResult, Encounter, TleEntry, BodyKind } from "../domain/types";

export class JsonDataAdapter {
  private coordinatesCache: CoordinatesJson | null = null;
  private conjunctionsCache: ConjunctionsJson | null = null;

  async loadCoordinatesFromUrl(url: string): Promise<CoordinatesJson> {
    const res = await fetch(url, { cache: "no-store" });
    if (!res.ok) throw new Error(`Failed to fetch ${url}: ${res.status}`);
    const data = await res.json();
    
    if (!validateCoordinatesJson(data)) {
      throw new Error("Invalid coordinates JSON schema");
    }
    
    this.coordinatesCache = data;
    return data;
  }

  async loadConjunctionsFromUrl(url: string): Promise<ConjunctionsJson> {
    const res = await fetch(url, { cache: "no-store" });
    if (!res.ok) throw new Error(`Failed to fetch ${url}: ${res.status}`);
    const data = await res.json();
    
    if (!validateConjunctionsJson(data)) {
      throw new Error("Invalid conjunctions JSON schema");
    }
    
    this.conjunctionsCache = data;
    return data;
  }

  // Load both coordinates and conjunctions from public directory
  async loadAllData(): Promise<{ coordinates: CoordinatesJson; conjunctions: ConjunctionsJson }> {
    const [coordinates, conjunctions] = await Promise.all([
      this.loadCoordinatesFromUrl("/coordinates.json"),
      this.loadConjunctionsFromUrl("/conjunctions.json")
    ]);
    
    return { coordinates, conjunctions };
  }

  // Convert coordinates JSON to Track format for compatibility
  coordinatesToTracks(coordinates: CoordinatesJson): Track[] {
    return coordinates.satellites.map((sat, index) => {
      const state: StateECI = {
        t: coordinates.timestamp_minutes * 60 * 1000, // convert to milliseconds
        r: sat.position_km,
        v: sat.velocity_km_s
      };

      return {
        id: `satellite_${index}`,
        kind: "satellite" as BodyKind,
        states: [state] // Single state for now, can be extended
      };
    });
  }

  // Convert conjunctions JSON to AnalysisResult format
  conjunctionsToAnalysisResult(conjunctions: ConjunctionsJson): AnalysisResult {
    const encounters: Encounter[] = conjunctions.conjunction_pairs.map(pair => {
      const missMeters = pair.distance_km * 1000; // convert to meters
      const relSpeedMps = pair.relative_velocity_km_s * 1000; // convert to m/s
      
      // Determine severity based on distance
      let severity: "High" | "Medium" | "Low";
      if (missMeters < 1000) severity = "High";
      else if (missMeters < 5000) severity = "Medium";
      else severity = "Low";

      // Calculate probability proxy
      const k = 0.001;
      const d0 = 1000;
      const pcProxy = 1 / (1 + Math.exp(k * (missMeters - d0)));

      return {
        aId: pair.satellite_a,
        bId: pair.satellite_b,
        tcaUtc: pair.time_minutes * 60 * 1000, // convert to milliseconds
        missMeters,
        relSpeedMps,
        pcProxy,
        severity
      };
    });

    return { encounters };
  }

  // Mock TLE parsing for compatibility (reads from static data instead)
  parseTle(text: string, kind: BodyKind): TleEntry[] {
    const lines = text.trim().split('\n').map(l => l.trim()).filter(l => l);
    const entries: TleEntry[] = [];
    
    for (let i = 0; i < lines.length; i += 3) {
      if (i + 2 < lines.length) {
        entries.push({
          name: lines[i],
          line1: lines[i + 1],
          line2: lines[i + 2],
          kind
        });
      }
    }
    
    return entries;
  }

  // Get simulation data from JSON instead of computing
  async getSimulationTracks(): Promise<Track[]> {
    if (!this.coordinatesCache) {
      await this.loadCoordinatesFromUrl("/coordinates.json");
    }
    
    return this.coordinatesToTracks(this.coordinatesCache!);
  }

  // Get analysis results from JSON instead of computing
  async getAnalysisResults(): Promise<AnalysisResult> {
    if (!this.conjunctionsCache) {
      await this.loadConjunctionsFromUrl("/conjunctions.json");
    }
    
    return this.conjunctionsToAnalysisResult(this.conjunctionsCache!);
  }
}

export const jsonDataAdapter = new JsonDataAdapter();
