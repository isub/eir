#include "app_eir.h"

static int app_eir_entry (const char *p_pszConfFile)
{
  /* ����������� ������� ��������� ���� */
  CHECK_FCT (fd_peer_validate_register (app_pcrf_peer_validate));

  /* �������� ������� ������ */
  CHECK_FCT (app_eir_loadblacklist (p_pszConfFile));

  /* ������������� ������� */
  CHECK_FCT (eir_dict_init (NULL));

  /* ����������� callback ������� ��������� ECR �������� */
  CHECK_FCT (app_eir_server_init ());

  /* ����������� ���������� */
  CHECK_FCT (fd_disp_app_support (g_psoDictAppEIR, g_psoDict3GPPVend, 1, 0));

  return 0;
}

void fd_ext_fini (void)
{
  app_eir_server_fini ();
}

EXTENSION_ENTRY ("app_eir", app_eir_entry);
