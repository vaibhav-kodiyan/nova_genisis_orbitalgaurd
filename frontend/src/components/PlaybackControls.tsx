import React, { useEffect, useCallback } from 'react';
import { useAppStore } from '../state/appStore';

const PLAYBACK_SPEEDS = [0.25, 0.5, 1, 2, 4, 8, 16, 32];

export function PlaybackControls() {
  const {
    state,
    tracks,
    playbackTime,
    isPlaying,
    playbackSpeed,
    setPlaybackTime,
    setIsPlaying,
    setPlaybackSpeed
  } = useAppStore();

  const isVisible = state === 'Analysed';
  
  // Get time bounds from tracks
  const { minTime, maxTime, duration } = React.useMemo(() => {
    if (tracks.length === 0) return { minTime: 0, maxTime: 0, duration: 0 };
    
    let min = Infinity;
    let max = -Infinity;
    
    tracks.forEach(track => {
      track.states.forEach(state => {
        min = Math.min(min, state.t);
        max = Math.max(max, state.t);
      });
    });
    
    return { minTime: min, maxTime: max, duration: max - min };
  }, [tracks]);

  // Playback animation
  useEffect(() => {
    if (!isPlaying || !isVisible) return;
    
    const interval = setInterval(() => {
      setPlaybackTime((prev: number) => {
        const next = prev + (1000 * playbackSpeed); // 1 second * speed
        return next > maxTime ? minTime : next; // Loop back to start
      });
    }, 100); // Update every 100ms for smooth animation
    
    return () => clearInterval(interval);
  }, [isPlaying, playbackSpeed, maxTime, minTime, setPlaybackTime, isVisible]);

  // Keyboard shortcuts
  const handleKeyPress = useCallback((e: KeyboardEvent) => {
    if (!isVisible) return;
    
    switch (e.code) {
      case 'Space':
        e.preventDefault();
        setIsPlaying(!isPlaying);
        break;
      case 'ArrowLeft':
        e.preventDefault();
        setPlaybackTime(Math.max(minTime, playbackTime - 10000)); // -10 seconds
        break;
      case 'ArrowRight':
        e.preventDefault();
        setPlaybackTime(Math.min(maxTime, playbackTime + 10000)); // +10 seconds
        break;
      case 'BracketLeft':
        e.preventDefault();
        const currentIndex = PLAYBACK_SPEEDS.indexOf(playbackSpeed);
        if (currentIndex > 0) {
          setPlaybackSpeed(PLAYBACK_SPEEDS[currentIndex - 1]);
        }
        break;
      case 'BracketRight':
        e.preventDefault();
        const currentIdx = PLAYBACK_SPEEDS.indexOf(playbackSpeed);
        if (currentIdx < PLAYBACK_SPEEDS.length - 1) {
          setPlaybackSpeed(PLAYBACK_SPEEDS[currentIdx + 1]);
        }
        break;
    }
  }, [isVisible, isPlaying, playbackTime, playbackSpeed, minTime, maxTime, setIsPlaying, setPlaybackTime, setPlaybackSpeed]);

  useEffect(() => {
    window.addEventListener('keydown', handleKeyPress);
    return () => window.removeEventListener('keydown', handleKeyPress);
  }, [handleKeyPress]);

  if (!isVisible) {
    return (
      <div className="h-16 bg-gray-100 border-t border-gray-200 flex items-center justify-center">
        <div className="text-gray-500 text-sm">
          Playback controls will be available after running analysis.
        </div>
      </div>
    );
  }

  const progressPercent = duration > 0 ? ((playbackTime - minTime) / duration) * 100 : 0;

  return (
    <div className="h-16 bg-white border-t border-gray-200 px-6 flex items-center space-x-4">
      {/* Play/Pause */}
      <button
        onClick={() => setIsPlaying(!isPlaying)}
        className="p-2 rounded-md bg-blue-600 text-white hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-blue-500"
      >
        {isPlaying ? (
          <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 20 20">
            <path fillRule="evenodd" d="M18 10a8 8 0 11-16 0 8 8 0 0116 0zM7 8a1 1 0 012 0v4a1 1 0 11-2 0V8zm5-1a1 1 0 00-1 1v4a1 1 0 102 0V8a1 1 0 00-1-1z" clipRule="evenodd" />
          </svg>
        ) : (
          <svg className="w-4 h-4" fill="currentColor" viewBox="0 0 20 20">
            <path fillRule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zM9.555 7.168A1 1 0 008 8v4a1 1 0 001.555.832l3-2a1 1 0 000-1.664l-3-2z" clipRule="evenodd" />
          </svg>
        )}
      </button>

      {/* Seek bar */}
      <div className="flex-1 mx-4">
        <input
          type="range"
          min={minTime}
          max={maxTime}
          value={playbackTime}
          onChange={(e) => setPlaybackTime(Number(e.target.value))}
          className="w-full h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer"
          style={{
            background: `linear-gradient(to right, #3b82f6 0%, #3b82f6 ${progressPercent}%, #e5e7eb ${progressPercent}%, #e5e7eb 100%)`
          }}
        />
      </div>

      {/* Speed controls */}
      <div className="flex items-center space-x-2">
        <span className="text-sm text-gray-600">Speed:</span>
        <select
          value={playbackSpeed}
          onChange={(e) => setPlaybackSpeed(Number(e.target.value))}
          className="text-sm border border-gray-300 rounded px-2 py-1 focus:outline-none focus:ring-2 focus:ring-blue-500"
        >
          {PLAYBACK_SPEEDS.map(speed => (
            <option key={speed} value={speed}>
              {speed}×
            </option>
          ))}
        </select>
      </div>

      {/* Time display */}
      <div className="text-sm text-gray-600 min-w-max">
        {new Date(playbackTime).toISOString().substr(11, 8)}
      </div>

      {/* Keyboard shortcuts hint */}
      <div className="text-xs text-gray-400 min-w-max">
        Space: Play/Pause • ←/→: ±10s • [/]: Speed
      </div>
    </div>
  );
}
