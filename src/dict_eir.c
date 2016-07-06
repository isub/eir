#include <freeDiameter/extension.h>

/*
Command Name                      Abbreviation  Code    Section
ME-Identity-Check-Request         ECR           324     7.2.19
ME-Identity-Check-Answer          ECA           324     7.2.20
Application-Id 16777252
*/
struct dict_object *g_psoDict3GPPVend;
struct dict_object *g_psoDictAppEIR;
/*
ME-Identity-Check-Request (ECR) Command
Message Format
< ME-Identity-Check-Request > ::=     < Diameter Header: 324, REQ, PXY, 16777252 >
< Session-Id >
[ Vendor-Specific-Application-Id ]
{ Auth-Session-State }
{ Origin-Host }
{ Origin-Realm }
[ Destination-Host ]
{ Destination-Realm }
{ Terminal-Information }
[ User-Name ]
*[ AVP ]
*[ Proxy-Info ]
*[ Route-Record ]
*/
struct dict_object *g_psoDictCmdECR;
/*
ME-Identity-Check-Answer (ECA) Command
Message Format
< ME-Identity-Check-Answer> ::=
< Diameter Header: 324, PXY, 16777252 >
< Session-Id >
[ DRMP ]
[ Vendor-Specific-Application-Id ]
{ Auth-Session-State }
{ Origin-Host }
{ Origin-Realm }
[ Destination-Host ]
{ Destination-Realm }
{ Terminal-Information }
[ User-Name ]
*[ AVP ]
*[ Proxy-Info ]
*[ Route-Record ]
*/
struct dict_object *g_psoDictCmdECA;

/* идентификаторы объетов словаря, необходимые для работы */
struct dict_object *g_psoAuthSessionState;
struct dict_object *g_psoOriginHost;
struct dict_object *g_psoOriginRealm;
struct dict_object *g_psoDestinationHost;
struct dict_object *g_psoDestinationRealm;
struct dict_object *g_psoTerminalInformation;
struct dict_object *g_psoIMEI;
struct dict_object *g_pso3GPP2MEID;
struct dict_object *g_psoSoftwareVersion;
struct dict_object *g_psoDictEquipmentStatus;

/* The content of this file follows the same structure as dict_base_proto.c */

#define CHECK_dict_new(_type, _data, _parent, _ref)			\
  CHECK_FCT(fd_dict_new(fd_g_config->cnf_dict, (_type), (_data), (_parent), (_ref)));

#define CHECK_dict_search(_type, _criteria, _what, _result)		\
  CHECK_FCT(fd_dict_search(fd_g_config->cnf_dict, (_type), (_criteria), (_what), (_result), ENOENT));
/*==================================================================*/

#define RULE_ORDER(_position ) ((((_position) == RULE_FIXED_HEAD) || ((_position) == RULE_FIXED_TAIL)) ? 1 : 0)

/*==================================================================*/
#define PARSE_loc_rules(_rulearray, _parent, _avp_search_flag) {	\
    int __ar;								\
    for (__ar=0; __ar < sizeof(_rulearray) / sizeof((_rulearray)[0]); __ar++) {	\
      struct dict_rule_data __data = { NULL,				\
				       (_rulearray)[__ar].position,	\
				       0,				\
				       (_rulearray)[__ar].min,		\
				       (_rulearray)[__ar].max};		\
      __data.rule_order = RULE_ORDER(__data.rule_position);		\
                                                                        \
      CHECK_FCT(  fd_dict_search(					\
				 fd_g_config->cnf_dict,			\
				 DICT_AVP,				\
				 _avp_search_flag,			\
				 (_rulearray)[__ar].avp_name,		\
				 &__data.rule_avp, 0 ) );		\
      if ( !__data.rule_avp ) {						\
	TRACE_DEBUG(INFO, "AVP Not found: '%s'", (_rulearray)[__ar].avp_name );	\
	return ENOENT;							\
            }									\
                                                                        \
      CHECK_FCT_DO(fd_dict_new(fd_g_config->cnf_dict, DICT_RULE, &__data, _parent, NULL), \
        		    {							\
		      TRACE_DEBUG(INFO, "Error on rule with AVP '%s'",	\
				  (_rulearray)[__ar].avp_name );	\
		      return EINVAL;					\
        		    } );						\
        }									\
    }

struct local_rules_definition
{
  char                *avp_name;
  enum rule_position  position;
  int                 min;
  int                 max;
};

int eir_dict_init (const char *p_pszConfFile)
{
  {
    struct dict_vendor_data vendor_data = { 10415, "3GPP" };
    CHECK_dict_new (DICT_VENDOR, &vendor_data, NULL, &g_psoDict3GPPVend);
  }

  {
    struct dict_application_data data = { 16777252, "Diameter S13/S13' Application" };
    CHECK_dict_new (DICT_APPLICATION, &data, g_psoDict3GPPVend, &g_psoDictAppEIR);
  }
  /* Result codes */
  {
    struct dict_object *ResultCodeType;
    CHECK_dict_search (DICT_TYPE, TYPE_BY_NAME, "Enumerated*(Result-Code)", &ResultCodeType);

    {
      struct dict_enumval_data error_code = {"DIAMETER_ERROR_EQUIPMENT_UNKNOWN", { .u32 = 5422 }};
      CHECK_dict_new (DICT_ENUMVAL, &error_code, ResultCodeType, NULL);
    }
  }

  /* additional AVPs */
  struct dict_object *UTF8String_type;
  CHECK_dict_search (DICT_TYPE, TYPE_BY_NAME, "UTF8String", &UTF8String_type);
  /* IMEI */
  {
    /*
    UTF8String.
    */
    struct dict_avp_data    data = {
      1402,                                   /* Code */
      10415,                                  /* Vendor */
      "IMEI",                                 /* Name */
      AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY,   /* Fixed flags */
      AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,   /* Fixed flag values */
      AVP_TYPE_OCTETSTRING                    /* base type of data */
    };
    CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
  }
  /* Software-Version */
  {
    /*
    UTF8String.
    */
    struct dict_avp_data    data = {
      1403,                                   /* Code */
      10415,                                  /* Vendor */
      "Software-Version",                     /* Name */
      AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY,   /* Fixed flags */
      AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,   /* Fixed flag values */
      AVP_TYPE_OCTETSTRING                    /* base type of data */
    };
    CHECK_dict_new (DICT_AVP, &data, UTF8String_type, NULL);
  }
  /* 3GPP2-MEID */
  {
    /*
    OctetString.
    */
    struct dict_avp_data data = {
      1471,                                   /* Code */
      10415,                                  /* Vendor */
      "3GPP2-MEID",                           /* Name */
      AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY,   /* Fixed flags */
      AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,   /* Fixed flag values */
      AVP_TYPE_OCTETSTRING                    /* base type of data */
    };
    CHECK_dict_new (DICT_AVP, &data, NULL, NULL);
  }
  /* Terminal-Information */
  {
    /*
    Grouped
    */
    struct dict_object * avp;
    struct dict_avp_data data = {
      1401,                                   /* Code */
      10415,                                  /* Vendor */
      "Terminal-Information",                 /* Name */
      AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY,   /* Fixed flags */
      AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,   /* Fixed flag values */
      AVP_TYPE_GROUPED                        /* base type of data */
    };
    struct local_rules_definition rules[] = {
      { "IMEI", RULE_OPTIONAL, -1, 1 },
      { "3GPP2-MEID", RULE_OPTIONAL, -1, 1 },
      { "Software-Version", RULE_OPTIONAL, -1, 1 }
    };
    CHECK_dict_new (DICT_AVP, &data, NULL, &avp);
    PARSE_loc_rules (rules, avp, AVP_BY_NAME_ALL_VENDORS);
  }
  /* Equipment-Status */
  {
    /*
    Enumerated
    */
    struct dict_object *type;
    struct dict_type_data           tdata = { AVP_TYPE_UNSIGNED32, "Enumerated(Equipment-Status)", NULL, NULL, NULL };
    struct dict_avp_data data = {
      1445,                                   /* Code */
      10415,                                  /* Vendor */
      "Equipment-Status",                     /* Name */
      AVP_FLAG_VENDOR | AVP_FLAG_MANDATORY,   /* Fixed flags */
      AVP_FLAG_MANDATORY | AVP_FLAG_VENDOR,   /* Fixed flag values */
      AVP_TYPE_UNSIGNED32                     /* base type of data */
    };
    CHECK_dict_new (DICT_TYPE, &tdata, NULL, &type);
    {
      struct dict_enumval_data enum_value = { "WHITELISTED", { .u32 = 0 } };
      CHECK_dict_new (DICT_ENUMVAL, &enum_value, type, NULL);
    }
    {
      struct dict_enumval_data enum_value = { "BLACKLISTED", { .u32 = 1 } };
      CHECK_dict_new (DICT_ENUMVAL, &enum_value, type, NULL);
    }
    {
      struct dict_enumval_data enum_value = { "GREYLISTED", { .u32 = 2 } };
      CHECK_dict_new (DICT_ENUMVAL, &enum_value, type, NULL);
    }
    CHECK_dict_new (DICT_AVP, &data, type, NULL);
  }

  /* command section*/
  {
    /* ME-Identity-Check-Request (ECR) Command */
    struct dict_cmd_data  data =
    {
      324,                   	      /* Code */
      "ME-Identity-Check-Request",  /* Name */
      CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE,  /* Fixed flags */
      CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE   /* Fixed flag values */
    };

    struct local_rules_definition rules[] =
    {
      { "Session-Id", RULE_FIXED_HEAD, -1, 1 },
      { "Vendor-Specific-Application-Id", RULE_OPTIONAL, -1, 1 },
      { "Auth-Session-State", RULE_REQUIRED, -1, 1 },
      { "Origin-Host", RULE_REQUIRED, -1, 1 },
      { "Origin-Realm", RULE_REQUIRED, -1, 1 },
      { "Destination-Host", RULE_OPTIONAL, -1, 1 },
      { "Destination-Realm", RULE_REQUIRED, -1, 1 },
      { "Terminal-Information", RULE_REQUIRED, -1, 1 },
      { "User-Name", RULE_OPTIONAL, -1, 1 },
      { "Proxy-Info", RULE_OPTIONAL, -1, -1 },
      { "Route-Record", RULE_OPTIONAL, -1, -1 }
    };

    CHECK_dict_new (DICT_COMMAND, &data, g_psoDictAppEIR, &g_psoDictCmdECR);
    PARSE_loc_rules (rules, g_psoDictCmdECR, AVP_BY_NAME_ALL_VENDORS);
  }
  {
    /* ME-Identity-Check-Answer (ECA) Command */
    struct dict_cmd_data  data =
    {
      324,                   	                /* Code */
      "ME-Identity-Check-Answer",             /* Name */
      CMD_FLAG_REQUEST | CMD_FLAG_PROXIABLE,  /* Fixed flags */
      CMD_FLAG_PROXIABLE                      /* Fixed flag values */
    };

    struct local_rules_definition rules[] =
    {
      { "Session-Id", RULE_FIXED_HEAD, -1, 1 },
/*      { "DRMP", RULE_OPTIONAL, -1, 1 }, */
      { "Vendor-Specific-Application-Id", RULE_OPTIONAL, -1, 1 },
      { "Auth-Session-State", RULE_REQUIRED, -1, 1 },
      { "Origin-Host", RULE_REQUIRED, -1, 1 },
      { "Origin-Realm", RULE_REQUIRED, -1, 1 },
      { "Destination-Host", RULE_OPTIONAL, -1, 1 },
      { "Destination-Realm", RULE_REQUIRED, -1, 1 },
      { "Terminal-Information", RULE_REQUIRED, -1, 1 },
      { "User-Name", RULE_OPTIONAL, -1, 1 },
      { "Proxy-Info", RULE_OPTIONAL, -1, -1 },
      { "Route-Record", RULE_OPTIONAL, -1, -1 }
    };

    CHECK_dict_new (DICT_COMMAND, &data, g_psoDictAppEIR, &g_psoDictCmdECA);
    PARSE_loc_rules (rules, g_psoDictCmdECA, AVP_BY_NAME_ALL_VENDORS);
  }

  /* кешируем необходимые идентификаторы */
  /* Auth-Session-State */
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Auth-Session-State", &g_psoAuthSessionState);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Origin-Host", &g_psoOriginHost);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Origin-Realm", &g_psoOriginRealm);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Destination-Host", &g_psoDestinationHost);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME, "Destination-Realm", &g_psoDestinationRealm);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Terminal-Information", &g_psoTerminalInformation);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "IMEI", &g_psoIMEI);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "3GPP2-MEID", &g_pso3GPP2MEID);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Software-Version", &g_psoSoftwareVersion);
  CHECK_dict_search (DICT_AVP, AVP_BY_NAME_ALL_VENDORS, "Equipment-Status", &g_psoDictEquipmentStatus);

  LOG_D ("module dict_eir is initialized");

  return 0;
}
