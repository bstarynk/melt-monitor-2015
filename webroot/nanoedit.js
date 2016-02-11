// file nanoedit.js - Javascript & Jquery for nanoedit.html.

/**   Copyright (C)  2016 Basile Starynkevitch, later FSF
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

var $editdiv;
var $editlog;
var $cleareditbut;
var $exitbut;
var $resetcmdbut;
var $commandtext;
var $commanddiv;
var $sendcmdbut;
var $rawmodebox;
var $parsedcmddiv;
var $clipboardh;

var mom_eval_counter=0;
var mom_menuitemcount = 0;
var mom_menuitem = null;

/// in our command text, we want to be able to type the 4 keys $ a n d
/// then the key Escape to get ∧
var mom_escape_encoding_dict = {
    "Lambda": "Λ",
    "alpha": "α",
    "and": "∧",
    "asterism": "⁂",
    "beta": "β",
    "bullet": "•",
    "equiv": "⇔",
    "exists" : "∃",
    "forall": "∀",
    "gamma": "γ",
    "ge": "≥",
    "imply": "⇒",
    "in": "∈",
    "include": "⊆",
    "integer": "ℤ",
    "intersection": "∩",
    "lambda": "λ",
    "le": "≤",
    "leftarrow": "←",
    "natural": "ℕ",
    "not": "¬",
    "notin": "∉",
    "or": "∨",
    "rightarrow": "→",
    "subset": "⊂",
    "union": "∪",
    "~": null //// last place-holder
}

////////////////////////////////////////////////////////////////
///// utility functions

// from http://stackoverflow.com/a/1219983/841108
function htmlEncode(value){
    //create a in-memory div, set it's inner text(which jQuery automatically encodes)
    //then grab the encoded contents back out.  The div never exists on the page.
    return $('<div/>').text(value).html();
}

function htmlDecode(value){
    return $('<div/>').html(value).text();
}


var mom_name_regexp = /^[A-Za-z0-9_]*$/;


var mom_name_cache = {};

/// see http://mikemurko.com/general/jquery-keycode-cheatsheet/

///insert string in textarea at current caret
/// from http://stackoverflow.com/a/4456598/841108
$.fn.insertAtCaret = function(text) {
    return this.each(function() {
        if (document.selection && this.tagName == 'TEXTAREA') {
            //IE textarea support
            this.focus();
            sel = document.selection.createRange();
            sel.text = text;
            this.focus();
        } else if (this.selectionStart || this.selectionStart == '0') {
            //MOZILLA/NETSCAPE support
            startPos = this.selectionStart;
            endPos = this.selectionEnd;
            scrollTop = this.scrollTop;
            this.value = this.value.substring(0, startPos) + text + this.value.substring(endPos, this.value.length);
            this.focus();
            this.selectionStart = startPos + text.length;
            this.selectionEnd = startPos + text.length;
            this.scrollTop = scrollTop;
        } else {
            // IE input[type=text] and other browsers
            this.value += text;
            this.focus();
            this.value = this.value;    // forces cursor to end
        }
    });
};

/// check if a name is some known item, with memoization & AJAX do_knownitem
function mom_known_item(name) {
    if (!name.match(mom_name_regexp))
        return false;
    if (mom_name_cache[name])
        return true;
    var res = null;
    console.log("mom_known_item name=", name, " before ajaxing");
    $.ajax
    ({url: "/nanoedit",
      method: "POST",
      async: false,
      data: {"do_knownitem": name},
      dataType: "json",
      success: function (data, stat, jh) {
          console.log("mom_known_item success data=", data, " stat=", stat, " jh=", jh);
          if (data) {
              mom_known_item[name] = data;
              res = true;
          }
          else res = false;
      },
      error: function (jq, stat, err) {
          console.warn("mom_known_item error jq=", jq, " stat=", stat, " err=", err);
          res = false;
      }
     });
    console.log ("mom_known_item name=", name, " res=", res);
    return res;
}                              // end mom_known_item


//// give the completion of some name string, using AJAX do_completename
function mom_complete_name(name) {
    var res = null;
    if (!name.match(mom_name_regexp)) {
        console.log ("mom_complete_name bad name=", name);
        return false;
    }
    else 
        console.log("mom_complete_name name=", name, " before ajaxing");
    $.ajax
    ({url: "/nanoedit",
      method: "POST",
      async: false,
      data: {"do_completename": name},
      dataType: "json",
      success: function (data) {
          console.log("mom_complete_name success data=", data);
          console.trace();
          res = data;
      },
      error: function (jq, stat, err) {
          console.warn("mom_complete_name error jq=", jq, " stat=", stat, " err=", err);
          res = false;
      }
     });
    console.log ("mom_complete_name name=", name, " res=", res);
    return res;
}                              // end mom_complete_name




/// this function is called by /nanoedit AJAX for do_fillpage at document loading
function mom_ajaxfill(htmlc) {
    console.log("mom_ajaxfill:\n", htmlc, "\n### endajaxfill\n");
    console.trace();
    $editdiv.html(htmlc);
    $rawmodebox = $("#momrawbox_id");
    $rawmodebox.on("change", function() {
        var valmode = $rawmodebox.val();
        var valchecked = $rawmodebox.prop("checked");
        console.log("mom_ajaxfill valmode=", valmode,
                    " valchecked=", valchecked, " $rawmodebox=", $rawmodebox);
        $.ajax
        ({url: "/nanoedit",
          method: "POST",
          data: {"do_fillpage": true, "rawmode": valchecked},
          dataType: "html",
          success: mom_ajaxfill
         });
    });
    console.log("mom_ajaxfill before $editdiv=", $editdiv);
    $editdiv.find(".mom_itemdisplaycontent_cl").each(function (ix, el) {
        console.log("mom_ajaxfill found itemdispcont ix=", ix, " el=", el);
        var elnamitem = $(el).data("dispitem");
        console.log("itemdispcont el=", el, " elnamitem=", elnamitem);
        $(el).find(".buthideitem_cl").click(function (ev) {
            console.log("ajaxfill itemdispcont $(this)=", $(this),
                        " ev=", ev, " elnamitem=", elnamitem);
            $.ajax
            ({url:"/nanoedit",
              method: "POST",
              data: {"do_hideitem": elnamitem},
              dataType: "html",
              success: function (htmlc) {
                  console.log("ajaxfill hide htmlc=", htmlc);
                  var valchecked = $rawmodebox.prop("checked");
                  console.log ("ajaxfill hideitem valchecked=", valchecked);
                  $parsedcmddiv.html(htmlc);
                  mom_ajaxfill(htmlc);
                  console.log ("ajaxfill hideitem htmlc=", htmlc);
                  $.ajax
                  ({url: "/nanoedit",
                    method: "POST",
                    data: {"do_fillpage": true, "rawmode": valchecked},
                    dataType: "html",
                    success: mom_ajaxfill
                   });
              }
             });
        });
    });
    function removeitemmenu(itmen) {
        console.log("removeitemmenu itmen=", itmen, " mom_menuitem=", mom_menuitem);
        if (itmen) {
            itmen.menu("destroy");
            itmen.remove();
        }
        if (mom_menuitem) {
            if (mom_menuitem != itmen) {
                mom_menuitem.menu("destroy");
                mom_menuitem.remove();
            }
        }
        itmen = null;
        mom_menuitem = null;
    };
    function handleitemspan(ix, el) {   
        console.log("handleitemspan ix=", ix, " el=", el);
        $(el).contextmenu(function (ev) { return false; });
        $(el).mousedown(function (ev) {   
            console.log("handleitemspan-mousedown el=", el,
                        "el-text=", $(el).text(), " ev=", ev);
            if (ev.which != 3) return false;
            mom_menuitemcount ++;
            console.log ("handleitemspan down ev=", ev,
                         " #", mom_menuitemcount, " old mom_menuitem=", mom_menuitem);
            var itemmenu = null;
            var menupos = null;
            var menuid = "mom_itemmenu_" + mom_menuitemcount + "_id";
            var edivheight = $editdiv.innerHeight();
            var edivwidth = $editdiv.innerWidth();
            var edivoff = $editdiv.offset(); // {top=, left=...} relative to the document
            console.log (" handleitemspan mouse3 el=", el, " ev=", ev,
                         // pageX & pageY are mousepos relative to top,left of the entire document
                         " ev.pageX=", ev.pageX, " ev.pageY=", ev.pageY,
                         " edivheight=", edivheight, " edivwidth=", edivwidth, " edivoff=", edivoff);
            var itemname = $(el).text();
            console.log (" handleitemspan mouse3 itemname=", itemname);
            $editdiv.after("<ul class='mom_itemmenu_cl' id='"+menuid
			   +"' data-momitem='"+itemname+"'>"
                           +"<li class='ui-state-disabled'>* <i>"+itemname
                           +"</i> *</li>"
                           // the text inside the following <li> matters, see switch uitext below
                           +"<li>Display</li>"
                           +"<li>Copy</li>"
                           +"<li>Hilight</li>"
                           +"</ul>");
            if (mom_menuitem) {
                var oldmenuitem = mom_menuitem;
                mom_menuitem = null;
                console.log ("handleitemspan-mouse3 oldmenuitem=", oldmenuitem);
                oldmenuitem.destroy();
                oldmenuitem.remove();
                oldmenuitem = null;
            };
            itemmenu = $("#" + menuid);
            var menuheight = itemmenu.innerHeight();
            var menuwidth = itemmenu.innerWidth();
            var menutop = 10;
            var menuleft = 20;
            console.log (" handleitemspan mouse3 itemmenu=", itemmenu,
                         " menuheight=", menuheight, " menuwidth=", menuwidth);
            if (ev.pageX + menuwidth + 10 < edivwidth)
                menuleft = Math.round(ev.pageX + 5);
            else
                menuleft = Math.round(ev.pageX - menuwidth + 3);
            if (ev.pageY + menuheight + 10 < edivheight)
                menutop = Math.round(ev.pageY + 5);
            else
                menutop = Math.round(ev.pageY + menuheight - 5);
            console.log ("handleitemspan mouse3 menuleft=", menuleft, " menutop=", menutop);
            itemmenu[0].style.top = menutop + "px";
            itemmenu[0].style.left = menuleft + "px";
            itemmenu[0].style.zIndex = "99";
            itemmenu.menu({
                select: function(ev,ui) {
                    var uitext;
                    console.log ("handleitemspan-select ev=", ev, " ui=", ui,
                                 " ui.item=", ui.item,
                                 " itemmenu=", itemmenu);
                    uitext = ui.item.text();
                    console.log ("handleitemspan-select uitext=", uitext, " itemname=", itemname);
                    switch (uitext) {
                    case "Display":
                        console.log ("handleitemspan-select should display itemname=", itemname);
                        break;
                    case "Copy":
			ui.item.select();
			document.execCommand('copy');
                        console.log ("handleitemspan-select copying itemname=", itemname, "ui.item=", ui.item);
                        break;
                    case "Hilight":
                        console.log ("handleitemspan-select should hilight itemname=", itemname);
                        break;
                    default:
                        console.error("handleitemspan-select bad uitext=", uitext);
                    }
                    setTimeout(function ()
                               { var itmen = itemmenu;
                                 itemmenu = null;
                                 removeitemmenu(itmen);
                               },
                               250);
                    console.log ("handleitemspan-select done uitext=", uitext);
                },
                blur: function(ev,ui) {
                    console.log ("handleitemspan-blur ev=", ev, " ui=", ui, " itemmenu=", itemmenu);
                }
            });
            mom_menuitem = itemmenu;
            console.log("handleitemspan mouse3 mom_menuitem=", mom_menuitem);
        });
    };
    $editdiv.find(".momitemref_cl").each(handleitemspan);
    $editdiv.find(".momitemval_cl").each(handleitemspan);
    console.log("mom_ajaxfill end $rawmodebox=", $rawmodebox,
                " $editdiv=", $editdiv);
}

function mom_doexit(jsex) {
    console.log("nanoedit mom_doexit jsex=", jsex);
    var realt = jsex.elapsedreal;
    var cput = jsex.processcpu;
    $editdiv.html("<b>exited</b> ("+realt.toPrecision(3)+" real seconds, "
                  +cput.toPrecision(3)+" cpu seconds).");
}



function mom_ajaxparsecommand(js,rstat,jqxhr) {
    var badnamedlg = null;
    var badnamedid = null;
    var badcommid = null;
    var badcomminp = null;
    console.log("mom_ajaxparsecommand js=", js, " rstat=", rstat, " jqxhr=", jqxhr);
    if (js.html)
        $parsedcmddiv.html(js.html);
    console.log("mom_ajaxparsecommand updated $parsedcmddiv=", $parsedcmddiv);
    if (js.bad_name) {
        console.log("mom_ajaxparsecommand bad_name=", js.bad_name);
        badnamedid = "mom_badnamed_" + js.bad_name;
        badcommid = "mom_badcomm_" + js.bad_name;
        badnamedlg = $parsedcmddiv.append("<div class='mom_asknewname_cl ui-widget' title='create item?'>"
                                          +"<p>Create new item <tt>"+js.bad_name+"</tt> ?</p>"
                                          +"<label for='"+ badcommid + "'>Comment:</label>"
                                          +" <input id='" + badcommid + "' name='comment' type='text' size='48'/>"
                                          +"</div>\n");
        badcomminp = $("#"+badcommid);
        badnamedlg.dialog
        ({
            modal: true,
            title: "create " + js.bad_name + " ?",
            buttons: [
                {text: "create",
                 click: function() {
                     var self=$(this);
                     var itemname = js.bad_name;
                     var itemcomment = badcomminp.val();
                     console.log("should create item " + itemname + " with comment " + itemcomment);
                     $.ajax({
                         url: "/nanoedit",
                         method: "POST",
                         data: {"do_createitem": itemname,
                                "comment": itemcomment },
                         dataType: 'html',
                         success: function (hdata) {
                             console.log("do_createitem hdata=", hdata);
                             $parsedcmddiv.html(hdata);
                             self.dialog("close");
                             badnamedlg = null;
                         }
                     });
                 }},
                {text: "cancel",
                 click: function() {
                     $( this ).dialog("close");
                     console.log("cancel new item " + js.bad_name);
                     badnamedlg = null;
                 }}
            ]
        });
        console.log ("mom_ajaxparsecommand badnamedlg=", badnamedlg, " badcomminp=", badcomminp);
        return;
    }
    else if (js.error_from) {
        console.log ("mom_ajaxparsecommand error from ", js.error_from);
        if (js.error_from > 0)
            $commandtext[0].selectionStart = $commandtext[0].selectionEnd = js.error_from; 
    }
    else if (js.expr_inside) {
        var expinside = js.expr_inside;
        mom_eval_counter++;
        var evalid = "momeval_" + expinside + "_id_" + mom_eval_counter;
        var evalbut;
        console.log ("mom_ajaxparsecommand expinside ", expinside, " evalid=", evalid);
        $parsedcmddiv.append("<br/><input type='button' name='evalexpr' "
                             +" id='" + evalid + "'"
                             +" value='evaluate'/>");
        evalbut = $("#" + evalid);
        console.log ("mom_ajaxparsecommand evalbut=", evalbut);
        evalbut.click(function (ev) {
            console.log ("mom evalcmd click ev=", ev, " evalid=", evalid, " $parsedcmddiv=", $parsedcmddiv, " expinside=", expinside);
            $parsedcmddiv.html("");
            $.ajax({
                url: "/nanoedit",
                async: false,
                method: "POST",
                data: {"do_eval": expinside},
                dataType: "json",
                success: mom_ajaxparsecommand,
                error: function (jqxhr, rstat, errt) {
                    console.log("mom evalcmd error expinside=", expinside, " jqxhr=", jqxhr,
                                " rstat=", rstat, " errt=", errt);
                }
            });
        });
        evalbut.focus();
    }
    else if (js.resultcount) {
        var valchecked = $rawmodebox.prop("checked");
        console.log ("mom_ajaxparsecommand result count ", js.resultcount,
                     " valchecked=", valchecked);
        mom_ajaxfill("<br/> in " + js.resultcount + " steps.");
        console.log ("mom_ajaxparsecommand after ajaxfill js=", js);
        $.ajax
        ({url: "/nanoedit",
          method: "POST",
          data: {"do_fillpage": true, "rawmode": valchecked},
          dataType: "html",
          success: mom_ajaxfill
         });
    }
    console.log ("mom_ajaxparsecommand done js=", js);
}                               // end of mom_ajaxparsecommand



var mom_menucmdcount = 0;
var mom_menucmdel = null;
var mom_menutimeout = null;





function mom_removecmdmenu(oldmenu) {
    if (!oldmenu)
        oldmenu = mom_menucmdel;
    mom_menucmdel = null;
    if (oldmenu) {
        oldmenu.menu("destroy");
        oldmenu.remove();
    };
    var oldtimeout = mom_menutimeout;
    mom_menutimeout = null;
    // see http://stackoverflow.com/a/34848872/841108
    if (oldtimeout)
        clearTimeout(oldtimeout);
    oldmenu = null;
}

function mom_position_menucmd() {
    var tsel = $commandtext.getSelection();
    var cmdpos = $commandtext.position(); // relative to parent, i.e. body
    var cmdoff = $commandtext.offset(); // relative to document
    var cmdwidth = $commandtext.innerWidth();
    var cmdheight = $commandtext.innerHeight();
    var menuwidth = mom_menucmdel.innerWidth();
    var menuheight = mom_menucmdel.innerHeight();
    var menutop = null, menuleft = null;
    coords = getCaretCoordinates($commandtext[0], tsel.end);
    // coords is {top=..., left=...} w.r.t. $commandtext
    // cmdoff is {top=..., left=...} w.r.t. document            
    console.log("mom_position_menucmd coords=", coords,
                " tsel=", tsel, " cmdpos=", cmdpos, " cmdoff=", cmdoff,
                " cmdwidth=", cmdwidth,
                " cmdheight=", cmdheight,
                " menuheight=", menuheight,
                " menuwidth=", menuwidth);
    if (coords.left < cmdwidth/2) {
        if (coords.top < cmdheight/2) {
            console.log("mom_position_menucmd coords topleft ", coords);
            menutop = Math.round(cmdoff.top + coords.top + 10);
            menuleft = Math.round(cmdoff.left + coords.left + 10);
        }
        else {
            menutop = Math.round(cmdoff.top + cmdheight - menuheight - 10);
            menuleft = Math.round(cmdoff.left + coords.left + 10);
            console.log("mom_position_menucmd coords bottomleft ", coords);
        }
    }
    else { /* coords.left >= cmdwidth/2 */
        if (coords.top < cmdheight/2) {
            console.log("mom_position_menucmd coords topright ", coords);
            menutop = Math.round(cmdoff.top + coords.top + 10);
            menuleft = Math.round(cmdoff.left + cmdwidth - menuwidth - 10);
        }
        else {
            menutop = Math.round(cmdoff.top + cmdheight - menuheight - 10);
            menuleft = Math.round(cmdoff.left + cmdwidth - menuwidth - 10);
            console.log("mom_position_menucmd coords bottomright ", coords);
        }
    }
    console.log("mom_position_menucmd menuleft=", menuleft, " menutop=", menutop);
    mom_menucmdel[0].style.top = menutop + "px";
    mom_menucmdel[0].style.left = menuleft + "px";
    mom_menucmdel[0].style.zIndex = "99";
    console.log("mom_position_menucmd mom_menucmdel=", mom_menucmdel,
                " menutop=", menutop,  " menuleft=", menuleft);
}                               // end of mom_position_menucmd


function mom_cmdkeypress(evt) {
    if (evt.which === " ".charCodeAt(0) && evt.ctrlKey) { // control space for autocompletion
        /// see http://stackoverflow.com/a/7745958/841108
        var curspos = $commandtext.prop("selectionStart");
        console.log("mom_cmdkeypress ctrlspace curspos=", curspos);
        /// see http://stackoverflow.com/a/5592852/841108
        var result = /[A-Za-z0-9_]+$/.exec(this.value.slice(0, curspos));
        var lastword = result ? result[0] : null;
        var coords = null;
        console.log("mom_cmdkeypress ctrlspace evt=", evt, " curspos=", curspos,
                    " result=", result,
                    " lastword=", lastword,
                    " $commandtext=", $commandtext);
        if (lastword.length >= 2) {
            var acomp = mom_complete_name(lastword);
            var nbcomp = acomp.length;
            console.log("mom_cmdkeypress ctrlspace acomp=", acomp, " lastword=", lastword);
            if (!acomp || nbcomp==0) {
                alert ("word '"+lastword+"' without completion");
                return false;
            }
            else if (nbcomp === 1) {
                var complword = acomp[0];
                console.assert (complword.length >= lastword.length,
                                "bad complword=", complword,
                                " acomp=", acomp,
                                " lastword=", lastword,
                                " curspos=", curspos);
                var restword = complword.substr (lastword.length);
                console.log("mom_cmdkeypress ctrlspace singlecompletion restword=",
                            restword);
                $commandtext.insertAtCaret (restword);
                return false;
            }
            else {
                mom_menucmdcount += 1;
                var menuid = 'mom_commandmenu_id' + mom_menucmdcount;
                var menuhtml = "<ul class='mom_commandmenu_cl' id='"+menuid+"'><li>-</li></ul>";
                $commandtext.after(menuhtml);
                mom_menucmdel = $("#"+menuid);
                for (var ix=0; ix<nbcomp; ix++) {
                    mom_menucmdel.append("<li>"+acomp[ix]+"</li>");
                }
                mom_position_menucmd();
                mom_menucmdel.menu({
                    select: function(ev,ui) {
                        var selword = ui.item.text();
                        var subword = selword.substr(lastword.length);
                        console.log("mom_cmdkeypress-menu-sel ev=",
                                    ev, " ui=", ui,
                                    " mom_menucmdel=", mom_menucmdel,
                                    " selword=", selword,
                                    " subword=", subword);
                        $commandtext.insertAtCaret (subword);
                        setTimeout(function() {
                            console.log("mom_cmdkeypress-menutimeout mom_menucmdel=", mom_menucmdel);
                            mom_removecmdmenu();
                        }, 100);
                    },
                    blur: function(ev,ui) {
                        console.log("mom_cmdkeypress-menu-blur ev=",
                                    ev, " ui=", ui,
                                    " mom_menucmdel=", mom_menucmdel);
                        mom_removecmdmenu();
                    },
                    disabled: false
                });
                mom_menutimeout = setTimeout(function()
                                             {
                                                 console.log("mom_cmdkeypress-delayedmenudestroy mom_menucmdel=",
                                                             mom_menucmdel);
                                                 mom_removecmdmenu();
                                             }, 6500);
            }
        }
    }
    else if (evt.keyCode === $.ui.keyCode.ESCAPE) {
        console.log("mom_cmdkeypress escape evt=", evt, " mom_menucmdel=", mom_menucmdel);
        if (mom_menucmdel)
            mom_removecmdmenu();
        else {
            var curspos = $commandtext.prop("selectionStart");
            var endpos = $commandtext.prop("selectionEnd");
            var scrollpos = $commandtext.prop("scrollTop");
            var result = /\$\S*$/.exec(this.value.slice(0, curspos));
            var lastdoll = result ? result[0] : null;
            console.log("mom_cmdkeypress escape evt=", evt,  " lastdoll=", lastdoll, " curspos=", curspos);
            if (lastdoll) {
                var dollname = lastdoll.substr(1);
                var dollnamelen = dollname.length;
                var replacedollar = function (dollval) {
                    var oldval = $commandtext.val();
                    console.log("replacedollar dollname=", dollname, " dollval=", dollval);
                    $commandtext.val(oldval.substr(0, curspos-lastdoll.length) + dollval
                                     + oldval.substr(curspos));
                    oldval = null;
                    $commandtext[0].focus();
                    $commandtext[0].selectionStart = $commandtext[0].selectionEnd = curspos + dollval.length;
                    $commandtext[0].scrollTop = scrollpos;
                };
                if (mom_escape_encoding_dict.hasOwnProperty(dollname)) {
                    var dollval = mom_escape_encoding_dict[dollname];
                    replacedollar(dollval);
                }
                else {
                    var dollvalseq = new Array();
                    for (var kname in mom_escape_encoding_dict) {
                        console.log ("mom_cmdkeypress escape kname=", kname);
                        if (kname.length > dollnamelen && kname != "~" && kname.substr(0, dollnamelen) == dollname)
                            dollvalseq.push(kname);
                    }
                    dollvalseq = dollvalseq.sort();
                    console.log ("mom_cmdkeypress dollvalseq=", dollvalseq);
                    if (dollvalseq.length == 0) {
                        dollvalseq = Object.getOwnPropertyNames(mom_escape_encoding_dict).sort();
                        dollvalseq.pop(); // remove the "~"
                        console.log ("mom_cmdkeypress all dollvalseq=", dollvalseq);                    
                    };
                    if (dollvalseq.length == 1)
                        replacedollar(mom_escape_encoding_dict[dollvalseq[0]]);
                    else {
                        // popup a replacement menu
                        mom_menucmdcount += 1;
                        var menuid = 'mom_commandreplmenu_id' + mom_menucmdcount;
                        var menuhtml = "<ul class='mom_commandmenu_cl' id='"+menuid+"'><li>-</li></ul>";
                        $commandtext.after(menuhtml);
                        mom_menucmdel = $("#"+menuid);
                        dollvalseq.forEach(function (kel,ix,arr) {
                            var repl = mom_escape_encoding_dict[kel];
                            console.log ("mom_cmdkeypress kel=", kel, " repl=", repl);
                            mom_menucmdel.append("<li data-dollrepl='"+repl+"'><i>$"+kel+"</i> &nbsp;: <b>"+repl+"</b></li>");
                        });
                        mom_position_menucmd();
                        mom_menucmdel.menu({
                            select: function(ev,ui) {
                                console.log("mom_cmdkeypress-replmenu-sel ev=", ev, " ui=", ui);
                                var repstr = ui.item.data("dollrepl");
                                console.log("mom_cmdkeypress-replmenu-sel repstr=", repstr);
                                replacedollar(repstr);
                                setTimeout(function() {
                                    console.log("mom_cmdkeypress-menutimeout mom_menucmdel=", mom_menucmdel);
                                    mom_menucmdel.fadeOut(200+25*dollvalseq.length,mom_removecmdmenu);
                                }, 200);
                            },
                            blur: function(ev,ui) {
                                console.log("mom_cmdkeypress-replmenu-blur ev=",
                                            ev, " ui=", ui,
                                            " mom_menucmdel=", mom_menucmdel);
                                mom_removecmdmenu();
                            },
                            disabled: false
                        });
                        var curmenu = mom_menucmdel;
                        curmenu.mousemove
                        (function(ev)
                         { console.log("momdelayrepl movefinishing ev=", ev, " curmenu=", curmenu);
                           clearTimeout(mom_menutimeout);
                         });
                        mom_menutimeout = setTimeout(function()
                                                     {
                                                         console.log("mom_cmdkeypress-delayedreplmenudestroy curmenu=",
                                                                     curmenu);
                                                         curmenu.delay(100).fadeOut(800+75*dollvalseq.length,
                                                                                    function () {
                                                                                        console.log ("momdelayrepl finalfaderemove curmenu=", curmenu);
                                                                                        mom_removecmdmenu();
                                                                                    });
                                                     }, 9500);                                          
                    }
                }
            }
        }
    }
}


/***
    function mom_commandautocomplete(requ,resp) {
    console.log("commandautocomplete requ=", requ);
    var acomp = mom_complete_name(requ.term);
    console.log("commandautocomplete acomp=", acomp);
    if (acomp) resp(acomp);
    else resp(null);
    console.log("commandautocomplete done acomp=", acomp);
    }
****/

$(document).ready(function(){
    console.log("nanoedit document ready");
    // see http://stackoverflow.com/a/10556743/841108
    window.onerror = function(msg,url,line,col,error) {
        console.error("window error ", url, ":", line, ": ", msg, " /", error);
    };
    $editdiv = $("#nanoedit_id");
    $editlog = $("#editlog_id");
    $cleareditbut = $("#cleareditbut_id");
    $exitbut = $("#exitbut_id");
    $resetcmdbut = $("#commandclear_id");
    $commanddiv = $("#commanddiv_id");
    $commandtext = $("#commandtext_id");
    $sendcmdbut = $("#commandsend_id");
    $parsedcmddiv = $("#parsedcommand_id");
    console.log ("nanoedit readying $editdiv=", $editdiv, " $editlog=", $editlog, " $cleareditbut=", $cleareditbut);
    $clipboardh = new Clipboard(".momitemref_cl", {
	text: function (tri) {
	    console.log ("$clipboardh.text tri=", tri, " $clipboardh=", $clipboardh, " mom_menuitem=", mom_menuitem);
	    console.trace();
	}
    });
    console.log ("nanoedit $clipboardh=", $clipboardh);
    $clipboardh.on("success", function (ev) {
	console.log("clipboardsuccess ev=", ev, " $clipboardh=", $clipboardh);
    });
    /***
        $commandtext.autocomplete({
        delay: 300,
        minLength: 2,
        source: mom_commandautocomplete,
        disabled: false
        });
    ***/
    $commandtext.keypress(mom_cmdkeypress);
    $cleareditbut.click(function(evt){
        console.log("clearedit evt=", evt);
        $editlog.html("");
    });
    $resetcmdbut.click(function(evt) {
        console.log("resetcmd evt=", evt);      
        $commandtext.val("");
    });
    $sendcmdbut.click(function(evt) {
        var cmdtext = $commandtext.val();
        console.log("sendcmd evt=", evt, " cmdtext=", cmdtext);
        $.ajax
        ({url: "/nanoedit",
          method: "POST",
          data: {"do_parsecommand": cmdtext},
          dataType: "json",
          success: mom_ajaxparsecommand,
          error: function (jqxhr, rstat, errt) {
              console.log("sendparsecmd error jqxhr=", jqxhr,
                          " rstat=", rstat, " errt=", errt);
          }
         });      
    });
    console.log("nanoedit before ajax do_fillpage");
    $.ajax
    ({url: "/nanoedit",
      method: "POST",
      data: {"do_fillpage": true, "rawmode": true},
      dataType: "html",
      success: mom_ajaxfill
     });
    $exitbut.click(function (evt) {
        console.log ("exit button clicked evt=", evt);
        $.ajax({url: "/nanoedit",
                method: "POST",
                data: {"do_exit": true},
                dataType: "json",
                success: mom_doexit
               })
    });
    console.log("nanoedit document done ready");
});


// end of file nanoedit.js
