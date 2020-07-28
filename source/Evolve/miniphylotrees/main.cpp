#include <iostream>
#include <vector>
#include <memory>
#include <assert.h>
#include "../Systematics.h"

//g++ -std=c++17 -I../../ main.cpp -o main.o && ./main.o
//for i in {1..5}; do g++ -std=c++17 -I../../ main.cpp -o main.o && ./main.o; done
//for i in {1..100}; do ./main.o; done

/*This is the null model for the systematics normalization project. This program generates a random tree with a depth of
 * 100 generations. This can be changed in the numGens variable. This tree has 10 organisms per generation and uses a clade
 * as the method of tracking in systematics.h. The program returns phylogenetic diversity after each generation and prints the
 * final phylogenetic diversity to a csv file of your choice. You will have to uncomment the writeToFile function and give it a
 * file path to use this function.
 * You can also use this tree to test the systematics.h FindPhyloData() function. You can uncomment the line sys.FindPhyloData() to
 * use this function of the program. */


using namespace std;

int numOrgs = 10;
int parentNum;
int numGens = 100;
int nextClade = 0;
int systime = 0;

class Organism{
public:
    int clade;

    Organism(){
        clade = nextClade;
        nextClade++;
    }
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

    emp::Systematics<Organism, int> sys(taxonFunc); //optional 3rd arg
    sys.SetTrackSynchronous(true);

    //current gen (vector)
    vector<Organism> currentGen; //begins with currentGen
    //child gen (vector)
    vector<Organism> childGen;


    for (int i = 0; i < numOrgs; i++) {
        currentGen.emplace_back(); //currentGen is filled with 10 organism
        sys.AddOrg(currentGen[i], i, systime); //parent is null (original organisms have no parent pointers)
    }


    for (int i = 0; i < numGens; i++) {
        cout << "generation: " << i << endl;
        assert(currentGen.size() == 10);

        for(int r = 0; r < 10; r++){
            chooseOrg(currentGen, randNum); //chooses the parent of the next generation
            sys.SetNextParent(parentNum);

            childGen.emplace_back(); //fills childGen vector with Organisms
            emp::WorldPosition pos(r, 1);
            sys.AddOrg(childGen[r], pos, systime);
        }

        //sys.PrintStatus(); //uncomment to see more info about systematics tracking
        cout << "phylogenetic diversity: " << sys.GetPhylogeneticDiversity() << endl;

        if(i == numGens - 1){
            /* uncomment these for different program functionality described in file header. */

            //sys.FindPhyloData();
            //writeToFile("filename.csv", sys.GetPhylogeneticDiversity());
            }


        for(int j = 0; j < currentGen.size(); j++){
            sys.RemoveOrg(j, systime);
        }

        switchGens(currentGen,childGen, sys); //puts contents of child vector into current vector and deletes content of child vector
        systime++;
    }

    int total_orgs = numGens * numOrgs;

    cout << "generations: " << numGens << " / total organisms: " << total_orgs << endl;
    };

bool writeToFile(string filename, int field_one){
    ofstream file;
    file.open(filename, ios_base::app);
    file << field_one << ",";
    file.close();

    return true;
}
