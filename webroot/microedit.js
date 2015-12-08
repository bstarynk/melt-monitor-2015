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
};

function htmlDecode(value){
    return $('<div/>').html(value).text();
};

var momc_ast_count=0;
var momv_entriesvar=null;
var mom_name_regexp = /^[A-Za-z0-9_]*$/;

var mom_name_cache = new Object();

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
};                              // end mom_known_item

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
};                              // end mom_complete_name



// this utility function is assigning a unique number for ASTs
function mom_ast_numbered(obj) {
    momc_ast_count = momc_ast_count+1;
    obj.mom_ast_inum = momc_ast_count;
    return momc_ast_count;
};
/// this function is called by /microedit AJAX for do_fillpage at document loading
function mom_ajaxfillscript(script) {
    console.log("mom_ajaxfillscript:\n", script, "\n### endajaxfillscript\n");
    console.trace();
};

function mome_begin_fill(statitmid) {
    console.log("mome_begin_fill statitmid=", statitmid, " $editdiv=", $editdiv);
    $editdiv.empty();
    $("<p class='mombeginfill_cl'>" + statitmid + "</p>").appendTo($editdiv);
    console.log("mome_begin_fill $editdiv=", $editdiv, " this=", this);
    mom_name_cache = new Object();
};

function mome_mtime(timestr) {
    console.log("mome_mtime timestr=", timestr);
    $("<p class='mommtime_cl'>" + htmlEncode(timestr) + "</p>").appendTo($editdiv);
    console.log("mome_mtime $editdiv=", $editdiv, " this=", this);
};

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
        curent.realize(entdl);
        curent = null;
    };
    console.log("mome_entries entdl=", entdl, " $editdiv=", $editdiv, " entarr=", entarr);
};


function mome_generated(msg) {
    console.log("mome_generated start msg=", msg, "; $editdiv=", $editdiv, " this=", this);
    console.log("mome_generated ending $editlog=", $editlog, " msg=", msg);
    $editlog.append(msg);
};                              // end mome_generated


function mom_set_jdom_for_ast(jdom,ast) {
    console.log("mom_set_jdom_for_ast jdom=", jdom, " ast=", ast);
    console.assert(jdom instanceof jQuery, "mom_set_jdom_for_ast bad jdom=",
                   jdom);
    console.assert("mom_ast" in ast, "mom_set_jdom_for_ast bad ast=", ast);
    ast.mom_jdom = jdom;
    jdom.data("mom_ast", ast);
};

function mom_put_jdom_in(jdom,incont) {
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
};

////////////////
function MomeEntry(entitm,entval) {
    this.mom_entry_item= entitm;
    this.mom_entry_val= entval;
    this.mom_ast = "entry";
    mom_ast_numbered(this);
};
var momp_entry = {
    name: "MomeEntry",
    realize: function (incont) {
        console.log("MomeEntry-realize start incont=", incont, " this=", this);
        console.trace();
        console.assert('mom_entry_item' in this && 'mom_entry_val' in this,
                       "MomeEntry-realize bad this:", this);
        var eattelem = $("<dt class='statattr_cl'></dt>");
        var evalelem = $("<dd class='statval_cl'></dd>");
        var self = this;
        var thisitem = this.mom_entry_item;
        var thisval = this.mom_entry_val;
        console.log("MomeEntry-realize this=", this,
                    " eattelem=", eattelem, " evalelem=", evalelem, " thisitem=", thisitem);
        thisitem.realize(eattelem,self);
        console.log("MomeEntry-realize this=", this, " thisitem=", thisitem, " incont=", incont, " eattelem=", eattelem);
        mom_put_jdom_in(eattelem,incont);
        console.log("MomeEntry-realize this=", this, " thisval=", thisval, " evalelem=", evalelem);
        thisval.realize(evalelem,self);
        console.log("MomeEntry-realize this=", this, " evalelem=", evalelem);
        mom_put_jdom_in(evalelem,incont);
        console.log("MomeEntry-realize end this=", this, " incont=", incont, " eattelem=", eattelem, " evalelem=", evalelem);
    }
};
MomeEntry.prototype = momp_entry;
function mome_entry(entitm,entval) {
    console.log("mome_entry entitm=", entitm, " entval=", entval);
    var res = new MomeEntry(entitm, entval);
    console.log("mome_entry res=", res);
    return res;
};


var momp_value = {
    name: "MompValue",
    realize: function (incont,fromast) {
        console.error("MomeValue-realize bad incont=", incont, " fromast=", fromast);
    }
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
        var str = null;
        console.log("MomeNameRef-onitemchange this=", this, " ev=", ev, " data=", data);
	var oldinp = data.mom_jdom;
	console.log("MomeNameRef-onitemchange this=", this, " oldinp=", oldinp);
	var jthis = $(this);
        str = jthis.val();
        console.log("MomeNameRef-onitemchange this=", this, " jthis=", jthis, " str=", str);
        if (mom_known_item(str)) {
            var self = this;
            console.log("MomeNameRef-onitemchange this=", this, " known str=", str);
            self.__proto__ = momp_item_ref;
            console.log("MomeNameRef-onitemchange mutated self=", self,
			" $(self)=", $(self), " jthis=", jthis);
            self.realize(function (del) {
                console.log("MomeNameRef replace/realize onitemchange self=", self,
                            " $(self)=", $(self),
                            " data=", data,
                            " $(data)=", $(data),
                            " this=", this,
                            " jthis=", jthis,
			    " oldinp=", oldinp,
                            " del=", del);
                oldinp.replaceWith(del);
		data.mom_jdom = del;
                console.log("MomeNameRef done replace onitemchange self=", self, ' data=', data);
            }, null);
            console.log("MomeNameRef-onitemchange updated self=", self);
        }
        else {
            console.log("MomeNameRef-onitemchange this=", this, " unknown str=", str);
        }
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
    replace_by_item_input: function (str) {
        console.log("MomeNameRef-replace_by_item_input this=", this, " str=", str);
        var einput = $("<input class='mom_item_input_cl' type='text' pattern='^[A-Za-z0-9_]*$'/>");
        einput.val(str);
        einput.on("input", null, this, this.oniteminput);
        einput.on("change", null, this, this.onitemchange);
        einput.on("blur", null, this, this.onitemblur);
        einput.on("focus", null, this, this.onitemfocus);
        this.mom_jdom.replaceWith(einput);
	this.mom_jdom = einput;
        einput.autocomplete({
            minLength:2,
            source: this.itemautocomplete
        });
        console.log("MomeNameRef-replace_by_item_input this=", this, " einput=", einput);
        einput.focus();
        if (str.length>0)
            einput[0].setSelectionRange(str.length,str.length);
        console.log("MomeNameRef-replace_by_item_input done this=", this,
                    " einput=", einput, " activeElement=", document.activeElement);
    },
    gotkeyup: function (ev) {
        var self = this;
        var data = ev.data;
        console.log("MomeNameRef-gotkeyup self=", self, " ev=", ev, " which=", ev.which, " keyCode=", ev.keyCode,
                    " charCode=", ev.charCode, " key=", ev.key, " this=", this, " data=", data);
        if (ev.which === " ".charCodeAt(0) && !ev.ctrlKey && !ev.metaKey) { // space
            console.log("MomeNameRef-gotkeyup space ev=", ev, " this=", this, " $(this)=", $(this));
            console.assert(data.replace_by_item_input, "MomeNameRef-gotkeyup for replace_by_item_input bad data=", data);
            data.replace_by_item_input("");
        }
        else if (/[A-Za-z]/.test(String.fromCharCode(ev.which)) && !ev.ctrlKey && !ev.metaKey) { // letter
            var kstr = null;
            if ('key' in ev)
                kstr = ev.key;
            else
                kstr = String.fromCharCode(ev.which);
            console.log("MomeNameRef-gotkeyup letter ev=", ev, " this=", this, " kstr=`", kstr, "'");
            data.replace_by_item_input(kstr);
        }
        else if (ev.which == $.ui.keyCode.ESCAPE && !ev.ctrlKey && !ev.metaKey) { // escape
            console.log("MomeNameRef-gotkeyup escape ev=", ev, " this=", this);
        }
        else {
            console.log("MomeNameRef-gotkeyup ordinary ignored ev=", ev, " this=", this);
            return false;
        }
    }
};

var momp_item_ref = {
    name: "MomeItemRef",
    realize: function (incont,fromast) {
        var self = this;
        console.log("MomeItemRef-realize start incont=", incont, " of type:", typeof incont,
                    " fromast=", fromast, " this=", this);
        console.assert (incont instanceof jQuery || typeof incont === 'function',
                        "MomeItemRef-realize bad incont=", incont);
        console.assert (!fromast || 'mom_ast' in fromast, "MomeItemRef-realize bad fromast=",
                        fromast);
        var eitelem = $("<span class='momitem_bcl momitemref_cl' tabindex='0'>"+this.mom_item_name+"</span>");
	console.log("MomeItemRef-realize eitelem=", eitelem, " incont=", incont, " this=", this);
        mom_put_jdom_in(eitelem,incont);
        mom_set_jdom_for_ast(eitelem,this);
        if (fromast && !('mom_from_ast' in this))
            this.mom_from_ast = fromast;
        /// dont handle keypress, but keyup, see http://stackoverflow.com/a/28502629/841108
        /// maybe we want to use http://dmauro.github.io/Keypress/
        /// see also http://unixpapa.com/js/key.html & http://www.openjs.com/scripts/events/keyboard_shortcuts/
        /// and avoid functionkeys e.g. F1, see http://unixpapa.com/js/key.html
        eitelem.on("keyup", null, this, this.gotkeyup);
        console.log("MomeItemRef-realize end this=", this, " eitelem=", eitelem);
    },
    __proto__: momp_name_ref
};
function MomeItemRef(iname) {
    this.mom_item_name = iname;
    this.mom_ast = "item_ref";
    mom_ast_numbered(this);
};
MomeItemRef.prototype = momp_item_ref;
function mome_item_ref(itmname) {
    var res = new MomeItemRef(itmname);
    console.log ("mome_item_ref res=", res);
    return res;
};


////////////////
var momp_nil_value = {
    name: "MomeNilValue",
    realize: function (incont,fromast) {
        console.error("MomeNilValue-realize unimplemented incont=", incont,
                      " fromast=", fromast, " this=", this);
    }
};
function MomeNilValue() {
    mom_ast_numbered(this);
    this.mom_ast = "nilval";
};
MomeNilValue.prototype = momp_nil_value;

function mome_nil_val() { /// a nil value
    var res = new MomeNilValue();
    console.log ("mome_nil_val res=", res);
    return res;
};

////////////////
var momp_nil_ref = {
    name: "MomeNilRef",
    realize: function (incont,fromast) {
        console.log ("MomeNilRef-realize this=", this);
        console.error("MomeNilRef-realize unimplemented incont=", incont, " fromast=", fromast,
                      " this=", this);
    }
};
function MomeNilRef() {
    mom_ast_numbered(this);
    this.mom_ast = "nilref";
};
MomeNilRef.prototype = momp_nil_ref;

function mome_nil_ref() {       /// a nil item reference
    var res = new MomeNilRef();
    console.log ("mome_nil_ref res=", res);
    return res;
};
////////////////

var momp_item_value = {
    name: "MomeItemValue",
    realize: function (incont,fromast) {
        console.log ("MomeItemValue-realize this=", this, " incont=", incont, " fromast=", fromast);
        console.assert ('mom_item_name' in this, "MomeItemValue-realize bad this=", this);
        var self = this;
        var eitmelem = $("<span class='mom_item_bcl momitemval_cl' tabindex='0'>"+ this.mom_item_name +"</span>");
        console.log ("MomeItemValue-realize this=", this, " eitmelem=", eitmelem);
        mom_put_jdom_in(eitmelem,incont);
        mom_set_jdom_for_ast(eitmelem,this);
        console.log("MomeItemValue-realize done incont=", incont,
                    " fromast=", fromast, " this=", this, " eitmelem=", eitmelem);
    }
};
function MomeItemValue(iname) {
    console.assert (typeof(iname)==='string',
                    "MomeItemValue bad iname=", iname);
    mom_ast_numbered(this);
    this.mom_item_name = iname;
    this.mom_ast = "itemval";
};
MomeItemValue.prototype = momp_item_value;

function mome_item_val(itemname) { /// a nil value
    var res = new MomeItemValue(itemname);
    console.log ("mome_item_val res=", res);
    return res;
};


////////////////
var momp_number_value = {
    name: "momp-number-value",
    realize:function (incont,fromast) {
        console.error("momp-number-value-realize unimplemented incont=", incont,
                      " fromast=", fromast, " this=", this);
    }
}
////////////////
var momp_int_value = {
    name: "MomeIntValue",
    __proto__: momp_number_value
};
function MomeIntValue(ival) {
    console.assert (typeof ival === 'number',
                    "MomeIntValue check ival is number");
    mom_ast_numbered(this);
    this.mom_num_val = ival;
};
MomeIntValue.prototype = momp_int_value;

function mome_int (num) {
    var res = new MomeIntValue(num);
    console.log ("mome_int res=", res);
    return res;
};

////////////////
var momp_double_value = {
    name: "MomeDoubleValue",
    __proto__: momp_number_value
};
function MomeDoubleValue(ival) {
    console.assert (typeof ival === 'number',
                    "MomeDoubleValue check ival is number");
    mom_ast_numbered(this);
    this.mom_num_val = ival;
};
MomeDoubleValue.prototype = momp_double_value;

function mome_double (num) {
    var res = new MomeDoubleValue(num);
    console.log ("mome_double res=", res);
    return res;
};

////////////////
var momp_string_value = {
    name: 'MomeStringValue',
    __proto__: momp_value,
    realize: function (incont,fromast) {
        console.error("MomeStringValue-realize unimplemented incont=", incont,
                      " fromast=", fromast, " this=", this);
    }
};
function MomeStringValue(sval) {
    console.assert (typeof sval === 'string',
                    "MomeStringValue check sval is number");
    mom_ast_numbered(this);
    this.string_val = sval;
};
MomeStringValue.prototype = momp_string_value;

function mome_string (str) {
    var res = new MomeStringValue(str);
    console.log ("mome_string res=", str);
    return res;
};


////////////////
var momp_sequence_value = {
    name: 'momp-sequence-value',
    __proto__: momp_value,
    realize: function (incont,fromast) {
        console.error("momp-sequence-value-realize unimplemented incont=", incont,
                      " fromast=", fromast, " this=", this);
    }
};

////////////////
var momp_tuple_value = {
    name: 'MomeTupleValue',
    __proto__: momp_sequence_value
};
function MomeTupleValue(tval) {
    console.assert (Array.isArray(tval),
                    "MomeTupleValue check tval=", tval);
    mom_ast_numbered(this);
    this.mom_seq = tval;
};
MomeTupleValue.prototype = momp_tuple_value;

function mome_tuple (arr) {
    var res = new MomeTupleValue(arr);
    console.log ("mome_tuple res=", res);
    return res;
};

////////////////
var momp_set_value = {
    name: 'MomeTupleValue',
    __proto__: momp_sequence_value
};
function MomeSetValue(tval) {
    console.assert (Array.isArray(tval),
                    "MomeSetValue check tval=", tval);
    mom_ast_numbered(this);
    this.mom_seq = tval;
};
MomeSetValue.prototype = momp_tuple_value;

function mome_set (arr) {
    var res = new MomeSetValue(arr);
    console.log ("mome_set res=", res);
    return res;
};

////////////////
var momp_node_value = {
    __proto__: momp_value,
    name: "MomeNodeValue",
    realize: function (incont,fromast) {
        console.error("MomeNodeValue-realize unimplemented incont=", incont,
                      " fromast=", fromast, " this=", this);
    }
};
function MomeNodeValue(conn,sons) {
    mom_ast_numbered(this);
    console.assert(typeof conn==='object', "MomeNodeValue bad conn=", conn);
    console.assert(Array.isArray(sons), "MomeNodeValue bad sons=", sons);
    this.mom_conn = conn;
    this.mom_sons = sons;
}
function mome_node(connitm, sonsarr) {
    var res = new MomeNodeValue(connitm,sonsarr);
    console.log ("mome_node res=", res);
    return res;
};
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
