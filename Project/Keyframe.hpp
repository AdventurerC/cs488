#include <glm/glm.hpp>

class Keyframe {
public:
    Keyframe();

    glm::vec3 position;
    glm::vec3 scale;

    glm::vec3 nextPosition;
    glm::vec3 nextScale;
}