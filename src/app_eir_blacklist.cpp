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

static uint32_t app_eir_get_eq_status( SPSRequest *p_psoResponse, size_t p_stLen );

extern "C"
uint32_t app_eir_is_imei_in_blacklist( octet_string *p_pIMEI, octet_string *p_pSV, octet_string *p_pIMSI )
{
  if ( ( NULL != p_pIMEI && NULL != p_pIMEI->data ) || ( NULL != p_pIMSI && NULL != p_pIMSI->data ) ) {
  } else {
    return 0;
  }

  CTimeMeasurer coTM;
  CTimeMeasurer coTMEIRWS;
  uint32_t uiRetVal = 0;
  CIPConnector coIPConn( 1 );

  /* задаем таймаут для операций с сокетом (0.1 сек.) */
  coIPConn.SetTimeout( 100 );

  /* устанавливаем соединение с удаленным сервером */
  if ( 0 != coIPConn.Connect( g_psoConf->m_pszCServerName, g_psoConf->m_usCServerPort, IPPROTO_UDP ) ) {
    LOG_E( "can not connect to remote server" );
    return uiRetVal;
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
	  if( p_pIMEI->len == 15 ) {
		  iFnRes = coPSPack.AddAttr( reinterpret_cast< SPSRequest* >( mcBuf ), sizeof( mcBuf ), EIR_IMEI, p_pIMEI->data, p_pIMEI->len );
	  } else if( 14 == p_pIMEI->len ) {
		  std::string strIMEI;

		  strIMEI.assign( reinterpret_cast< const char* >( p_pIMEI->data ), p_pIMEI->len );
		  strIMEI += '0';
		  iFnRes = coPSPack.AddAttr( reinterpret_cast< SPSRequest* >( mcBuf ), sizeof( mcBuf ), EIR_IMEI, strIMEI.data(), strIMEI.length() );
	  } else {
		  iFnRes = EINVAL;
	  }

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
    LOG_D( "can not send request to remote server" );
    goto clean_and_exit;
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
    uiRetVal = app_eir_get_eq_status( reinterpret_cast<SPSRequest*>( mcBuf ), static_cast<size_t>( iFnRes ) );
  } else {
    LOG_D( "can not receive response" );
    stat_measure( g_psoStat, "failed", &coTMEIRWS );
    goto clean_and_exit;
  }

  /* если ответ получен */

  clean_and_exit:
  coIPConn.DisConnect();

  stat_measure( g_psoStat, __FUNCTION__, &coTM );

  return uiRetVal;
}

static uint32_t app_eir_get_eq_status( SPSRequest *p_psoResponse, size_t p_stLen )
{
  uint32_t uiRetVal = 0;
  CPSPacket coPack;
  std::multimap<__uint16_t, SPSReqAttrParsed> mapAttrList;
  std::multimap<__uint16_t, SPSReqAttrParsed>::iterator iter;

  if ( 0 == coPack.Parse( p_psoResponse, p_stLen, mapAttrList ) ) {
    LOG_D( "response was parsed successfully" );
    iter = mapAttrList.find( EIR_EQUIPMENT_STATUS );
    if ( iter != mapAttrList.end() ) {
      switch ( iter->second.m_usDataLen ) {
        case 4:
          uiRetVal = ntohl( *reinterpret_cast<uint32_t*>( iter->second.m_pvData ) );
          LOG_D( "equepment status presented in uint32_t: value: %u", uiRetVal );
          break;
        case 2:
          uiRetVal = static_cast<uint32_t>( ntohs( *reinterpret_cast<uint16_t*>( iter->second.m_pvData ) ) );
          LOG_D( "equepment status presented in uint16_t: value: %u", uiRetVal );
          break;
        case 1:
          uiRetVal = static_cast<uint32_t>( *reinterpret_cast<uint8_t*>( iter->second.m_pvData ) );
          LOG_D( "equepment status presented in uint8_t: value: %u", uiRetVal );
          break;
        default:
          LOG_E( "equepment status length '%u': unsupported data type", iter->second.m_usDataLen );
          break;
      }
    } else {
      LOG_D( "EIR_EQUIPMENT_STATUS attribute was not found" );
    }
  } else {
    LOG_D( "response parsing failed" );
  }

  return uiRetVal;
}

int app_eir_init()
{
  g_pcoLog = new CLog();
  CHECK_FCT( g_pcoLog->Init( g_psoConf->m_pszLogFileMask ) );

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
