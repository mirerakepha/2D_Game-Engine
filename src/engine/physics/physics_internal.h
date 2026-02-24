#pragma once

#include "src/engine/array_list.h"
#include "src/engine/types.h"

typedef struct
{
    f32 gravity;
    f32 terminal_velocity;
    Array_List *body_list;
    Array_List *static_body_list;
} Physics_State_Internal;
