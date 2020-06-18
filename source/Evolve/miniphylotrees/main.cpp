#include <iostream>
#include <vector>
#include <memory>
#include <cstdlib>
#include <fstream>
//#include "../base/Ptr.h"
#include "../Systematics.h"

//g++ -std=c++17 -I../../Evolve/ -I../../ -I../../base/ main.cpp -o main.o
using namespace std;

int randScope;
int numOrgs = 10;
int parentNum;
int numRounds = 100;

class Organism{
public:
    void reproduce(vector<Organism> &childGen){
        for(int i = 0; i < 10; i++){
            childGen.emplace_back(); //fills childGen vector with Organisms
        }
        //cout << "child generation created" << endl;
    }
};

int chooseOrg(vector<Organism> &currentGen){
    parentNum = rand() % randScope; //chooses random spot in array for parent
    //cout << "parent chosen is in spot " << parentNum << " in currentGen array which is " << size(currentGen) << " long" << endl;
    return parentNum;
}

void switchGens(vector<Organism> &currentGen, vector<Organism> &childGen){
    currentGen.swap(childGen);
    childGen.clear();
}

bool writeToFile(string filename, int field_one, int field_two);



int main() {
    //current gen (vector)
    vector<Organism> currentGen; //begins with currentGen
    //child gen (vector)
    vector<Organism> childGen;

    emp::GetTypeID<Organism>();



    currentGen.reserve(numOrgs);
    for (int i = 0; i < numOrgs; i++) {
        currentGen.emplace_back(); //currentGen is filled with 10 organism
    }

    for (int i = 0; i < numRounds; i++) {
        cout << "generation: " << i << endl;
        randScope = size(currentGen); //this tells the chooseOrg function how large the vector is
        chooseOrg(currentGen); //chooses the parent of the next generation
        int parent = parentNum; //records parent as the num chosen by chooseOrg
        cout << "parent: " << parent << endl;
        currentGen[parentNum].reproduce(childGen); //fills childGen with 10 Organisms
        //cout << "size of child population: " << size(childGen) << endl;
        switchGens(currentGen,
                   childGen); //puts contents of child vector into current vector and deletes content of child vector

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