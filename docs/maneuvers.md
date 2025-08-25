# Maneuver System Documentation

## Overview

The maneuver system provides orbital maneuvering capabilities for spacecraft collision avoidance and trajectory modification. It implements the Tsiolkovsky rocket equation for fuel consumption calculations and provides heuristic-based avoidance planning.

## Core Functions

### `fuel_consumption()`
Calculates required propellant mass using the rocket equation:
- **Full equation**: Δv = Isp × g₀ × ln(m₀/m₁)
- **Linear approximation**: For small Δv where Δv/(Isp×g₀) < 1×10⁻³

**Parameters:**
- `delta_v_km_s`: Delta-v magnitude in km/s (converted internally to m/s)
- `specific_impulse_s`: Specific impulse in seconds (default: 300s for chemical)
- `dry_mass_kg`: Spacecraft dry mass in kg
- `propellant_mass_kg`: Available propellant mass in kg
- `efficiency`: Propulsion efficiency (0-1 scalar, defaults to 1.0 if ≤0)

**Units & Constants:**
- g₀ = 9.80665 m/s² (standard gravitational acceleration)
- Efficiency affects effective Δv: dv_eff = dv / max(ε, efficiency)

### `plan_avoidance()`
Generates collision avoidance maneuvers using simplified heuristics:
- **Displacement model**: along-track displacement ≈ Δv × Δt
- **Required Δv**: target_distance_km × 1000 / Δt (m/s)
- **Direction**: Along current velocity vector (along-track burn)
- **Execution time**: At encounter time (or slightly before for practical execution)

**Limitations:**
- Simplified linear displacement model
- No optimization for fuel efficiency
- Single-burn solution only
- Uses placeholder masses for fuel estimation

### `apply_maneuver()`
Applies instantaneous velocity changes to orbital state:
1. Propagates orbital elements to maneuver execution time
2. Adds delta-v vector to velocity (position unchanged at impulse instant)
3. Returns post-maneuver state vector in ECI coordinates

**Time Handling:**
- Repository uses Julian dates as timebase
- Maneuver epochs are in Julian date format
- Delta-v components are in ECI frame, m/s

## Default Parameters

### Specific Impulse Presets
- **Chemical propulsion**: 300 seconds (`isp_default_chemical()`)
- **Electric propulsion**: 3000 seconds (`isp_electric()`) - for future use

### Maneuver Structure
```cpp
struct Maneuver {
    char   id[32];        // Deterministic ID: "AVOID_<epoch_microseconds>"
    double epoch;         // Execution time (Julian date)
    double delta_v[3];    // ECI components, m/s
    double fuel_cost;     // Required fuel mass, kg (-1.0 if unavailable)
};
```

## Assumptions & Simplifications

### Physics Model
- **Instantaneous burns**: No finite burn time effects
- **Two-body dynamics**: No perturbations during maneuver execution
- **Impulsive maneuvers**: Delta-v applied instantaneously
- **Chemical propulsion**: Default Isp=300s, no throttling effects

### Avoidance Planning
- **Linear displacement**: Actual orbital mechanics are more complex
- **Along-track burns only**: No radial or normal components
- **Single encounter**: No multi-revolution analysis
- **Fixed execution time**: No optimization of burn timing

### Mass Properties
- **Placeholder masses**: 1000kg dry, 100kg propellant (when actual values unavailable)
- **Perfect efficiency**: Default 100% propulsive efficiency
- **No mass depletion**: Fuel consumption doesn't affect subsequent calculations

## Risks & Limitations

### Technical Risks
- **Over-simplification**: Real avoidance maneuvers require sophisticated trajectory optimization
- **Accuracy limitations**: Linear heuristics may be inadequate for close encounters
- **No collision probability**: Deterministic distance-based planning only
- **Single-burn assumption**: Multi-burn strategies often more fuel-efficient

### Implementation Risks
- **Propagation dependencies**: Relies on existing SGP4-based propagator
- **Unit consistency**: Mixed m/s and km/s units require careful conversion
- **Time synchronization**: Julian date vs Unix timestamp handling
- **Mass availability**: Fuel estimates may be meaningless without actual spacecraft data

### Operational Risks
- **No optimization**: Fuel consumption not minimized
- **No constraints**: Thrust limits, attitude constraints ignored
- **No verification**: No closed-loop validation of maneuver effectiveness
- **Deterministic IDs**: May conflict if multiple maneuvers planned simultaneously

## Future Enhancements (Backlog)

### Multi-Burn Targeting
- Hohmann transfers for fuel efficiency
- Lambert targeting for rendezvous
- Bi-propellant vs monopropellant optimization

### Advanced Planning
- Monte Carlo collision probability analysis
- Multi-revolution encounter prediction
- Optimal timing and magnitude selection
- Thruster selection and attitude planning

### Integration Features
- Real spacecraft mass tracking
- Fuel budget management
- Maneuver execution monitoring
- Closed-loop guidance corrections

## Testing Strategy

The test suite covers three main areas:
1. **Fuel computation**: Validates rocket equation implementation vs hand calculations
2. **Planning logic**: Success/failure conditions and parameter validation
3. **State application**: Proper velocity increments and propagation integration

**Tolerances:**
- Fuel calculations: 1×10⁻⁶ kg
- Velocity changes: 1×10⁻⁹ km/s
- Position preservation: 1×10⁻⁹ km
