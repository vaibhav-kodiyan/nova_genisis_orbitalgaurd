import { useEffect, useState } from 'react';
import { Toaster } from 'react-hot-toast';
import { ControlPanel } from './components/ControlPanel';
import { Viewer3D } from './components/Viewer3D';
import { Counters } from './components/Counters';
import ErrorBoundary from './components/ErrorBoundary';
import { ResultsTable, AnalysisResults } from './components/ResultsTable';
import { wasmBridge } from './domain/wasmBridge';
import { useAppStore } from './state/appStore';
import { analyzeScenarioFromStore } from './lib/analyzeScenarioFromStore';
import toast from 'react-hot-toast';
import './split.css';

export default function App() {
  const { satelliteTles, debrisTles, tracks } = useAppStore();
  const [isRunning, setIsRunning] = useState(false);
  const [results, setResults] = useState<AnalysisResults | null>(null);
  const [errMsg, setErrMsg] = useState<string>('');

  useEffect(() => {
    // Initialize WASM bridge on app start
    wasmBridge.init().catch(console.error);
  }, []);

  const runAnalysis = async () => {
    setIsRunning(true);
    setErrMsg('');
    
    try {
      const analysisResults = await analyzeScenarioFromStore(tracks);
      setResults(analysisResults);
      toast.success('Analysis complete. Results displayed below.');
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : 'Analysis failed';
      setErrMsg(errorMsg);
      toast.error(errorMsg);
    } finally {
      setIsRunning(false);
    }
  };

  const hasData = satelliteTles.length > 0 && debrisTles.length > 0;

  return (
    <div className="h-screen bg-slate-900 text-slate-200 flex flex-col">
      {/* Header */}
      <header className="sticky top-0 bg-slate-900/95 backdrop-blur border-b border-slate-700 px-6 py-4 flex items-center justify-between z-10">
        <h1 className="text-xl font-semibold text-slate-200">
          OrbitalGuard Nova — Sandbox
        </h1>
        <Counters />
      </header>

      {/* Main Layout */}
      <div className="flex-1 flex overflow-hidden">
        {/* Left Control Panel */}
        <ControlPanel onRunAnalysis={runAnalysis} />
        
        {/* Right Split Area */}
        <div className="flex-1 split-container">
          {/* Top Pane - Playback (always mounted) */}
          <div className="split-pane split-pane-top">
            <ErrorBoundary>
              {hasData ? (
                <Viewer3D paused={isRunning} />
              ) : (
                <div className="h-full flex items-center justify-center">
                  <div className="text-center text-slate-400">
                    <div className="text-lg mb-2">Load Satellite & Debris TLEs to begin.</div>
                    <div className="text-sm">Use the control panel on the left to upload your data files.</div>
                  </div>
                </div>
              )}
            </ErrorBoundary>
          </div>
          
          {/* Bottom Pane - Controls + Results */}
          <div className="split-pane split-pane-bottom">
            <div className="split-toolbar">
              {errMsg && (
                <div className="px-2 py-1 bg-red-500/20 text-red-300 border border-red-500/40 rounded text-sm">
                  {errMsg}
                </div>
              )}
              {isRunning && (
                <div className="px-2 py-1 bg-blue-500/20 text-blue-300 border border-blue-500/40 rounded text-sm">
                  Analysis running...
                </div>
              )}
            </div>
            <div className="split-results">
              <ResultsTable results={results} />
            </div>
          </div>
        </div>
      </div>
      
      {/* Toast notifications */}
      <Toaster 
        position="top-right"
        toastOptions={{
          duration: 3000,
          style: {
            background: '#1e293b',
            color: '#e2e8f0',
            border: '1px solid #475569',
          },
        }}
      />
    </div>
  );
}
