/* The linked image must contain only camera_init_view from the camera API. */
#include "zgdk/camera.h"

int main(void) {
    camera_t camera;
    return camera_init_view(&camera, 1600, 960, 320, 240);
}
