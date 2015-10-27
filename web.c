// file web.c - web interface

/**   Copyright (C)  2015  Basile Starynkevitch and later the FSF
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.
  
    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.
  
    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

#include "monimelt.h"

static onion *onion_mom;

static volatile atomic_long webcount_mom;

static onion_connection_status
handle_web_mom (void *data, onion_request *requ, onion_response *resp);

#define MAX_ONIONTHREADS_MOM 4
void
mom_start_web (const char *webservice)
{
  MOM_DEBUGPRINTF (web, "mom_start_web starting webservice=%s", webservice);
  char webhostname[80];
  memset (webhostname, 0, sizeof (webhostname));
  int webport = 0;
  int pos = -1;
  if (isalpha (webservice[0]))
    {
      if (sscanf
          (webservice, "%70[A-Za-z0-9_.+-]:%d %n", webhostname, &webport,
           &pos) >= 2 && pos > 0)
        {
          MOM_DEBUGPRINTF (web, "webhostname=%s webport#%d", webhostname,
                           webport);
        }
      else
        MOM_FATAPRINTF ("bad webservice %s with host", webservice);
    }
  else if (webservice[0] == ':' && isdigit (webservice[1])
           && (webport = atoi (webservice + 1)) > 0)
    {
      MOM_DEBUGPRINTF (web, "webport#%d", webport);
    }
  else
    MOM_FATAPRINTF ("invalid webservice %s", webservice);
  onion_mom =
    onion_new (O_THREADED | O_DETACH_LISTEN | O_NO_SIGTERM | O_NO_SIGPIPE);
  onion_set_max_threads (onion_mom, MAX_ONIONTHREADS_MOM);
  if (webhostname[0])
    onion_set_hostname (onion_mom, webhostname);
  if (webport > 0)
    {
      char portbuf[16];
      memset (portbuf, 0, sizeof (portbuf));
      snprintf (portbuf, sizeof (portbuf), "%d", webport);
      onion_set_port (onion_mom, portbuf);
    }
  {
    onion_url *ourl = onion_root_url (onion_mom);
    int onerr = 0;
    if ((onerr =
         onion_url_add_handler (ourl, "^",
                                onion_handler_new (handle_web_mom, NULL,
                                                   NULL))) != 0)
      MOM_FATAPRINTF ("failed to add generic webhandler (onionerr#%d)",
                      onerr);
  }
  MOM_DEBUGPRINTF (web, "mom_start_web before listening @%p", onion_mom);
  onion_listen (onion_mom);
  atomic_store (&mom_should_run, true);
  MOM_INFORMPRINTF ("web service on %s started with libonion", webservice);
}                               /* end mom_start_web */


char *
mom_webmethod_name (unsigned wm)
{
  switch (wm)
    {
    case MOMWEBM_NONE:
      return "*none*";
    case MOMWEBM_HEAD:
      return "HEAD";
    case MOMWEBM_GET:
      return "GET";
    case MOMWEBM_POST:
      return "POST";
    default:
      {
        char buf[16];
        memset (buf, 0, sizeof (buf));
        snprintf (buf, sizeof (buf), "?webmeth#%u?", wm);
        return GC_STRDUP (buf);
      }
    }
}                               /* end mom_webmethod_name */



static onion_connection_status
handle_web_mom (void *data, onion_request *requ, onion_response *resp)
{
  enum mom_webmethod_en wmeth = MOMWEBM_NONE;
  assert (data == NULL);
  assert (requ != NULL);
  assert (resp != NULL);
  long reqcnt = atomic_fetch_add (&webcount_mom, 1) + 1;
  const char *reqfupath = onion_request_get_fullpath (requ);
  const char *reqpath = onion_request_get_path (requ);
  unsigned reqflags = onion_request_get_flags (requ);
  MOM_DEBUGPRINTF (web, "webrequest#%ld fullpath '%s' path '%s' flags %#x",
                   reqcnt, reqfupath, reqpath, reqflags);
  if ((reqflags & OR_METHODS) == OR_HEAD)
    wmeth = MOMWEBM_HEAD;
  else if ((reqflags & OR_METHODS) == OR_GET)
    wmeth = MOMWEBM_GET;
  else if ((reqflags & OR_METHODS) == OR_POST)
    wmeth = MOMWEBM_POST;
  else
    {
      MOM_WARNPRINTF
        ("webrequest#%ld fullpath '%s' bad flags %#x with unknown method",
         reqcnt, reqfupath, reqflags);
      return OCS_INTERNAL_ERROR;
    }
  MOM_DEBUGPRINTF (web, "webrequest#%ld fullpath %s wmeth %s",
                   reqcnt, reqfupath, mom_webmethod_name (wmeth));
  /// reject .. in full path, so reject URL going outside the web doc root
  if (strstr (reqfupath, ".."))
    {
      MOM_DEBUGPRINTF (web, "webrequest#%ld twodots in fullpath %s", reqcnt,
                       reqfupath);
      return OCS_NOT_PROCESSED;
    }
  // reject path starting with dot
  if (reqfupath[0] == '.' || reqfupath[1] == '.')
    {
      MOM_DEBUGPRINTF (web, "webrequest#%ld dot starting fullpath %s", reqcnt,
                       reqfupath);
      return OCS_NOT_PROCESSED;
    }
  // reject too long paths
  if (strlen (reqfupath) >= MOM_PATH_MAX - 16)
    {
      MOM_DEBUGPRINTF (web, "webrequest#%ld too long (%d) fullpath %s",
                       reqcnt, (int) strlen (reqfupath), reqfupath);
      return OCS_NOT_PROCESSED;
    }
  /// scan the mom_webdir-s
  if (wmeth != MOMWEBM_POST && reqfupath[0] == '/' && isalnum (reqfupath[1]))
    {
      MOM_DEBUGPRINTF (web, "webrequest#%ld reqfupath %s could be file",
                       reqcnt, reqfupath);
      char fpath[MOM_PATH_MAX + 64];
      memset (fpath, 0, sizeof (fpath));
      for (unsigned wix = 0; wix < MOM_MAX_WEBDIR; wix++)
        {
          if (!mom_webdir[wix])
            continue;
          memset (fpath, 0, sizeof (fpath));
          if (snprintf (fpath, sizeof (fpath), "%s%s",
                        mom_webdir[wix], reqfupath)
              < (int) sizeof (fpath) && !access (fpath, R_OK))
            {
              MOM_DEBUGPRINTF (web, "handle_web_mom request#%ld got fpath %s",
                               reqcnt, fpath);
              return onion_shortcut_response_file (fpath, requ, resp);
            }
        }
    }
  return OCS_NOT_PROCESSED;
}                               /* end of handle_web_mom */


////////////////

void
mom_webexch_payload_cleanup (struct mom_item_st *itm,   //
                             struct mom_webexch_st *payl)
{
  MOM_DEBUGPRINTF (web,
                   "webexch_payload_cleanup itm %s payl@%p count#%ld",
                   mom_item_cstring (itm), (void *) payl, payl->webx_count);
  if (payl->webx_requ)
    {
      onion_request *orq = payl->webx_requ;
      payl->webx_requ = NULL;
      onion_request_free (orq);
    }
  if (payl->webx_resp)
    {
      onion_response *ors = payl->webx_resp;
      payl->webx_resp = NULL;
      onion_response_free (ors);
    }
  if (payl->webx_outfil)
    {
      FILE *outf = payl->webx_outfil;
      payl->webx_outfil = NULL;
      fclose (outf);
    }
  if (payl->webx_outbuf)
    {
      char *obuf = payl->webx_outbuf;
      payl->webx_outbuf = NULL;
      free (obuf);
    };
  payl->webx_outsiz = 0;
  payl->webx_path = NULL;
  payl->webx_sessitm = NULL;
  pthread_cond_destroy (&payl->webx_donecond);
}                               /* end of mom_webexch_payload_cleanup */




////////////////
void
mom_websession_payload_cleanup (struct mom_item_st *itm,        //
                                struct mom_websession_st *payl)
{
  MOM_DEBUGPRINTF (web, "websession_payload_cleanup itm %s payl@%p",
                   mom_item_cstring (itm), (void *) payl);
  if (payl->wbss_websock)
    {
      onion_websocket *ws = payl->wbss_websock;
      payl->wbss_websock = NULL;
      onion_websocket_close (ws);
      onion_websocket_free (ws);
    }
  payl->wbss_inbuf = NULL;
  payl->wbss_insiz = 0;
  payl->wbss_inoff = 0;
}                               /* end of mom_websession_payload_cleanup */
