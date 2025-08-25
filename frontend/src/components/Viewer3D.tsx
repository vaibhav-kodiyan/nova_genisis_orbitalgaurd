import { useMemo, useRef } from 'react';
import { Canvas, useFrame, useLoader } from '@react-three/fiber';
import { OrbitControls, Stars } from '@react-three/drei';
import { TextureLoader, BufferGeometry, Float32BufferAttribute, LineBasicMaterial, Line } from 'three';
import { useAppStore } from '../state/appStore';
import { Track, StateECI } from '../domain/types';

// Earth component with texture
function Earth() {
  const earthTexture = useLoader(TextureLoader, 'https://unpkg.com/three-globe/example/img/earth-blue-marble.jpg');
  
  return (
    <mesh>
      <sphereGeometry args={[6.371, 64, 64]} />
      <meshStandardMaterial map={earthTexture} />
    </mesh>
  );
}

// Orbit trail component
function OrbitTrail({ states, color }: { states: StateECI[]; color: string }) {
  const points = useMemo(() => {
    const positions = new Float32Array(states.length * 3);
    states.forEach((state, i) => {
      positions[i * 3] = state.r[0];
      positions[i * 3 + 1] = state.r[1];
      positions[i * 3 + 2] = state.r[2];
    });
    return positions;
  }, [states]);

  const geometry = useMemo(() => {
    const geom = new BufferGeometry();
    geom.setAttribute('position', new Float32BufferAttribute(points, 3));
    return geom;
  }, [points]);

  const material = useMemo(() => new LineBasicMaterial({ color }), [color]);

  return <primitive object={new Line(geometry, material)} />;
}

// Satellite/debris dots
function SpaceObjects({ tracks, currentTime, focusedPair }: { 
  tracks: Track[]; 
  currentTime: number;
  focusedPair: { aId: string; bId: string } | null;
}) {
  const getCurrentState = (track: Track): StateECI | null => {
    if (track.states.length === 0) return null;
    
    // Find closest time state
    let closestIndex = 0;
    let minDiff = Math.abs(track.states[0].t - currentTime);
    
    for (let i = 1; i < track.states.length; i++) {
      const diff = Math.abs(track.states[i].t - currentTime);
      if (diff < minDiff) {
        minDiff = diff;
        closestIndex = i;
      }
    }
    
    return track.states[closestIndex];
  };

  return (
    <group>
      {tracks.map((track) => {
        const state = getCurrentState(track);
        if (!state) return null;
        
        const isFocused = focusedPair && (focusedPair.aId === track.id || focusedPair.bId === track.id);
        const color = track.kind === 'satellite' ? '#22c55e' : '#ef4444'; // green for satellites, red for debris
        const size = isFocused ? 0.15 : 0.08;
        
        return (
          <group key={track.id}>
            {/* Orbit trail */}
            <OrbitTrail 
              states={track.states} 
              color={isFocused ? '#ffffff' : '#ffffff'} 
            />
            
            {/* Current position dot */}
            <mesh position={[state.r[0], state.r[1], state.r[2]]}>
              <sphereGeometry args={[size, 16, 16]} />
              <meshStandardMaterial 
                color={color}
                emissive={color}
                emissiveIntensity={0.3}
              />
            </mesh>
          </group>
        );
      })}
    </group>
  );
}

// Camera controller for focused pairs
function CameraController({ focusedPair, tracks, paused }: {
  focusedPair: { aId: string; bId: string } | null;
  tracks: Track[];
  paused: boolean;
}) {
  const controlsRef = useRef<any>();
  
  useFrame(() => {
    if (paused) return; // Skip animation when paused
    
    if (focusedPair && controlsRef.current) {
      const trackA = tracks.find(t => t.id === focusedPair.aId);
      const trackB = tracks.find(t => t.id === focusedPair.bId);
      
      if (trackA && trackB && trackA.states.length > 0 && trackB.states.length > 0) {
        // Focus on midpoint between the two objects
        const stateA = trackA.states[0]; // Use first state for now
        const stateB = trackB.states[0];
        
        const midpoint = [
          (stateA.r[0] + stateB.r[0]) / 2,
          (stateA.r[1] + stateB.r[1]) / 2,
          (stateA.r[2] + stateB.r[2]) / 2
        ];
        
        controlsRef.current.target.set(midpoint[0], midpoint[1], midpoint[2]);
      }
    }
  });
  
  return <OrbitControls ref={controlsRef} enableDamping dampingFactor={0.1} />;
}

interface Viewer3DProps {
  paused?: boolean;
}

export function Viewer3D({ paused = false }: Viewer3DProps) {
  const { tracks, playbackTime, focusedPair } = useAppStore();
  
  return (
    <div className="w-full h-full bg-black">
      <Canvas camera={{ position: [0, 0, 50], near: 0.1, far: 1000 }}>
        {/* Starfield background */}
        <Stars radius={300} depth={60} count={20000} factor={7} saturation={0} fade />
        
        {/* Lighting */}
        <ambientLight intensity={0.3} />
        <directionalLight position={[10, 10, 10]} intensity={1} />
        
        {/* Earth */}
        <Earth />
        
        {/* Space objects and orbits */}
        <SpaceObjects 
          tracks={tracks} 
          currentTime={playbackTime}
          focusedPair={focusedPair}
        />
        
        {/* Camera controls */}
        <CameraController focusedPair={focusedPair} tracks={tracks} paused={paused} />
      </Canvas>
    </div>
  );
}
