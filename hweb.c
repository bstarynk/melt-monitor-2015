// file hweb.c - handling web interface

/**   Copyright (C)  2015, 2016  Basile Starynkevitch and later the FSF
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

#include "meltmoni.h"

#define SESSION_COOKIE_MOM "MOMSESSION"
#define SESSION_TIMEOUT_MOM 4000        /* a bit more than one hour of inactivity */

// maximal reply delay for a web request - in seconds
#define REPLY_TIMEOUT_MOM (MOM_IS_DEBUGGING(web)?10.2:4.5)      /* reply timeout in seconds */

const char *web_hostname_mom;
static struct mom_hashset_st *sessions_hset_mom;
static pthread_mutex_t sessions_mtx_mom = PTHREAD_MUTEX_INITIALIZER;

static onion *onion_mom;

static volatile atomic_long webcount_mom;

MOM_PRIVATE onion_connection_status
handle_web_mom (void *data, onion_request *requ, onion_response *resp);

MOM_PRIVATE void
handle_onion_memory_failure (const char *msg)
{
  MOM_FATAPRINTF ("onion memory failure %s (%m)", msg);
}


#define MAX_ONIONTHREADS_MOM 4
void
mom_start_web (const char *webservice)
{
  MOM_DEBUGPRINTF (web, "mom_start_web starting webservice=%s", webservice);
  onion_low_initialize_memory_allocation (mom_gc_alloc, mom_gc_alloc_atomic,
                                          mom_gc_calloc, GC_realloc,
                                          GC_strdup, GC_free,
                                          handle_onion_memory_failure);
#warning should call onion_low_initialize_threads
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
    {
      web_hostname_mom = GC_STRDUP (webhostname);
      onion_set_hostname (onion_mom, webhostname);
    }
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
      fputs ("#include \"meltmoni.h\"\n\n", fhack);
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
              {
                const char *errmsg = GC_STRDUP (dlerror ());
                MOM_WARNPRINTF ("hack_code #%ld failed to dlopen %s : %s",
                                reqcnt, modulpath, errmsg);
                if (outf)
                  fprintf (outf, "\n **dlopen %s failure : %s ** \n",
                           modulpath, errmsg);
                goto buildfailure;
              }
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
        buildfailure:
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
      onion_mom = NULL;
      MOM_DEBUGPRINTF (web, "hackc_code #%ld do_stop stopping", reqcnt);
      return OCS_PROCESSED;
    }
  return OCS_INTERNAL_ERROR;
}                               // end mom_hackc_code



////////////////////////////////////////////////////////////////
struct mom_item_st *
make_session_item_mom (onion_response *resp)
{
  struct mom_item_st *sessitm = mom_clone_item (MOM_PREDEFITM (web_session));
  struct mom_websession_st *wsess = mom_gc_alloc (sizeof (*wsess));
  wsess->va_itype = MOMITY_WEBSESSION;
  uint32_t r1 = mom_random_uint32 ();
  uint32_t r2 = mom_random_uint32 ();
  wsess->wbss_rand1 = r1;
  wsess->wbss_rand2 = r2;
  time_t now = 0;
  time (&now);
  wsess->wbss_obstime = now + SESSION_TIMEOUT_MOM;
  sessitm->itm_payload = (struct mom_anyvalue_st *) wsess;
  pthread_mutex_lock (&sessions_mtx_mom);
  sessions_hset_mom = mom_hashset_insert (sessions_hset_mom, sessitm);
  pthread_mutex_unlock (&sessions_mtx_mom);
  char cookiestr[128];
  memset (cookiestr, 0, sizeof (cookiestr));
  if (snprintf (cookiestr, sizeof (cookiestr),
                "%s/%u/%u", mom_item_cstring (sessitm), r1, r2)
      >= (int) sizeof (cookiestr) - 2)
    /// this should never happen
    MOM_FATAPRINTF ("too long cookiestr %s", cookiestr);
  MOM_DEBUGPRINTF (web,
                   "make_session_item sessitm=%s r1=%u r2=%u now %ld obstime %ld, cookiestr: %s",
                   mom_item_cstring (sessitm), (unsigned) r1, (unsigned) r2,
                   (long) now, (long) wsess->wbss_obstime, cookiestr);
  onion_response_add_cookie (resp, SESSION_COOKIE_MOM, cookiestr,
                             SESSION_TIMEOUT_MOM, "/", web_hostname_mom, 0);
  return sessitm;
}



////////////////////////////////////////////////////////////////

struct mom_item_st *
mom_web_handler_exchange (long reqcnt, const char *fullpath,
                          enum mom_webmethod_en wmeth,
                          onion_request *requ, onion_response *resp)
{
  assert (wmeth > MOMWEBM_NONE);
  assert (requ != NULL);
  assert (resp != NULL);
  double wtime = mom_clock_time (CLOCK_REALTIME);
  if (!fullpath || fullpath[0] != '/' || strstr (fullpath, "..")
      || strstr (fullpath, "//") || strlen (fullpath) > MOM_PATH_MAX)
    {
      MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld bad fullpath %s",
                       reqcnt, fullpath);
      return NULL;
    }
  MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld fullpath=%s", reqcnt,
                   fullpath);
  /// we should sometimes forget the old sessions, but we dont bother yet
#define MAX_ELEM_WEB_PATH_MOM 20
  int nbelem = 0;
  const struct mom_item_st *elemarr[MAX_ELEM_WEB_PATH_MOM] = { NULL };
  char *nextslash = NULL;
  const char *pc = NULL;
  for (pc = fullpath + 1; pc && *pc; pc = nextslash ? (nextslash + 1) : NULL)
    {
      if (!isalpha (*pc))
        break;
      if (nbelem >= MAX_ELEM_WEB_PATH_MOM)
        {
          MOM_DEBUGPRINTF (web,
                           "web_handler_exchange #%ld too many (%d) elements for %s",
                           reqcnt, nbelem, fullpath);
          return NULL;
        }
      nextslash = strchr (pc, '/');
      if (nextslash)
        {
          char curnam[MOM_PATH_MAX / 2];
          memset (curnam, 0, sizeof (curnam));
          if (nextslash - pc >= (int) sizeof (curnam) - 2)
            {
              MOM_DEBUGPRINTF (web,
                               "web_handler_exchange #%ld too long element %s for %s",
                               reqcnt, pc, fullpath);
              return NULL;
            }
          int nix = 0;
          for (const char *np = pc; isalnum (*np) || *np == '_'; np++)
            curnam[nix++] = *np;
          const char *end = NULL;
          struct mom_item_st *curitm =
            mom_find_item_from_string (curnam, &end);
          MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld curnam %s", reqcnt,
                           curnam);
          if (curitm)
            {
              MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld curitm#%d %s",
                               reqcnt, nbelem, mom_item_cstring (curitm));
              elemarr[nbelem++] = curitm;
            }
          else
            break;

        }
      else
        {
          const char *end = NULL;
          struct mom_item_st *lastitm = mom_find_item_from_string (pc, &end);
          MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld lastitm=%s pc=%s",
                           reqcnt, mom_item_cstring (lastitm), pc);
          if (lastitm)
            elemarr[nbelem++] = lastitm;
          else
            break;
        }
    }
  MOM_DEBUGPRINTF (web,
                   "web_handler_exchange #%ld nbelem=%d fullpath='%s' pc='%s'",
                   reqcnt, nbelem, fullpath, pc);
  if (MOM_IS_DEBUGGING (web))
    for (int ix = 0; ix < nbelem; ix++)
      MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld elemarr[%d]= %s",
                       reqcnt, ix, mom_item_cstring (elemarr[ix]));
  const struct mom_hashedvalue_st *hdlrval = NULL;
  const struct mom_hashedvalue_st *hdlrkey = NULL;
  MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld nbelem=%d", reqcnt,
                   nbelem);
  if (nbelem > 0)
    {
      mom_item_lock (MOM_PREDEFITM (web_handlers));
      MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld webhandlerpayload@%p ityp=%s nbelem=%d", reqcnt, MOM_PREDEFITM (web_handlers)->itm_payload, mom_itype_str (MOM_PREDEFITM (web_handlers)  //
                                                                                                                                                                  ->itm_payload),
                       nbelem);
      if (nbelem == 1)
        {
          hdlrval =
            mom_hashassoc_get ((struct mom_hashassoc_st *)
                               MOM_PREDEFITM (web_handlers)->itm_payload,
                               (const struct mom_hashedvalue_st *)
                               elemarr[0]);
          if (hdlrval)
            {
              hdlrkey = (struct mom_hashedvalue_st *) elemarr[0];
              MOM_DEBUGPRINTF (web,
                               "web_handler_exchange #%ld singleitem hdlrkey=%s hdlrval=%s",
                               reqcnt, mom_value_cstring (hdlrkey),
                               mom_value_cstring (hdlrval));
            }
          else
            MOM_DEBUGPRINTF (web,
                             "web_handler_exchange #%ld singleitem %s nokey",
                             reqcnt, mom_item_cstring (elemarr[0]));
        }
      if (!hdlrval)
        {
          const struct mom_boxtuple_st *tup =
            mom_boxtuple_make_arr (nbelem, elemarr);
          hdlrval =
            mom_hashassoc_get ((struct mom_hashassoc_st *)
                               MOM_PREDEFITM (web_handlers)->itm_payload,
                               (const struct mom_hashedvalue_st *) tup);
          if (hdlrval)
            {
              hdlrkey = (struct mom_hashedvalue_st *) tup;
              MOM_DEBUGPRINTF (web,
                               "web_handler_exchange #%ld tup %s hdlrval %s",
                               reqcnt, mom_value_cstring (hdlrkey),
                               mom_value_cstring (hdlrval));
            }
          else
            MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld tup %s nokey",
                             reqcnt,
                             mom_value_cstring ((struct mom_hashedvalue_st *)
                                                tup));
        }
      mom_item_unlock (MOM_PREDEFITM (web_handlers));
    }
  if (!hdlrval)
    {
      MOM_DEBUGPRINTF (web,
                       "web_handler_exchange #%ld not found handler fullpath=%s",
                       reqcnt, fullpath);
      return NULL;
    };
  MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld hdlrval %s", reqcnt,
                   mom_value_cstring (hdlrval));
  const struct mom_boxnode_st *hdlrnod = mom_dyncast_node (hdlrval);
  if (!hdlrnod)
    {
      MOM_WARNPRINTF
        ("web_handler_exchange #%ld fullpath %s hdlrkey %s got non-node hdlrval %s",
         reqcnt, fullpath, mom_value_cstring (hdlrkey),
         mom_value_cstring (hdlrval));
      return NULL;
    }
  struct mom_item_st *wexitm = mom_clone_item (hdlrnod->nod_connitm);
  assert (wexitm != NULL);
  struct mom_item_st *sessitm = NULL;
  const char *sesscookie =
    onion_request_get_cookie (requ, SESSION_COOKIE_MOM);
  MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld wexitm %s sesscookie %s",
                   reqcnt, mom_item_cstring (wexitm), sesscookie);
  if (sesscookie)
    {
      MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld sesscookie=%s", reqcnt,
                       sesscookie);
      unsigned r1 = 0, r2 = 0;
      char sessname[64];
      memset (sessname, 0, sizeof (sessname));
      int pos = -1;
      if (sscanf (sesscookie, " %60[A-Za-z0-9_]/%u/%u %n",
                  sessname, &r1, &r2, &pos) >= 3 && pos > 0)
        {
          struct mom_item_st *sitm =
            mom_find_item_from_string (sessname, NULL);
          MOM_DEBUGPRINTF (web,
                           "web_handler_exchange #%ld sitm %s r1=%u r2=%u",
                           reqcnt, mom_item_cstring (sitm), r1, r2);
          if (sitm)
            {
              mom_item_lock (sitm);
              if (sitm->itm_payload
                  && sitm->itm_payload->va_itype == MOMITY_WEBSESSION)
                {
                  struct mom_websession_st *wsess =
                    (struct mom_websession_st *) sitm->itm_payload;
                  if (wsess->wbss_rand1 == r1 && wsess->wbss_rand2 == r2)
                    sessitm = sitm;
                }
              mom_item_unlock (sitm);
            }
        }
    };
  MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld fullpath %s sessitm %s",
                   reqcnt, fullpath, mom_item_cstring (sessitm));
  if (!sessitm)
    {
      sessitm = make_session_item_mom (resp);
      MOM_DEBUGPRINTF (web,
                       "web_handler_exchange #%ld fullpath %s fresh sessitm %s",
                       reqcnt, fullpath, mom_item_cstring (sessitm));
    }
  else
    {
      bool badsession = false;
      pthread_mutex_lock (&sessitm->itm_mtx);
      struct mom_websession_st *wsess =
        (struct mom_websession_st *) sessitm->itm_payload;
      if (wsess && wsess->va_itype == MOMITY_WEBSESSION)
        {
          time_t now = 0;
          time (&now);
          MOM_DEBUGPRINTF (web,
                           "web_handler_exchange#%ld sessitm %s obstime=%ld now=%ld",
                           reqcnt, mom_item_cstring (sessitm),
                           (long) wsess->wbss_obstime, (long) now);
          if (wsess->wbss_obstime <= now + SESSION_TIMEOUT_MOM / 4)
            {
              wsess->wbss_obstime = now + SESSION_TIMEOUT_MOM;
              char cookiestr[128];
              memset (cookiestr, 0, sizeof (cookiestr));
              if (snprintf (cookiestr, sizeof (cookiestr),
                            "%s/%u/%u",
                            mom_item_cstring (sessitm), wsess->wbss_rand1,
                            wsess->wbss_rand2) >=
                  (int) sizeof (cookiestr) - 2)
                /// this should never happen
                MOM_FATAPRINTF ("too long cookiestr %s", cookiestr);
              MOM_DEBUGPRINTF (web,
                               "web_handler_exchange#%ld sessitm=%s r1=%u r2=%u update cookie: %s",
                               reqcnt, mom_item_cstring (sessitm),
                               (unsigned) wsess->wbss_rand1,
                               (unsigned) wsess->wbss_rand2, cookiestr);
              onion_response_add_cookie (resp, SESSION_COOKIE_MOM, cookiestr,
                                         SESSION_TIMEOUT_MOM, "/",
                                         web_hostname_mom, 0);
            }
          else
            MOM_DEBUGPRINTF (web,
                             "web_handler_exchange#%ld sessitm=%s valid sesscookie %s",
                             reqcnt, mom_item_cstring (sessitm), sesscookie);
        }
      else
        badsession = true;
      pthread_mutex_unlock (&sessitm->itm_mtx);
      if (badsession)
        {
          sessitm = make_session_item_mom (resp);
          MOM_DEBUGPRINTF (web,
                           "web_handler_exchange#%ld got bad session so made sessitm %s",
                           reqcnt, mom_item_cstring (sessitm));
        }
    };
  {
    struct mom_webexch_st *wexch = mom_gc_alloc (sizeof (*wexch));
    wexch->va_itype = MOMITY_WEBEXCH;
    wexch->webx_meth = wmeth;
    wexch->webx_time = wtime;
    wexch->webx_count = reqcnt;
    wexch->webx_key = hdlrkey;
    wexch->webx_clos = hdlrnod;
    wexch->webx_restpath = mom_boxstring_make (pc);
    wexch->webx_requ = requ;
    wexch->webx_resp = resp;
    wexch->webx_sessitm = sessitm;
    wexch->webx_outbuf = NULL;
    wexch->webx_outsiz = 0;
    wexch->webx_outfil =
      open_memstream (&wexch->webx_outbuf, &wexch->webx_outsiz);
    if (!wexch->webx_outfil)
      MOM_FATAPRINTF
        ("web_handler_exchange #%ld fullpath %s failed to open outfile (%m)",
         reqcnt, fullpath);
    pthread_cond_init (&wexch->webx_donecond, NULL);
    wexitm->itm_payload = (struct mom_anyvalue_st *) wexch;
  }
  MOM_DEBUGPRINTF (web, "web_handler_exchange #%ld fullpath %s wexitm %s",
                   reqcnt, fullpath, mom_item_cstring (wexitm));
  return wexitm;
}                               /* end of mom_web_handler_exchange */




////////////////////////////////////////////////////////////////

MOM_PRIVATE onion_connection_status
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
  if (!wexitm)
    {
      MOM_DEBUGPRINTF (web,
                       "webrequest#%ld reqfupath %s no handler so no wexitm",
                       reqcnt, reqfupath);
      return OCS_NOT_PROCESSED;
    };
  errno = 0;
  {
    const struct mom_boxnode_st *wexclos = NULL;
    mom_item_lock (wexitm);
    if (mom_itype (wexitm->itm_payload) == MOMITY_WEBEXCH)
      wexclos = ((struct mom_webexch_st *) wexitm->itm_payload)->webx_clos;
    mom_item_unlock (wexitm);
    MOM_DEBUGPRINTF (web, "webrequest#%ld fupath %s wexclos %s",
                     reqcnt, reqfupath,
                     mom_value_cstring ((struct mom_hashedvalue_st *)
                                        wexclos));
    assert (wexclos->va_itype == MOMITY_NODE);
    {
      struct mom_item_st *taskitm = mom_clone_item (wexclos->nod_connitm);
      unsigned wexarity = mom_raw_size (wexclos);
      struct mom_hashedvalue_st *smallarr[16] = { 0 };
      const struct mom_hashedvalue_st **arr =   //
        ((wexarity + 1) < (sizeof (smallarr) / sizeof (smallarr[0])))   //
        ? smallarr              //
        : mom_gc_alloc ((wexarity + 2) * sizeof (void *));
      arr[0] = (struct mom_hashedvalue_st *) wexitm;
      for (unsigned ix = 0; ix < wexarity; ix++)
        arr[ix + 1] = wexclos->nod_sons[ix];
      const struct mom_boxnode_st *taskclos =
        mom_boxnode_make (wexclos->nod_connitm, wexarity + 1, arr);
      taskitm->itm_payload = (struct mom_anyvalue_st *) taskclos;
      MOM_DEBUGPRINTF (web, "webrequest#%ld taskitm=%s taskclos=%s",
                       reqcnt, mom_item_cstring (taskitm),
                       mom_value_cstring ((struct mom_hashedvalue_st *)
                                          taskclos));
      mom_agenda_add_tasklet_front (taskitm);
    }
  }
  double elapstim = mom_clock_time (CLOCK_REALTIME) + REPLY_TIMEOUT_MOM;
  MOM_DEBUGPRINTF (web,
                   "webrequest#%ld elapstim=%.1f wexitm=%s replytimeout %.2f sec",
                   reqcnt, elapstim, mom_item_cstring (wexitm),
                   REPLY_TIMEOUT_MOM);
  bool waitreply = false;
  long nbloop = 0;
  do
    {
      nbloop++;
      assert (wexitm && wexitm->va_itype == MOMITY_ITEM);
      mom_item_lock (wexitm);
      double nowtim = mom_clock_time (CLOCK_REALTIME);
      double remaintim = elapstim - nowtim;
      struct mom_webexch_st *wexch = NULL;
      MOM_DEBUGPRINTF (web,
                       "webrequest#%ld wexitm %s loop reqfupath %s remaintim %.3f sec loop#%ld",
                       reqcnt, mom_item_cstring (wexitm), reqfupath,
                       remaintim, nbloop);
      struct timespec ts = (remaintim > 0.0)
        ? mom_timespec (nowtim + remaintim * 0.35 + 0.01)
        : mom_timespec (nowtim + 0.02);
      if (wexitm->itm_payload && wexitm->itm_payload != MOM_EMPTY_SLOT
          && wexitm->itm_payload->va_itype == MOMITY_WEBEXCH)
        wexch = (struct mom_webexch_st *) wexitm->itm_payload;
      if (!wexch || wexch->webx_count != reqcnt)
        MOM_FATAPRINTF ("webrequest#%ld bad wexitm %s for reqfupath %s",
                        reqcnt, mom_item_cstring (wexitm), reqfupath);
      assert (wexch->webx_resp == resp);
      MOM_DEBUGPRINTF (web,
                       "webrequest#%ld wexitm %s waiting for reqfupath %s...",
                       reqcnt, mom_item_cstring (wexitm), reqfupath);
      int waiterr =
        pthread_cond_timedwait (&wexch->webx_donecond, &wexitm->itm_mtx, &ts);
      MOM_DEBUGPRINTF (web,
                       "webrequest#%ld afterwait reqfupath %s code %d mimetype %s waiterr#%d (%s)",
                       reqcnt, reqfupath, wexch->webx_code,
                       wexch->webx_mimetype, waiterr, strerror (waiterr));
      if (waiterr == 0 && atomic_load (&wexch->webx_code) > 0
          && isalpha (wexch->webx_mimetype[0]))
        {
          assert (wexch->webx_outfil);
          fflush (wexch->webx_outfil);
          MOM_DEBUGPRINTF (web, "webrequest#%ld got code %d mimetype %s",
                           reqcnt, wexch->webx_code, wexch->webx_mimetype);
          onion_response_set_code (resp, atomic_load (&wexch->webx_code));
          if ((!strncmp (wexch->webx_mimetype, "text/", 5)
               || strstr (wexch->webx_mimetype, "json") != NULL
               || strstr (wexch->webx_mimetype, "xml") != NULL
               || strstr (wexch->webx_mimetype, "javascript") != NULL)
              && !strstr (wexch->webx_mimetype, "charset"))
            {
              char fullmime[sizeof (wexch->webx_mimetype) + 16];
              snprintf (fullmime, sizeof (fullmime), "%s; charset=UTF-8",
                        wexch->webx_mimetype);
              onion_response_set_header (resp, "Content-Type", fullmime);
            }
          else
            onion_response_set_header (resp, "Content-Type",
                                       wexch->webx_mimetype);
          {
            char servbuf[80];
            memset (servbuf, 0, sizeof (servbuf));
            snprintf (servbuf, sizeof (servbuf), "Monimelt/%.12s %s",
                      monimelt_lastgitcommit, monimelt_timestamp);
            onion_response_set_header (resp, "Server", servbuf);
            MOM_DEBUGPRINTF (web, "webrequest#%ld servbuf %s", reqcnt,
                             servbuf);
          }
          long off = ftell (wexch->webx_outfil);
          MOM_DEBUGPRINTF (web, "webrequest#%ld off %ld", reqcnt, off);
          if (MOM_IS_DEBUGGING (web)
              && ((!strncmp (wexch->webx_mimetype, "text/", 5)
                   || strstr (wexch->webx_mimetype, "json")
                   || strstr (wexch->webx_mimetype, "javascript")
                   || strstr (wexch->webx_mimetype, "xml"))))
            MOM_DEBUGPRINTF (web,
                             "webrequest#%ld textual outbuf:\n%s\n#### %ld bytes for webrequest#%ld\n",
                             reqcnt, wexch->webx_outbuf, off, reqcnt);
          onion_response_set_length (resp, off);
          onion_response_write (resp, wexch->webx_outbuf, off);
          onion_response_flush (resp);
          wexch->webx_resp = NULL;
          wexch->webx_requ = NULL;
          waitreply = false;
        }
      else if (waiterr == ETIMEDOUT
               && mom_clock_time (CLOCK_REALTIME) >= elapstim + 0.01)
        {
          MOM_DEBUGPRINTF (web, "webrequest#%ld timedout", reqcnt);
          MOM_WARNPRINTF ("webrequest#%ld timeout %s fullpath %s", reqcnt,
                          mom_webmethod_name (wmeth), reqfupath);
          onion_response_set_code (resp, HTTP_INTERNAL_ERROR);
          onion_response_set_header (resp, "Content-Type",
                                     "text/html; charset=UTF-8");
          char timbuf[80];
          memset (timbuf, 0, sizeof (timbuf));
          mom_now_strftime_centi (timbuf, sizeof (timbuf) - 1,
                                  "%Y %b %d, %H:%M:%S.__ %Z");
          char *htmlbuf = NULL;
          size_t htmlsiz = 0;
          FILE *htmlout = open_memstream (&htmlbuf, &htmlsiz);
          if (!htmlout)
            MOM_FATAPRINTF
              ("failed to open_memstream for webrequest#%ld timeout %s fullpath %s (%m)",
               reqcnt, mom_webmethod_name (wmeth), reqfupath);
          fprintf (htmlout,
                   "<!doctype html>\n"
                   "<html><head><title>MONIMELT timedout</title></head>\n"
                   "<body><h1>MONIMELT request #%ld timedout</h1>\n"
                   "%s request to <tt>%s</tt> timed out with wexitm %s on %s"
                   "<hr/><small>monimelt <i>%s</i> gitcommit <tt>%s<tt></small>"
                   "</body></html>\n\n",
                   reqcnt, mom_webmethod_name (wmeth),
                   reqfupath,
                   mom_item_cstring (wexitm), timbuf,
                   monimelt_timestamp, monimelt_lastgitcommit);
          fflush (htmlout);
          long htmloff = ftell (htmlout);
          onion_response_set_length (resp, htmloff);
          onion_response_write (resp, htmlbuf, htmloff);
          fclose (htmlout), htmlout = NULL;
          free (htmlbuf), htmlbuf = NULL;
          onion_response_flush (resp);
          wexch->webx_requ = NULL;
          wexch->webx_resp = NULL;
          waitreply = false;
        }
      else
        {
          usleep (5 * 1000);
          waitreply = true;
          MOM_DEBUGPRINTF (web, "webrequest#%ld waiting again", reqcnt);
        }
      MOM_DEBUGPRINTF (web,
                       "webrequest#%ld reqfupath %s wexitm %s waitreply %s endbody loop#%ld",
                       reqcnt, reqfupath, mom_item_cstring (wexitm),
                       waitreply ? "true" : "false", nbloop);
      mom_item_unlock (wexitm);
      usleep (2000);
    }
  while (waitreply);
  MOM_DEBUGPRINTF (web, "webrequest#%ld reqfupath %s end did %ld loops",
                   reqcnt, reqfupath, nbloop);
  return OCS_PROCESSED;
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
  payl->webx_restpath = NULL;
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


void
mom_wexch_reply (struct mom_webexch_st *wex, int httpcode,
                 const char *mimetype)
{
  if (!wex || wex == MOM_EMPTY_SLOT || wex->va_itype != MOMITY_WEBEXCH)
    return;
  if (!mimetype || !isalpha (mimetype[0]))
    return;
  if (!wex->webx_outfil)
    return;
  if (MOM_UNLIKELY (atomic_load (&wex->webx_code) != 0))
    return;
  fflush (wex->webx_outfil);
  MOM_DEBUGPRINTF (web, "mom_wexch_reply req#%ld httpcode=%d mimetype=%s",
                   wex->webx_count, httpcode, mimetype);
  atomic_store (&wex->webx_code, httpcode);
  strncpy (wex->webx_mimetype, mimetype, sizeof (wex->webx_mimetype) - 1);
  pthread_cond_broadcast (&wex->webx_donecond);
  MOM_DEBUGPRINTF (web, "mom_wexch_reply req#%ld done", wex->webx_count);
}                               /* end mom_wexch_reply */


void
mom_stop_web (void)
{
  MOM_DEBUGPRINTF (web, "mom_stop_web onion_mom=%p", onion_mom);
  if (onion_mom)
    {
      onion_listen_stop (onion_mom);
      onion_mom = NULL;
      MOM_INFORMPRINTF ("stopping web serving");
    }
}                               /* end of mom_stop_web */

/// eof hweb.c
