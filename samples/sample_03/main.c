#include <stdio.h>

#include <amplitude_engine.h>
#include <amplitude_memory.h>
#include <amplitude_thread.h>

static const am_uint32 kAppModeMainMenu = 0;
static const am_uint32 kAppModeCollectionTest = 1;
static const am_uint32 kAppModeSwitchContainerTest = 2;

typedef struct am_sample_data
{
    am_filesystem_handle fs_handle;

    am_uint32 mode;

    am_channel_handle main_menu_bg_channel;
    am_channel_handle collection_sample_channel;
    am_channel_handle switch_container_sample_channel;

    am_uint32 current_switch_state;

    am_bool pause;
    am_bool stop;
} am_sample_data;

static void am_log(const char* fmt, va_list args)
{
#if defined(AM_WCHAR_SUPPORTED)
    vfwprintf(stdout, AM_STRING_TO_OS_STRING(fmt), args);
#else
    vfprintf(stdout, fmt, args);
#endif
}

void am_run(am_voidptr param)
{
    am_sample_data* ctx = (am_sample_data*)param;

    const am_filesystem_config fs_config = am_filesystem_config_init_disk();
    ctx->fs_handle = am_filesystem_create(&fs_config);
    am_filesystem_set_base_path(ctx->fs_handle, AM_OS_STRING("test"));

    const char* name = am_memory_manager_get_memory_pool_name(am_memory_pool_kind_amplimix);
    printf("%s\n", name);
    am_memory_free_str(name);

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
