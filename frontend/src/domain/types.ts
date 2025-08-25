// Domain types for orbital debris tracking system
export type BodyKind = "satellite" | "debris";

export interface TleEntry { 
  name: string; 
  line1: string; 
  line2: string; 
  kind: BodyKind; 
}

export interface SimParams { 
  startEpochMs: number; 
  stopEpochMs: number; 
  stepSeconds: number; 
}

export interface StateECI { 
  t: number; 
  r: [number, number, number]; 
  v: [number, number, number]; 
}

export interface Track { 
  id: string; 
  kind: BodyKind; 
  states: StateECI[]; 
}

export interface AnalysisParams { 
  syncToleranceSec: number; 
}

export type Severity = "High" | "Medium" | "Low";

export interface Encounter {
  aId: string; 
  bId: string; 
  tcaUtc: number; 
  missMeters: number;
  relSpeedMps: number; 
  pcProxy: number; 
  severity: Severity;
}

export interface AnalysisResult { 
  encounters: Encounter[]; 
}

// App state machine
export type AppState = "Idle" | "DataReady" | "SimReady" | "Analysed";

export interface AppStore {
  state: AppState;
  satelliteTles: TleEntry[];
  debrisTles: TleEntry[];
  tracks: Track[];
  analysisResult: AnalysisResult | null;
  focusedPair: { aId: string; bId: string } | null;
  playbackTime: number;
  isPlaying: boolean;
  playbackSpeed: number;
}
