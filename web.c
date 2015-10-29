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
  if (MOM_IS_DEBUGGING (web))
    for (unsigned wix = 0; wix < MOM_MAX_WEBDIR; wix++)
      {
        if (mom_webdir[wix])
          MOM_DEBUGPRINTF (web, "mom_start_web mom_webdir[%d]='%s'",
                           wix, mom_webdir[wix]);
      };
  MOM_DEBUGPRINTF (web,
                   "mom_start_web before listening @%p MOM_MAX_WEBDIR=%d",
                   onion_mom, MOM_MAX_WEBDIR);
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


#define MOM_MAX_WEB_FILE_SIZE (4096*1024)       /* 4 megabytes */

onion_connection_status
mom_hackc_code (long reqcnt, onion_request *requ, onion_response *resp)
{
  typedef void momhackfun_t (struct mom_item_st *);
  struct mom_item_st *hackitm = NULL;
  const char *hackitmstr = NULL;
  char hackpath[80];
  memset (hackpath, 0, sizeof (hackpath));
  MOM_DEBUGPRINTF (web, "hackc_code start webrequest#%ld", reqcnt);
  const char *do_hackc = onion_request_get_post (requ, "do_hackc");
  const char *do_stop = onion_request_get_post (requ, "do_stop");
  MOM_DEBUGPRINTF (web, "hackc_code #%ld do_hackc=%s do_stop=%s",
                   reqcnt, do_hackc, do_stop);
  if (do_hackc && do_hackc[0])
    {
      const char *prologuetxt = onion_request_get_post (requ, "prologuetxt");
      const char *initialtxt = onion_request_get_post (requ, "initialtxt");
      MOM_DEBUGPRINTF (web, "hackc_code #%ld prologuetxt=%s", reqcnt,
                       prologuetxt);
      MOM_DEBUGPRINTF (web, "hackc_code #%ld initialtxt=%s", reqcnt,
                       initialtxt);
      hackitm = mom_clone_item (MOM_PREDEFITM (hackc));
      hackitmstr = mom_item_cstring (hackitm);
      MOM_DEBUGPRINTF (web, "hackc_code #%ld hackitm=%s", reqcnt, hackitmstr);
      snprintf (hackpath, sizeof (hackpath), "modules/momg_%s.c", hackitmstr);
      FILE *fhack = fopen (hackpath, "w");
      if (!fhack)
        MOM_FATAPRINTF ("failed to open hack module %s - %m", hackpath);
      mom_output_gplv3_notice (fhack, "///-", "", basename (hackpath));
      fputs ("\n\n", fhack);
      fputs ("#include \"monimelt.h\"\n\n", fhack);
      fprintf (fhack, "const char momtimestamp_%s[]= __TIMESTAMP__;\n\n",
               hackitmstr);
      if (prologuetxt && prologuetxt[0])
        {
          fprintf (fhack, "#line 1 \"%s-prologue\"\n", hackitmstr);
          fputs (prologuetxt, fhack);
          fputs ("\n\n", fhack);
        }
      fprintf (fhack, "\n#line 1 \"%s-fun\"\n", hackitmstr);
      fprintf (fhack, "void momhack_%s(struct mom_item_st*momhackitm) {\n"
               "  MOM_DEBUGPRINTF(web, \"momhack_%s start\");\n",
               hackitmstr, hackitmstr);
      fprintf (fhack,
               "  assert(momhackitm && momhackitm->va_itype==MOMITY_ITEM);\n");
      if (initialtxt && initialtxt[0])
        {
          fprintf (fhack, "#line 1 \"%s-initial\"\n", hackitmstr);
          fputs (initialtxt, fhack);
          fputs ("\n\n", fhack);
        }
      fprintf (fhack,
               "\n#line 1 \"%s-tail\"\n"
               "  MOM_DEBUGPRINTF(web, \"momhack_%s end %%s\",\n"
               "                  mom_item_cstring(momhackitm));\n",
               hackitmstr, hackitmstr);
      fprintf (fhack, "  MOM_INFORMPRINTF(\"done momhack %s\");\n",
               hackitmstr);
      fprintf (fhack, "} // end momhack_%s\n\n", hackitmstr);
      fprintf (fhack, "\n" "// eof generated %s\n", basename (hackpath));
      fclose (fhack), fhack = NULL;
      FILE *cmdf = NULL;
      {
        char compilcmd[256];
        memset (compilcmd, 0, sizeof (compilcmd));
        snprintf (compilcmd, sizeof (compilcmd),
                  "make modules/momg_%s.so 2>&1", hackitmstr);
        MOM_DEBUGPRINTF (web, "hackc_code #%ld compilcmd:%s", reqcnt,
                         compilcmd);
        fflush (NULL);
        cmdf = popen (compilcmd, "r");
        if (!cmdf)
          MOM_FATAPRINTF ("failed to popen %s - %m", compilcmd);
        usleep (50000);
      }
      char *outbuf = NULL;
      size_t outsiz = 0;
      FILE *outf = open_memstream (&outbuf, &outsiz);
      char *linbuf = NULL;
      size_t linsiz = 0;
      int lincnt = 0;
      do
        {
          ssize_t linlen = getline (&linbuf, &linsiz, cmdf);
          if (linlen <= 0)
            break;
          lincnt++;
          MOM_DEBUGPRINTF (web, "hack_code #%ld compilin#%d: %s", reqcnt,
                           lincnt, linbuf);
          if (lincnt > 1000)
            MOM_FATAPRINTF
              ("hack_code #%ld too many (%d) lines from compilation of momg_%s.c",
               reqcnt, lincnt, hackitmstr);
          fputs (linbuf, outf);
        }
      while (!feof (cmdf));
      int comperr = pclose (cmdf);
      fflush (outf);
      MOM_DEBUGPRINTF (web,
                       "hack_code #%ld comperr %d lincnt %d outbuf==\n%s\n##end outbuf%ld",
                       reqcnt, comperr, lincnt, outbuf, reqcnt);
      if (!comperr)
        {
          // backup the generated C code
          {
            char backupath[80];
            memset (backupath, 0, sizeof (backupath));
            snprintf (backupath, sizeof (backupath), "%s~", hackpath);
            if (rename (hackpath, backupath))
              MOM_FATAPRINTF ("failed to rename %s to %s - %m", hackpath,
                              backupath);
            else
              MOM_INFORMPRINTF ("backed-up hackc %s -> %s", hackpath,
                                backupath);
          }
          /// load the module and run its momhack_* function
          {
            char modulpath[80];
            memset (modulpath, 0, sizeof (modulpath));
            snprintf (modulpath, sizeof (modulpath), "modules/momg_%s.so",
                      hackitmstr);
            void *dlh = dlopen (modulpath, RTLD_NOW);
            if (!dlh)
              MOM_FATAPRINTF ("hack_code #%ld failed to dlopen %s : %s",
                              reqcnt, modulpath, dlerror ());
            MOM_DEBUGPRINTF (web, "hack_code #%ld dlopen %s succeeded @%p",
                             reqcnt, modulpath, dlh);
            char funbuf[80];
            memset (funbuf, 0, sizeof (funbuf));
            snprintf (funbuf, sizeof (funbuf), "momhack_%s", hackitmstr);
            MOM_DEBUGPRINTF (web, "hack_code #%ld funbuf %s", reqcnt, funbuf);
            momhackfun_t *pfun = dlsym (dlh, funbuf);
            if (!pfun)
              MOM_FATAPRINTF ("hack_code #%ld failed to dlsym %s : %s",
                              reqcnt, funbuf, dlerror ());
            MOM_DEBUGPRINTF (web,
                             "hack_code #%ld funbuf %s before running pfun@%p",
                             reqcnt, funbuf, (void *) pfun);
            pthread_mutex_lock (&hackitm->itm_mtx);
            (*pfun) (hackitm);
            pthread_mutex_unlock (&hackitm->itm_mtx);
            MOM_DEBUGPRINTF (web, "hack_code #%ld done funbuf %s", reqcnt,
                             funbuf);
          }
          json_t *jreply =
            json_pack ("{s:s,s:b,s:s}", "compileroutput", outbuf,
                       "compilation",
                       true,
                       "hackitem", hackitmstr);
          // json_dumps will use GC_STRDUP...
          MOM_DEBUGPRINTF (web, "hack success jreply=%s",
                           json_dumps (jreply, JSON_INDENT (1)));
          char *jrepl = json_dumps (jreply, 0);
          int lenjrepl = strlen (jrepl);
          onion_response_set_header (resp, "Content-Type", "text/json");
          onion_response_set_length (resp, lenjrepl + 1);
          onion_response_set_code (resp, HTTP_OK);
          onion_response_write0 (resp, jrepl);
          onion_response_write0 (resp, "\n");
          return OCS_PROCESSED;
        }
      else
        {
          MOM_WARNPRINTF ("hack_code #%ld compilation %s failed:\n%s\n",
                          reqcnt, hackitmstr, outbuf);
          json_t *jreply =
            json_pack ("{s:s,s:b,s:s}", "compileroutput", outbuf,
                       "compilation",
                       false,
                       "hackitem", hackitmstr);
          // json_dumps will use GC_STRDUP...
          MOM_DEBUGPRINTF (web, "hack failure jreply=%s",
                           json_dumps (jreply, JSON_INDENT (1)));
          char *jrepl = json_dumps (jreply, 0);
          int lenjrepl = strlen (jrepl);
          onion_response_set_header (resp, "Content-Type", "text/json");
          onion_response_set_length (resp, lenjrepl + 1);
          onion_response_set_code (resp, HTTP_OK);
          onion_response_write0 (resp, jrepl);
          onion_response_write0 (resp, "\n");
          return OCS_PROCESSED;
        }
    }                           /* end do_hackc */
  else if (do_stop && do_stop[0])
    {
      MOM_DEBUGPRINTF (web, "hackc_code #%ld do_stop", reqcnt);
      json_t *jreply = json_pack ("{s:f,s:f}",
                                  "elapsedreal", mom_elapsed_real_time (),
                                  "processcpu", mom_process_cpu_time ());;
      // json_dumps will use GC_STRDUP...
      MOM_DEBUGPRINTF (web, "hack stop jreply=%s",
                       json_dumps (jreply, JSON_INDENT (1)));
      char *jrepl = json_dumps (jreply, 0);
      int lenjrepl = strlen (jrepl);
      onion_response_set_header (resp, "Content-Type", "text/json");
      onion_response_set_length (resp, lenjrepl + 1);
      onion_response_set_code (resp, HTTP_OK);
      onion_response_write0 (resp, jrepl);
      onion_response_write0 (resp, "\n");
      mom_stop_and_dump ();
      onion_listen_stop (onion_mom);
      MOM_DEBUGPRINTF (web, "hackc_code #%ld do_stop stopping", reqcnt);
      return OCS_PROCESSED;
    }
  return OCS_INTERNAL_ERROR;
}                               // end mom_hackc_code



struct mom_item_st *
mom_web_handler_exchange (long reqcnt, const char *fullpath,
                          enum mom_webmethod_en wmeth,
                          onion_request *requ, onion_response *resp)
{
  assert (wmeth > MOMWEBM_NONE);
  assert (requ != NULL);
  assert (resp != NULL);
  if (!fullpath || fullpath[0] != '/' || strstr (fullpath, "..")
      || strstr (fullpath, "//") || strlen (fullpath) > MOM_PATH_MAX)
    {
      MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld bad fullpath %s",
                       reqcnt, fullpath);
      return NULL;
    }
  MOM_DEBUGPRINTF (web, "web_handler_exchange fullpath=%s", fullpath);
#warning mom_web_handler_exchange should look for existing item names between /
  MOM_FATAPRINTF ("unimplemented mom_web_handler_exchange fullpath=%s",
                  fullpath);
}                               /* end of mom_web_handler_exchange */



static onion_connection_status
handle_web_mom (void *data, onion_request *requ, onion_response *resp)
{
#warning should use the web_handlers predefined
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
      MOM_WARNPRINTF ("webrequest#%ld dot starting fullpath %s", reqcnt,
                      reqfupath);
      return OCS_NOT_PROCESSED;
    }
  // reject too long paths
  if (strlen (reqfupath) >= MOM_PATH_MAX - 16)
    {
      MOM_WARNPRINTF ("webrequest#%ld too long (%d) fullpath %s",
                      reqcnt, (int) strlen (reqfupath), reqfupath);
      return OCS_NOT_PROCESSED;
    }
  if (!reqpath[0] && !strcmp (reqfupath, "/"))
    {
      reqfupath = "/index.html";
      MOM_DEBUGPRINTF (web, "webrequest#%ld set reqfupath %s", reqcnt,
                       reqfupath);
    }
  /// scan the mom_webdir-s
  if (wmeth != MOMWEBM_POST && reqfupath[0] == '/' && isalnum (reqfupath[1]))
    {
      MOM_DEBUGPRINTF (web, "webrequest#%ld reqfupath %s could be file",
                       reqcnt, reqfupath);
      char fpath[2 * MOM_PATH_MAX];
      memset (fpath, 0, sizeof (fpath));
      for (unsigned wix = 0; wix < MOM_MAX_WEBDIR; wix++)
        {
          if (!mom_webdir[wix])
            continue;
          memset (fpath, 0, sizeof (fpath));
          if (snprintf (fpath, sizeof (fpath), "%s%s",
                        mom_webdir[wix], reqfupath)
              >= (int) sizeof (fpath) - 2)
            {
              MOM_WARNPRINTF
                ("for webrequest#%ld wix=%d reqfupath %s too long fpath %s",
                 webcount_mom, wix, reqfupath, fpath);
              continue;
            }
          MOM_DEBUGPRINTF (web, "webrequest#%ld trying wix=%d fpath=%s",
                           reqcnt, wix, fpath);
          struct stat myfstat = { 0 };
          if (!stat (fpath, &myfstat)
              && ((myfstat.st_mode & S_IFMT) == S_IFREG
                  || ((errno = EBADF), false))
              && (myfstat.st_size < MOM_MAX_WEB_FILE_SIZE
                  || ((errno = EFBIG), false)))
            {
              MOM_DEBUGPRINTF (web,
                               "handle_web_mom request#%ld wix#%d got fpath %s of %ld bytes",
                               reqcnt, wix, fpath, (long) myfstat.st_size);
              return onion_shortcut_response_file (fpath, requ, resp);
            }
          else
            MOM_DEBUGPRINTF (web,
                             "webrequest#%ld for wix=%d fpath=%s missing or bad (%m)",
                             reqcnt, wix, fpath);
          if (errno == EFBIG && myfstat.st_size >= MOM_MAX_WEB_FILE_SIZE)
            MOM_WARNPRINTF
              ("file %s is too big (%lld) and won't be served for webrequest#%ld full path %s",
               fpath, (long long) myfstat.st_size, reqcnt, reqfupath);
        }
    }
  if (wmeth == MOMWEBM_POST && !strcmp (reqfupath, "/mom_hackc_code"))
    return mom_hackc_code (reqcnt, requ, resp);
  struct mom_item_st *wexitm =
    mom_web_handler_exchange (reqcnt, reqfupath, wmeth, requ, resp);
#warning should do something with wexitm
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
