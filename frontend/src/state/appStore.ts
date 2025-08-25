// Zustand store for app state management
import { create } from 'zustand';
import { AppState, TleEntry, Track, AnalysisResult } from '../domain/types';

interface AppStore {
  // State machine
  state: AppState;
  
  // Data
  satelliteTles: TleEntry[];
  debrisTles: TleEntry[];
  tracks: Track[];
  analysisResult: AnalysisResult | null;
  
  // UI state
  focusedPair: { aId: string; bId: string } | null;
  isPlaying: boolean;
  speed: number;
  t: number;
  tMax: number;
  selectedId: string | null;
  
  // Actions
  setSatelliteTles: (tles: TleEntry[]) => void;
  setDebrisTles: (tles: TleEntry[]) => void;
  setTracks: (tracks: Track[]) => void;
  setAnalysisResult: (result: AnalysisResult) => void;
  setFocusedPair: (pair: { aId: string; bId: string } | null) => void;
  setPlaying: (playing: boolean) => void;
  setSpeed: (speed: number) => void;
  setT: (t: number) => void;
  setTMax: (tMax: number) => void;
  setSelectedId: (id: string | null) => void;
  
  // State transitions
  transitionToDataReady: () => void;
  transitionToSimReady: () => void;
  transitionToAnalysed: () => void;
  reset: () => void;
}

export const useAppStore = create<AppStore>((set, get) => ({
  // Initial state
  state: 'Idle',
  satelliteTles: [],
  debrisTles: [],
  tracks: [],
  analysisResult: null,
  focusedPair: null,
  isPlaying: true,
  speed: 1,
  t: 0,
  tMax: 86400,
  selectedId: null,
  
  // Actions
  setSatelliteTles: (tles) => {
    set({ satelliteTles: tles });
    const { debrisTles } = get();
    if (tles.length > 0 && debrisTles.length > 0) {
      get().transitionToDataReady();
    }
  },
  
  setDebrisTles: (tles) => {
    set({ debrisTles: tles });
    const { satelliteTles } = get();
    if (tles.length > 0 && satelliteTles.length > 0) {
      get().transitionToDataReady();
    }
  },
  
  setTracks: (tracks) => set({ tracks }),
  setAnalysisResult: (result) => set({ analysisResult: result }),
  setFocusedPair: (pair) => set({ focusedPair: pair }),
  setPlaying: (playing) => set({ isPlaying: playing }),
  setSpeed: (speed) => set({ speed: Math.max(0.25, Math.min(32, speed)) }),
  setT: (t) => set({ t: Math.max(0, t) }),
  setTMax: (tMax) => set({ tMax }),
  setSelectedId: (id) => set({ selectedId: id }),
  
  // State transitions
  transitionToDataReady: () => set({ state: 'DataReady' }),
  transitionToSimReady: () => set({ state: 'SimReady' }),
  transitionToAnalysed: () => set({ state: 'Analysed' }),
  
  reset: () => set({
    state: 'Idle',
    satelliteTles: [],
    debrisTles: [],
    tracks: [],
    analysisResult: null,
    focusedPair: null,
    isPlaying: true,
    speed: 1,
    t: 0,
    tMax: 86400,
    selectedId: null
  })
}));
