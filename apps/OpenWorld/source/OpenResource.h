/// These are resources floating around the OpenWorld.

#ifndef OPEN_RESOURCE_H
#define OPEN_RESOURCE_H

struct OpenResource {
  size_t resource_type;  // What type of resource is this?
  size_t surface_id;     // Which surface object represents this resource?
};

#endif
