#include "app_eir.h"

#include "utils/stat/stat.h"


static int app_eir_entry( const char *p_pszLogFile )
{
  /* инициализация модуля логирования */
  CHECK_FCT( app_eir_init( p_pszLogFile ) );

  /* регистрация функции валидации пира */
  CHECK_FCT( fd_peer_validate_register( app_pcrf_peer_validate ) );

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
