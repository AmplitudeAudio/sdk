#include <stdio.h>

#include <amplitude_engine.h>
#include <amplitude_memory.h>

int main()
{
    am_memory_manager_config config = am_memory_manager_config_init();
    am_memory_manager_initialize(&config);
    printf(am_memory_manager_get_memory_pool_name(am_memory_pool_kind_amplimix));
    am_bool res = am_engine_is_initialized();

    am_engine_destroy_instance();
    am_memory_manager_deinitialize();

    return 0;
}
