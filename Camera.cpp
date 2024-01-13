#include "Camera.hpp"

namespace gps {

    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
    }

    // Return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    // Update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        float velocity = speed;  // You may adjust the speed factor as needed

        glm::vec3 cameraFront = glm::normalize(cameraTarget - cameraPosition);
        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUpDirection));

        switch (direction) {
        case MOVE_RIGHT:
            this->cameraPosition.x += speed;
            this->cameraTarget.x += speed;
            break;
        case MOVE_LEFT:
            this->cameraPosition.x -= speed;
            this->cameraTarget.x -= speed;
            break;
        case MOVE_FORWARD:
            this->cameraPosition.z -= speed;
            this->cameraTarget.z -= speed;
            break;
        case MOVE_BACKWARD:
            this->cameraPosition.z += speed;
            //this->cameraTarget.z += speed;
            break;
        }
    }

    // Update the camera internal parameters following a camera rotate event
    // yaw - camera rotation around the y-axis
    // pitch - camera rotation around the x-axis
    void Camera::rotate(float pitch, float yaw) {
        // Adjust pitch (rotation around the x-axis)
        pitch = glm::radians(pitch);
        const float maxPitch = 89.0f;  // Set your maximum pitch angle here
        pitch = glm::clamp(pitch, -maxPitch, maxPitch);  // Limit pitch angle

        glm::mat4 pitchMatrix = glm::rotate(glm::mat4(1.0f), pitch, cameraRightDirection);
        cameraUpDirection = glm::vec3(pitchMatrix * glm::vec4(cameraUpDirection, 0.0f));
        glm::vec3 rotatedFront = glm::vec3(pitchMatrix * glm::vec4(cameraTarget - cameraPosition, 0.0f));

        // Adjust yaw (rotation around the y-axis)
        yaw = glm::radians(yaw);
        glm::mat4 yawMatrix = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        cameraRightDirection = glm::vec3(yawMatrix * glm::vec4(cameraRightDirection, 0.0f));
        cameraUpDirection = glm::vec3(yawMatrix * glm::vec4(cameraUpDirection, 0.0f));
        glm::vec3 finalFront = glm::vec3(yawMatrix * glm::vec4(rotatedFront, 0.0f));

        // Update the cameraTarget position
        cameraTarget = cameraPosition + finalFront;
        // namespace gps
    }

}