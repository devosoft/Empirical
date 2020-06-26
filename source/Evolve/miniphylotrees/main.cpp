#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <assert.h>
#include "../Systematics.h"
#include "../../tools/Random.h"

//g++ -std=c++17 -I../../ main.cpp -o main.o && ./main.o
//for i in {1..5}; do g++ -std=c++17 -I../../ main.cpp -o main.o && ./main.o; done
//for i in {1..100}; do ./main.o; done


using namespace std;

int randScope;
int numOrgs = 10;
int parentNum;
int numRounds = 100;
int nextClade = 0;
int systime = 0;

class Organism{
public:
    int clade;

    Organism(){
        clade = nextClade;
        nextClade++;

        assert(nextClade == clade + 1);

    }

    static const char * printVect(){
        return "currentgen org";
    }

//    static void reproduce(vector<Organism> &childGen, emp::Systematics<Organism, int> &sys){
//        for(int i = 0; i < 2; i++){
//            childGen.emplace_back(); //fills childGen vector with Organisms
//
//            //ORG & org, WorldPosition pos, int update=-1
//            emp::WorldPosition pos(i, 1);
//            sys.AddOrg(childGen[i], pos, systime); //removed brackets childGen[i], {i, 0}
//        }
//        cout << "child generation created" << endl;
//    //}
};

int chooseOrg(vector<Organism> &currentGen, emp::Random &randNum){
    parentNum = randNum.GetInt(10);  //chooses random spot in array for parent
    //cout << "parent chosen is in spot " << parentNum << " in currentGen array which is " << size(currentGen) << " long" << endl;
    return parentNum;
}

void switchGens(vector<Organism> &currentGen, vector<Organism> &childGen, emp::Systematics<Organism, int> &sys){
    currentGen.swap(childGen);
    childGen.clear();
    sys.Update();
}

bool writeToFile(string filename, int field_one);


int main() {

    emp::Random randNum;

    function<int(Organism)> taxonFunc = [](Organism org){return org.clade;};//takes org return int

    //function<int(int)> square = [](int squaredNum){return (squaredNum*squaredNum);};

    emp::Systematics<Organism, int> sys(taxonFunc); //optional 3rd arg
    sys.SetTrackSynchronous(true);

    //current gen (vector)
    vector<Organism> currentGen; //begins with currentGen
    //child gen (vector)
    vector<Organism> childGen;


    for (int i = 0; i < numOrgs; i++) {
        currentGen.emplace_back(); //currentGen is filled with 10 organism
        sys.AddOrg(currentGen[i], i, systime); //parent is null (removed brackets)
    }

//    for(int i = 0; i < currentGen.size(); i++){
//        cout << currentGen[i] . printVect() << " " << endl;
//    }

    for (int i = 0; i < numRounds; i++) {
        cout << "generation: " << i << endl;
        randScope = size(currentGen); //this tells the chooseOrg function how large the vector is
        assert(currentGen.size() == 10);

        for(int r = 0; r < 10; r++){
            chooseOrg(currentGen, randNum); //chooses the parent of the next generation
            //cout << "parent: " << parentNum << endl;
            sys.SetNextParent(parentNum);
            //currentGen[parentNum].reproduce(childGen, sys); //fills childGen with 10 Organisms

            childGen.emplace_back(); //fills childGen vector with Organisms
            emp::WorldPosition pos(r, 1);
            sys.AddOrg(childGen[r], pos, systime); //removed brackets childGen[i], {i, 0}
            //cout << "size of child population: " << size(childGen) << endl;
        }
        //sys.PrintStatus();
        cout << "phylogenetic diversity: " << sys.GetPhylogeneticDiversity() << endl;
        bool writeFile = writeToFile("treedata.csv", sys.GetPhylogeneticDiversity());


        for(int j = 0; j < currentGen.size(); j++){
            sys.RemoveOrg(j, systime);
        }

        switchGens(currentGen,childGen, sys); //puts contents of child vector into current vector and deletes content of child vector
        systime++;
    }


    int total_orgs = numRounds * numOrgs;

    cout << "generations: " << numRounds << " / total organisms: " << total_orgs << endl;
};

bool writeToFile(string filename, int field_one){
    ofstream file;
    file.open(filename, ios_base::app);
    file << field_one << ",";
    file.close();

    return true;
}

//use only end phylo num

//use python script to convert each value to percentile
//order smallest to larget -- '

//end up with file with 100 nums --star num and second num that starts percentile
//in systematics class, we will want to modify some tree calcualtions

//modify phpy diversity and others
//give it optional arg, string pointing to file with percentiles
//file as arg, should open file and read everything

//check out file.h class (tools) -- use to read in file
//at end of phylo funct, if data is providec to normalize, normalize

//figure out which percentile it falls into
