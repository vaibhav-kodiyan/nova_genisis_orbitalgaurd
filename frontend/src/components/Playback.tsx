import React from 'react';
import { useAppStore } from '../state/appStore';

export function Playback() {
  const {
    state,
    tracks,
    isPlaying,
    speed,
    t,
    tMax,
    setPlaying,
    setSpeed,
    setT,
    setTMax
  } = useAppStore();

  const isVisible = state === 'Analysed';

  if (!isVisible) return null;

  // Calculate tMax from tracks if not set
  React.useEffect(() => {
    if (tracks.length === 0) return;
    
    let max = -Infinity;
    tracks.forEach(track => {
      track.states.forEach(state => {
        max = Math.max(max, state.t);
      });
    });
    
    // Update tMax in store if different
    if (tMax !== max) {
      setTMax(max);
      setT(Math.min(t, max));
    }
  }, [tracks, t]);

  const speedOptions = [0.25, 0.5, 1, 2, 4, 8, 16, 32];
  const speedIndex = speedOptions.indexOf(speed);

  return (
    <div className="rounded-2xl border border-slate-700/60 bg-slate-900/70 p-4 mb-4 shadow">
      <h3 className="text-slate-200 font-medium mb-4">Playback</h3>
      
      <div className="space-y-4">
        {/* Play/Pause Button */}
        <div className="flex items-center justify-center">
          <button
            onClick={() => setPlaying(!isPlaying)}
            className="p-3 rounded-xl bg-emerald-500 hover:bg-emerald-600 text-slate-900 transition"
          >
            {isPlaying ? "Pause" : "Play"}
          </button>
        </div>

        {/* Speed Slider */}
        <div className="space-y-2">
          <label className="text-sm text-slate-400">Speed: {speed}×</label>
          <input
            type="range"
            min={0}
            max={7}
            step={1}
            value={speedIndex >= 0 ? speedIndex : 2}
            onChange={(e) => setSpeed(speedOptions[+e.target.value])}
            className="w-full h-2 bg-slate-700 rounded-lg appearance-none cursor-pointer"
          />
          <div className="flex justify-between text-xs text-slate-500">
            <span>0.25×</span>
            <span>0.5×</span>
            <span>1×</span>
            <span>2×</span>
            <span>4×</span>
            <span>8×</span>
            <span>16×</span>
            <span>32×</span>
          </div>
        </div>

        {/* Seek Bar */}
        <div className="space-y-2">
          <label className="text-sm text-slate-400">Time</label>
          <input
            type="range"
            min={0}
            max={tMax}
            step={0.01}
            value={t}
            onChange={(e) => setT(parseFloat(e.target.value))}
            className="w-full h-2 bg-slate-700 rounded-lg appearance-none cursor-pointer"
          />
          <div className="text-xs text-slate-400 font-mono text-center">
            {new Date(t).toISOString().substr(11, 8)}
          </div>
        </div>
      </div>
    </div>
  );
}
