To create a new world:

class YourWorld : public emp::World<YourOrg>{
private:
....

public:

void Update() {
     emp::World<YourOrg>::Update(); //This pings the update signal
     //whatever should happen on an update 
     //Possibly:
     vector<size_t> schedule = emp::GetPermutation(random, GetSize()); //makes sure organisms are executed randomly so no bias to 'beginning' of world
     for (size_t : schedule) {
     	 if (IsOccupied(i) == false) continue;
	    pop[i]->Process(random); //You should have Process method in your organism that does everything it needs to do on an update. You probably want to pass in your random number generator
	    //Check/do reproduction if it doesn't make sense to do it in Process

In your .cc file (ex symbulation.cc):
#include "your world or the premade one"

in main:
emp::Random random(my_seed); // Your probably want a seeded random number generator
MyWorldName (or just World<MyOrg>) my_world(random);
my_world.SetGrid(grid_x, grid_y); //You can set lots of things, look in World.h for options

To put organisms in your world:
for (size_t i = 0; i < POP_SIZE; i++) {
    MyOrg *new_org = new MyHost(); //(Maybe don't need to do via pointers?)
    my_world.Inject(*new_org); }

To loop for my_updates:
for (int i = 0; i < num_updates; i++) {
    my_world.Update();
}
