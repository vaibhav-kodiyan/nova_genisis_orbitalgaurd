import React, { useRef } from 'react';
import { useAppStore } from '../state/appStore';
import { wasmBridge } from '../domain/wasmBridge';
import toast from 'react-hot-toast';

export function SidebarDataPanel() {
  const {
    state,
    satelliteTles,
    debrisTles,
    setSatelliteTles,
    setDebrisTles,
    setTracks,
    setAnalysisResult,
    transitionToSimReady,
    transitionToAnalysed
  } = useAppStore();

  const satelliteFileRef = useRef<HTMLInputElement>(null);
  const debrisFileRef = useRef<HTMLInputElement>(null);

  const handleFileLoad = async (file: File, kind: 'satellite' | 'debris') => {
    try {
      const text = await file.text();
      const tles = wasmBridge.parseTle(text, kind);
      
      if (kind === 'satellite') {
        setSatelliteTles(tles);
        toast.success(`Loaded ${tles.length} satellites`);
      } else {
        setDebrisTles(tles);
        toast.success(`Loaded ${tles.length} debris objects`);
      }
    } catch (error) {
      toast.error(`Failed to load ${kind} TLEs: ${error}`);
    }
  };

  const loadSampleData = async (kind: 'satellite' | 'debris') => {
    try {
      const response = await fetch(`/src/samples/sample-${kind === 'satellite' ? 'satellites' : 'debris'}.tle`);
      const text = await response.text();
      const tles = wasmBridge.parseTle(text, kind);
      
      if (kind === 'satellite') {
        setSatelliteTles(tles);
        toast.success(`Loaded ${tles.length} sample satellites`);
      } else {
        setDebrisTles(tles);
        toast.success(`Loaded ${tles.length} sample debris objects`);
      }
    } catch (error) {
      toast.error(`Failed to load sample ${kind} data: ${error}`);
    }
  };

  const computeSimulation = async () => {
    if (state !== 'DataReady') return;
    
    try {
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
      toast.success('Simulation computed successfully.');
    } catch (error) {
      toast.error(`Simulation failed: ${error}`);
    }
  };

  const runAnalysis = async () => {
    if (state !== 'SimReady') return;
    
    try {
      const tracks = useAppStore.getState().tracks;
      const result = wasmBridge.runAnalysis(tracks, 60); // 60 second sync tolerance
      
      setAnalysisResult(result);
      transitionToAnalysed();
      toast.success('Analysis complete. Playback enabled.');
    } catch (error) {
      toast.error(`Analysis failed: ${error}`);
    }
  };

  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    const files = Array.from(e.dataTransfer.files);
    if (files.length > 0) {
      // For demo, assume first file is satellites if it contains "sat" in name
      const file = files[0];
      const kind = file.name.toLowerCase().includes('sat') ? 'satellite' : 'debris';
      handleFileLoad(file, kind);
    }
  };

  const canComputeSimulation = state === 'DataReady' && satelliteTles.length > 0 && debrisTles.length > 0;
  const canRunAnalysis = state === 'SimReady';

  return (
    <div className="w-80 bg-white border-r border-gray-200 p-6 flex flex-col">
      <h2 className="text-xl font-semibold mb-6 text-gray-900">Data</h2>
      
      {/* File input refs */}
      <input
        ref={satelliteFileRef}
        type="file"
        accept=".tle,.txt"
        className="hidden"
        onChange={(e) => e.target.files?.[0] && handleFileLoad(e.target.files[0], 'satellite')}
      />
      <input
        ref={debrisFileRef}
        type="file"
        accept=".tle,.txt"
        className="hidden"
        onChange={(e) => e.target.files?.[0] && handleFileLoad(e.target.files[0], 'debris')}
      />

      {/* Buttons in exact order */}
      <div className="space-y-3 mb-6">
        <button
          onClick={() => satelliteFileRef.current?.click()}
          className="w-full px-4 py-2 text-sm font-medium text-gray-700 bg-white border border-gray-300 rounded-md hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-blue-500"
        >
          Load Custom Satellite TLEs
        </button>
        
        <button
          onClick={() => debrisFileRef.current?.click()}
          className="w-full px-4 py-2 text-sm font-medium text-gray-700 bg-white border border-gray-300 rounded-md hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-blue-500"
        >
          Load Custom Debris TLEs
        </button>
        
        <button
          onClick={() => loadSampleData('satellite')}
          className="w-full px-4 py-2 text-sm font-medium text-gray-700 bg-white border border-gray-300 rounded-md hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-blue-500"
        >
          Load Sample Satellite TLEs
        </button>
        
        <button
          onClick={() => loadSampleData('debris')}
          className="w-full px-4 py-2 text-sm font-medium text-gray-700 bg-white border border-gray-300 rounded-md hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-blue-500"
        >
          Load Sample Debris TLEs
        </button>
        
        <button
          onClick={computeSimulation}
          disabled={!canComputeSimulation}
          className={`w-full px-4 py-2 text-sm font-medium rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 ${
            canComputeSimulation
              ? 'text-white bg-blue-600 hover:bg-blue-700'
              : 'text-gray-400 bg-gray-100 cursor-not-allowed'
          }`}
        >
          Compute Simulation
        </button>
        
        <button
          onClick={runAnalysis}
          disabled={!canRunAnalysis}
          className={`w-full px-4 py-2 text-sm font-medium rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500 ${
            canRunAnalysis
              ? 'text-white bg-blue-600 hover:bg-blue-700'
              : 'text-gray-400 bg-gray-100 cursor-not-allowed'
          }`}
        >
          Run Risk Analysis
        </button>
      </div>

      {/* Drag & drop zone */}
      <div
        onDrop={handleDrop}
        onDragOver={(e) => e.preventDefault()}
        className="border-2 border-dashed border-gray-300 rounded-lg p-6 text-center text-gray-500 mb-6 hover:border-gray-400 transition-colors"
      >
        Drag & drop .tle/.txt here
      </div>

      {/* Status counters */}
      <div className="space-y-2 text-sm text-gray-600">
        <div>Satellites: {satelliteTles.length}</div>
        <div>Debris: {debrisTles.length}</div>
      </div>

      {/* Hint for compute simulation */}
      {!canComputeSimulation && state === 'Idle' && (
        <div className="mt-4 p-3 bg-yellow-50 border border-yellow-200 rounded-md text-sm text-yellow-800">
          Load both Satellite and Debris TLEs to proceed.
        </div>
      )}
    </div>
  );
}
