#pragma once
#include "zgdk/hud/tilemap.h"
#include "zgdk/deprecation.h"

/* Source compatibility only: direct aliases with a diagnostic on use. */
#define print_string \
    ZGDK_DEPRECATED_WARNING_("print_string is deprecated; use hud_tilemap_put_s") \
    hud_tilemap_put_s
#define print_string_layer \
    ZGDK_DEPRECATED_WARNING_("print_string_layer is deprecated; use hud_tilemap_put_s_layer") \
    hud_tilemap_put_s_layer
#define nprint_string \
    ZGDK_DEPRECATED_WARNING_("nprint_string is deprecated; use hud_tilemap_nput_s") \
    hud_tilemap_nput_s
#define nprint_string_layer \
    ZGDK_DEPRECATED_WARNING_("nprint_string_layer is deprecated; use hud_tilemap_nput_s_layer") \
    hud_tilemap_nput_s_layer
#define hud_print_string \
    ZGDK_DEPRECATED_WARNING_("hud_print_string is deprecated; use hud_tilemap_put_s") \
    hud_tilemap_put_s
#define hud_print_string_layer \
    ZGDK_DEPRECATED_WARNING_("hud_print_string_layer is deprecated; use hud_tilemap_put_s_layer") \
    hud_tilemap_put_s_layer
#define hud_nprint_string \
    ZGDK_DEPRECATED_WARNING_("hud_nprint_string is deprecated; use hud_tilemap_nput_s") \
    hud_tilemap_nput_s
#define hud_nprint_string_layer \
    ZGDK_DEPRECATED_WARNING_("hud_nprint_string_layer is deprecated; use hud_tilemap_nput_s_layer") \
    hud_tilemap_nput_s_layer
