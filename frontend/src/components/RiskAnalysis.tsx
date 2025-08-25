import { useState } from 'react';
import { useAppStore } from '../state/appStore';

export function RiskAnalysis() {
  const {
    state,
    tracks,
    analysisResult,
    setAnalysisResult,
    transitionToAnalysed
  } = useAppStore();

  const [thresholdKm, setThresholdKm] = useState(5);
  const [isRunning, setIsRunning] = useState(false);

  const canRunAnalysis = state === 'SimReady' && tracks.length > 0;

  const runAnalysis = async () => {
    if (!canRunAnalysis) return;
    
    setIsRunning(true);
    try {
      const { wasmBridge } = await import('../domain/wasmBridge');
      const result = wasmBridge.runAnalysis(tracks, 60); // 60 second sync tolerance
      
      setAnalysisResult(result);
      transitionToAnalysed();
      
      const toast = await import('react-hot-toast');
      toast.default.success('Analysis complete. Playback enabled.');
    } catch (error) {
      const toast = await import('react-hot-toast');
      toast.default.error(`Analysis failed: ${error}`);
    } finally {
      setIsRunning(false);
    }
  };

  const exportCSV = () => {
    if (!analysisResult?.encounters) return;

    const headers = ['Pair A', 'Pair B', 'TCA (UTC)', 'Min Dist (km)', 'Rel Speed (m/s)', 'Pc Proxy', 'Severity'];
    const rows = analysisResult.encounters.map(e => [
      e.aId,
      e.bId,
      new Date(e.tcaUtc).toISOString(),
      (e.missMeters / 1000).toFixed(3),
      e.relSpeedMps.toFixed(1),
      e.pcProxy.toFixed(6),
      e.severity
    ]);

    const csvContent = [headers, ...rows].map(row => row.join(',')).join('\n');
    const blob = new Blob([csvContent], { type: 'text/csv' });
    const url = URL.createObjectURL(blob);
    
    const a = document.createElement('a');
    a.href = url;
    a.download = `risk-analysis-${new Date().toISOString().split('T')[0]}.csv`;
    a.click();
    
    URL.revokeObjectURL(url);
  };

  return (
    <div className="rounded-2xl border border-slate-700/60 bg-slate-900/70 p-4 mb-4 shadow">
      <h3 className="text-slate-200 font-medium mb-4">Risk Analysis</h3>
      
      <div className="space-y-4">
        <div>
          <label className="text-slate-300 text-sm mb-1 block">
            Threshold (km)
          </label>
          <input
            type="number"
            value={thresholdKm}
            onChange={(e) => setThresholdKm(Number(e.target.value))}
            min="0.1"
            max="1000"
            step="0.1"
            className="w-full px-3 py-2 bg-slate-800 border border-slate-600 rounded-xl text-slate-200 focus:outline-none focus:border-emerald-400"
          />
        </div>
        
        <button
          type="button"
          onClick={runAnalysis}
          disabled={!canRunAnalysis || isRunning}
          className={`w-full px-3 py-2 rounded-xl font-semibold transition ${
            canRunAnalysis && !isRunning
              ? 'bg-emerald-500 hover:bg-emerald-600 text-slate-900'
              : 'bg-slate-700 text-slate-400 opacity-40 cursor-not-allowed'
          }`}
        >
          {isRunning ? 'Running...' : 'Run Risk Analysis'}
        </button>
        
        {analysisResult && (
          <div className="space-y-3">
            <div className="text-sm text-slate-300">
              Found {analysisResult.encounters.length} encounters
            </div>
            
            {analysisResult.encounters.length > 0 && (
              <>
                <div className="max-h-32 overflow-y-auto">
                  <table className="w-full text-xs text-slate-300">
                    <thead className="text-slate-400">
                      <tr>
                        <th className="text-left py-1">Pair</th>
                        <th className="text-left py-1">TCA (UTC)</th>
                        <th className="text-left py-1">Min Dist (km)</th>
                      </tr>
                    </thead>
                    <tbody>
                      {analysisResult.encounters.slice(0, 5).map((encounter, i) => (
                        <tr key={i} className="border-t border-slate-700">
                          <td className="py-1 font-mono">{encounter.aId.slice(0, 8)}.../{encounter.bId.slice(0, 8)}...</td>
                          <td className="py-1 font-mono">{new Date(encounter.tcaUtc).toISOString().substr(11, 8)}</td>
                          <td className="py-1 font-mono">{(encounter.missMeters / 1000).toFixed(1)}</td>
                        </tr>
                      ))}
                    </tbody>
                  </table>
                  {analysisResult.encounters.length > 5 && (
                    <div className="text-xs text-slate-400 mt-2">
                      ...and {analysisResult.encounters.length - 5} more
                    </div>
                  )}
                </div>
                
                <button
                  onClick={exportCSV}
                  className="w-full px-3 py-2 text-sm text-slate-300 border border-slate-600 rounded-xl hover:border-slate-500 hover:bg-slate-800/50 transition"
                >
                  Export CSV
                </button>
              </>
            )}
          </div>
        )}
      </div>
    </div>
  );
}
