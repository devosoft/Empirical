#define CATCH_CONFIG_MAIN
#ifndef NDEBUG
	#define TDEBUG
#endif

#include "third-party/Catch/single_include/catch2/catch.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include "../../include/emp/Evolve/Systematics.hpp"

using namespace std;

int numOrgs = 10;
int parentNum;
int numGens = 10;
double mutRate = 0.05;
int TenGens = 10;

class Organism {
public:
    int genotype = 0;
    Organism() {
    }
    Organism(int _genotype) { 
        genotype = _genotype;
    }
    int MutateGenotype(emp::Random &RandNum) {

        double randMutation = RandNum.GetDouble(0, 1);
        if (randMutation < mutRate) {
            int MutatedGenotype = genotype - RandNum.GetInt(-3, 3);
            genotype = MutatedGenotype;
        } else {
        }
        return genotype;
    }
};



int chooseOrg(vector<Organism> &currentGen, emp::Random &randNum){
    parentNum = randNum.GetInt(currentGen.size());  //chooses random spot in array for parent
    return parentNum;
}

void calcFitness(vector<Organism> &currentGen, vector<double> &fitnessVect, emp::Random &randNum) {
    fitnessVect.resize(0);
    vector<int> fitnessCalc;
    fitnessCalc.reserve((int) currentGen.size());
    for (int i = 0; i < (int) currentGen.size(); i++) {
        fitnessCalc.push_back(currentGen[i].genotype);
    }
    map<int, int> CountMap;

    for (int j = 0; j < (int) fitnessCalc.size(); j++) {
        if (emp::Has(CountMap, fitnessCalc[j])) {
            CountMap[fitnessCalc[j]]++;
        } else {
            CountMap[fitnessCalc[j]] = 1;
        }
    }

    for(int k = 0; k < (int) fitnessCalc.size(); k++){
        fitnessVect.push_back(1.0/CountMap[fitnessCalc[k]]);
    }
}

void switchGens(vector<Organism> &currentGen, vector<Organism> &childGen, emp::Systematics<Organism, int> &sys){
    currentGen.swap(childGen);
    childGen.clear();
    sys.Update();
}

TEST_CASE("Test Systematics GetPhyloDiversity()", "[Evolve]"){
    emp::Random randNum(1);
    function<int(Organism)> taxonFunc = [](Organism org){return org.genotype;};
    emp::Systematics<Organism, int> sys(taxonFunc); 
    sys.SetTrackSynchronous(true);
    vector<Organism> currentGen; 
    vector<Organism> childGen;
    vector<double> fitnessVect;

    for (int i = 0; i < numOrgs; i++) {
        currentGen.emplace_back(); 
        sys.AddOrg(currentGen[i], i); 
    }

    for (int i = 0; i < numGens; i++) {
        calcFitness(currentGen, fitnessVect,randNum);
        for(int r = 0; r < numOrgs; r++){
            chooseOrg(currentGen, randNum);
            sys.SetNextParent(parentNum);
            childGen.emplace_back(currentGen[parentNum].genotype); 
            childGen[r].MutateGenotype(randNum);
            emp::WorldPosition pos(r, 1);
            sys.AddOrg(childGen[r], pos); 
        }

        if(i == TenGens - 1){
            CHECK(sys.GetPhylogeneticDiversity() == 5); 
            CHECK(sys.GetPhylogeneticDiversity(TenGens, "assets/TensChooseOrgGenotype.csv") == 91); 
            CHECK(((int) sys.ancestor_taxa.size() + (int) sys.active_taxa.size()) == 6); 
            TenGens = TenGens + 10;
        }

        for(int j = 0; j < (int) currentGen.size(); j++){
            sys.RemoveOrg(j);
        }
        switchGens(currentGen,childGen, sys);
    }
};