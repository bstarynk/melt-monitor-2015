// file microedit.js - Javascript & Jquery for microedit.html.

/**   Copyright (C)  2015 Basile Starynkevitch, later FSF
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

var momc_ast_count=0;
var momv_entriesvar=null;
var mom_name_regexp = /^[A-Za-z0-9_]*$/;

var mom_name_cache = {};

/// see http://mikemurko.com/general/jquery-keycode-cheatsheet/


/// check if a name is some known item, with memoization & AJAX do_knownitem
function mom_known_item(name) {
    if (!name.match(mom_name_regexp))
        return false;
    if (mom_name_cache[name])
        return true;
    var res = null;
    console.log("mom_known_item name=", name, " before ajaxing");
    $.ajax
    ({url: "/microedit",
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
    ({url: "/microedit",
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



// this utility function is assigning a unique number for ASTs
function mom_ast_numbered(obj) {
    momc_ast_count = momc_ast_count+1;
    obj.mom_ast_inum = momc_ast_count;
    return momc_ast_count;
}

/// this function is called by /microedit AJAX for do_fillpage at document loading
function mom_ajaxfillscript(script) {
    console.log("mom_ajaxfillscript:\n", script, "\n### endajaxfillscript\n");
    console.trace();
}

function mome_begin_fill(statitmid) {
    console.log("mome_begin_fill statitmid=", statitmid, " $editdiv=", $editdiv);
    $editdiv.empty();
    $("<p class='mombeginfill_cl'>" + statitmid + "</p>").appendTo($editdiv);
    console.log("mome_begin_fill $editdiv=", $editdiv, " this=", this);
    mom_name_cache = new Object();
}

function mome_mtime(timestr) {
    console.log("mome_mtime timestr=", timestr);
    $("<p class='mommtime_cl'>" + htmlEncode(timestr) + "</p>").appendTo($editdiv);
    console.log("mome_mtime $editdiv=", $editdiv, " this=", this);
}

function mome_entries(entarr) {
    console.log("mome_entries entarr=", entarr, " $editdiv=", $editdiv,
                " this=", this);
    momv_entriesvar = entarr;
    console.assert(Array.isArray(entarr),
                   "mome_entries bad entarr=", entarr);
    var nbent = entarr.length;
    var entdl = $("<dl class='momentrieslist_cl'></dl>");
    var curent = null;
    entdl.appendTo($editdiv);
    entarr.mom_ast = "entries";
    console.log("mome_entries before realize entarr=", entarr, " entdl=", entdl);
    for (var ix=0; ix<nbent; ix++) {
        curent = entarr[ix];
        console.log("mome_entries curent=", curent, " ix#", ix);
        curent.realize_entry(entdl);
        console.log("mome_entries realized curent=", curent);
        curent = null;
    }
    console.log("mome_entries entdl=", entdl, " $editdiv=", $editdiv, " entarr=", entarr);
}


function mome_generated(msg) {
    console.log("mome_generated start msg=", msg, "; $editdiv=", $editdiv, " this=", this);
    console.log("mome_generated ending $editlog=", $editlog, " msg=", msg);
    $editlog.append(msg);
}                              // end mome_generated


function mom_put_jdom_in(jdom,incont) {
    console.log("mom_put_jdom_in jdom=", jdom, " incont=", incont);
    console.trace();
    console.assert (jdom instanceof jQuery, "mom_put_jdom_in bad jdom=", jdom);
    if (incont instanceof jQuery) {
        console.log("mom_put_jdom_in jdom=", jdom, " appendTo incont=", incont);
        jdom.appendTo(incont);
    }
    else if (typeof incont=='function') {
        console.log("mom_put_jdom_in jdom=", jdom, " calling incont=", incont);
        incont(jdom);
    }
    else
        console.error("mom_put_jdom_in bad incont=", incont, " jdom=", jdom);
    console.log("mom_put_jdom_in done jdom=", jdom, " incont=", incont);
}

////////////////
function MomeEntry(entitm,entval) {
    this.mom_entry_item= entitm;
    this.mom_entry_val= entval;
    this.mom_ast = "entry";
    mom_ast_numbered(this);
}

var momp_entry = {
    name: "MomeEntry",
    realize_entry: function (incont) {
        console.log("MomeEntry-realize_entry start incont=", incont, " this=", this);
        console.trace();
        console.assert('mom_entry_item' in this && 'mom_entry_val' in this,
                       "MomeEntry-realize_entry bad this:", this);
        var eattelem = $("<dt class='statattr_cl'></dt>");
        var evalelem = $("<dd class='statval_cl'></dd>");
        var self = this;
        var thisitem = this.mom_entry_item;
        var thisval = this.mom_entry_val;
        var elemitem;
        var elemval;
        console.log("MomeEntry-realize_entry this=", this,
                    " eattelem=", eattelem, " evalelem=", evalelem, " thisitem=", thisitem);
        elemitem = thisitem.make_jdom();
        console.log("MomeEntry-realize_entry this=", this, " elemitem=", elemitem);
        mom_put_jdom_in(elemitem, eattelem);
        console.log("MomeEntry-realize_entry this=", this, " thisitem=", thisitem, " elemitem=", elemitem, " eattelem=", eattelem);
        mom_put_jdom_in(eattelem,incont);
        console.log("MomeEntry-realize_entry this=", this, " thisval=", thisval, " evalelem=", evalelem);
        elemval = thisval.make_jdom();
        console.log("MomeEntry-realize_entry this=", this, " elemval=", elemval, " evalelem=", evalelem);
        mom_put_jdom_in(elemval,evalelem);
        mom_put_jdom_in(evalelem,incont);
        console.log("MomeEntry-realize_entry end this=", this, " incont=", incont,
                    " eattelem=", eattelem, " evalelem=", evalelem);
    }
};
MomeEntry.prototype = momp_entry;
function mome_entry(entitm,entval) {
    console.log("mome_entry entitm=", entitm, " entval=", entval);
    var res = new MomeEntry(entitm, entval);
    console.log("mome_entry res=", res);
    return res;
}


var momp_value = {
    name: "MompValue",
    dispatchkeypress: function (ev) {
        console.log ("MompValue-dispatchkeypress ev=", ev, " this=", this);
    },
    gotfocus: function (ev) {
        console.log ("MompValue-gotfocus ev=", ev, " this=", this);
    },
    gotblur: function (ev) {
        console.log ("MompValue-gotblur ev=", ev, " this=", this);
    },
    gotkeypress: function (ev) {
	var ast = ev.data;
        console.log ("MompValue-gotkeypress ev=", ev, " this=", this, " ast=", ast);
    },
    install_jdom: function (jel) {
        console.log ("MompValue-install_jdom jel=", jel, " this=", this);
        jel.on("blur", null, this, this.gotblur);
        jel.on("focus", null, this, this.gotfocus);
        jel.on("keypress", null, this, this.gotkeypress);
        jel.data("mom_this", this);
    },
    make_jdom: function () { console.error("MompValue bad make_jdom arguments=", arguments); }  
};
////////////////////////////////////////////////////////////////

var momp_name_ref = {
    name: "MomeNameRef",
    oniteminput: function (ev) {
        var data = ev.data;
        console.log("MomeNameRef-oniteminput this=", this, " ev=", ev, " data=", data);
    },
    onitemchange: function (ev) {
        var data = ev.data;
        var str = $(this).val();
        console.log("MomeNameRef-onitemchange this=", this, " ev=", ev, " data=", data, " str=", str);
        if (mom_known_item(str)) {
            var orig = data.mom_orig;
            var self = this;
            data.mom_name = str;
            console.log("MomeNameRef-onitemchange this=", this, " ev=", ev, " data=", data, " knownitem str=", str);
            console.assert('mom_item_name' in orig, "MomeNameRef-onitemchange bad orig=", orig);
            orig.mom_item_name = str;
            var newel = orig.make_jdom();
            console.log("MomeNameRef-onitemchange this=", this, " ev=", ev, " orig=", orig, " newel=", newel);
            $(this).replaceWith(newel);
            console.log("MomeNameRef-onitemchange this=", this, " done knownitem str=", str, " ev=", ev);
        }
        else {
            console.log("MomeNameRef-onitemchange this=", this, " ev=", ev, " unknownitem str=", str);
        }
        console.log("MomeNameRef-onitemchange done this=", this, " ev=", ev);
    },
    onitemblur: function (ev) {
        var data = ev.data;
        console.log("MomeNameRef-onitemblur this=", this, " ev=", ev, " data=", data);
    },
    onitemfocus: function (ev) {
        var data = ev.data;
        console.log("MomeNameRef-onitemfocus this=", this, " ev=", ev, " data=", data);
    },
    itemautocomplete: function(requ, respfun) {
        var compl = null;
        console.log ("MomeNameRef-itemautocomplete start requ=", requ, " respfun=", respfun);
        compl = mom_complete_name(requ.term);
        console.log ("MomeNameRef-itemautocomplete compl=", compl, " for term=", requ.term);
        if (compl) respfun(compl);
        else respfun("");
    },
    make_jdom: function () {
        console.log("MomeNameRef-make_jdom this=", this);
        var einput = $("<input class='mom_item_input_cl' type='text' pattern='^[A-Za-z0-9_]*$'/>");
        einput.on("input", null, this, this.oniteminput);
        einput.on("change", null, this, this.onitemchange);
        einput.on("blur", null, this, this.onitemblur);
        einput.on("focus", null, this, this.onitemfocus);
        einput.data("mom_this", this);
        einput.autocomplete({
            minLength:2,
            autoFocus: true,
            source: this.itemautocomplete,
            close: this.itemclose,
        });
        console.log("MomeNameRef-make_jdom this=", this, " gives einput=", einput);
        return einput;
    },
    jdom_set_str: function (einp, str) {
        console.log("MomeNameRef-jdom_set_str einp=", einp, " str=", str);
        einp.val(str);
        einp.focus();
        if (str.length>0)
            einp[0].setSelectionRange(str.length,str.length);
    },
    itemclose: function (ev) {
        console.log("MomeNameRef-itemclose this=", this, " ev=", ev);
        var origev = ev.originalEvent;
        var data, orig;
        if (origev.keyCode === $.ui.keyCode.ESCAPE) {
            /// see https://forum.jquery.com/topic/jquery-s-autocomplete-customize-behavior-of-%E2%80%9Cescape%E2%80%9D-key-press
            console.log("MomeNameRef-itemclose this=", this, " escaped ev=", ev, " origev=", origev);
            data = $(this).data("mom_this");
            console.log("MomeNameRef-itemclose this=", this, " escaped ev=", ev, " data=", data);
            orig = data.mom_orig;
            var elorig = erig.make_jdom();
            console.log("MomeNameRef-itemclose this=", this, " escaped ev=", ev, " orig=", orig, " elorig=", elorig);
            $(this).replaceWith(elorig);
            return false;           
        }
    }
};

function MomeNameRef(name, orig) {
    this.mom_name= name;
    if (orig) {
        this.mom_orig= orig;
        this.mom_ast_inum = orig.mom_ast_inum;
    }
    else
        mom_ast_numbered(this);
    this.mom_ast = "nameref";
}
MomeNameRef.prototype = momp_name_ref;

////////////////

var momp_item_ref = {
    name: "MomeItemRef",
    __proto__: momp_name_ref,
    gotfocus: function (ev) {
        console.log("MomeItemRef-gotfocus ev=", ev);
    },
    gotblur: function (ev) {
        console.log("MomeItemRef-gotblur ev=", ev);
    },
    gotuparrow: function(ev) {
	var self=this;
        console.log("MomeItemRef-gotuparrow ev=", ev, " this=", this);
	var par = $(ev.target).parents();
	console.log("MomeItemRef-gotuparrow par=", par, " this=", this);
	par.each(function (ix, el) {
	    var tdata = $(el).data("mom_this");
	    console.log("MomeItemRef-gotuparrow par=", par, " each  ix=", ix,
			" el=", el, " tdata=", tdata, " self=", self);
	    if (tdata) {
		console.log("MomeItemRef-gotuparrow good el=", el, " tdata=", tdata);
		$(el).focus();
		return false;
	    }
	});
    },
    gotdownarrow: function (ev) {
        console.log("MomeItemRef-gotdownarrow ev=", ev, " this=", this);
    },
    gotleftarrow: function (ev) {
        console.log("MomeItemRef-gotleftarrow ev=", ev, " this=", this);
    },
    gotrightarrow: function (ev) {
        console.log("MomeItemRef-gotrightarrow ev=", ev, " this=", this);
    },
    gotkeypress: function (ev) {
        var ast = ev.data;
        console.log("MomeItemRef-gotkeypress ev=", ev, " which=", ev.which,
		    " keyCode=", ev.keyCode, " this=", this, " ast=", ast);
        var njdom;
        console.assert('mom_ast' in ast, "MomeItemRef-gotkeypress bad ast=", ast);
        if (ev.which === " ".charCodeAt(0) && !ev.ctrlKey && !ev.metaKey) { // space
            console.log("MomeItemRef-gotkeypress space ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
            var nref = new MomeNameRef(this.mom_item_name, ast);
            console.log("MomeItemRef-gotkeypress space ev=", ev, " nref=", nref);
            var namelem = nref.make_jdom();
            console.log("MomeItemRef-gotkeypress space ev=", ev, " namelem=", namelem, " nref=", nref);
            $(this).replaceWith(namelem);
            namelem.focus();
            console.log("MomeItemRef-gotkeypress space ev=", ev, " done namelem=", namelem);
        }
        else if (ev.keyCode === 38 /*UP ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeItemRef-gotkeypress uparrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
	    ast.gotuparrow(ev);
            console.log("MomeItemRef-gotkeypress uparrow done ev=", ev, " this=", this);
        }
        else if (ev.keyCode === 40 /*DOWN ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeItemRef-gotkeypress downarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
	    this.gotdownarrow(ev);
        }
        else if (ev.keyCode === 37 /*LEFT ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeItemRef-gotkeypress leftarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
	    this.gotleftarrow(ev);
        }
        else if (ev.keyCode === 39 /*RIGHT ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeItemRef-gotkeypress rightarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
	    this.gotrightarrow(ev);
        }
    },
    install_jdom: function (jel) {
        console.log ("MomeItemRef-install_jdom jel=", jel, " this=", this);
        jel.on("blur", null, this, this.gotblur);
        jel.on("focus", null, this, this.gotfocus);
        jel.on("keypress", null, this, this.gotkeypress);
        jel.data("mom_this", this);
    },
    make_jdom: function () {
        var self= this;
        console.log("MomeItemRef-make_jdom start self=", self);
        console.assert ('mom_item_name' in self, "MomeItemRef-make_jdom bad self=", self);
        var eitelem = $("<span class='momitem_bcl momitemref_cl' tabindex='0'>"+this.mom_item_name+"</span>");
        console.log("MomeItemRef-make_jdom installing eitelem=", eitelem);
        this.install_jdom(eitelem);
        console.log("MomeItemRef-make_jdom eitelem=", eitelem);
        return eitelem;
    }
};
function MomeItemRef(iname) {
    this.mom_item_name = iname;
    this.mom_ast = "item_ref";
    mom_ast_numbered(this);
}
MomeItemRef.prototype = momp_item_ref;
function mome_item_ref(itmname) {
    var res = new MomeItemRef(itmname);
    console.log ("mome_item_ref res=", res);
    return res;
}


////////////////
var momp_nil_value = {
    name: "MomeNilValue",
    __proto__: momp_value,
    make_jdom: function () {
        var enilelem = $("<span class='momval_bcl momemptyval_cl' tabindex='0'>~</span>");
        console.log("MomeNilValue-make_jdom this=", this, " enilelem=", enilelem);
        this.install_jdom(enilelem);
        return enilelem;
    }
};
function MomeNilValue() {
    mom_ast_numbered(this);
    this.mom_ast = "nilval";
}
MomeNilValue.prototype = momp_nil_value;

function mome_nil_val() { /// a nil value
    var res = new MomeNilValue();
    console.log ("mome_nil_val res=", res);
    return res;
}

////////////////
var momp_nil_ref = {
    name: "MomeNilRef",
    __proto__: momp_item_ref,
    make_jdom: function() {
        var enilelem = $("<span class=' mom_item_bcl momrefnil_cl' tabindex='0'>~</span>");
        enilelem.data("mom_this", this);
        console.log("MomeNilRef-make_jdom this=", this, " enilelem=", enilelem);
        this.install_jdom(enilelem);
        return enilelem;
    }
};
function MomeNilRef() {
    mom_ast_numbered(this);
    this.mom_ast = "nilref";
}
MomeNilRef.prototype = momp_nil_ref;

function mome_nil_ref() {       /// a nil item reference
    var res = new MomeNilRef();
    console.log ("mome_nil_ref res=", res);
    return res;
}
////////////////

var momp_item_value = {
    name: "MomeItemValue",
    __proto__: momp_value,
    make_jdom: function() {
        console.log ("MomeItemValue-make_jdom this=", this);
        console.assert('mom_item_name' in this, "MomeItemValue-make_jdom bad this=", this);
        var self = this;
        var eitmelem = $("<span class='mom_item_bcl momitemval_cl' tabindex='0'>"+ this.mom_item_name +"</span>");
        console.log ("MomeItemValue-make_jdom this=", this, " eitmelem=", eitmelem);
        this.install_jdom(eitmelem);
        return eitmelem;
    }
};
function MomeItemValue(iname) {
    console.assert (typeof(iname)==='string',
                    "MomeItemValue bad iname=", iname);
    mom_ast_numbered(this);
    this.mom_item_name = iname;
    this.mom_ast = "itemval";
}
MomeItemValue.prototype = momp_item_value;

function mome_item_val(itemname) { /// a nil value
    var res = new MomeItemValue(itemname);
    console.log ("mome_item_val res=", res);
    return res;
}


////////////////
var momp_number_value = {
    name: "momp-number-value",
    __proto__: momp_value,
    make_jdom: function () {
        console.error("momp-number-value-make_jdom this=", this);
    }
};
////////////////
var momp_int_value = {
    name: "MomeIntValue",
    __proto__: momp_number_value,
    make_jdom: function () {
        var eintelem = $("<span class='mom_value_bcl momnumber_cl' tabindex='0'>"+ this.mom_int_val.toString() +"</span>");
        this.install_jdom(eintelem);
        console.log("MomeIntValue-make_jdom this=", this, " eintelem=", eintelem);
        return eintelem;
    }
};
function MomeIntValue(ival) {
    console.assert (typeof ival === 'number',
                    "MomeIntValue check ival is number");
    mom_ast_numbered(this);
    this.mom_int_val = ival;
    this.mom_ast = "int";
}
MomeIntValue.prototype = momp_int_value;

function mome_int (num) {
    var res = new MomeIntValue(num);
    console.log ("mome_int res=", res);
    return res;
}

////////////////
var momp_double_value = {
    name: "MomeDoubleValue",
    __proto__: momp_number_value,
    make_jdom: function () {
        var edblelem = $("<span class='mom_value_bcl momnumber_cl' tabindex='0'>"+ this.mom_dbl_val.toString() +"</span>");
        this.install_jdom(edblelem);
        console.log("MomeDoubleValue-make_jdom this=", this, " edblelem=", edblelem);
        return edblelem;
    }
};
function MomeDoubleValue(dval) {
    console.assert (typeof dval === 'number',
                    "MomeDoubleValue check dval is number");
    mom_ast_numbered(this);
    this.mom_dbl_val = dval;
}
MomeDoubleValue.prototype = momp_double_value;

function mome_double (num) {
    var res = new MomeDoubleValue(num);
    console.log ("mome_double res=", res);
    return res;
}

////////////////
var momp_string_value = {
    name: 'MomeStringValue',
    __proto__: momp_value,
    make_jdom: function () {
        console.assert ('mom_str' in this, "MomeStringValue-make_jdom bad this=", this);
        var estrelem = $("<q class='momstrquote_cl' tabindex='0'><span class='mom_value_bcl momstring_cl' tabindex='0'>"+ htmlEncode(this.mom_str) +"</span></q>");
        this.install_jdom(estrelem);
        console.log("MomeStringValue-make_jdom this=", this, " estrelem=", estrelem);
        return estrelem;
    }
};
function MomeStringValue(sval) {
    console.assert (typeof sval === 'string',
                    "MomeStringValue check sval is number");
    mom_ast_numbered(this);
    this.mom_str = sval;
}
MomeStringValue.prototype = momp_string_value;

function mome_string (str) {
    var res = new MomeStringValue(str);
    console.log ("mome_string res=", str);
    return res;
}




/// what about inserting before, after, first/last, removing?
////////////////
var momp_tuple_value = {
    name: 'MomeTupleValue',
    __proto__: momp_value,
    make_jdom: function () {
        var self = this;
        console.assert(Array.isArray (self.mom_comps),
                       "MomeTupleValue no mom_comps in self=", self);
        var etupelem = $("<span class='mom_value_bcl momtuple_cl' tabindex='0'></span>");
        var et = null;
        et = document.createTextNode("[");
        $(et).appendTo(etupelem);
        var nbcomp = self.mom_comps.length;
        for (var compix=0; compix<nbcomp; compix++) {
            var curcomp = self.mom_comps[compix];
            console.log("MomeTupleValue-make_jdom compix=", compix,
                        " curcomp=", curcomp,
                        " self=", self, " etupelem=", etupelem);
            if (compix > 0) {
                et = document.createTextNode(" ");
                $(et).appendTo(etupelem);
                et = null;
            }
            var curjdom = curcomp.make_jdom();
            curjdom.appendTo(etupelem);
            et = null;
        }
        et = document.createTextNode("]");
        $(et).appendTo(etupelem);
        this.install_jdom(etupelem);
        console.log("MomeTupleValue-make_jdom self=", self, " etupelem=", etupelem);
        return etupelem;
    }
};
function MomeTupleValue(tval) {
    console.assert (Array.isArray(tval),
                    "MomeTupleValue check tval=", tval);
    mom_ast_numbered(this);
    this.mom_comps = tval;
    this.mom_ast = "tuple";
}
MomeTupleValue.prototype = momp_tuple_value;

function mome_tuple (arr) {
    var res = new MomeTupleValue(arr);
    console.log ("mome_tuple res=", res);
    return res;
}

////////////////
var momp_set_value = {
    name: 'MomeSetValue',
    __proto__: momp_value,
    gotblur: function (ev) {
        console.log("MomeSetValue-gotblur ev=", ev, " this=", this);
    },
    gotfocus: function (ev) {
        console.log("MomeSetValue-gotfocus ev=", ev, " this=", this);
    },
    gotkeypress: function (ev) {
        console.log("MomeSetValue-gotkeypress ev=", ev, " which=", ev.which);
        var ast = ev.data;
        console.assert('mom_ast' in ast, "MomeSetValue-gotkeypress bad ast=", ast);
        if (ev.which === " ".charCodeAt(0) && !ev.ctrlKey && !ev.metaKey) { // space
            console.log("MomeSetValue-gotkeypress space ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
        else if (ev.keyCode === 38 /*UP ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeSetValue-gotkeypress uparrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
        else if (ev.keyCode === 40 /*DOWN ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeSetValue-gotkeypress downarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
        else if (ev.keyCode === 37 /*LEFT ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeSetValue-gotkeypress leftarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
        else if (ev.keyCode === 39 /*RIGHT ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeSetValue-gotkeypress rightarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
    },
    make_jdom: function () {
        var self = this;
        console.assert(Array.isArray (self.mom_elems),
                       "MomeSetValue no mom_elems in self=", self);
        var esetelem = $("<span class='mom_value_bcl momset_cl' tabindex='0'></span>");
        var et = null;
        et = document.createTextNode("{");
        $(et).appendTo(esetelem);
        var nbcomp = self.mom_elems.length;
        for (var compix=0; compix<nbcomp; compix++) {
            var curcomp = self.mom_elems[compix];
            console.log("MomeSetValue-make_jdom compix=", compix,
                        " curcomp=", curcomp,
                        " self=", self, " esetelem=", esetelem);
            if (compix > 0) {
                et = document.createTextNode(" ");
                $(et).appendTo(esetelem);
                et = null;
            }
            var curjdom = curcomp.make_jdom();
            curjdom.appendTo(esetelem);
            et = null;
        }
        et = document.createTextNode("}");
        $(et).appendTo(esetelem);
        this.install_jdom(esetelem);
        console.log("MomeSetValue-make_jdom self=", self, " esetelem=", esetelem);
        return esetelem;
    }
};
function MomeSetValue(tval) {
    console.assert (Array.isArray(tval),
                    "MomeSetValue check tval=", tval);
    mom_ast_numbered(this);
    this.mom_elems = tval;
    this.mom_ast = "set";
}
MomeSetValue.prototype = momp_set_value;

function mome_set (arr) {
    var res = new MomeSetValue(arr);
    console.log ("mome_set res=", res);
    return res;
}

////////////////
var momp_node_value = {
    __proto__: momp_value,
    name: "MomeNodeValue",
    gotblur: function (ev) {
        console.log("MomeNodeValue-gotblur ev=", ev, " this=", this);
    },
    gotfocus: function (ev) {
        console.log("MomeNodeValue-gotfocus ev=", ev, " this=", this);
    },
    gotkeypress: function (ev) {
        console.log("MomeNodeValue-gotkeypress ev=", ev, " which=", ev.which);
        var ast = ev.data;
        console.assert('mom_ast' in ast, "MomeNodeValue-gotkeypress bad ast=", ast);
        if (ev.which === " ".charCodeAt(0) && !ev.ctrlKey && !ev.metaKey) { // space
            console.log("MomeNodeValue-gotkeypress space ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
        else if (ev.keyCode === 38 /*UP ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeNodeValue-gotkeypress uparrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
        else if (ev.keyCode === 40 /*DOWN ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeNodeValue-gotkeypress downarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
        else if (ev.keyCode === 37 /*LEFT ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeNodeValue-gotkeypress leftarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
        else if (ev.keyCode === 39 /*RIGHT ARROW key*/ && !ev.ctrlKey && !ev.metaKey) {
            console.log("MomeNodeValue-gotkeypress rightarrow ev=", ev, " this=", this, " $(this)=", $(this), " ast=", ast);
        }
    },
    make_jdom: function () {
        var self = this;
        console.assert(typeof (this.mom_conn) === "object" &&
                       Array.isArray(this.mom_sons),
                       "MomeNodeValue no mom_conn&mom_sons in this=", this);
        var enodelem = $("<span class='mom_value_bcl momnode_cl' tabindex='0'></span>");
        this.install_jdom(enodelem);
        var et = null;
        var econnelem = $("<span class='momconn_cl' tabindex='0'>*</span>");
        econnelem.appendTo(enodelem);
        var connielem = this.mom_conn.make_jdom();
        connielem.appendTo(econnelem);
        et = document.createTextNode("(");
        $(et).appendTo(enodelem);
        var nbsons = this.mom_sons.length;
        for (var sonix=0; sonix<nbsons; sonix++) {
            var curson = this.mom_sons[sonix];
            if (sonix>0) {
                et = document.createTextNode(" ");
                $(et).appendTo(enodelem);
                et = null;
            }
            var sonjdom = curson.make_jdom();
            sonjdom.appendTo(enodelem);
        }
        et = document.createTextNode(")");
        $(et).appendTo(enodelem);
        et = null;
        console.log("MomeNodeValue-make_jdom this=", this, " enodelem=", enodelem);
        return enodelem;
    }
};
function MomeNodeValue(conn,sons) {
    mom_ast_numbered(this);
    console.assert(typeof conn==='object', "MomeNodeValue bad conn=", conn);
    console.assert(Array.isArray(sons), "MomeNodeValue bad sons=", sons);
    this.mom_conn = conn;
    this.mom_sons = sons;
    this.mom_ast = "node";
}
MomeNodeValue.prototype= momp_node_value;
function mome_node(connitm, sonsarr) {
    var res = new MomeNodeValue(connitm,sonsarr);
    console.log ("mome_node res=", res);
    return res;
}
////////////////////////////////////////////////////////////////



$(document).ready(function(){
    console.log("microedit document ready");
    // see http://stackoverflow.com/a/10556743/841108
    window.onerror = function(msg,url,line,col,error) {
        console.error("window error ", url, ":", line, ": ", msg, " /", error);
    };
    $editdiv = $("#microedit_id");
    $editlog = $("#editlog_id");
    $cleareditbut = $("#cleareditbut_id");
    console.log ("microedit readying $editdiv=", $editdiv, " $editlog=", $editlog, " $cleareditbut=", $cleareditbut);
    $cleareditbut.click(function(evt){
        console.log("clearedit evt=", evt);
        $editlog.html("");
    });
    console.log("microedit before ajax do_fillpage");
    $.ajax
    ({url: "/microedit",
      method: "POST",
      data: {"do_fillpage": true},
      dataType: "script",
      success: mom_ajaxfillscript
     });
    console.log("microedit document done ready");
});
console.warn("incomplete new microedit.js, see also _old_microedit.js");
