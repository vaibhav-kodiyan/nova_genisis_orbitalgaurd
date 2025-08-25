// WASM bridge for C++ orbital mechanics functions
import { TleEntry, Track, AnalysisResult, BodyKind } from './types';

// For now, we'll use a mock implementation until WASM is compiled
// This maintains the exact API contract specified

export class WasmBridge {
  // private _module: any = null; // TODO: Will be used when WASM is loaded
  private initialized = false;

  async init(): Promise<void> {
    if (this.initialized) return;
    
    // TODO: Load actual WASM module when compiled
    // For now, use mock implementation
    this.initialized = true;
  }

  // Parse TLE text into JSON array of {name,line1,line2,kind}
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

  // Propagate all bodies over window
  // Returns array of Track {id,kind,states:[{t,r[3],v[3]}]}
  computeSimulation(tleEntries: TleEntry[], startMs: number, stopMs: number, stepS: number): Track[] {
    const tracks: Track[] = [];
    const duration = stopMs - startMs;
    const steps = Math.floor(duration / (stepS * 1000));
    
    tleEntries.forEach((tle, index) => {
      const states = [];
      
      for (let step = 0; step <= steps; step++) {
        const t = startMs + (step * stepS * 1000);
        
        // Mock orbital propagation - generate realistic-looking orbits
        const angle = (step * stepS) / 5400; // ~90 min orbit
        const radius = 6371 + 400 + (Math.random() * 1000); // 400-1400 km altitude
        const inclination = Math.random() * Math.PI;
        
        const r: [number, number, number] = [
          radius * Math.cos(angle) * Math.cos(inclination),
          radius * Math.sin(angle) * Math.cos(inclination),
          radius * Math.sin(inclination)
        ];
        
        const v: [number, number, number] = [
          -7.5 * Math.sin(angle),
          7.5 * Math.cos(angle),
          0
        ];
        
        states.push({ t, r, v });
      }
      
      tracks.push({
        id: `${tle.kind}_${index}`,
        kind: tle.kind,
        states
      });
    });
    
    return tracks;
  }

  // Screen conjunctions; severity buckets
  // Returns { encounters: [...] }
  runAnalysis(tracks: Track[], _syncTolS: number): AnalysisResult {
    const encounters = [];
    
    // Mock conjunction analysis
    for (let i = 0; i < tracks.length; i++) {
      for (let j = i + 1; j < tracks.length; j++) {
        const trackA = tracks[i];
        const trackB = tracks[j];
        
        // Find closest approach (simplified)
        let minDistance = Infinity;
        let tcaTime = 0;
        let relSpeed = 0;
        
        for (let k = 0; k < Math.min(trackA.states.length, trackB.states.length); k++) {
          const stateA = trackA.states[k];
          const stateB = trackB.states[k];
          
          const dx = stateA.r[0] - stateB.r[0];
          const dy = stateA.r[1] - stateB.r[1];
          const dz = stateA.r[2] - stateB.r[2];
          const distance = Math.sqrt(dx*dx + dy*dy + dz*dz) * 1000; // convert to meters
          
          if (distance < minDistance) {
            minDistance = distance;
            tcaTime = stateA.t;
            
            const dvx = stateA.v[0] - stateB.v[0];
            const dvy = stateA.v[1] - stateB.v[1];
            const dvz = stateA.v[2] - stateB.v[2];
            relSpeed = Math.sqrt(dvx*dvx + dvy*dvy + dvz*dvz) * 1000; // convert to m/s
          }
        }
        
        // Only include encounters within 25km
        if (minDistance < 25000) {
          let severity: "High" | "Medium" | "Low";
          if (minDistance < 1000) severity = "High";
          else if (minDistance < 5000) severity = "Medium";
          else severity = "Low";
          
          // Logistic Pc proxy: pc = 1/(1+exp(k*(d - d0)))
          const k = 0.001;
          const d0 = 1000;
          const pcProxy = 1 / (1 + Math.exp(k * (minDistance - d0)));
          
          encounters.push({
            aId: trackA.id,
            bId: trackB.id,
            tcaUtc: tcaTime,
            missMeters: minDistance,
            relSpeedMps: relSpeed,
            pcProxy,
            severity
          });
        }
      }
    }
    
    return { encounters };
  }
}

export const wasmBridge = new WasmBridge();
