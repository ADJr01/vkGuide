//
// Created by adnan on 6/26/25.
//

#ifndef RENDERVUTIL_H
#define RENDERVUTIL_H

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    bool isValidGraphicsFamily() {
        return graphicsFamily >=0;
    }
};



#endif //RENDERVUTIL_H
