import { AnalysisResult, Encounter } from '../domain/types';
import { AnalysisResults, EncounterRow } from '../components/ResultsTable';

/**
 * Convert AnalysisResult from domain types to AnalysisResults for UI components
 */
export function mapAnalysisResultToAnalysisResults(result: AnalysisResult): AnalysisResults {
  const encounters: EncounterRow[] = result.encounters.map((encounter: Encounter) => ({
    aId: encounter.aId,
    bId: encounter.bId,
    tcaUtc: new Date(encounter.tcaUtc).toISOString(),
    minDistKm: encounter.missMeters / 1000, // convert meters to km
    relSpeedKms: encounter.relSpeedMps / 1000 // convert m/s to km/s
  }));

  return {
    encounters,
    summary: {
      totalEncounters: encounters.length,
      window: '24 hours'
    }
  };
}
