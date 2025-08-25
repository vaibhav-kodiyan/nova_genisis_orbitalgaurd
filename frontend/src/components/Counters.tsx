import { useAppStore } from '../state/appStore';

export function Counters() {
  const { satelliteTles, debrisTles } = useAppStore();

  return (
    <div className="flex items-center space-x-4 text-sm font-mono">
      <span className="text-slate-200">
        Satellites: <span className="text-emerald-400">{satelliteTles.length}</span>
      </span>
      <span className="text-slate-400">·</span>
      <span className="text-slate-200">
        Debris: <span className="text-emerald-400">{debrisTles.length}</span>
      </span>
    </div>
  );
}
