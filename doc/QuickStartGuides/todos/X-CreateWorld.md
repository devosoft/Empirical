# Create a world

To create a new world:

```cpp
class YourWorld : public emp::World<YourOrg>{
    private: ....

    public:

    void Update() {

        // This pings the update signal whatever should happen on an update
        // Possibly: vector<size_t>
        emp::World<YourOrg>::Update();

        // make sure organisms are executed randomly so no bias to 'beginning' of world
        schedule = emp::GetPermutation(random, GetSize());
        for (size_t : schedule) { if (IsOccupied(i) == false) continue;

        // You should have Process method in your organism that
        // does everything it needs to do on an update.
        // You probably want to pass in your random number generator
        // Check/do reproduction if it doesn't make sense to do it in Process
        pop[i]->Process(random);
```

In your .cc file (ex symbulation.cc):

```cpp
#include "your world or the premade one"
```

in main:

```cpp
int main() {
    /* ... */

    emp::Random random(my_seed); // Your probably want a seeded random number generator
    YourWorld my_world(random); /* or just emp::World<YourOrg> */

    // You can set lots of things, look in World.h for options
    my_world.SetGrid(grid_x, grid_y);

    // To put organisms in your world:
    for (size_t i = 0; i < POP_SIZE; i++) {
        MyOrg *new_org = new MyHost();
        my_world.Inject(*new_org);
    }

    // To loop for my_updates:
    for (int i = 0; i < num_updates; i++) {
        my_world.Update();
    }
}
```
