import { useAppStore } from '../state/appStore';

export function SimControls() {
  const {
    state,
    satelliteTles,
    debrisTles,
    playbackSpeed,
    setPlaybackSpeed,
    reset
  } = useAppStore();

  const canComputeSimulation = state === 'DataReady' && satelliteTles.length > 0 && debrisTles.length > 0;
  const showSpeedSlider = state === 'SimReady' || state === 'Analysed';

  // Use existing computeSimulation function from SidebarDataPanel
  const computeSimulation = async () => {
    if (!canComputeSimulation) return;
    
    try {
      const { wasmBridge } = await import('../domain/wasmBridge');
      const { setTracks, transitionToSimReady } = useAppStore.getState();
      
      await wasmBridge.init();
      
      const allTles = [...satelliteTles, ...debrisTles];
      const now = Date.now();
      const tracks = wasmBridge.computeSimulation(
        allTles,
        now,
        now + (24 * 60 * 60 * 1000), // 24 hours
        60 // 60 second steps
      );
      
      setTracks(tracks);
      transitionToSimReady();
      
      const toast = await import('react-hot-toast');
      toast.default.success('Simulation computed successfully.');
    } catch (error) {
      const toast = await import('react-hot-toast');
      toast.default.error(`Simulation failed: ${error}`);
    }
  };

  return (
    <div className="rounded-2xl border border-slate-700/60 bg-slate-900/70 p-4 mb-4 shadow">
      <h3 className="text-slate-200 font-medium mb-4">Simulation Controls</h3>
      
      <div className="space-y-3">
        <button
          onClick={computeSimulation}
          disabled={!canComputeSimulation}
          className={`w-full px-3 py-2 rounded-xl font-semibold transition ${
            canComputeSimulation
              ? 'bg-emerald-500 hover:bg-emerald-600 text-slate-900'
              : 'bg-slate-700 text-slate-400 opacity-40 cursor-not-allowed'
          }`}
        >
          Compute Simulation
        </button>
        
        <button
          onClick={reset}
          className="w-full px-3 py-2 rounded-xl font-semibold bg-slate-700 hover:bg-slate-600 text-slate-200 transition"
        >
          Reset
        </button>
        
        {showSpeedSlider && (
          <div className="pt-2">
            <label className="text-slate-300 text-sm mb-1 block">
              Playback Speed: {playbackSpeed}×
            </label>
            <input
              type="range"
              min="0.25"
              max="4"
              step="0.25"
              value={playbackSpeed}
              onChange={(e) => setPlaybackSpeed(Number(e.target.value))}
              className="w-full h-2 bg-slate-700 rounded-lg appearance-none cursor-pointer slider"
            />
          </div>
        )}
      </div>
    </div>
  );
}
