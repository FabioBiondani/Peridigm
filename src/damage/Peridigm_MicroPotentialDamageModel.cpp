/*! \file Peridigm_MicroPotentialDamageModel.cpp */

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

#include <cmath>
#include "Peridigm_MicroPotentialDamageModel.hpp"
#include "Peridigm_Field.hpp"

using namespace std;

PeridigmNS::MicropotentialDamageModel::MicropotentialDamageModel(const Teuchos::ParameterList& params)
  : DamageModel(params), m_CritJintegral(0.0), alternativeCriterion(false), m_modelCoordinatesFieldId(-1), m_horizonFieldId(-1), m_damageFieldId(-1), m_bondDamageFieldId(-1), m_microPotentialFieldId(-1), m_specularBondPositionFieldId(-1)
{
  obj_CritJintegral.set(params,"Critical J_integral");
  m_CritJintegral= obj_CritJintegral.compute(0.0);

  if (params.isParameter("Alternative MicroPotential Criterion"))
      alternativeCriterion  = params.get<bool>("Alternative MicroPotential Criterion");


  PeridigmNS::FieldManager& fieldManager = PeridigmNS::FieldManager::self();
  m_modelCoordinatesFieldId = fieldManager.getFieldId("Model_Coordinates");
  m_horizonFieldId = fieldManager.getFieldId(PeridigmField::ELEMENT, PeridigmField::SCALAR, PeridigmField::CONSTANT, "Horizon");
  m_damageFieldId = fieldManager.getFieldId(PeridigmNS::PeridigmField::ELEMENT, PeridigmNS::PeridigmField::SCALAR, PeridigmNS::PeridigmField::TWO_STEP, "Damage");
  m_bondDamageFieldId = fieldManager.getFieldId(PeridigmNS::PeridigmField::BOND, PeridigmNS::PeridigmField::SCALAR, PeridigmNS::PeridigmField::TWO_STEP, "Bond_Damage");
  m_microPotentialFieldId = fieldManager.getFieldId(PeridigmField::BOND, PeridigmField::SCALAR, PeridigmField::TWO_STEP, "Micro-Potential");
  m_specularBondPositionFieldId = fieldManager.getFieldId(PeridigmField::BOND, PeridigmField::SCALAR, PeridigmField::CONSTANT, "Specular_Bond_Position");


  m_fieldIds.push_back(m_modelCoordinatesFieldId);
  m_fieldIds.push_back(m_horizonFieldId);
  m_fieldIds.push_back(m_damageFieldId);
  m_fieldIds.push_back(m_bondDamageFieldId);
  m_fieldIds.push_back(m_microPotentialFieldId);
  m_fieldIds.push_back(m_specularBondPositionFieldId);
}

PeridigmNS::MicropotentialDamageModel::~MicropotentialDamageModel()
{
}

void
PeridigmNS::MicropotentialDamageModel::initialize(const double dt,
                                                   const int numOwnedPoints,
                                                   const int* ownedIDs,
                                                   const int* neighborhoodList,
                                                   PeridigmNS::DataManager& dataManager) const
{
  double *damage, *bondDamage;
  dataManager.getData(m_damageFieldId, PeridigmField::STEP_NP1)->ExtractView(&damage);
  dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_NP1)->ExtractView(&bondDamage);

  // Initialize damage to zero
  int neighborhoodListIndex = 0;
  int bondIndex = 0;
  for(int iID=0 ; iID<numOwnedPoints ; ++iID ){
  	int nodeID = ownedIDs[iID];
    damage[nodeID] = 0.0;
	int numNeighbors = neighborhoodList[neighborhoodListIndex++];
    neighborhoodListIndex += numNeighbors;
	for(int iNID=0 ; iNID<numNeighbors ; ++iNID){
      bondDamage[bondIndex++] = 0.0;
	}
  }
}

void
PeridigmNS::MicropotentialDamageModel::computeDamage(const double dt,
                                                      const int numOwnedPoints,
                                                      const int* ownedIDs,
                                                      const int* neighborhoodList,
                                                      PeridigmNS::DataManager& dataManager) const
{
  double *x;
  dataManager.getData(m_modelCoordinatesFieldId, PeridigmField::STEP_NONE)->ExtractView(&x);
  double *horizon, *damageN, *damageNP1, *bondDamageN, *bondDamageNP1, *miPot, *specu;
  dataManager.getData(m_horizonFieldId, PeridigmField::STEP_NONE)->ExtractView(&horizon);
  dataManager.getData(m_damageFieldId, PeridigmField::STEP_N)->ExtractView(&damageN);
  dataManager.getData(m_damageFieldId, PeridigmField::STEP_NP1)->ExtractView(&damageNP1);
  dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_N)->ExtractView(&bondDamageN);
  dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_NP1)->ExtractView(&bondDamageNP1);
  dataManager.getData(m_microPotentialFieldId, PeridigmField::STEP_N)->ExtractView(&miPot);
  dataManager.getData(m_specularBondPositionFieldId, PeridigmField::STEP_NONE)->ExtractView(&specu);

  double trialDamage(0.0), totalDamage;
  int neighborhoodListIndex(0), bondIndex(0);
  int nodeId, numNeighbors, neighborID, iID, iNID;
  double nodeInitialX[3], initialDistance(0.0);

  // Set the bond damage to the previous value
  *(dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_NP1)) = *(dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_N));

  // Update the bond damage
  // Break bonds if the extension is greater than the critical extension

  for(iID=0 ; iID<numOwnedPoints ; ++iID){
	nodeId = ownedIDs[iID];
	nodeInitialX[0] = x[nodeId*3];
	nodeInitialX[1] = x[nodeId*3+1];
	nodeInitialX[2] = x[nodeId*3+2];
    numNeighbors = neighborhoodList[neighborhoodListIndex++];
    
    double local_horizon = *(horizon+nodeId);
    double temp = 4.0/(m_pi*local_horizon*local_horizon*local_horizon*local_horizon);
    for(iNID=0 ; iNID<numNeighbors ; ++iNID){

        if(bondDamageN[bondIndex]==1.0){
            neighborhoodListIndex += 1;
            bondIndex += 1;
            continue;
        }

        int specuID = int(specu[bondIndex]);
	    neighborID = neighborhoodList[neighborhoodListIndex];
        if(alternativeCriterion)
            initialDistance = distance(nodeInitialX[0], nodeInitialX[1], nodeInitialX[2], x[neighborID*3], x[neighborID*3+1],x[neighborID*3+2]);
        
        double m_criticalMicroPotential = temp*m_CritJintegral;
        if(alternativeCriterion)
            m_criticalMicroPotential *= 3./2. /(local_horizon*local_horizon)*initialDistance*initialDistance;
//             6.0/(m_pi*local_horizon*local_horizon*local_horizon*local_horizon*local_horizon*local_horizon)*initialDistance*initialDistance;

        double bondMicroPotential = miPot[bondIndex] ;

        trialDamage = 0.0;
        if(bondMicroPotential > m_criticalMicroPotential)
          trialDamage = 1.0;
        if(trialDamage > bondDamageNP1[bondIndex]){
          bondDamageNP1[bondIndex] = trialDamage;
          bondDamageNP1[specuID]   = trialDamage;
        }

        neighborhoodListIndex += 1;
        bondIndex += 1;
    }
  }

  //  Update the element damage (percent of bonds broken)

  neighborhoodListIndex = 0;
  bondIndex = 0;
  for(iID=0 ; iID<numOwnedPoints ; ++iID){
	nodeId = ownedIDs[iID];
	numNeighbors = neighborhoodList[neighborhoodListIndex++];
    neighborhoodListIndex += numNeighbors;
	totalDamage = 0.0;
	for(iNID=0 ; iNID<numNeighbors ; ++iNID){
	  totalDamage += bondDamageNP1[bondIndex++];
	}
	if(numNeighbors > 0)
	  totalDamage /= numNeighbors;
	else
	  totalDamage = 0.0;
 	damageNP1[nodeId] = totalDamage;
  }
}
