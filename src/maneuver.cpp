#include "maneuver.h"
#include "propagation.h"
#include <cstring>
#include <cstdio>
#include <cmath>

// Standard gravitational acceleration (m/s²)
const double G0 = 9.80665;

// Keep math inline for clarity

double fuel_consumption(double delta_v_km_s, double specific_impulse_s,
                       double dry_mass_kg, double propellant_mass_kg, double efficiency) {
    if (specific_impulse_s <= 0.0) return -1.0;
    if (efficiency <= 0.0) efficiency = 1.0;
    const double dv = delta_v_km_s * 1000.0 / efficiency; // m/s
    const double m0 = dry_mass_kg + propellant_mass_kg;
    const double m1 = m0 * exp(-dv / (specific_impulse_s * G0));
    double fuel = m0 - m1;
    if (fuel < 0.0) fuel = 0.0;
    if (fuel > propellant_mass_kg) fuel = propellant_mass_kg;
    return fuel;
}

// Beginner-friendly: delta_v in m/s, Isp in s, initial mass in kg
double fuel_required_simple(double delta_v_mps, double specific_impulse_s, double initial_mass_kg) {
    if (specific_impulse_s <= 0.0 || initial_mass_kg <= 0.0) return -1.0;
    const double exponent = -delta_v_mps / (specific_impulse_s * G0);
    const double final_mass = initial_mass_kg * exp(exponent);
    const double prop_mass = initial_mass_kg - final_mass;
    return (prop_mass < 0.0) ? 0.0 : prop_mass;
}

int plan_avoidance(const OrbitalElements* primary, const OrbitalElements* secondary,
                  double encounter_time, double target_distance_km, 
                  double max_delta_v_mps, Maneuver* out_maneuver) {
    if (!primary || !out_maneuver) return -1;
    (void)secondary; // unused
    memset(out_maneuver, 0, sizeof(Maneuver));
    double dt_sec = (encounter_time - primary->epoch) * SECONDS_PER_DAY;
    if (dt_sec <= 0.0) return -1;
    double dv_ms = (target_distance_km * 1000.0) / dt_sec;
    if (dv_ms < 0.0) dv_ms = 0.0;
    if (dv_ms > max_delta_v_mps) return -1;
    out_maneuver->delta_v[0] = dv_ms; // fixed +X direction
    out_maneuver->delta_v[1] = 0.0;
    out_maneuver->delta_v[2] = 0.0;
    out_maneuver->epoch = encounter_time;
    long long epoch_int = (long long)(encounter_time * 1000000);
    snprintf(out_maneuver->id, sizeof(out_maneuver->id), "AVOID_%lld", epoch_int);
    out_maneuver->fuel_cost = -1.0; // unknown; keep basic
    return 0;
}

void apply_maneuver(const OrbitalElements* elements, const Maneuver* m,
                   double current_unix_ms, StateVectorECI* out_state) {
    (void)current_unix_ms; // unused; repo uses Julian dates
    if (!elements || !m || !out_state) return;
    memset(out_state, 0, sizeof(StateVectorECI));
    double maneuver_time = m->epoch;
    double minutes_since_epoch = (maneuver_time - elements->epoch) * MINUTES_PER_DAY;
    StateVectorECI pre;
    if (propagate(elements, minutes_since_epoch, &pre) != PROPAGATION_SUCCESS) return;
    out_state->t = maneuver_time;
    out_state->r[0] = pre.r[0]; out_state->r[1] = pre.r[1]; out_state->r[2] = pre.r[2];
    out_state->v[0] = pre.v[0] + m->delta_v[0]/1000.0;
    out_state->v[1] = pre.v[1] + m->delta_v[1]/1000.0;
    out_state->v[2] = pre.v[2] + m->delta_v[2]/1000.0;
}

double isp_default_chemical() { return 300.0; }
double isp_electric() { return 3000.0; }
