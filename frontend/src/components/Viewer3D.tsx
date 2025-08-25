import { useMemo, useRef, useEffect } from 'react';
import { Canvas, useFrame, useLoader, useThree } from '@react-three/fiber';
import { OrbitControls, Stars } from '@react-three/drei';
import { TextureLoader, Raycaster } from 'three';
import * as THREE from 'three';
import { useAppStore } from '../state/appStore';
import { Track } from '../domain/types';
import PlaybackBar from './PlaybackBar';

// Get store state fresh each frame to avoid stale closures
const get = () => useAppStore.getState();

// Phase A - Telemetry tags
const PLAY_TAG = "[PLAY]";
const SAMPLE_TAG = "[SAMPLE]";
const PATH_TAG = "[PATH]";

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


// Phase A - Telemetry that can't lie
let lastStamp = -1;
function telem(t: number, speed: number, isPlaying: boolean) {
  const s = Math.floor(t);
  if (s !== lastStamp) {
    lastStamp = s;
    console.log(PLAY_TAG, { t: +t.toFixed(2), speed, isPlaying });
  }
}

// Phase B - Single source of truth: positionAt(id, tSec)
type State = { r: [number, number, number] };

const MU = 398600.4418; // km^3/s^2

// Provide these accessors based on your store:
function tleEpochSecFor(_id: string): number | undefined { 
  // Placeholder - would need TLE epoch mapping in store
  return undefined; 
}
function ephemFor(_id: string): { tSec: number, r: [number, number, number] }[] | undefined { 
  // Placeholder - would need ephemeris data in store
  return undefined; 
}

function sampleEphemeris(e: { tSec: number, r: [number, number, number] }[], t: number): State {
  let lo = 0, hi = e.length - 1;
  if (t <= e[0].tSec) return { r: e[0].r };
  if (t >= e[hi].tSec) return { r: e[hi].r };
  while (hi - lo > 1) { const m = (lo + hi >> 1); (e[m].tSec < t ? lo = m : hi = m); }
  const a = e[lo], b = e[hi], u = (t - a.tSec) / (b.tSec - a.tSec);
  return {
    r: [
      a.r[0] + u * (b.r[0] - a.r[0]),
      a.r[1] + u * (b.r[1] - a.r[1]),
      a.r[2] + u * (b.r[2] - a.r[2])
    ]
  };
}

// ✅ Expect a wrapper you already have that takes minutes-since-epoch
function sgp4StateAtMinutes(_id: string, _tsinceMin: number): State | null {
  // Placeholder - would integrate with actual SGP4 library
  return null;
}

function circularFallback(id: string, t: number): State | null {
  // Find track by ID to get initial state for circular fallback
  const track = get().tracks.find(t => t.id === id);
  if (!track?.states?.[0]) return null;
  
  const state0 = track.states[0];
  const r0 = state0.r;
  const v0 = state0.v;
  const R = Math.hypot(r0[0], r0[1], r0[2]);
  const omega = Math.sqrt(MU / (R * R * R)); // rad/s
  
  // Build an orthonormal basis in the orbital plane
  const u0 = new THREE.Vector3().fromArray(r0).normalize();
  const w = v0 ? new THREE.Vector3().fromArray(v0).cross(u0).normalize()
               : new THREE.Vector3(0, 1, 0);
  const v0u = new THREE.Vector3().copy(w).cross(u0).normalize();
  const ang = omega * t;
  const r = new THREE.Vector3()
    .addScaledVector(u0, Math.cos(ang) * R)
    .addScaledVector(v0u, Math.sin(ang) * R);
  return { r: [r.x, r.y, r.z] };
}

function positionAt(id: string, t: number): State | null {
  const eph = ephemFor(id);
  if (eph && eph.length > 1) {
    console.log(PATH_TAG, id, "ephemeris");
    return sampleEphemeris(eph, t);
  }
  const tleEpoch = tleEpochSecFor(id);
  if (tleEpoch !== undefined) {
    // Choose a visual epoch; default to tleEpoch
    const visualStart = 0; // Placeholder for visualStartEpochSec
    const tsinceMin = (visualStart + t - tleEpoch) / 60.0;
    const st = sgp4StateAtMinutes?.(id, tsinceMin) ?? null;
    console.log(PATH_TAG, id, "sgp4", { tsinceMin: +tsinceMin.toFixed(3) });
    if (st) return st;
  }
  const fb = circularFallback(id, t);
  if (fb) { console.log(PATH_TAG, id, "fallback"); return fb; }
  return null;
}

// Legacy function for backward compatibility with existing tracks
function stateAt(track: Track, t: number): State | null {
  if (track.states.length === 0) return null;
  
  // Use existing states as ephemeris data
  const ephem = track.states.map(s => ({ tSec: s.t, r: s.r as [number, number, number] }));
  if (ephem.length > 1) {
    console.log(PATH_TAG, track.id, "ephemeris");
    return sampleEphemeris(ephem, t);
  }
  return { r: ephem[0]?.r || [0, 0, 0] };
}

// Phase C - RAF uses fresh store each frame; updates instances with positionAt
let rafId: number;
let last = performance.now();

function startRAFLoop(updateInstances: (t: number) => void) {
  function tick(now: number) {
    const dt = (now - last) / 1000;
    last = now;
    
    // Pull fresh state EACH FRAME
    const { isPlaying, speed, t, tMax, setT } = get();
    const tNext = isPlaying ? Math.min(t + dt * speed, tMax) : t;
    
    if (tNext !== t) setT(tNext);
    
    // Update visuals for current time
    updateInstances(tNext);
    
    telem(tNext, speed, isPlaying);
    
    rafId = requestAnimationFrame(tick);
  }
  rafId = requestAnimationFrame(tick);
}

function stopRAFLoop() {
  if (rafId) cancelAnimationFrame(rafId);
}

// Optimized space objects with instanced rendering
function SpaceObjects() {
  const { tracks, selectedId, setSelectedId } = useAppStore();
  const { camera, gl } = useThree();
  
  const satDotsRef = useRef<THREE.InstancedMesh>(null);
  const debDotsRef = useRef<THREE.InstancedMesh>(null);
  const satOrbitsRef = useRef<THREE.Group>(null);
  const debOrbitsRef = useRef<THREE.Group>(null);
  const earthMeshRef = useRef<THREE.Mesh>(null);
  
  const satellites = useMemo(() => tracks.filter(t => t.kind === 'satellite'), [tracks]);
  const debris = useMemo(() => tracks.filter(t => t.kind === 'debris'), [tracks]);
  
  // Phase D - Acceptance testing and motion probe
  useEffect(() => {
    // Add a one-off probe after data loads
    (function probe() {
      const trackId = satellites?.[0]?.id ?? debris?.[0]?.id;
      if (!trackId) return;
      
      const p0 = positionAt(trackId, 0) ?? stateAt(satellites?.[0] ?? debris?.[0], 0);
      const p60 = positionAt(trackId, 60) ?? stateAt(satellites?.[0] ?? debris?.[0], 60);
      const p600 = positionAt(trackId, 600) ?? stateAt(satellites?.[0] ?? debris?.[0], 600);
      const d = (a?: [number, number, number], b?: [number, number, number]) => 
        a && b ? Math.hypot(a[0] - b[0], a[1] - b[1], a[2] - b[2]) : NaN;
      console.log(SAMPLE_TAG, { id: trackId, d0_60: d(p0?.r, p60?.r), d60_600: d(p60?.r, p600?.r) });
    })();
  }, [satellites, debris]);
  
  // Constants for scaling
  const EARTH_RADIUS_KM = 6378.137;
  const earthGeomRadius = 6.371; // our sphere geometry radius
  const KM_TO_UNITS = earthGeomRadius / EARTH_RADIUS_KM;
  const DOT_RADIUS_UNITS = Math.max(earthGeomRadius * 0.004, 0.01);
  
  // Create instanced meshes with proper setup
  const dotGeometry = useMemo(() => new THREE.SphereGeometry(DOT_RADIUS_UNITS, 12, 12), [DOT_RADIUS_UNITS]);
  const satMaterial = useMemo(() => new THREE.MeshBasicMaterial({ color: 0x22c55e, vertexColors: false }), []);
  const debMaterial = useMemo(() => new THREE.MeshBasicMaterial({ color: 0xef4444, vertexColors: false }), []);
  const orbitMat = useMemo(() => new THREE.LineBasicMaterial({ color: 0xffffff, transparent: true, opacity: 0.8, depthWrite: false }), []);
  
  // Helper function for orbit lines with proper scaling
  const makeCircularOrbitLine = (r0_km: THREE.Vector3, v0_km?: THREE.Vector3, segments = 192) => {
    const R = r0_km.length() * KM_TO_UNITS;
    if (!(R > 0)) return null;
    const n = v0_km && v0_km.length() > 0 ? r0_km.clone().cross(v0_km).normalize() : new THREE.Vector3(0, 1, 0);
    const u = r0_km.clone().normalize();
    const w = n.clone().cross(u).normalize();
    const pts = Array.from({ length: segments }, (_, i) => {
      const a = i / segments * Math.PI * 2;
      return new THREE.Vector3().addScaledVector(u, Math.cos(a) * R).addScaledVector(w, Math.sin(a) * R);
    });
    return new THREE.LineLoop(new THREE.BufferGeometry().setFromPoints(pts), orbitMat);
  };
  
  // Generate orbit lines when data loads
  useEffect(() => {
    if (!satOrbitsRef.current || !debOrbitsRef.current) return;
    
    // Clear existing orbits
    satOrbitsRef.current.clear();
    debOrbitsRef.current.clear();
    
    // Generate satellite orbits
    for (const s of satellites) {
      if (s.states.length > 0) {
        const state = s.states[0];
        const r0 = new THREE.Vector3(state.r[0], state.r[1], state.r[2]);
        const v0 = state.v ? new THREE.Vector3(state.v[0], state.v[1], state.v[2]) : undefined;
        const L = makeCircularOrbitLine(r0, v0);
        if (L) satOrbitsRef.current.add(L);
      }
    }
    
    // Generate debris orbits
    for (const b of debris) {
      if (b.states.length > 0) {
        const state = b.states[0];
        const r0 = new THREE.Vector3(state.r[0], state.r[1], state.r[2]);
        const v0 = state.v ? new THREE.Vector3(state.v[0], state.v[1], state.v[2]) : undefined;
        const L = makeCircularOrbitLine(r0, v0);
        if (L) debOrbitsRef.current.add(L);
      }
    }
    
    console.log("[VIS] orbits", { sat: satOrbitsRef.current.children.length, deb: debOrbitsRef.current.children.length });
  }, [satellites, debris, KM_TO_UNITS, orbitMat]);
  
  // Phase C - RAF uses fresh store each frame; updates instances with positionAt
  const updateInstances = (t: number) => {
    if (!satDotsRef.current || !debDotsRef.current) return;
    
    // Make sure you don't capture stale state. Use the store getter:
    const dummy = new THREE.Object3D();
    
    // Use existing tracks from store - fallback to current arrays
    const currentSatellites = satellites;
    const currentDebris = debris;
    
    // Update satellite positions
    let i = 0;
    for (const s of currentSatellites) {
      const st = positionAt(s.id, t) ?? stateAt(s, t); // fallback to legacy
      if (!st?.r) continue;
      dummy.position.set(st.r[0] * KM_TO_UNITS, st.r[1] * KM_TO_UNITS, st.r[2] * KM_TO_UNITS);
      if (get().selectedId === s.id) {
        dummy.scale.setScalar(1.6);
      } else {
        dummy.scale.setScalar(1);
      }
      dummy.updateMatrix();
      satDotsRef.current.setMatrixAt(i++, dummy.matrix);
    }
    satDotsRef.current.count = i;
    satDotsRef.current.instanceMatrix.needsUpdate = true;
    
    // Update debris positions
    i = 0;
    for (const d of currentDebris) {
      const st = positionAt(d.id, t) ?? stateAt(d, t); // fallback to legacy
      if (!st?.r) continue;
      dummy.position.set(st.r[0] * KM_TO_UNITS, st.r[1] * KM_TO_UNITS, st.r[2] * KM_TO_UNITS);
      if (get().selectedId === d.id) {
        dummy.scale.setScalar(1.6);
      } else {
        dummy.scale.setScalar(1);
      }
      dummy.updateMatrix();
      debDotsRef.current.setMatrixAt(i++, dummy.matrix);
    }
    debDotsRef.current.count = i;
    debDotsRef.current.instanceMatrix.needsUpdate = true;
  };
  
  // Start RAF loop when component mounts
  useEffect(() => {
    startRAFLoop(updateInstances);
    return () => stopRAFLoop();
  }, [satellites, debris]);
  
  // Handle selection
  const handleClick = (event: any) => {
    event.stopPropagation();
    const raycaster = new Raycaster();
    const mouse = new THREE.Vector2(
      (event.clientX / gl.domElement.clientWidth) * 2 - 1,
      -(event.clientY / gl.domElement.clientHeight) * 2 + 1
    );
    
    raycaster.setFromCamera(mouse, camera);
    const meshes = [satDotsRef.current, debDotsRef.current].filter(Boolean) as THREE.Mesh[];
    const intersects = raycaster.intersectObjects(meshes);
    
    if (intersects.length > 0) {
      // Simple selection toggle for now
      setSelectedId(selectedId ? null : tracks[0]?.id || null);
    } else {
      setSelectedId(null);
    }
  };
  
  useEffect(() => {
    gl.domElement.addEventListener('click', handleClick);
    return () => gl.domElement.removeEventListener('click', handleClick);
  }, [gl, camera, selectedId, setSelectedId, tracks]);
  
  // Setup instanced meshes with proper configuration
  useEffect(() => {
    if (satDotsRef.current) {
      satDotsRef.current.count = satellites.length;
      satDotsRef.current.frustumCulled = false;
    }
    if (debDotsRef.current) {
      debDotsRef.current.count = debris.length;
      debDotsRef.current.frustumCulled = false;
    }
    if (satOrbitsRef.current) {
      satOrbitsRef.current.renderOrder = 2;
      satOrbitsRef.current.frustumCulled = false;
    }
    if (debOrbitsRef.current) {
      debOrbitsRef.current.renderOrder = 2;
      debOrbitsRef.current.frustumCulled = false;
    }
  }, [satellites.length, debris.length]);
  
  return (
    <group>
      {/* Earth mesh ref for scaling calculations */}
      <mesh ref={earthMeshRef} visible={false}>
        <sphereGeometry args={[6.371, 64, 64]} />
      </mesh>
      
      {/* Satellite dots */}
      <instancedMesh
        ref={satDotsRef}
        args={[dotGeometry, satMaterial, satellites.length]}
        visible={satellites.length > 0}
      />
      
      {/* Debris dots */}
      <instancedMesh
        ref={debDotsRef}
        args={[dotGeometry, debMaterial, debris.length]}
        visible={debris.length > 0}
      />
      
      {/* Satellite orbits */}
      <group
        ref={satOrbitsRef}
        visible={satellites.length > 0}
      />
      
      {/* Debris orbits */}
      <group
        ref={debOrbitsRef}
        visible={debris.length > 0}
      />
    </group>
  );
}

// Playback loop controller (simplified - RAF handles timing now)
function PlaybackController() {
  const { tracks, tMax, setTMax, setT } = useAppStore();
  
  // Calculate tMax from tracks
  useEffect(() => {
    if (tracks.length === 0) return;
    
    let max = -Infinity;
    tracks.forEach(track => {
      track.states.forEach(state => {
        max = Math.max(max, state.t);
      });
    });
    
    // Update tMax in store if needed
    if (max > 0 && tMax !== max) {
      setTMax(max);
      setT(0); // Reset to start
    }
  }, [tracks, tMax, setT, setTMax]);
  
  return null;
}

// Camera controller
function CameraController({ focusedPair, tracks, paused }: {
  focusedPair: { aId: string; bId: string } | null;
  tracks: Track[];
  paused: boolean;
}) {
  const controlsRef = useRef<any>();
  
  useFrame(() => {
    if (paused) return;
    
    if (focusedPair && controlsRef.current) {
      const trackA = tracks.find(t => t.id === focusedPair.aId);
      const trackB = tracks.find(t => t.id === focusedPair.bId);
      
      if (trackA && trackB && trackA.states.length > 0 && trackB.states.length > 0) {
        const stateA = trackA.states[0];
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
  const { focusedPair, tracks } = useAppStore();
  
  return (
    <div className="relative rounded-2xl overflow-hidden min-h-[360px] w-full h-full bg-black">
      <Canvas camera={{ position: [0, 0, 50], near: 0.1, far: 1000 }}>
        {/* Starfield background */}
        <Stars radius={300} depth={60} count={20000} factor={7} saturation={0} fade />
        
        {/* Lighting */}
        <ambientLight intensity={0.3} />
        <directionalLight position={[10, 10, 10]} intensity={1} />
        
        {/* Earth */}
        <Earth />
        
        {/* Space objects and orbits */}
        <SpaceObjects />
        
        {/* Playback controller */}
        <PlaybackController />
        
        {/* Camera controls */}
        <CameraController focusedPair={focusedPair} tracks={tracks} paused={paused} />
      </Canvas>
      
      {/* Playback Bar */}
      <PlaybackBar />
    </div>
  );
}
