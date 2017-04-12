#ifndef DReactionStepVertexInfo_h
#define DReactionStepVertexInfo_h

#include <memory>
#include <set>
#include <map>
#include <vector>
#include <algorithm>

#include "particleType.h"
#include "ANALYSIS/DReaction.h"

using namespace std;

//reaction-based vertex info
namespace DAnalysis
{

enum DReactionState_t
{
	d_InitialState = 0,
	d_FinalState = 1,
	d_EitherState = 2
};

class DReactionStepVertexInfo
{
	//CONSTRUCTORS
	DReactionStepVertexInfo(void) = delete;
	DReactionStepVertexInfo(const DReaction* locReaction, size_t locStartStepIndex);

	//ADDING STEPS & PARTICLES
	void Add_ReactionStep(size_t locStepIndex);
	void Set_ParticleIndices(const vector<pair<int, int>>& locFullConstrainParticles, const vector<pair<int, int>>& locDecayingParticles,
			const vector<pair<int, int>>& locOnlyConstrainTimeParticles, const vector<pair<int, int>>& locNoConstrainParticles);

	//REGISTER DECAY VERTICES
	void Register_DecayingNoConstrainUseVertex(const pair<int, int>& locDecayingNoConstrainPair, shared_ptr<DReactionStepVertexInfo>& locVertexInfo);
	void Register_DecayingParticleConstraints(const vector<pair<int, int>>& locNoConstrainDecayingParticles,
			const map<pair<int, int>, shared_ptr<DReactionStepVertexInfo>>& locFullConstrainDecayingParticles);
	void Set_DanglingVertexFlag(bool locIsDanglingVertexFlag){dIsDanglingVertexFlag = locIsDanglingVertexFlag;}

	//GET REACTION INFO
	const DReaction* Get_Reaction(void) const{return dReaction;}
	vector<size_t> Get_StepIndices(void) const{return dReactionStepIndices;}
	bool Get_ProductionVertexFlag(void) const{return dIsProductionVertexFlag;}
	bool Get_DanglingVertexFlag(void) const{return dIsDanglingVertexFlag;}

	//GET PARTICLES
	vector<pair<int, int>> Get_FullConstrainParticles(DReactionState_t locState = d_EitherState, Charge_t locCharge = d_AllCharges, bool locIncludeDecayingFlag = true) const;
	vector<pair<int, int>> Get_DecayingParticles(DReactionState_t locState = d_EitherState, Charge_t locCharge = d_AllCharges) const;
	vector<pair<int, int>> Get_OnlyConstrainTimeParticles(void) const{return dOnlyConstrainTimeParticles;}
	vector<pair<int, int>> Get_NoConstrainParticles(DReactionState_t locState = d_EitherState, Charge_t locCharge = d_AllCharges,
			bool locIncludeDecayingFlag = true, bool locIncludeMissingFlag = true, bool locIncludeTargetFlag = true) const;
	vector<pair<int, int>> Get_MissingParticles(DReactionState_t locState = d_EitherState, Charge_t locCharge = d_AllCharges) const;

	//sorted decaying
	map<pair<int, int>, weak_ptr<DReactionStepVertexInfo>> Get_DecayingParticles_NoConstrain(void) const{return dDecayingParticles_NoConstrain;}
	map<pair<int, int>, shared_ptr<DReactionStepVertexInfo>> Get_DecayingParticles_FullConstrain(void) const{return dDecayingParticles_FullConstrain;}

	//independent of state
	vector<pair<int, int>> Get_Particles(DReactionState_t locState = d_EitherState, Charge_t locCharge = d_AllCharges,
			bool locIncludeDecayingFlag = true, bool locIncludeMissingFlag = true, bool locIncludeTargetFlag = true) const;

private:

	//PRIVATE METHODS
	vector<pair<int, int>> Filter_Particles(vector<pair<int, int>> locParticles, DReactionState_t locState, Charge_t locCharge,
			bool locIncludeDecayingFlag = true, bool locIncludeMissingFlag = true, bool locIncludeTargetFlag = true) const;

	//REACTION SUMMARY INFO
	const DReaction* dReaction;
	vector<size_t> dReactionStepIndices; //in order from smallest to largest
	bool dIsProductionVertexFlag = false;

	//PARTICLE INFO
	//pair: step, particle indices (including all: beam, target, decaying, detected, and missing)
	vector<pair<int, int>> dFullConstrainParticles; //detected charged tracks & beam, decaying when registered
	vector<pair<int, int>> dOnlyConstrainTimeParticles; //detected photons
	vector<pair<int, int>> dNoConstrainParticles; //missing, massive neutrals, decaying when registered

	//DECAY INFO
	//Note, decaying particles that decay in-place at this vertex (e.g. pi0) will only appear once: with their "final-state" indices
	vector<pair<int, int>> dDecayingParticles; //all, whether used to constrain or not
	//below must be weak_ptr or else will have cyclic reference (memory leak!)
	map<pair<int, int>, weak_ptr<DReactionStepVertexInfo>> dDecayingParticles_NoConstrain; //vertex-info: where it is used to constrain (nullptr if not)
	map<pair<int, int>, shared_ptr<DReactionStepVertexInfo>> dDecayingParticles_FullConstrain; //vertex-info: where it was defined

	//DANGLING
	//is it dangling? //if is true, then vertex parent is either:
		//in dDecayingParticles_NoConstrain if it's not empty (will have size 1), or is center of target
	bool dIsDanglingVertexFlag = false;
};

/****************************************************** NAMESPACE-SCOPE NON-INLINE FUNCTION DECLARATIONS *******************************************************/

string Build_VertexConstraintString(const DReactionStepVertexInfo* locVertexInfo, bool locSpacetimeFitFlag);

/************************************************************** CONSTRUCTORS & OPERATORS ***************************************************************/

inline DReactionStepVertexInfo::DReactionStepVertexInfo(const DReaction* locReaction, size_t locStartStepIndex) :
		dReaction(locReaction), dReactionStepIndices{locStartStepIndex}
{
	dIsProductionVertexFlag = ((locStartStepIndex == 0) && DAnalysis::Get_IsFirstStepBeam(locReaction));
}

/************************************************************** INLINE FUNCTIONS ***************************************************************/

inline void DReactionStepVertexInfo::Add_ReactionStep(size_t locStepIndex)
{
	dReactionStepIndices.push_back(locStepIndex);
	std::sort(dReactionStepIndices.begin(), dReactionStepIndices.end()); //just in case
}

inline void DReactionStepVertexInfo::Register_DecayingNoConstrainUseVertex(const pair<int, int>& locDecayingNoConstrainPair, shared_ptr<DReactionStepVertexInfo>& locVertexInfo)
{
	dDecayingParticles_NoConstrain[locDecayingNoConstrainPair] = locVertexInfo;
}

inline vector<pair<int, int>> DReactionStepVertexInfo::Get_FullConstrainParticles(DReactionState_t locState, Charge_t locCharge, bool locIncludeDecayingFlag) const
{
	return Filter_Particles(dFullConstrainParticles, locState, locCharge, locIncludeDecayingFlag);
}

inline vector<pair<int, int>> DReactionStepVertexInfo::Get_DecayingParticles(DReactionState_t locState, Charge_t locCharge) const
{
	return Filter_Particles(dDecayingParticles, locState, locCharge);
}

inline vector<pair<int, int>> DReactionStepVertexInfo::Get_NoConstrainParticles(DReactionState_t locState, Charge_t locCharge,
		bool locIncludeDecayingFlag, bool locIncludeMissingFlag, bool locIncludeTargetFlag) const
{
	return Filter_Particles(dNoConstrainParticles, locState, locCharge, locIncludeDecayingFlag, locIncludeMissingFlag, locIncludeTargetFlag);
}

inline vector<pair<int, int>> DReactionStepVertexInfo::Get_MissingParticles(DReactionState_t locState, Charge_t locCharge) const
{
	vector<pair<int, int>> locParticles = Filter_Particles(dNoConstrainParticles, locState, locCharge, false, true, false);

	auto Check_NotMissing = [&dReaction](const pair<int, int>& locIndices) -> bool
	{return (locIndices.second != dReaction->Get_ReactionStep(locIndices.first)->Get_MissingParticleIndex());};

	locParticles.erase(std::remove_if(locParticles.begin(), locParticles.end(), Check_NotMissing), locParticles.end());
	return locParticles;
}

inline vector<pair<int, int>> DReactionStepVertexInfo::Get_Particles(DReactionState_t locState, Charge_t locCharge, bool locIncludeDecayingFlag, bool locIncludeMissingFlag, bool locIncludeTargetFlag) const
{
	//ASSUMES Object has been fully created before calling
	vector<pair<int, int>> locParticles = dFullConstrainParticles;
	locParticles.insert(locParticles.end(), dOnlyConstrainTimeParticles.begin(), dOnlyConstrainTimeParticles.end());
	locParticles.insert(locParticles.end(), dNoConstrainParticles.begin(), dNoConstrainParticles.end());
	std::sort(locParticles.begin(), locParticles.end());
	return Filter_Particles(locParticles, locState, locCharge, locIncludeDecayingFlag, locIncludeMissingFlag, locIncludeTargetFlag);
}

} //end DAnalysis namespace

#endif // DReactionStepVertexInfo_h
