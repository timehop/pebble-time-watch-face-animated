#include <pebble.h>

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;

static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame
  if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
    app_timer_register(next_delay, timer_handler, NULL); // Timer for that delay
  } else {
    // Start again
    gbitmap_sequence_restart(s_sequence);
  }
}

static void update_datetime() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char time_buffer[] = "00:00";
  static char date_buffer[] = "Sun, Oct 25";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(time_buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  strftime(date_buffer, sizeof("Sun, Oct 25"), "%a, %b %e", tick_time);

  // Display on the TextLayer
  text_layer_set_text(s_time_layer, time_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_datetime();
}

static void load_sequence() {
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }

  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIMATION);
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  app_timer_register(1, timer_handler, NULL);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  // BitmapLayer - begin
  s_bitmap_layer = bitmap_layer_create(GRect(48, 0, 98, 98));
  bitmap_layer_set_background_color(s_bitmap_layer, (GColor8){ .argb = 0b11111000 });
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  load_sequence();
  // BitmapLayer - end

  // Time - begin
  s_time_layer = text_layer_create(GRect(0, 99, 144, 42));
  text_layer_set_background_color(s_time_layer, (GColor8){ .argb = 0b11111000 });
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");

  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  // Time - end

  // Date - begin
  s_date_layer = text_layer_create(GRect(0, 142, 144, 24));
  text_layer_set_background_color(s_date_layer, (GColor8){ .argb = 0b11111000 });
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "00:00");

  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  // Date - end
  update_datetime();
}

static void main_window_unload(Window *window) {
  bitmap_layer_destroy(s_bitmap_layer);
  gbitmap_sequence_destroy(s_sequence);
  gbitmap_destroy(s_bitmap);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
}

static void init() {
  s_main_window = window_create();

  window_set_background_color(s_main_window, (GColor8){ .argb = 0b11111000 });
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}