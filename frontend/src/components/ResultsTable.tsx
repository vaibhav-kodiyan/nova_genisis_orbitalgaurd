
export interface EncounterRow {
  aId: string;
  bId: string;
  tcaUtc: string;
  minDistKm: number;
  relSpeedKms: number;
  riskLevel: 'low' | 'medium' | 'high';
}

export interface AnalysisResults {
  summary?: {
    totalEncounters: number;
    window: string;
  };
  encounters: EncounterRow[];
}

interface ResultsTableProps {
  results: AnalysisResults | null;
}

function getRiskLevelColor(riskLevel: 'low' | 'medium' | 'high'): string {
  switch (riskLevel) {
    case 'high':
      return 'text-red-400 bg-red-500/20 border border-red-500/40';
    case 'medium':
      return 'text-yellow-400 bg-yellow-500/20 border border-yellow-500/40';
    case 'low':
      return 'text-green-400 bg-green-500/20 border border-green-500/40';
    default:
      return 'text-slate-400 bg-slate-500/20 border border-slate-500/40';
  }
}

export function ResultsTable({ results }: ResultsTableProps) {
  if (!results) {
    return (
      <div className="flex-1 flex items-center justify-center text-slate-400 text-sm">
        Run analysis to see encounters here.
      </div>
    );
  }

  const { encounters, summary } = results;

  return (
    <div className="flex-1 flex flex-col overflow-hidden">
      {summary && (
        <div className="px-3 py-2 text-sm text-slate-300 border-b border-slate-700/60">
          Found {summary.totalEncounters} encounters in {summary.window}
        </div>
      )}
      
      {encounters.length === 0 ? (
        <div className="flex-1 flex items-center justify-center text-slate-400 text-sm">
          No encounters found in analysis.
        </div>
      ) : (
        <div className="flex-1 overflow-auto">
          <table className="w-full text-sm">
            <thead className="sticky top-0 bg-slate-900/90 backdrop-blur border-b border-slate-700/60">
              <tr className="[&>th]:py-2 [&>th]:px-3 text-slate-300 text-left">
                <th>Primary</th>
                <th>Secondary</th>
                <th>TCA (UTC)</th>
                <th>Min Dist (km)</th>
                <th>Rel Speed (km/s)</th>
                <th>Risk Level</th>
              </tr>
            </thead>
            <tbody>
              {encounters.map((encounter, i) => (
                <tr key={i} className="border-b border-slate-800/70 [&>td]:py-2 [&>td]:px-3 text-slate-200 hover:bg-slate-800/30">
                  <td className="font-mono text-emerald-300">{encounter.aId}</td>
                  <td className="font-mono text-red-300">{encounter.bId}</td>
                  <td className="font-mono">{encounter.tcaUtc.replace('T', ' ').substr(0, 19)}</td>
                  <td className="font-mono">{encounter.minDistKm.toFixed(3)}</td>
                  <td className="font-mono">{encounter.relSpeedKms.toFixed(2)}</td>
                  <td>
                    <span className={`px-2 py-1 rounded text-xs font-medium uppercase tracking-wide ${getRiskLevelColor(encounter.riskLevel)}`}>
                      {encounter.riskLevel}
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  );
}
