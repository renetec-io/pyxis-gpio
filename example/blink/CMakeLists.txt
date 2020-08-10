# Copyright (c) Renetec, Inc. All rights reserved.

add_executable(blink blink.c)
target_include_directories(blink PRIVATE ${PROJECT_SOURCE_DIR}/inc)
