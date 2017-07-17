#include <string>

#include "app_eir.h"

#include "utils/ipconnector/ipconnector.h"
#include "utils/ps_common.h"
#include "utils/pspacket/pspacket.h"
#include "utils/timemeasurer/timemeasurer.h"
#include "utils/stat/stat.h"
#include "utils/log/log.h"

CLog *g_pcoLog;
SStat *g_psoStat;

static int app_eir_get_eq_status( SPSRequest *p_psoResponse, size_t p_stLen );

extern "C"
int app_eir_imei_in_blacklist( octet_string *p_pIMEI, octet_string *p_pSV, octet_string *p_pIMSI )
{
  if ( ( NULL != p_pIMEI && NULL != p_pIMEI->data ) || ( NULL != p_pIMSI && NULL != p_pIMSI->data ) ) {
  } else {
    return 0;
  }

  CTimeMeasurer coTM;
  CTimeMeasurer coTMEIRWS;
  int iRetVal = 0;
  CIPConnector coIPConn( 1 );

  /* устанавливаем соединение с удаленным сервером */
  if ( 0 != coIPConn.Connect( "localhost", 999, IPPROTO_UDP ) ) {
    LOG_E( "can not connect to remote server" );
    return iRetVal;
  } else {
    LOG_D( "connection to remote server established successfully" );
  }

  CPSPacket coPSPack;
  char mcBuf[ 256 ];
  int iFnRes;

  /* инициализация PS пакета */
  if ( 0 != coPSPack.Init( reinterpret_cast<SPSRequest*>( mcBuf ), sizeof( mcBuf ), rand() % 0xffff, EIR_IDENTITYCHK_REQ ) ) {
    LOG_E( "can not initialise PS packet" );
    goto clean_and_exit;
  } else {
    LOG_D( "PS packet is initialized successfully" );
  }

  /* задаем атрибуты */
  /* IMEI */
  if ( NULL != p_pIMEI && NULL != p_pIMEI->data ) {
    iFnRes = coPSPack.AddAttr( reinterpret_cast<SPSRequest*>( mcBuf ), sizeof( mcBuf ), EIR_IMEI, p_pIMEI->data, p_pIMEI->len );
    if ( 0 < iFnRes ) {
    } else {
      goto clean_and_exit;
    }
  }
  /* SV */
  if ( NULL != p_pSV && NULL != p_pSV->data ) {
    iFnRes = coPSPack.AddAttr( reinterpret_cast<SPSRequest*>( mcBuf ), sizeof( mcBuf ), EIR_SV, p_pSV->data, p_pSV->len );
    if ( 0 < iFnRes ) {
    } else {
      goto clean_and_exit;
    }
  }
  /* IMSI */
  if ( NULL != p_pIMSI && NULL != p_pIMSI->data ) {
    /* IMEI */
    iFnRes = coPSPack.AddAttr( reinterpret_cast<SPSRequest*>( mcBuf ), sizeof( mcBuf ), EIR_IMSI, p_pIMSI->data, p_pIMSI->len );
    if ( 0 < iFnRes ) {
    } else {
      goto clean_and_exit;
    }
  }

  coTMEIRWS.Set();

  /* отправляем данные на удаленный сервер */
  iFnRes = coIPConn.Send( mcBuf, iFnRes );
  if ( 0 == iFnRes ) {
#ifdef DEBUG
    LOG_D( "request was sent successfully" );
    char mcParsed[ 1024 ];
    coPSPack.Parse( reinterpret_cast<SPSRequest*>( mcBuf ), sizeof( mcBuf ), mcParsed, sizeof( mcParsed ) );
    LOG_D( "request content: %s", mcParsed );
#endif
  } else {
    goto clean_and_exit;
    LOG_E( "can not send request to remote server" );
  }

  /* если данные успешно отправлены */
  /* пытаемся получить ответ */
  iFnRes = coIPConn.Recv( mcBuf, sizeof( mcBuf ) );
  if ( 0 < iFnRes ) {
#ifdef DEBUG
    LOG_D( "response was received: length '%d'", iFnRes );
    char mcParsed[ 1024 ];
    coPSPack.Parse( reinterpret_cast<SPSRequest*>( mcBuf ), static_cast<size_t>( iFnRes ), mcParsed, sizeof( mcParsed ) );
    LOG_D( "response content: %s", mcParsed );
#endif
    stat_measure( g_psoStat, "EIR_WS", &coTMEIRWS );
    iRetVal = app_eir_get_eq_status( reinterpret_cast<SPSRequest*>( mcBuf ), static_cast<size_t>( iFnRes ) );
  } else {
    goto clean_and_exit;
    LOG_E( "can not receive response" );
  }

  /* если ответ получен */

  clean_and_exit:
  coIPConn.DisConnect();

  stat_measure( g_psoStat, __FUNCTION__, &coTM );

  return iRetVal;
}

static int app_eir_get_eq_status( SPSRequest *p_psoResponse, size_t p_stLen )
{
  int iRetVal = 0;
  CPSPacket coPack;
  std::multimap<__uint16_t, SPSReqAttrParsed> mapAttrList;
  std::multimap<__uint16_t, SPSReqAttrParsed>::iterator iter;

  if ( 0 == coPack.Parse( p_psoResponse, p_stLen, mapAttrList ) ) {
    LOG_D( "response was parsed successfully" );
    iter = mapAttrList.find( EIR_EQUIPMENT_STATUS );
    if ( iter != mapAttrList.end() ) {
      switch ( iter->second.m_usDataLen ) {
        case 4:
          iRetVal = *reinterpret_cast<int*>(iter->second.m_pvData);
          LOG_D( "equepment status presented in uint32_t: value: %d", iRetVal );
          break;
        case 2:
          *reinterpret_cast<uint16_t*>(&iRetVal) = *reinterpret_cast<uint16_t*>( iter->second.m_pvData );
          LOG_D( "equepment status presented in uint16_t: value: %d", iRetVal );
          break;
        case 1:
          *reinterpret_cast<uint8_t*>( &iRetVal ) = *reinterpret_cast<uint8_t*>( iter->second.m_pvData );
          LOG_D( "equepment status presented in uint8_t: value: %d", iRetVal );
          break;
        default:
          LOG_D( "equepment status length '%u': unsupported data type", iter->second.m_usDataLen );
          break;
      }
    } else {
      LOG_D( "EIR_EQUIPMENT_STATUS attribute was not found" );
    }
  } else {
    LOG_D( "response parsing failed" );
  }

  return iRetVal;
}

int app_eir_init( const char *p_pszLogFile )
{
  g_pcoLog = new CLog();
  CHECK_FCT( g_pcoLog->Init( p_pszLogFile ) );

  /* инициализация модуля статистики */
  CHECK_FCT( stat_init() );
  g_psoStat = stat_get_branch( "eir" );

  return 0;
}

void app_eir_fini()
{
  stat_fin();
  if ( NULL != g_pcoLog ) {
    delete g_pcoLog;
    g_pcoLog = NULL;
  }
}
