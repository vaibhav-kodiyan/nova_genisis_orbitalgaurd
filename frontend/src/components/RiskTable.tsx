import { useState, useMemo } from 'react';
import { useAppStore } from '../state/appStore';
import { Encounter, Severity } from '../domain/types';

type SortField = 'missMeters' | 'tcaUtc' | 'relSpeedMps' | 'pcProxy';
type SortDirection = 'asc' | 'desc';

export function RiskTable() {
  const { state, analysisResult, setFocusedPair, focusedPair } = useAppStore();
  const [activeTab, setActiveTab] = useState<'All' | Severity>('All');
  const [sortField, setSortField] = useState<SortField>('missMeters');
  const [sortDirection, setSortDirection] = useState<SortDirection>('asc');

  const isVisible = state === 'Analysed' && analysisResult;

  // Filter encounters by severity
  const filteredEncounters = useMemo(() => {
    if (!analysisResult) return [];
    
    if (activeTab === 'All') {
      return analysisResult.encounters;
    }
    
    return analysisResult.encounters.filter(e => e.severity === activeTab);
  }, [analysisResult, activeTab]);

  // Sort encounters
  const sortedEncounters = useMemo(() => {
    return [...filteredEncounters].sort((a, b) => {
      let aVal: number, bVal: number;
      
      switch (sortField) {
        case 'missMeters':
          aVal = a.missMeters;
          bVal = b.missMeters;
          break;
        case 'tcaUtc':
          aVal = a.tcaUtc;
          bVal = b.tcaUtc;
          break;
        case 'relSpeedMps':
          aVal = a.relSpeedMps;
          bVal = b.relSpeedMps;
          break;
        case 'pcProxy':
          aVal = a.pcProxy;
          bVal = b.pcProxy;
          break;
        default:
          return 0;
      }
      
      const result = aVal - bVal;
      return sortDirection === 'asc' ? result : -result;
    });
  }, [filteredEncounters, sortField, sortDirection]);

  // Count encounters by severity
  const counts = useMemo(() => {
    if (!analysisResult) return { All: 0, High: 0, Medium: 0, Low: 0 };
    
    const high = analysisResult.encounters.filter(e => e.severity === 'High').length;
    const medium = analysisResult.encounters.filter(e => e.severity === 'Medium').length;
    const low = analysisResult.encounters.filter(e => e.severity === 'Low').length;
    
    return {
      All: analysisResult.encounters.length,
      High: high,
      Medium: medium,
      Low: low
    };
  }, [analysisResult]);

  const handleSort = (field: SortField) => {
    if (sortField === field) {
      setSortDirection(sortDirection === 'asc' ? 'desc' : 'asc');
    } else {
      setSortField(field);
      setSortDirection('asc');
    }
  };

  const handleRowClick = (encounter: Encounter) => {
    const newFocusedPair = { aId: encounter.aId, bId: encounter.bId };
    
    // Toggle focus if clicking the same pair
    if (focusedPair && focusedPair.aId === newFocusedPair.aId && focusedPair.bId === newFocusedPair.bId) {
      setFocusedPair(null);
    } else {
      setFocusedPair(newFocusedPair);
    }
  };

  const getSeverityColor = (severity: Severity) => {
    switch (severity) {
      case 'High': return 'text-red-600 bg-red-50';
      case 'Medium': return 'text-yellow-600 bg-yellow-50';
      case 'Low': return 'text-green-600 bg-green-50';
    }
  };

  const SortIcon = ({ field }: { field: SortField }) => {
    if (sortField !== field) {
      return (
        <svg className="w-4 h-4 text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M8 9l4-4 4 4m0 6l-4 4-4-4" />
        </svg>
      );
    }
    
    return sortDirection === 'asc' ? (
      <svg className="w-4 h-4 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 15l7-7 7 7" />
      </svg>
    ) : (
      <svg className="w-4 h-4 text-blue-600" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M19 9l-7 7-7-7" />
      </svg>
    );
  };

  if (!isVisible) {
    return (
      <div className="h-full bg-white border-t border-gray-200 flex items-center justify-center">
        <div className="text-center">
          <div className="text-gray-500 text-lg mb-2">Risk Analysis</div>
          <div className="text-gray-400 text-sm">
            Playback controls will be available after running analysis.
          </div>
        </div>
      </div>
    );
  }

  return (
    <div className="h-full bg-white flex flex-col">
      {/* Tabs */}
      <div className="border-b border-gray-200">
        <nav className="flex space-x-8 px-6">
          {(['All', 'High', 'Medium', 'Low'] as const).map((tab) => (
            <button
              key={tab}
              onClick={() => setActiveTab(tab)}
              className={`py-4 px-1 border-b-2 font-medium text-sm ${
                activeTab === tab
                  ? 'border-blue-500 text-blue-600'
                  : 'border-transparent text-gray-500 hover:text-gray-700 hover:border-gray-300'
              }`}
            >
              {tab} ({counts[tab]})
            </button>
          ))}
        </nav>
      </div>

      {/* Table */}
      <div className="flex-1 overflow-auto">
        <table className="min-w-full divide-y divide-gray-200">
          <thead className="bg-gray-50 sticky top-0">
            <tr>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                Pair
              </th>
              <th 
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
                onClick={() => handleSort('tcaUtc')}
              >
                <div className="flex items-center space-x-1">
                  <span>TCA (UTC)</span>
                  <SortIcon field="tcaUtc" />
                </div>
              </th>
              <th 
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
                onClick={() => handleSort('missMeters')}
              >
                <div className="flex items-center space-x-1">
                  <span>Miss (m)</span>
                  <SortIcon field="missMeters" />
                </div>
              </th>
              <th 
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
                onClick={() => handleSort('relSpeedMps')}
              >
                <div className="flex items-center space-x-1">
                  <span>Rel Speed (m/s)</span>
                  <SortIcon field="relSpeedMps" />
                </div>
              </th>
              <th 
                className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider cursor-pointer hover:bg-gray-100"
                onClick={() => handleSort('pcProxy')}
              >
                <div className="flex items-center space-x-1">
                  <span>Pc Proxy Risk</span>
                  <SortIcon field="pcProxy" />
                </div>
              </th>
              <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase tracking-wider">
                Focus
              </th>
            </tr>
          </thead>
          <tbody className="bg-white divide-y divide-gray-200">
            {sortedEncounters.map((encounter) => {
              const isFocused = focusedPair && 
                focusedPair.aId === encounter.aId && 
                focusedPair.bId === encounter.bId;
              
              return (
                <tr 
                  key={`${encounter.aId}-${encounter.bId}`}
                  className={`hover:bg-gray-50 cursor-pointer ${isFocused ? 'bg-blue-50' : ''}`}
                  onClick={() => handleRowClick(encounter)}
                >
                  <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                    <div className="flex flex-col">
                      <span className="font-medium">{encounter.aId}</span>
                      <span className="text-gray-500">{encounter.bId}</span>
                    </div>
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                    {new Date(encounter.tcaUtc).toISOString().replace('T', ' ').substr(0, 19)}
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                    {encounter.missMeters.toFixed(1)}
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                    {encounter.relSpeedMps.toFixed(1)}
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                    <div className="flex items-center space-x-2">
                      <span>{encounter.pcProxy.toFixed(6)}</span>
                      <span className={`px-2 py-1 text-xs font-medium rounded-full ${getSeverityColor(encounter.severity)}`}>
                        {encounter.severity}
                      </span>
                    </div>
                  </td>
                  <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-900">
                    <button
                      className={`px-3 py-1 text-xs font-medium rounded-md ${
                        isFocused 
                          ? 'bg-blue-600 text-white' 
                          : 'bg-gray-100 text-gray-700 hover:bg-gray-200'
                      }`}
                    >
                      {isFocused ? 'Focused' : 'Focus'}
                    </button>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>
    </div>
  );
}
