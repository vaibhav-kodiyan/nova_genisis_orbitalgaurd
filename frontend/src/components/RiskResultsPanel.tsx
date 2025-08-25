import React from "react";
import { useAppStore } from "../state/appStore";

type RiskRow = { 
  idA: string; 
  idB: string; 
  tcaUtc: string; 
  minDistKm: number; 
  severity: "Low" | "Med" | "High" 
};

const computeSeverity = (d: number): RiskRow["severity"] =>
  d <= 1 ? "High" : d <= 5 ? "Med" : "Low";

export default function RiskResultsPanel() {
  const {
    state,
    tracks,
    setAnalysisResult,
    transitionToAnalysed
  } = useAppStore();

  const [thresholdKm, setThreshold] = React.useState(5);
  const [riskLoading, setRiskLoading] = React.useState(false);
  const [riskRows, setRiskRows] = React.useState<RiskRow[]>([]);

  const onRunRisk = async () => {
    try {
      setRiskLoading(true);
      const { wasmBridge } = await import('../domain/wasmBridge');
      const raw = wasmBridge.runAnalysis(tracks, 60); // existing API
      
      const rows: RiskRow[] = Array.isArray(raw?.encounters) ? raw.encounters.map((r: any) => ({
        idA: String(r.aId ?? r.a ?? ""),
        idB: String(r.bId ?? r.b ?? ""),
        tcaUtc: new Date(r.tcaUtc ?? r.time ?? Date.now()).toISOString(),
        minDistKm: Number.isFinite(r.missMeters) ? (r.missMeters / 1000) : Number.POSITIVE_INFINITY,
        severity: computeSeverity(Number.isFinite(r.missMeters) ? (r.missMeters / 1000) : Infinity),
      })) : [];
      
      setRiskRows(rows);
      setAnalysisResult(raw);
      transitionToAnalysed();
      
      const toast = await import('react-hot-toast');
      toast.default.success('Analysis complete. Playback enabled.');
    } catch (e) {
      console.error("RISK_ANALYSIS_ERROR", e);
      setRiskRows([]); // never undefined
      const toast = await import('react-hot-toast');
      toast.default.error(`Analysis failed: ${e}`);
    } finally {
      setRiskLoading(false);
    }
  };

  const exportCsv = (rows: RiskRow[]) => {
    const header = ["idA", "idB", "tcaUtc", "minDistKm", "severity"];
    const body = rows.map(r => [r.idA, r.idB, r.tcaUtc, r.minDistKm, r.severity].join(",")).join("\n");
    const blob = new Blob([[header.join(","), "\n", body].join("")], { type: "text/csv" });
    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = "risk_results.csv";
    a.click();
    URL.revokeObjectURL(a.href);
  };

  const rows = riskRows ?? [];
  const canRunAnalysis = state === 'SimReady' && tracks.length > 0;

  return (
    <div className="h-full flex flex-col rounded-2xl border border-slate-700/60 bg-slate-900/70 overflow-hidden">
      <div className="flex items-end gap-3 p-3 border-b border-slate-700/60">
        <div className="flex-1">
          <label className="block text-slate-300 text-sm">Threshold (km)</label>
          <input
            type="number"
            min={0.1}
            step={0.1}
            value={thresholdKm}
            onChange={(e) => setThreshold(parseFloat(e.target.value) || 0)}
            className="mt-1 w-32 rounded-md bg-slate-800 border border-slate-600 px-2 py-1 text-slate-100"
          />
        </div>
        <button
          type="button"
          onClick={onRunRisk}
          disabled={riskLoading || !canRunAnalysis}
          className="bg-emerald-500 hover:bg-emerald-600 text-slate-900 font-semibold px-3 py-2 rounded-xl disabled:opacity-40 disabled:cursor-not-allowed"
        >
          {riskLoading ? "Running…" : "Run Risk Analysis"}
        </button>
        <button
          onClick={() => exportCsv(riskRows)}
          disabled={!rows.length}
          className="bg-slate-700 hover:bg-slate-600 text-slate-100 font-medium px-3 py-2 rounded-xl disabled:opacity-40 disabled:cursor-not-allowed"
        >
          Export CSV
        </button>
      </div>
      <div className="flex-1 overflow-auto p-3">
        {(!riskRows || !riskRows.length) ? (
          <div className="text-slate-400 text-sm">No encounters yet. Run analysis above.</div>
        ) : (
          <table className="w-full text-sm">
            <thead className="sticky top-0 bg-slate-900/90 backdrop-blur border-b border-slate-700/60">
              <tr className="[&>th]:py-2 [&>th]:px-2 text-slate-300">
                <th className="text-left">Pair</th>
                <th className="text-left">TCA (UTC)</th>
                <th className="text-left">Min Dist (km)</th>
                <th className="text-left">Severity</th>
              </tr>
            </thead>
            <tbody>
              {riskRows.map((r, i) => (
                <tr key={i} className="border-b border-slate-800/70 [&>td]:py-2 [&>td]:px-2 text-slate-200">
                  <td className="font-mono">{r.idA} × {r.idB}</td>
                  <td className="font-mono">{r.tcaUtc.replace('T', ' ').substr(0, 19)}</td>
                  <td className="font-mono">{Number.isFinite(r.minDistKm) ? r.minDistKm.toFixed(3) : "—"}</td>
                  <td>
                    <span className={
                      r.severity === "High" ? "px-2 py-1 rounded-lg bg-rose-500/20 text-rose-300 border border-rose-500/40" :
                      r.severity === "Med" ? "px-2 py-1 rounded-lg bg-amber-500/20 text-amber-300 border border-amber-500/40" :
                                            "px-2 py-1 rounded-lg bg-emerald-500/20 text-emerald-300 border border-emerald-500/40"
                    }>
                      {r.severity}
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
}
