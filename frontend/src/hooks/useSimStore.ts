// Extended store hook that wraps the existing appStore with additional UI state
import { useAppStore } from '../state/appStore';

export type SimState = {
  satCount: number;
  debrisCount: number;
  ready: boolean;
  running: boolean;
  thresholdKm: number;
  setCounts: (s: number, d: number) => void;
  setReady: (b: boolean) => void;
  setRunning: (b: boolean) => void;
  setThreshold: (v: number) => void;
};

export const useSimStore = (): SimState => {
  const {
    satelliteTles,
    debrisTles,
    state,
    isPlaying
  } = useAppStore();

  return {
    satCount: satelliteTles.length,
    debrisCount: debrisTles.length,
    ready: state === 'SimReady' || state === 'Analysed',
    running: isPlaying,
    thresholdKm: 5, // Default threshold
    setCounts: () => {}, // Handled by existing store
    setReady: () => {}, // Handled by existing store
    setRunning: () => {}, // Handled by existing store
    setThreshold: () => {} // Will be implemented if needed
  };
};
