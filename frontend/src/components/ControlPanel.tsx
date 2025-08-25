import { useAppStore } from '../state/appStore';
import { wasmBridge } from '../domain/wasmBridge';
import toast from 'react-hot-toast';
import { Counters } from './Counters';
import { DropZone } from './DropZone';
import { SimControls } from './SimControls';
import { Playback } from './Playback';
import { RiskAnalysis } from './RiskAnalysis';

interface ControlPanelProps {
  onRunAnalysis?: () => Promise<void>;
}

export function ControlPanel({ onRunAnalysis }: ControlPanelProps) {
  const {
    satelliteTles,
    debrisTles,
    setSatelliteTles,
    setDebrisTles
  } = useAppStore();

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

  return (
    <div className="w-[380px] bg-slate-900/50 border-r border-slate-700 p-6 flex flex-col overflow-y-auto">
      <div className="mb-6">
        <Counters />
      </div>
      
      <div className="space-y-0">
        <DropZone
          title="Satellites"
          onFileLoad={(file) => handleFileLoad(file, 'satellite')}
          onLoadSample={() => loadSampleData('satellite')}
          fileCount={satelliteTles.length}
        />
        
        <DropZone
          title="Debris"
          onFileLoad={(file) => handleFileLoad(file, 'debris')}
          onLoadSample={() => loadSampleData('debris')}
          fileCount={debrisTles.length}
        />
        
        <SimControls />
        
        <Playback />
        
        <RiskAnalysis onRunAnalysis={onRunAnalysis} />
      </div>
    </div>
  );
}
