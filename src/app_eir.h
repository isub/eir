#include <freeDiameter/freeDiameter-host.h>
#include <freeDiameter/libfdproto.h>
#include <freeDiameter/extension.h>

struct octet_string {
  unsigned char *data;	/* bytes buffer */
  size_t          len;	/* length of the data buffer */
};

#ifdef __cplusplus
extern "C" {
#endif
  /* �������� ��������� */
  int app_eir_loadblacklist (const char *p_pszConfFile);
  /* �������� �������������� IMEI ������� ������ */
  int app_eir_imei_in_blacklist (struct octet_string *p_pIMEI);
#ifdef __cplusplus
}
#endif

/* ������������� ������� */
int eir_dict_init (const char *p_pszConfFile);
/* ������������� ����������� ECR ������ */
int app_eir_server_init ();
/* �������� ������� */
void app_eir_server_fini (void);

/* ��������� ������� */
int app_pcrf_peer_validate (struct peer_info *p_psoPeerInfo, int *p_piAuth, int (**cb2)(struct peer_info *));

/* ������������ ������� ������� */
extern struct dict_object *g_psoDict3GPPVend;
extern struct dict_object *g_psoDictAppEIR;
extern struct dict_object *g_psoDictCmdECR;
extern struct dict_object *g_psoDictCmdECA;

extern struct dict_object *g_psoAuthSessionState;
extern struct dict_object *g_psoOriginHost;
extern struct dict_object *g_psoOriginRealm;
extern struct dict_object *g_psoDestinationHost;
extern struct dict_object *g_psoDestinationRealm;
extern struct dict_object *g_psoTerminalInformation;
extern struct dict_object *g_psoIMEI;
extern struct dict_object *g_pso3GPP2MEID;
extern struct dict_object *g_psoSoftwareVersion;
extern struct dict_object *g_psoDictEquipmentStatus;