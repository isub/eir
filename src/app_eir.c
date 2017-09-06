#include "app_eir.h"

#include "utils/stat/stat.h"

static struct SEIRConf g_soConf;
struct SEIRConf * g_psoConf;

static int app_eir_entry( const char *p_pszConfFile )
{
  g_psoConf = &g_soConf;

  /* чтение файла конфигурации */
  CHECK_FCT( app_eir_conf_handle( (char*) p_pszConfFile ) );

  /* инициализация модуля логирования */
  CHECK_FCT( app_eir_init() );

  /* регистрация функции валидации пира */
  CHECK_FCT( fd_peer_validate_register( app_eir_peer_validate ) );

  /* инициализация словаря */
  CHECK_FCT( eir_dict_init() );

  /* регистрация приложения */
  CHECK_FCT( fd_disp_app_support( g_psoDictAppEIR, g_psoDict3GPPVend, 1, 0 ) );

  /* регистрация callback функции обработки ECR запросов */
  CHECK_FCT( app_eir_server_init() );

  return 0;
}

void fd_ext_fini (void)
{
  app_eir_server_fini();
  app_eir_fini();
}

EXTENSION_ENTRY ("app_eir", app_eir_entry);
