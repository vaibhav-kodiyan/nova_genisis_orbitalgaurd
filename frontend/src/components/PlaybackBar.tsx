import { useAppStore } from "../state/appStore";

const SPEEDS = [0.25, 0.5, 1, 2, 4, 8, 16, 32];

export default function PlaybackBar() {
  const { isPlaying, speed, t, tMax, setPlaying, setSpeed, setT } = useAppStore();
  const idx = Math.max(0, SPEEDS.indexOf(speed));

  return (
    <div className="absolute left-3 right-3 bottom-3 z-10
                    rounded-xl border border-slate-700/70 bg-slate-900/80 px-3 py-2
                    backdrop-blur">
      <div className="flex items-center gap-3">
        <button onClick={() => setPlaying(!isPlaying)}
                className="px-3 py-1 rounded-lg bg-emerald-500 hover:bg-emerald-600 text-slate-900 font-semibold">
          {isPlaying ? "Pause" : "Play"}
        </button>

        <div className="flex items-center gap-2">
          <span className="text-slate-300 text-sm">Speed</span>
          <input type="range" min={0} max={SPEEDS.length - 1} step={1}
                 value={idx < 0 ? 2 : idx}
                 onChange={e => setSpeed(SPEEDS[+e.target.value])} />
          <span className="font-mono text-slate-200 text-sm">{speed}×</span>
        </div>

        <div className="flex-1 flex items-center gap-2">
          <span className="text-slate-300 text-sm">Seek</span>
          <input className="w-full" type="range" min={0} max={tMax} step={0.1}
                 value={t} onChange={e => setT(parseFloat(e.target.value) || 0)} />
          <span className="font-mono text-slate-200 text-xs">{t.toFixed(1)}s</span>
        </div>
      </div>
    </div>
  );
}
