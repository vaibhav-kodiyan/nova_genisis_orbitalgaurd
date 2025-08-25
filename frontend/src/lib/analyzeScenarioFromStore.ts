import { AnalysisResults, EncounterRow } from '../components/ResultsTable';
import { wasmBridge } from '../domain/wasmBridge';
import { Track } from '../domain/types';

/**
 * Analyzes the current scenario from store data and returns results
 * in the format expected by ResultsTable component
 */
export async function analyzeScenarioFromStore(tracks: Track[]): Promise<AnalysisResults> {
  if (tracks.length === 0) {
    throw new Error('No tracks available for analysis');
  }

  // Run the existing WASM analysis
  const rawResult = wasmBridge.runAnalysis(tracks, 60); // 60 second sync tolerance
  
  if (!rawResult) {
    throw new Error('Analysis returned no results');
  }

  // Transform encounters to match ResultsTable format
  const encounters: EncounterRow[] = Array.isArray(rawResult.encounters) 
    ? rawResult.encounters.map((encounter: any) => ({
        aId: String(encounter.aId || encounter.a || ''),
        bId: String(encounter.bId || encounter.b || ''),
        tcaUtc: new Date(encounter.tcaUtc || encounter.time || Date.now()).toISOString(),
        minDistKm: Number.isFinite(encounter.missMeters) 
          ? encounter.missMeters / 1000 
          : Number.POSITIVE_INFINITY,
        relSpeedKms: Number.isFinite(encounter.relSpeedMps) 
          ? encounter.relSpeedMps / 1000 
          : 0
      }))
    : [];

  // Create summary
  const summary = {
    totalEncounters: encounters.length,
    window: '24 hours' // Default analysis window
  };

  return {
    summary,
    encounters
  };
}
