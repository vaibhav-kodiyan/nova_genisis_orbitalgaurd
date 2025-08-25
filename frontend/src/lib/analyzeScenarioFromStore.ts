import { AnalysisResults, EncounterRow } from '../components/ResultsTable';
import { wasmBridge } from '../domain/wasmBridge';
import { useAppStore } from '../state/appStore';

/**
 * Calculate risk level based on minimum distance and relative speed
 * Risk factors:
 * - High: < 5km distance OR > 15 km/s relative speed
 * - Medium: 5-20km distance OR 10-15 km/s relative speed  
 * - Low: > 20km distance AND < 10 km/s relative speed
 */
function calculateRiskLevel(minDistKm: number, relSpeedKms: number): 'low' | 'medium' | 'high' {
  // High risk conditions
  if (minDistKm < 5 || relSpeedKms > 15) {
    return 'high';
  }
  
  // Medium risk conditions
  if (minDistKm < 20 || relSpeedKms > 10) {
    return 'medium';
  }
  
  // Low risk (safe encounters)
  return 'low';
}

/**
 * Analyzes the current scenario from store data and returns results
 * in the format expected by ResultsTable component
 */
export async function analyzeScenarioFromStore(): Promise<AnalysisResults> {
  const tracks = useAppStore.getState().tracks;
  
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
    ? rawResult.encounters.map((encounter: any) => {
        const minDistKm = Number.isFinite(encounter.missMeters) 
          ? encounter.missMeters / 1000 
          : Number.POSITIVE_INFINITY;
        const relSpeedKms = Number.isFinite(encounter.relSpeedMps) 
          ? encounter.relSpeedMps / 1000 
          : 0;
        
        return {
          aId: String(encounter.aId || encounter.a || ''),
          bId: String(encounter.bId || encounter.b || ''),
          tcaUtc: new Date(encounter.tcaUtc || encounter.time || Date.now()).toISOString(),
          minDistKm,
          relSpeedKms,
          riskLevel: calculateRiskLevel(minDistKm, relSpeedKms)
        };
      })
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
