/*
 * Copyright (C) 2024 Davide Zamblera
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "InflowModel.hpp"
#include <cmath>

using namespace simulation;

InflowState HarmonicInflowModel::computeState(const PitchState &pitchState, const FlappingState &flappingState, const BladeForceState &aeroForcesState, const BodyState &bodyState, double Omega)
{
    double a0 = rotor->blade.a0;
    double R = rotor->blade.R;
    double s = rotor->solidity();
    double lmd_0 = computeUniformInflow(inflowState._0(), aeroForcesState.C_T, bodyState.mu(Omega, R), bodyState.mu_z(Omega, R));
    std::vector lmd_harmonic = computeHarmonicsInflow(pitchState, flappingState, bodyState, Omega, lmd_0);
    double lmd_1c = lmd_harmonic[0];
    double lmd_1s = lmd_harmonic[1];

    if (std::isnan(lmd_0))
    {
        lmd_0 = 0;
    }
    if (std::isnan(lmd_1c))
    {
        lmd_1c = 0;
    }
    if (std::isnan(lmd_1s))
    {
        lmd_1s = 0;
    }
    return InflowState(lmd_0, lmd_1c, lmd_1s);
}

double HarmonicInflowModel::computeUniformInflow(double lmd_0, double C_T, double mu, double mu_z)
{
    double f = 0.6;
    double tol = 0.001;
    

    double a0 = rotor->blade.a0;
    double s = rotor->solidity();
    double lmdPrev = lmd_0;
    double lmd = lmd_0;
    do {
        double Lambda = std::pow(mu,2) + std::pow(lmdPrev - mu_z,2);
        double h = -((2*lmdPrev*std::sqrt(Lambda) - C_T)*Lambda)/(2*std::pow(Lambda,1.5) + a0*s/4.0*Lambda - C_T*(mu_z - lmdPrev));
        double tmp = lmdPrev + f*h;
        lmdPrev = lmd;
        lmd = tmp;

    } while (std::abs(lmd - lmdPrev) > tol);
    return lmd;
}


std::vector<double> HarmonicInflowModel::computeHarmonicsInflow(const PitchState& pitchState, const FlappingState &flappingState, const BodyState& bodyState, double Omega, double lmd_0)
{
    double a0 = rotor->blade.a0;
    double s = rotor->solidity();
    double psi = bodyState.psi_w();
    double th_tw = rotor->blade.th_tw;
    double R = rotor->blade.R;

    double sin = std::sin(psi);
    double cos = std::cos(psi);

    double beta_0 = flappingState._0();
    double beta_1s = flappingState._1s();
    double beta_1c = flappingState._1c();

    double th_0 =   pitchState._0();
    double th_1s = pitchState._1s();
    double th_1c = pitchState._1c();

    double mu = bodyState.mu(Omega, R);
    double mu_z = bodyState.mu_z(Omega, R);

    double qBar = bodyState.omBar_H(Omega).Y();
    double pBar = bodyState.omBar_H(Omega).X();

    double C_prime = 1/(1 + a0*s/(16*lmd_0));
    double lmd_1c = C_prime*(a0*s)/(16*lmd_0)*(th_1c - beta_1s + qBar);
    double lmd_1s = C_prime*(a0*s)/(16*lmd_0)*(th_1s + beta_1c + pBar);

    std::vector<double> sol = {lmd_1c, lmd_1s};
    return sol;
}
