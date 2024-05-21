
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "firefly-scene.h"


static void delay(uint32_t duration) {
    vTaskDelay((duration + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS);
}

void app_main(void) {
  FfxScene scene = ffx_scene_init(32);
  FfxNode root = ffx_scene_root(scene);
  FfxSize size = { .width = 20, .height = 20 };
  FfxNode box = ffx_scene_createBox(scene, size, 0);
  ffx_scene_appendChild(root, box);
  printf("done!\n");

  while (1) {
    delay(1000);
  }
}
