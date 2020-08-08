To create a new world:

class YourWorld : public emp::World\<YourOrg\>{ private: \....

public:

void Update() {

:   emp::World\<YourOrg\>::Update(); //This pings the update signal
    //whatever should happen on an update //Possibly: vector\<size\_t\>
    schedule = emp::GetPermutation(random, GetSize()); //makes sure
    organisms are executed randomly so no bias to \'beginning\' of world
    for (size\_t : schedule) { if (IsOccupied(i) == false) continue;
    pop\[i\]-\>Process(random); //You should have Process method in your
    organism that does everything it needs to do on an update. You
    probably want to pass in your random number generator //Check/do
    reproduction if it doesn\'t make sense to do it in Process

In your .cc file (ex symbulation.cc): \#include \"your world or the
premade one\"

in main: emp::Random random(my\_seed); // Your probably want a seeded
random number generator MyWorldName (or just World\<MyOrg\>)
my\_world(random); my\_world.SetGrid(grid\_x, grid\_y); //You can set
lots of things, look in World.h for options

To put organisms in your world: for (size\_t i = 0; i \< POP\_SIZE; i++)
{ MyOrg *new\_org = new MyHost(); //(Maybe don\'t need to do via
pointers?) my\_world.Inject(*new\_org); }

To loop for my\_updates: for (int i = 0; i \< num\_updates; i++) {
my\_world.Update(); }
