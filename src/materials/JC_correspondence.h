//! \file JC_correspondence.h

//@HEADER
// ************************************************************************
//
//                             Peridigm
//                 Copyright (2011) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions?
// David J. Littlewood   djlittl@sandia.gov
// John A. Mitchell      jamitch@sandia.gov
// Michael L. Parks      mlparks@sandia.gov
// Stewart A. Silling    sasilli@sandia.gov
//
// ************************************************************************
//@HEADER
#ifndef JC_CORRESPONDENCE_H
#define JC_CORRESPONDENCE_H

namespace CORRESPONDENCE {

template<typename ScalarT>
void updateJohnsonCookCauchyStress
(
const ScalarT* unrotatedRateOfDeformation, 
const ScalarT* cauchyStressN, 
ScalarT* cauchyStressNP1, 
const ScalarT* vonMisesStressN,
ScalarT* vonMisesStressNP1,
const ScalarT* equivalentPlasticStrainN,
const ScalarT* accumulatedPlasticStrainN,
const ScalarT* DamageN,
const ScalarT* Dot_equivalentPlasticStrainN,
const ScalarT* Dot_accumulatedPlasticStrainN,
const ScalarT* Dot_DamageN,
ScalarT* equivalentPlasticStrainNP1,
ScalarT* accumulatedPlasticStrainNP1,
ScalarT* DamageNP1,
ScalarT* Dot_equivalentPlasticStrainNP1,
ScalarT* Dot_accumulatedPlasticStrainNP1,
ScalarT* Dot_DamageNP1,
const int numPoints, 
const double bulkMod,
const double shearMod,
const double dt,
const ScalarT* TemperatureN,
ScalarT* TemperatureNP1,
const double MeltingTemperature,
const double ReferenceTemperature,
const double constA,
const double constN,
const double constB,
const double constC,
const double constM,
const double constD1,
const double constD2,
const double constD3,
const double constD4,
const double constD5
);
}

#endif // ELASTIC_PLASTIC_CORRESPONDENCE_H
