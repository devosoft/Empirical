#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <fstream>
//#include "base/Ptr.h"
//#include "Evolve/Systematics.h"
#include "../Systematics.h"

//g++ -std=c++17 -I../../ main.cpp -o main.o

using namespace std;

int randScope;
int numOrgs = 10;
int parentNum;
int numRounds = 100;
int nextClade = 0;

class Organism{
public:
    int clade;

    Organism(){
        clade = nextClade;
        clade++;
    }

    static void reproduce(vector<Organism> &childGen, emp::Systematics<Organism, int> &sys){
        for(int i = 0; i < 10; i++){
            childGen.emplace_back(); //fills childGen vector with Organisms

            //ORG & org, WorldPosition pos, int update=-1
            sys.AddOrg(childGen[i], i, 1); //removed brackets childGen[i], {i, 0}
        }
        //cout << "child generation created" << endl;
    }
};

int chooseOrg(vector<Organism> &currentGen){
    parentNum = rand() % randScope; //chooses random spot in array for parent
    //cout << "parent chosen is in spot " << parentNum << " in currentGen array which is " << size(currentGen) << " long" << endl;
    return parentNum;
}

void switchGens(vector<Organism> &currentGen, vector<Organism> &childGen, emp::Systematics<Organism, int> &sys){
    currentGen.swap(childGen);
    childGen.clear();
    sys.Update();
}

bool writeToFile(string filename, int field_one, int field_two);



int main() {

    function<int(Organism)> taxonFunc = [](Organism org){return org.clade;};//takes org return int

    //function<int(int)> square = [](int squaredNum){return (squaredNum*squaredNum);};

    emp::Systematics<Organism, int> sys(taxonFunc); //optional 3rd arg
    sys.SetTrackSynchronous(true);

    //current gen (vector)
    vector<Organism> currentGen; //begins with currentGen
    //child gen (vector)
    vector<Organism> childGen;


    currentGen.reserve(numOrgs);
    for (int i = 0; i < numOrgs; i++) {
        currentGen.emplace_back(); //currentGen is filled with 10 organism
        sys.AddOrg(currentGen[i], i, 0); //parent is null (removed brackets)
    }

    for (int i = 0; i < numRounds; i++) {
        cout << "generation: " << i << endl;
        randScope = size(currentGen); //this tells the chooseOrg function how large the vector is
        chooseOrg(currentGen); //chooses the parent of the next generation
        int parent = parentNum; //records parent as the num chosen by chooseOrg
        cout << "parent: " << parent << endl;
        sys.SetNextParent(0);
        currentGen[parentNum].reproduce(childGen, sys); //fills childGen with 10 Organisms
        //cout << "size of child population: " << size(childGen) << endl;
        for(i = 0; i < currentGen.size(); i++){
            sys.RemoveOrg(i, 0); //removed brackets
        }
        switchGens(currentGen,
                   childGen, sys); //puts contents of child vector into current vector and deletes content of child vector

        bool writeFile = writeToFile("treedata.txt", i, parentNum);
    }


//update,num_taxa,total_orgs,ave_depth,num_roots,mrca_depth,diversity

    int total_orgs = numRounds * numOrgs;

    cout << "generations: " << numRounds << " / total organisms: " << total_orgs << endl;
};

bool writeToFile(string filename, int field_one, int field_two){
    ofstream file;
    file.open(filename, ios_base::app);
    file << field_one << "," << field_two << endl;
    file.close();

    return true;
}