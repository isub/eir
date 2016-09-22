#include "app_eir.h"

#include <set>

#define CR "\r"
#define LF "\n"

bool operator < (const octet_string &p_soLeft, const octet_string &p_soRight)
{
  /* строка в нашем случае считается меньшей, если ее длина больше
     т.к. последовательность строк в списке необходима от длинных строк к коротким
     т.е. от менее содержательного префикса к более содержательному */
  if (p_soLeft.len > p_soRight.len)
    return 1;

  return 0;
}

std::set<octet_string> g_psetBlackList;

extern "C"
int app_eir_loadblacklist (const char *p_pszConfFile)
{
  int iRetVal = 0;
  FILE *psoFile = NULL;
  char mcBuf[256];
  char *pszCR, *pszLF;
  octet_string soTmp;

  do {
    psoFile = fopen (p_pszConfFile, "r");
    if (NULL == psoFile) {
      iRetVal = errno;
      break;
    }
    while (fgets (mcBuf, sizeof (mcBuf), psoFile)) {
      memset (&soTmp, 0, sizeof (soTmp));
      /* очищаем строку от символов перевода строки */
      pszCR = strstr (mcBuf, CR);
      if (pszCR)
        *pszCR = '\0';
      pszLF = strstr (mcBuf, LF);
      if (pszLF)
        *pszLF = '\0';
      soTmp.len = strlen (mcBuf);
      if (soTmp.len) {
        soTmp.data = (unsigned char*)strdup (mcBuf);
        g_psetBlackList.insert (soTmp);
        LOG_D ("black list: new record: '%s'; len: %d", soTmp.data, soTmp.len);
      }
    }
    if (errno) {
      iRetVal = errno;
      break;
    }
  } while (0);

  if (psoFile) {
    fclose (psoFile);
  }

  return iRetVal;
}

extern "C"
int app_eir_imei_in_blacklist (octet_string *p_pIMEI)
{
  std::set<octet_string>::iterator iter;
  char mcTmp[32];

  memcpy (mcTmp, (char*)p_pIMEI->data, p_pIMEI->len > sizeof (mcTmp) - 1 ? sizeof (mcTmp) - 1 : p_pIMEI->len);
  mcTmp[p_pIMEI->len > sizeof (mcTmp) - 1 ? sizeof (mcTmp) - 1 : p_pIMEI->len] = '\0';

  for (iter = g_psetBlackList.begin (); iter != g_psetBlackList.end (); ++iter) {
    /* проверяем, подходит ли нам очередная строка */
    if (iter->len > p_pIMEI->len)
      continue;
    /* проверяем, совпадает ли префикс IMEI с записью в черном списке */
    if (0 == memcmp ((char*)iter->data, (char*)p_pIMEI->data, iter->len)) {
      LOG_D ("IMEI '%s' hit in black list '%s'", mcTmp, iter->data);
      return 1;
    }
  }

  LOG_D ("IMEI '%s' is clear", mcTmp);

  return 0;
}
