/*! \file Peridigm_ViscousBondBasedMaterial.hpp */

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

#ifndef PERIDIGM_VISCOUSBONDBASEDMATERIAL_HPP
#define PERIDIGM_VISCOUSBONDBASEDMATERIAL_HPP

#include "Peridigm_Material.hpp"
#include "Peridigm_BaseViscousMaterial.hpp"

namespace PeridigmNS {

  class ViscousBondBasedMaterial : public ViscousMaterial {
  public:

    //! Constructor.
    ViscousBondBasedMaterial(const Teuchos::ParameterList & params);

    //! Destructor.
    virtual ~ViscousBondBasedMaterial();

    //! Return name of material type
    virtual std::string Name() const {return("Viscous Bond Based");}

    //! Returns a vector of field IDs corresponding to the variables associated with the material.
    virtual std::vector<int> FieldIds() const { return m_fieldIds; }

    //! Initialized data containers and computes weighted volume.
    virtual void
    initialize(const double dt,
               const int numOwnedPoints,
               const int* ownedIDs,
               const int* neighborhoodList,
               PeridigmNS::DataManager& dataManager);

    //! Evaluate the internal force.
    virtual void
    computeForce(const double dt,
		 const int numOwnedPoints,
		 const int* ownedIDs,
		 const int* neighborhoodList,
                 PeridigmNS::DataManager& dataManager) const;

  protected:

    // material parameters
    double m_eta;
    double m_horizon;

    // field spec ids for all relevant data
    std::vector<int> m_fieldIds;
    int m_volumeFieldId;
    int m_damageFieldId;
    int m_modelCoordinatesFieldId;
    int m_coordinatesFieldId;
    int m_velocitiesFieldId;
    int m_forceDensityFieldId;
    int m_bondDamageFieldId;
  };
}

#endif /* PERIDIGM_VISCOUSBONDBASEDMATERIAL_HPP */






