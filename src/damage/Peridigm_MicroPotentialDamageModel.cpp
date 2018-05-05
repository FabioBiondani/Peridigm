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
  : DamageModel(params), m_Jintegral(0.0),m_materialModel(""),isCorrespondenceOrPalsMaterial(false), m_modelCoordinatesFieldId(-1), m_horizonFieldId(-1), m_damageFieldId(-1), m_bondDamageFieldId(-1), m_deltaTemperatureFieldId(-1),m_microPotentialFieldId(-1), m_specularBondPositionFieldId(-1),m_volumeRatioFieldId(-1),m_volumeFieldId(-1)
{
  obj_Jintegral.set(params,"J_integral");
  m_Jintegral= obj_Jintegral.compute(0.0);

  m_materialModel = params.get<string>("Material Model");
  if(m_materialModel.find("Correspondence")<m_materialModel.length()) {isCorrespondenceOrPalsMaterial = true;}
  if(m_materialModel.find("Pals")<m_materialModel.length()) {isCorrespondenceOrPalsMaterial = true;}

  PeridigmNS::FieldManager& fieldManager = PeridigmNS::FieldManager::self();
  m_modelCoordinatesFieldId = fieldManager.getFieldId("Model_Coordinates");
  m_horizonFieldId = fieldManager.getFieldId(PeridigmField::ELEMENT, PeridigmField::SCALAR, PeridigmField::CONSTANT, "Horizon");
  m_damageFieldId = fieldManager.getFieldId(PeridigmNS::PeridigmField::ELEMENT, PeridigmNS::PeridigmField::SCALAR, PeridigmNS::PeridigmField::TWO_STEP, "Damage");
  m_bondDamageFieldId = fieldManager.getFieldId(PeridigmNS::PeridigmField::BOND, PeridigmNS::PeridigmField::SCALAR, PeridigmNS::PeridigmField::TWO_STEP, "Bond_Damage");
  m_deltaTemperatureFieldId = fieldManager.getFieldId(PeridigmField::NODE, PeridigmField::SCALAR, PeridigmField::TWO_STEP, "Temperature_Change");
  m_microPotentialFieldId = fieldManager.getFieldId(PeridigmField::BOND, PeridigmField::SCALAR, PeridigmField::TWO_STEP, "Micro-Potential");
  m_specularBondPositionFieldId = fieldManager.getFieldId(PeridigmField::BOND, PeridigmField::SCALAR, PeridigmField::CONSTANT, "Specular_Bond_Position");
  m_volumeRatioFieldId = fieldManager.getFieldId(PeridigmField::ELEMENT, PeridigmField::SCALAR, PeridigmField::CONSTANT, "Volume Ratio");
  m_volumeFieldId = fieldManager.getFieldId(PeridigmField::ELEMENT, PeridigmField::SCALAR, PeridigmField::CONSTANT, "Volume");


  m_fieldIds.push_back(m_modelCoordinatesFieldId);
  m_fieldIds.push_back(m_horizonFieldId);
  m_fieldIds.push_back(m_damageFieldId);
  m_fieldIds.push_back(m_bondDamageFieldId);
  m_fieldIds.push_back(m_deltaTemperatureFieldId);
  m_fieldIds.push_back(m_microPotentialFieldId);
  m_fieldIds.push_back(m_specularBondPositionFieldId);
  m_fieldIds.push_back(m_volumeRatioFieldId);
  m_fieldIds.push_back(m_volumeFieldId);
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
  double *damage, *bondDamage/*, *BondsLeft*/;
  dataManager.getData(m_damageFieldId, PeridigmField::STEP_NP1)->ExtractView(&damage);
  dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_NP1)->ExtractView(&bondDamage);

  // Initialize damage to zero
  int neighborhoodListIndex = 0;
  int bondIndex = 0;
  for(int iID=0 ; iID<numOwnedPoints ; ++iID /*, ++BondsLeft*/){
  	int nodeID = ownedIDs[iID];
    damage[nodeID] = 0.0;
	int numNeighbors = neighborhoodList[neighborhoodListIndex++];
    neighborhoodListIndex += numNeighbors;
	for(int iNID=0 ; iNID<numNeighbors ; ++iNID){
      bondDamage[bondIndex++] = 0.0;
	}
  }
  
  double *x;
  dataManager.getData(m_modelCoordinatesFieldId, PeridigmField::STEP_NONE)->ExtractView(&x);

  double *volRatio, *volume, *horizon;
  dataManager.getData(m_volumeRatioFieldId, PeridigmField::STEP_NONE)->ExtractView(&volRatio);
  dataManager.getData(m_volumeFieldId, PeridigmField::STEP_NONE)->ExtractView(&volume);
  dataManager.getData(m_horizonFieldId, PeridigmField::STEP_NONE)->ExtractView(&horizon);

  // compute volume ratio
  int neighborID;
  double initialDistance;
  neighborhoodListIndex = 0;
  bondIndex = 0;
  for(int iID=0 ; iID<numOwnedPoints ; ++iID /*, ++BondsLeft*/){
  	int nodeID = ownedIDs[iID];
    if(isCorrespondenceOrPalsMaterial)
        volRatio[nodeID] = 1.0;
    else{
        volRatio[nodeID] = 0.0;
        int numNeighbors = neighborhoodList[neighborhoodListIndex++];
        for(int iNID=0 ; iNID<numNeighbors ; ++iNID){
            neighborID = neighborhoodList[neighborhoodListIndex++];
            initialDistance = distance(x[nodeID*3], x[nodeID*3+1], x[nodeID*3+2], x[neighborID*3], x[neighborID*3+1], x[neighborID*3+2]);
            volRatio[nodeID] += initialDistance*volume[neighborID];
//             volRatio[nodeID] += initialDistance*initialDistance*volume[neighborID];
//             volRatio[nodeID] += initialDistance*initialDistance*initialDistance*volume[neighborID];
        }
        volRatio[nodeID] /= m_pi*pow(horizon[nodeID],4.0);
//         volRatio[nodeID] /= 4.0/5.0*m_pi*pow(horizon[nodeID],5.0);
//         volRatio[nodeID] /= 2.0/3.0*m_pi*pow(horizon[nodeID],6.0);
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
  double *horizon, *damage, *bondDamageN, *bondDamageNP1, *deltaTemperature, *miPot, *specu, *volRatio;
  dataManager.getData(m_horizonFieldId, PeridigmField::STEP_NONE)->ExtractView(&horizon);
  dataManager.getData(m_damageFieldId, PeridigmField::STEP_NP1)->ExtractView(&damage);
  dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_N)->ExtractView(&bondDamageN);
  dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_NP1)->ExtractView(&bondDamageNP1);
  dataManager.getData(m_deltaTemperatureFieldId, PeridigmField::STEP_NP1)->ExtractView(&deltaTemperature);
  dataManager.getData(m_microPotentialFieldId, PeridigmField::STEP_N)->ExtractView(&miPot);
  dataManager.getData(m_specularBondPositionFieldId, PeridigmField::STEP_NONE)->ExtractView(&specu);
  dataManager.getData(m_volumeRatioFieldId, PeridigmField::STEP_NONE)->ExtractView(&volRatio);

  double trialDamage(0.0), totalDamage;
  int neighborhoodListIndex(0), bondIndex(0);
  int nodeId, numNeighbors, neighborID, iID, iNID;
//   double nodeInitialX[3], initialDistance;
  double bond_Jintegral;

  // Set the bond damage to the previous value
  *(dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_NP1)) = *(dataManager.getData(m_bondDamageFieldId, PeridigmField::STEP_N));

  // Update the bond damage
  // Break bonds if the extension is greater than the critical extension

  for(iID=0 ; iID<numOwnedPoints ; ++iID){
	nodeId = ownedIDs[iID];
// 	nodeInitialX[0] = x[nodeId*3];
// 	nodeInitialX[1] = x[nodeId*3+1];
// 	nodeInitialX[2] = x[nodeId*3+2];
    double localT = *(deltaTemperature+nodeId);
    numNeighbors = neighborhoodList[neighborhoodListIndex++];
	for(iNID=0 ; iNID<numNeighbors ; ++iNID){

        int specuID = int(specu[bondIndex]);
	    neighborID = neighborhoodList[neighborhoodListIndex];
//         initialDistance = distance(nodeInitialX[0], nodeInitialX[1], nodeInitialX[2], x[neighborID*3], x[neighborID*3+1],x[neighborID*3+2]);
        double neighT = *(deltaTemperature+neighborID);
        bond_Jintegral = obj_Jintegral.compute((localT+neighT)/2.0);
        
        double local_horizon = *(horizon+nodeId);
        double specificJ = 4.0/(m_pi*local_horizon*local_horizon*local_horizon*local_horizon);
//         double specificJ = 5.0/(m_pi*local_horizon*local_horizon*local_horizon*local_horizon*local_horizon)*initialDistance;
//         double specificJ = 6.0/(m_pi*local_horizon*local_horizon*local_horizon*local_horizon*local_horizon*local_horizon)*initialDistance*initialDistance;
//         double specificJ = 7.0/(m_pi*local_horizon*local_horizon*local_horizon*local_horizon*local_horizon*local_horizon*local_horizon)*initialDistance*initialDistance*initialDistance;
        double m_criticalMicroPotential;
        if (isCorrespondenceOrPalsMaterial)
            m_criticalMicroPotential = specificJ*bond_Jintegral;
        else{
            double meanVolRatio = (*(volRatio+iID)+*(volRatio+neighborID))/2.0;
            m_criticalMicroPotential = specificJ*bond_Jintegral/meanVolRatio;
        }

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
 	damage[nodeId] = totalDamage;
  }
}