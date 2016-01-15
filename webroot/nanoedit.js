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

/// in our command text, we want to be able to type the 4 keys $ a n d
/// then the key Escape to get ∧
var mom_escape_encoding_dict = {
    "and": "∧",
    "alpha": "α",
    "asterism": "⁂",
    "beta": "β",
    "gamma": "γ",
    "bullet": "•",
    "exists" : "∃",
    "forall": "∀",
    "ge": "≥",
    "in": "∈",
    "subset": "⊂",
    "include": "⊆",
    "integer": "ℤ",
    "intersection": "∩",
    "le": "≤",
    "leftarrow": "←",
    "natural": "ℕ",
    "not": "¬",
    "notin": "∉",
    "or": "∨",
    "rightarrow": "→",
    "union": "∪",
    "imply": "⇒",
    "equiv": "⇔",
    "lambda": "λ",
    "Lambda": "Λ",
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


/// this function is called by /microedit AJAX for do_fillpage at document loading
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
}

function mom_doexit(jsex) {
    console.log("nanoedit mom_doexit jsex=", jsex);
    var realt = jsex.elapsedreal;
    var cput = jsex.processcpu;
    $editdiv.html("<b>exited</b> ("+realt.toPrecision(3)+" real seconds, "
		  +cput.toPrecision(3)+" cpu seconds).");
}

function mom_ajaxparsecommand(htmlc) {
    console.log("nanoedit mom_ajaxparsecommand htmlc=", htmlc);
    $parsedcmddiv.html(htmlc);
}

var mom_menucmdcount = 0;
var mom_menucmdel = null;
function mom_removecmdmenu(oldmenu) {
    if (!oldmenu) oldmenu = mom_menucmdel;
	mom_menucmdel = null;
	oldmenu.menu("destroy");
    oldmenu.remove();
}
    
function mom_cmdkeypress(evt) {
    if (evt.which === " ".charCodeAt(0) && evt.ctrlKey) {
	/// see http://stackoverflow.com/a/7745958/841108
	var curspos = $commandtext.prop("selectionStart");
	console.log("mom_cmdkeypress ctrlspace curspos=", curspos);
	/// see http://stackoverflow.com/a/5592852/841108
	var result = /\S+$/.exec(this.value.slice(0, curspos));
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
		alert ("<b>word</b> <tt>"+lastword+"</tt> <b>without completion</b>");
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
		var menuid = 'commandcompletemenu_id' + mom_menucmdcount;
		var menuhtml = "<ul class='mom_commandcompletemenu_cl' id='"+menuid+"'><li>-</li></ul>";
		$commandtext.after(menuhtml);
		mom_menucmdel = $("#"+menuid);
		for (var ix=0; ix<nbcomp; ix++) {
		    mom_menucmdel.append("<li>"+acomp[ix]+"</li>");
		}
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
		console.log("mom_cmdkeypress coords=", coords,
			    " tsel=", tsel, " cmdpos=", cmdpos, " cmdoff=", cmdoff,
			    " cmdwidth=", cmdwidth,
			    " cmdheight=", cmdheight,
			    " menuheight=", menuheight,
			    " menuwidth=", menuwidth);
		if (coords.left < cmdwidth/2) {
		    if (coords.top < cmdheight/2) {
			console.log("mom_cmdkeypress coords topleft ", coords);
			menutop = Math.round(cmdoff.top + coords.top + 10);
			menuleft = Math.round(cmdoff.left + coords.left + 10);
		    }
		    else {
			menutop = Math.round(cmdoff.top + cmdheight - menuheight - 10);
			menuleft = Math.round(cmdoff.left + coords.left + 10);
			console.log("mom_cmdkeypress coords bottomleft ", coords);
		    }
		}
		else { /* coords.left >= cmdwidth/2 */
		    if (coords.top < cmdheight/2) {
			console.log("mom_cmdkeypress coords topright ", coords);
			menutop = Math.round(cmdoff.top + coords.top + 10);
			menuleft = Math.round(cmdoff.left + cmdwidth - menuwidth - 10);
		    }
		    else {
			menutop = Math.round(cmdoff.top + cmdheight - menuheight - 10);
			menuleft = Math.round(cmdoff.left + cmdwidth - menuwidth - 10);
			console.log("mom_cmdkeypress coords bottomright ", coords);
		    }
		}
		console.log("mom_cmdkeypress menuleft=", menuleft, " menutop=", menutop);
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
		setTimeout(function()
			   {
			       console.log("mom_cmdkeypress-delayedmenudestroy mom_menucmdel=",
					   mom_menucmdel);
			       mom_removecmdmenu();
			   }, 6500);
		mom_menucmdel[0].style.top = menutop + "px";
		mom_menucmdel[0].style.left = menuleft + "px";
		mom_menucmdel[0].style.zIndex = "99";
		console.log("mom_cmdkeypress mom_menucmdel=", mom_menucmdel,
			    " menutop=", menutop,  " menuleft=", menuleft);
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
	    var result = /\$\S+$/.exec(this.value.slice(0, curspos));
	    var lastdoll = result ? result[0] : null;
	    console.log("mom_cmdkeypress escape evt=", evt,  " lastdoll=", lastdoll, " curspos=", curspos);
	    if (lastdoll) {
		var dollname = lastdoll.substr(1);
		if (mom_escape_encoding_dict.hasOwnProperty(dollname)) {
		    var dollval = mom_escape_encoding_dict[dollname];
		    var oldval = $commandtext.val();
		    console.log("mom_cmdkeypress escape evt=", evt,
				" dollname=", dollname, " dollval=", dollval);
		    $commandtext.val(oldval.substr(0, curspos-lastdoll.length) + dollval
				     + oldval.substr(curspos));
		    oldval = null;
		    $commandtext[0].focus();
		    $commandtext[0].selectionStart = $commandtext[0].selectionEnd = curspos + dollval.length;
		    $commandtext[0].scrollTop = scrollpos;		    
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
      dataType: "html",
      success: mom_ajaxparsecommand
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
