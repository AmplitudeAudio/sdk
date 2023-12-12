#include <stdio.h>

#include <amplitude_engine.h>
#include <amplitude_memory.h>
#include <amplitude_thread.h>

typedef struct am_sample_data
{
} am_sample_data;

void am_run(am_voidptr param)
{
    printf("am_run\n");

    printf("%s\n", am_memory_manager_get_memory_pool_name(am_memory_pool_kind_amplimix));
    am_bool res = am_engine_is_initialized();

    am_engine_destroy_instance();
}

int main()
{
    am_memory_manager_config config = am_memory_manager_config_init();
    am_memory_manager_initialize(&config);

    am_sample_data sample_data;
    am_thread_handle h_thread = am_thread_create(am_run, &sample_data);

    while (AM_TRUE)
    {
        am_uint32 option;
        scanf("%d", &option);

        if (option == 0)
            break;
    }

    am_thread_wait(h_thread);
    am_thread_release(h_thread);

    am_memory_manager_deinitialize();

    return 0;
}
