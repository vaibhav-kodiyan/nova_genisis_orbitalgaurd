import { AnalysisResult, Encounter } from '../domain/types';
import { AnalysisResults, EncounterRow } from '../components/ResultsTable';

/**
 * Convert AnalysisResult from domain types to AnalysisResults for UI components
 */
export function mapAnalysisResultToAnalysisResults(result: AnalysisResult): AnalysisResults {
  const encounters: EncounterRow[] = result.encounters.map((encounter: Encounter) => {
    const minDistKm = encounter.missMeters / 1000; // convert meters to km
    const relSpeedKms = encounter.relSpeedMps / 1000; // convert m/s to km/s

    let riskLevel: EncounterRow['riskLevel'] = 'low';
    if (minDistKm < 0.1) {
      riskLevel = 'high';
    } else if (minDistKm < 1) {
      riskLevel = 'medium';
    }

    return {
      aId: encounter.aId,
      bId: encounter.bId,
      tcaUtc: new Date(encounter.tcaUtc).toISOString(),
      minDistKm,
      relSpeedKms,
      riskLevel
    };
  });

  return {
    encounters,
    summary: {
      totalEncounters: encounters.length,
      window: '24 hours'
    }
  };
}
