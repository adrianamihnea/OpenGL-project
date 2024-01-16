#include "Camera.hpp"

namespace gps {

    // Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUpDirection, cameraFrontDirection));
        this->cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);

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
            break;
        case MOVE_UP:
            cameraPosition += cameraUpDirection * speed;
            break;
        case MOVE_DOWN:
            cameraPosition -= cameraUpDirection * speed;
            break;
        }
    }

    // Update the camera internal parameters following a camera rotate event
    // yaw - camera rotation around the y-axis
    // pitch - camera rotation around the x-axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFrontDirection = glm::normalize(front);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    void Camera::startVisualization(float angle) {

        this->cameraPosition = glm::vec3(-32.65, -0.66, -20.35); // set the camera position up in the sky   
        this->cameraTarget = glm::vec3(0.0, 1.0, 0.0); // look on the y axis
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0)); // create the rotationMatrix with the universeAngle value
        cameraPosition = glm::vec4(rotationMatrix * glm::vec4(this->cameraPosition, 1)); // update camera position with the rotation
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition); // update the front direction to view the scene in front
    }

}