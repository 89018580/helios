/*
Copyright (c) 2012, Esteban Pellegrino
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FISSIONPOLICY_HPP_
#define FISSIONPOLICY_HPP_

#include "../AceReader/ReactionContainer.hpp"
#include "../AceReader/NeutronTable.hpp"
#include "../AceReader/Ace.hpp"
#include "../AceReaction/NuSampler.hpp"
#include "../AceReaction/FissionReaction.hpp"
#include "../../../Environment/McModule.hpp"
#include "../../../Common/Common.hpp"
#include "../../Grid/MasterGrid.hpp"
#include "../../Isotope.hpp"

namespace Helios {

	class AceIsotopeBase;

	/* Base class to deal with fission treatment */
	class FissilePolicyBase {
		/* Constant reference to a CHILD grid */
		const ChildGrid* child_grid;
	protected:
		/* Fission cross section */
		Ace::CrossSection fission_xs;

	public:
		FissilePolicyBase(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid);

		/* Get fission cross section */
		double getFissionXs(Energy& energy) const;

		~FissilePolicyBase() {}
	};

	/* NU sampling schemes */
	class TotalNu {
		/* NU sampler */
		AceReaction::NuSampler* total_nu;   /* Total NU */
		AceReaction::NuSampler* prompt_nu;   /* Prompt NU */
	public:
		TotalNu(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table);

		/* Get average NU-bar at some energy */
		double getTotalNu(const Energy& energy) const {
			return total_nu->getNuBar(energy.second);
		}

		double getPromptNu(const Energy& energy) const {
			return prompt_nu->getNuBar(energy.second);
		}

		double getDelayedNu(const Energy& energy) const {
			return 0.0;
		}

		/* Return delayed neutron fraction */
		double getBeta(const Energy& energy) const {
			return 0.0;
		}

		~TotalNu() {
			delete total_nu;
			delete prompt_nu;
		}
	};

	class DelayedNu {
		/* NU sampler */
		AceReaction::NuSampler* total_nu;    /* Total NU   */
		AceReaction::NuSampler* prompt_nu;   /* Prompt NU  */
		AceReaction::NuSampler* delayed_nu;  /* Delayed NU */
	public:
		DelayedNu(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table);

		/* Get average NU-bar at some energy */
		double getTotalNu(const Energy& energy) const {
			/* If total NU is available in NU block, sample from it */
			if(total_nu) return total_nu->getNuBar(energy.second);
			/* If not, use a combination between prompt and delayed data */
			return getDelayedNu(energy) + getPromptNu(energy);
		}

		double getPromptNu(const Energy& energy) const {
			return prompt_nu->getNuBar(energy.second);
		}

		double getDelayedNu(const Energy& energy) const {
			return delayed_nu->getNuBar(energy.second);
		}

		/* Return delayed neutron fraction */
		double getBeta(const Energy& energy) const {
			return getDelayedNu(energy) / getPromptNu(energy);
		}

		~DelayedNu() {
			delete total_nu;
			delete prompt_nu;
			delete delayed_nu;
		}
	};

	/* Prompt fission cross section sampling (singles reaction or several chance fission available) */
	class SingleFissionReaction {
		/* Fission reaction */
		Reaction* fission_reaction;
		/* Reaction extracted from the table */
		const Ace::NeutronReaction* ace_reaction;
	public:
		SingleFissionReaction(AceIsotopeBase* _isotope, const Ace::CrossSection& fission_xs, const Ace::NeutronTable& _table, const ChildGrid* _child_grid);

		/* Return prompt fission reaction */
		Reaction* getPromptFission() const {
			return fission_reaction;
		}

		/* Return fission reaction (constant reference) */
		const Ace::NeutronReaction& getAceReaction() const {
			return *ace_reaction;
		}

		~SingleFissionReaction() {}
	};

	class ChanceFissionReaction {
		/* Fission reaction */
		Reaction* fission_reaction;
		/* Reaction extracted from the table */
		const Ace::NeutronReaction* ace_reaction;
	public:
		ChanceFissionReaction(AceIsotopeBase* _isotope, const Ace::CrossSection& fission_xs, const Ace::NeutronTable& _table, const ChildGrid* _child_grid);

		/* Return prompt fission reaction */
		Reaction* getPromptFission() const {
			return fission_reaction;
		}

		/* Return fission reaction (constant reference) */
		const Ace::NeutronReaction& getAceReaction() const {
			return *ace_reaction;
		}

		~ChanceFissionReaction() {
			delete fission_reaction;
		}
	};

	class NonFissile : public FissilePolicyBase {

	public:
		/* Constructor from table */
		NonFissile(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid) :
			FissilePolicyBase(_isotope, _table, _child_grid) {/* */};

		/* Fission reaction */
		Reaction* fission(Energy& energy, Random& random) const {
			return 0;
		};

		/* Get average NU-bar at some energy */
		double getNuBar(const Energy& energy) const {
			return 0.0;
		}

		~NonFissile() {/* */}
	};

	template<class FissionPolicy, class NuPolicy>
	class PromptFissionSampler : public FissilePolicyBase, public FissionPolicy, public NuPolicy {

	public:
		/* Constructor from table */
		PromptFissionSampler(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid) :
			FissilePolicyBase(_isotope, _table, _child_grid),
			FissionPolicy(_isotope, FissilePolicyBase::fission_xs, _table, _child_grid), NuPolicy(_isotope, _table) {
			/* Get reaction */
			const Ace::ReactionContainer& reactions(_table.getReactions());
			/* Get fission cross section */
			fission_xs = reactions.get_xs(18);
		}

		/* Fission reaction */
		Reaction* fission(Energy& energy, Random& random) const {
			return FissionPolicy::getPromptFission();
		}

		/* Get average NU-bar at some energy */
		double getNuBar(const Energy& energy) const {
			return NuPolicy::getTotalNu(energy);
		}

		PromptFissionSampler() {}
	};

	template<class FissionPolicy, class NuPolicy>
	class DelayedFissionSampler : public FissilePolicyBase, public FissionPolicy, public NuPolicy {
		/* Delayed fission reaction */
		Reaction* delayed_fission;
	public:
		/* Constructor from table */
		DelayedFissionSampler(AceIsotopeBase* _isotope, const Ace::NeutronTable& _table, const ChildGrid* _child_grid):
			FissilePolicyBase(_isotope, _table, _child_grid), FissionPolicy(_isotope, FissilePolicyBase::fission_xs, _table, _child_grid),
			NuPolicy(_isotope, _table), delayed_fission(0) {
			/* Get reaction */
			const Ace::ReactionContainer& reactions(_table.getReactions());
			/* Get fission cross section */
			fission_xs = reactions.get_xs(18);

			/* Get block where the delayed data is contained */
			const Ace::DLYBlock* del_block = _table.block<Ace::DLYBlock>();
			/* Create delayed fission reaction */
			delayed_fission = new AceReaction::DelayedFission(del_block->getDelayedData(), del_block->getEnergyDistribution(),
					FissionPolicy::getAceReaction());
		}

		/* Fission reaction */
		Reaction* fission(Energy& energy, Random& random) const {
			/* Sample random number */
			double rho(random.uniform());
			/* Sample particles with delayed spectrum */
			if(rho < NuPolicy::getBeta(energy)) return delayed_fission;
			/* Return prompt neutron spectrum */
			return FissionPolicy::getPromptFission();
		}

		/* Get average NU-bar at some energy */
		double getNuBar(const Energy& energy) const {
			return NuPolicy::getTotalNu(energy);
		}

		DelayedFissionSampler() {
			delete delayed_fission;
		}
	};

}

#endif /* FISSIONPOLICY_HPP_ */
