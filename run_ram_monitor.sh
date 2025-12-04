#!/bin/bash
# Clear snap-related environment variables that might interfere
unset GTK_EXE_PREFIX
unset GTK_PATH
unset GTK_IM_MODULE_FILE
unset LD_LIBRARY_PATH

# Run the Shared Memory Monitor
./shm_monitor
