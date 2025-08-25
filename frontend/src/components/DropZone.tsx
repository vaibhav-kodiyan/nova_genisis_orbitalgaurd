import React, { useRef, useState } from 'react';

interface DropZoneProps {
  title: string;
  onFileLoad: (file: File) => void;
  onLoadSample: () => void;
  fileCount: number;
  accept?: string;
}

export function DropZone({ title, onFileLoad, onLoadSample, fileCount, accept = ".tle,.txt" }: DropZoneProps) {
  const fileInputRef = useRef<HTMLInputElement>(null);
  const [isDragOver, setIsDragOver] = useState(false);
  const [fileName, setFileName] = useState<string>('');

  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    setIsDragOver(false);
    
    const files = Array.from(e.dataTransfer.files);
    if (files.length > 0) {
      const file = files[0];
      setFileName(file.name);
      onFileLoad(file);
    }
  };

  const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (file) {
      setFileName(file.name);
      onFileLoad(file);
    }
  };

  const handleClick = () => {
    fileInputRef.current?.click();
  };

  return (
    <div className="rounded-2xl border border-slate-700/60 bg-slate-900/70 p-4 mb-4 shadow">
      <div className="flex items-center justify-between mb-3">
        <h3 className="text-slate-200 font-medium">{title}</h3>
        <span className="text-slate-400 text-sm font-mono">{fileCount}</span>
      </div>
      
      <input
        ref={fileInputRef}
        type="file"
        accept={accept}
        className="hidden"
        onChange={handleFileSelect}
      />
      
      <div
        onClick={handleClick}
        onDrop={handleDrop}
        onDragOver={(e) => {
          e.preventDefault();
          setIsDragOver(true);
        }}
        onDragLeave={() => setIsDragOver(false)}
        className={`border-2 border-dashed rounded-xl p-6 hover:border-emerald-400 transition cursor-pointer ${
          isDragOver ? 'border-emerald-400 bg-emerald-400/5' : 'border-slate-600'
        }`}
      >
        <div className="text-center">
          <div className="text-slate-300 text-sm mb-2">
            Click to upload {accept} or drag & drop
          </div>
          {fileName && (
            <div className="text-emerald-400 text-xs mb-2 font-mono">
              {fileName}
            </div>
          )}
        </div>
      </div>
      
      <button
        onClick={onLoadSample}
        className="w-full mt-3 px-3 py-2 text-sm text-slate-300 border border-slate-600 rounded-xl hover:border-slate-500 hover:bg-slate-800/50 transition"
      >
        Load Sample
      </button>
    </div>
  );
}
