#include <pebble.h>

static Window *s_main_window;
static Layer *s_hands_layer;

// Helper function to draw a rectangular watch hand
static void draw_hand_rect(GContext *ctx, GPoint center, int32_t angle, int16_t length, int16_t tail_length, int16_t thickness) {
  // Define the relative coordinates of the four corners of the rectangle
  GPathInfo hand_path_info = {
    .num_points = 4,
    .points = (GPoint[]) {
      {-thickness / 2, -length},
      {thickness / 2, -length},
      {thickness / 2, tail_length},
      {-thickness / 2, tail_length}
    }
  };
  
  GPath *hand_path = gpath_create(&hand_path_info);
  gpath_move_to(hand_path, center);
  gpath_rotate_to(hand_path, angle);
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_draw_filled(ctx, hand_path);
  
  gpath_destroy(hand_path);
}

// Rendering procedure to draw the watch hands
static void hands_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  // Fill background with white
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  // Calculate hand lengths
  int16_t hour_hand_length = bounds.size.w / 2 * 0.8;
  int16_t minute_hand_length = bounds.size.w / 2 * 0.92;
  
  // Calculate angles
  int32_t hour_angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 60) + t->tm_min)) / (12 * 60);
  int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "[DEBUG] Time: %02d:%02d", t->tm_hour, t->tm_min);
  
  // Enable anti-aliasing for smooth edges
  graphics_context_set_antialiased(ctx, true);
  
  // Draw minute hand
  draw_hand_rect(ctx, center, minute_angle, minute_hand_length, 2, 22);
  
  // Draw hour hand
  draw_hand_rect(ctx, center, hour_angle, hour_hand_length, 0, 28);
}

// Tick timer handler called every minute
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_hands_layer); // Re-render the layer
}

// Setup the main window
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

static void main_window_unload(Window *window) {
  layer_destroy(s_hands_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  // Subscribe to minute ticks
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
