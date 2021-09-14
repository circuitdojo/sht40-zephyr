#
# Copyright (c) 2021 Circuit Dojo LLC
#

# Subdirectories
add_subdirectory_ifdef(CONFIG_SHT40 driver)

# Include headers
zephyr_include_directories(include)